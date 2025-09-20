// pairrangesummary - counts goldbach pairs in a window
// Copyright (C) 2025 Bill C. Riemers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <inttypes.h>
#include <getopt.h>
#include <getopt.h>
// at top (near other includes)
#include <ctype.h>
#include "libprime.h"

typedef enum { AGG_DECADE=0, AGG_PRIMORIAL=1 } agg_mode_t;
typedef enum { MODEL_EMPIRICAL=0, MODEL_HLA=1 } model_mode_t;
typedef enum { VER_0_1_5=0, VER_CURRENT=1 } compat_ver_t;


static void print_usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [OPTIONS] <prime_raw_file> <end_n>\n"
        "\n"
        "Counts Goldbach pairs in a sliding window of half-width delta = floor(alpha*n).\n"
        "By default applies the Euler safety cap M(n)=ceil(((2n+1)-sqrt(8n+1))/2)-1.\n"
        "\n"
        "Positional:\n"
        "  prime_raw_file       Path to raw uint64_t prime array file\n"
        "  end_n                Upper bound for n (exclusive)\n"
        "\n"
        "Options:\n"
        "  --alpha=VAL          Window half-width multiplier (long double) in [0,1]. Default: 0.5\n"
        "  --agg=MODE           Aggregation: decade (default) or primorial\n"
        "  --compat=VERSION     force results compatible with an version v0.1.5 or v0.2. Default: v0.2\n"
        "  --model=MODE         empirical (default) or hl-a\n"
        "  --start-n=N          Start n (uint64). Default: 4\n"
        "  --euler-cap          Enable Euler safety cap (default)\n"
        "  --no-euler-cap       Disable Euler safety cap\n"
        "  --header             Enable outputing the header line (default)\n"
        "  --no-header          Disable outputing the header line\n"
        "  --include-trivial    Include the trivial pair (n,n) when n is prime (default: off)\n"
        "  --config-line        Prepend a commented provenance line to CSV output (opt-in)\n"
        "  -h, --help           Show this help and exit\n"
        "  -V, --version        Show version and exit\n",
        prog
    );
}

static inline uint64_t log_floor_u64(uint64_t n, uint64_t base) {
    uint64_t k = 0;
    for(;n >= base;n/=base,k++);
    return k;               // largest base^k <= original n (returns 0 for n in [0..base])
}

static inline uint64_t ipow_u64(uint64_t base, uint64_t exp) {
    uint64_t p = 1;
    for (; exp; --exp, p *= base) ;       // empty body; multiplies in the increment
    return p;
}

static inline uint64_t M_of_n(uint64_t n) {
    // M(n) = ceil(((2n+1) - sqrt(8n+1)) / 2) - 1
    // Use long double to avoid overflow and for accurate sqrt on large n.
    long double nd = (long double)n;

    // this limit avoids the range where we cannot use Euler prime series to represent
    // the pair
    long double val = ceill( ((2.0L*nd + 1.0L) - sqrtl(8.0L*nd + 1.0L)) / 2.0L ) - 1.0L;
    if (val < 0.0L) return 0ULL;       // safety for tiny n (not really needed for n>=4)
    return (uint64_t)val;
}

// Small odd primes (no 2) – enough to cover huge n before overflow.
static const uint32_t ODD_PRIMES[] = {
    3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97
};
static const size_t ODD_PRIMES_N = sizeof(ODD_PRIMES)/sizeof(ODD_PRIMES[0]);

// Compute the largest odd-primorial P <= n, and the next threshold P_next = P * next_odd_prime
// (if that multiplication would overflow or exceed UINT64_MAX, leave P_next = 0 to mean "no next").
static inline void odd_primorial_base_and_next(uint64_t n, uint64_t *P, uint64_t *P_next) {
    uint64_t p = 1;
    uint64_t next = 0;
    for (size_t i = 0; i < ODD_PRIMES_N; ++i) {
        __uint128_t cand = (__uint128_t)p * (__uint128_t)ODD_PRIMES[i];
        if (cand > (__uint128_t)n) { // next would exceed n; stop
            if (i < ODD_PRIMES_N) {
                // next threshold if we later cross it
                if (cand <= (__uint128_t)UINT64_MAX) next = (uint64_t)cand;
            }
            break;
        }
        p = (uint64_t)cand;
        // try to set the next threshold for the *next* odd prime if possible
        if (i+1 < ODD_PRIMES_N) {
            __uint128_t cand2 = (__uint128_t)p * (__uint128_t)ODD_PRIMES[i+1];
            if (cand2 <= (__uint128_t)UINT64_MAX) next = (uint64_t)cand2;
            else next = 0;
        } else {
            next = 0;
        }
    }
    if (p < 3 && n >= 3) p = 3;      // ensure base >= 3 once n >= 3
    *P = p;
    *P_next = next;
}

// Given base B and current n, compute the right edge of the current bucket: next multiple of B.
static inline uint64_t next_multiple_ceiling(uint64_t n, uint64_t B) {
    if (B == 0) return n; // shouldn't happen
    uint64_t k = (n + B - 1) / B;
    return k * B;
}

static inline long double hlCorr(uint64_t n,uint64_t delta) {
    long double logN = logl(n);
    long double invlogNlogN = 1.0L/(logN*logN);
    long double invSum = 0.0L;
    long double sum = 0.0L;
    for(uint64_t m=1+(n&1);m <= delta;m+=2) {
        sum += 1.0L/(logl(n-m)*logl(n+m));
        invSum += invlogNlogN;
    }
    return (invSum > 0)?(sum/invSum):1.0L;
}

static inline uint64_t computeDelta(uint64_t n,long double alpha,int *euler_cap) {
    uint64_t delta = (uint64_t)floorl(alpha * (long double)n);

    // Euler cap (mutate euler_cap once it can never bind again)
    if (euler_cap != NULL && *euler_cap) {
        uint64_t cap = M_of_n(n);
        if (cap < 1ULL) cap = 1ULL;          // tiny-n guard
        if (delta > cap) {
            delta = cap;                      // cap binds
        }
        else if (delta + 1ULL <= cap) {
            *euler_cap = 0;                    // safe to disable forever
            if(fabsl(alpha - 1.0L) < 1.0E-18L) {
                 fprintf(stderr,
                     "FATAL: Euler cap bound violated at n=%" PRIu64
                     " (alpha=1): delta=%" PRIu64 " > M(n)=%" PRIu64 "\n",
                     n, delta, cap);
                 delta = ~0ULL;
            }
        }
    }
    return delta;
}

static inline uint64_t maxPrefEven(long double value,uint64_t minValue) {
    uint64_t retval = (~1ULL)&(uint64_t)ceill(value);
    return (retval >= minValue)?retval:minValue;
}

static inline uint64_t minPrefOdd(long double value,uint64_t maxValue) {
    uint64_t retval = 1ULL|(uint64_t)floorl(value);
    return (retval <= maxValue)?retval:maxValue;
}

int main(int argc, char* argv[]) {
    int exitStatus = 0;
    uint64_t* prime_array = NULL;
    struct stat st;
    int fd = -1;
    long double alpha = 0.5L;
    uint64_t n_start_opt = 4;
    // defaults
    int euler_cap    = 1; // default ON
    int header_line  = 1; // default print header line
    int include_trivial = 0;
    int write_config = 0;   // default OFF (backward compatible)
    agg_mode_t agg_mode = AGG_DECADE;
    model_mode_t model = MODEL_EMPIRICAL;
    compat_ver_t compat_ver = VER_CURRENT;

    static struct option long_opts[] = {
        {"alpha",           required_argument, 0,  0 },
        {"agg",             required_argument, 0,  0 },
        {"model",           required_argument, 0,  0 },
        {"start-n",         required_argument, 0,  0 },
        {"compat",          required_argument, 0,  0 },
        {"euler-cap",       no_argument,       0,  0 },
        {"no-euler-cap",    no_argument,       0,  0 },
        {"header",          no_argument,       0,  0 },
        {"no-header",       no_argument,       0,  0 },
        {"include-trivial", no_argument,       0,  0 },
        {"config-line",     no_argument,       0,  0 },  // NEW: opt-in
        {"help",            no_argument,       0, 'h'},
        {"version",         no_argument,       0, 'V'},
        {0,0,0,0}
    };

    int opt, long_index = 0;
    while ((opt = getopt_long(argc, argv, "hV", long_opts, &long_index)) != -1) {
        switch (opt) {
            case 0: {
                const char *name = long_opts[long_index].name;
                if (strcmp(name, "alpha") == 0) {
                    char *endp = NULL;
                    alpha = strtold(optarg, &endp);
                    if (!endp || *endp != '\0' || !(alpha >= 0.0L && alpha <= 1.0L)) {
                        fprintf(stderr, "Error: --alpha must be a number in [0,1]\n");
                        exitStatus = 1;
                        goto cleanup;
                    }
                } else if (strcmp(name, "start-n") == 0) {
                    char *endp = NULL;
                    unsigned long long tmp = strtoull(optarg, &endp, 10);
                    if (!endp || *endp != '\0' || tmp < 4ULL) {
                        fprintf(stderr, "Error: --start-n must be an integer >= 4\n");
                        exitStatus = 1;
                        goto cleanup;
                    }
                    n_start_opt = (uint64_t)tmp;
                } else if (strcmp(name, "compat") == 0) {
                    if(strcmp(optarg,"v0.1") == 0 || strncmp(optarg, "v0.1.5",6) == 0) {
                        if(alpha <= 0.5L) {
                            euler_cap = 0;
                        }
                        compat_ver = VER_0_1_5;
                    }
                    else if(strcmp(optarg,"current") == 0 || strcmp(optarg,"v0.2") == 0 || strncmp(optarg, "v0.2.",5) == 0) {
                        compat_ver = VER_CURRENT;
                    }
                    else {
                        fprintf(stderr, "Error: unrecognized compatability version.\n");
                        exitStatus = 1;
                        goto cleanup;
                    }
                } else if (strcmp(name, "euler-cap") == 0) {
                    euler_cap = 1;
                } else if (strcmp(name, "no-euler-cap") == 0) {
                    euler_cap = 0;
                } else if (strcmp(name, "header") == 0) {
                    header_line = 1;
                } else if (strcmp(name, "no-header") == 0) {
                    header_line = 0;
                } else if (strcmp(name, "include-trivial") == 0) {
                    include_trivial = 1;
                } else if (strcmp(name, "config-line") == 0) {
                    write_config = 1;
                } else if (strcmp(name, "agg") == 0) {
                    if (strcasecmp(optarg, "decade") == 0) {
                        agg_mode = AGG_DECADE;
                    } else if (strcasecmp(optarg, "primorial") == 0) {
                        agg_mode = AGG_PRIMORIAL;
                    } else {
                        fprintf(stderr, "Error: --agg must be 'decade' or 'primorial'\n");
                        exitStatus = 1;
                        goto cleanup;
                    }
                } else if (strcmp(name, "model") == 0) {
                    if (strcasecmp(optarg, "empirical") == 0) {
                        model = MODEL_EMPIRICAL;
                    } else if (strcasecmp(optarg, "hl-a") == 0 || strcasecmp(optarg, "hla") == 0) {
                        model = MODEL_HLA;
                    } else {
                        fprintf(stderr, "Error: --model must be empirical or hl-a\n");
                        exitStatus = 1; goto cleanup;
                    }
                    // store `model` where you keep other options (declare it with the others)
                }
                break;
            }
            case 'h': print_usage(argv[0]); exitStatus=0; goto cleanup;
            case 'V': fprintf(stderr, "pairrangesummary (GPL-3.0-or-later) v0.2.0\n"); exit(0);
            default : print_usage(argv[0]); exitStatus=1; goto cleanup;
        }
    }
    if (optind + 2 != argc) {
        print_usage(argv[0]);
        exitStatus=1;
        goto cleanup;
    }

    const char *prime_file = argv[optind];
    const uint64_t end_n = strtoull(argv[optind+1], NULL, 10);

    if (end_n <= n_start_opt) {
        fprintf(stderr, "Error: end_n (%" PRIu64 ") must be > start-n (%" PRIu64 ")\n", end_n, n_start_opt);
        exitStatus = 1;
        goto cleanup;
    }
    if (write_config) {
        fprintf(stdout,
            "# alpha=%.18Lg euler_cap=%d include_trivial=%d start_n=%" PRIu64
            " end_n=%" PRIu64 " model=%s\n",
            alpha, euler_cap, include_trivial, n_start_opt, end_n,
            (model == MODEL_EMPIRICAL ? "empirical" : "hl-a"));
    }

    // Open and mmap prime file
    fd = open(prime_file, O_RDONLY);
    if (fd < 0) {
        perror("open");
        exitStatus = 1;
        goto cleanup;
    }

    if (fstat(fd, &st) < 0) {
        perror("fstat");
        exitStatus = 1;
        goto cleanup;
    }

    prime_array = (uint64_t*)(
        mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0)
    );

    if (prime_array == MAP_FAILED) {
        perror("mmap");
        exitStatus = 1;
        goto cleanup;
    }

    size_t prime_array_len = (size_t)(st.st_size / sizeof(uint64_t));
    uint64_t *prime_array_end = prime_array + ((uint64_t)st.st_size/sizeof(uint64_t));
    uint64_t *current = prime_array;

    uint64_t n_start = n_start_opt;  // honor CLI start

    int decade = 0;
    uint64_t nextN = n_start+1;
    uint64_t nextDecade = 10;
    uint64_t powDecade = 1;
    uint64_t prim_base = 3;
    uint64_t prim_threshold_major = 0;
    uint64_t prim_threshold_minor = 0;
    uint64_t prim_right = 0;
    int odd_primorial_major = 5;
    int odd_primorial_minor = 3;

    if(header_line) {
        if (agg_mode == AGG_DECADE && compat_ver == VER_0_1_5) {
            if (model == MODEL_EMPIRICAL) {
                printf("DECADE,MIN AT,MIN,MAX AT,MAX,n_0,C_min,n_1,C_max,n_geom,<COUNT>,C_avg\n");
            }
            else {
                printf("DECADE,MIN AT,MIN,MAX AT,MAX,n_0,Cpred_min,n_1,Cpred_max,N_geom,<COUNT>,Cpred_avg,HLCorr\n");
            }
        }
        else {
            printf("START,minAt,G(minAt),maxAt,G(maxAt),n_0,C_min(n_0),n_1,C_max(n_1),n_geom,<COUNT>,C_avg\n");
        }
    }
    if (agg_mode != AGG_PRIMORIAL) {
        if(n_start < 4) {
           n_start = 4;
        }
        decade = log_floor_u64(n_start,10);
        powDecade = ipow_u64(10,decade);
        nextN = (n_start-(n_start%powDecade))+powDecade;
        n_start = nextN-powDecade;
        nextDecade = 10ULL*powDecade;
    }
    else {
        if(n_start < 6) {
           n_start = 6;
        }
        odd_primorial_base_and_next(n_start, &prim_threshold_minor, &prim_threshold_major);
        if (prim_threshold_minor <= prim_base) {
            prim_threshold_minor = prim_base;
            prim_threshold_major = prim_threshold_minor*odd_primorial_major;
        }
        else {
            uint64_t dummy;
            odd_primorial_base_and_next(prim_threshold_minor-1, &prim_base, &dummy);
            odd_primorial_major = (int)(prim_threshold_minor / prim_base);
            uint64_t prev;
            odd_primorial_base_and_next(prim_base-1, &prev, &dummy);
            odd_primorial_minor = (int)(prim_base / prev);
        }
        prim_right = next_multiple_ceiling(n_start, prim_base);
        if (prim_right == n_start) {
            prim_right += prim_base; // ensure progress
        }
        nextN = prim_right;         // first bucket end
        n_start = prim_right - prim_base;
    }
   
    long double pairCountMin = 0.0L;
    long double pairCountMax = 0.0L;
    long double pairCountMinNorm = 0.0L;
    long double pairCountMaxNorm = 0.0L;
    long double pairCountTotal = 0.0L;
    long double pairCountTotalNorm = 0.0L;

    uint64_t minAt = 0;
    uint64_t minNormAt = 0;
    uint64_t minAtDelta = 0;
    uint64_t minNormAtDelta = 0;
    uint64_t maxAt = 0;
    uint64_t maxNormAt = 0;
    uint64_t maxAtDelta = 0;
    uint64_t maxNormAtDelta = 0;

    int need_euler_cap = euler_cap;

    for (uint64_t n = n_start; n < end_n; ) {
        uint64_t delta = computeDelta(n,alpha,&need_euler_cap);
        if(delta == ~0ULL) {
             exitStatus = 2;
             goto cleanup;   // your centralized cleanup+exit
        }

        if(compat_ver != VER_0_1_5 || alpha > 0.5) {
            // Keep window valid: need n_min = n - delta - 1 >= 2 ⇒ delta ≤ n - 3 (for n ≥ 4)
            uint64_t max_delta = (n > 3) ? (n - 3) : 1;
            if (delta > max_delta) {
                delta = max_delta;
            }
        }

        uint64_t n_min = n - delta - 1;          // guaranteed: n_min ≥ 2

        const long double logN  = logl((long double)n);
        const long double logNlogN = logN*logN;
        const long double deltaL = (long double)delta;

        // Diagonal (n,n) contributes one ordered pair; reflect that as +0.5 in the width
        long double denom = (include_trivial ? 0.5L : 0.0L) + deltaL;
        long double norm  = (denom > 0.0) ? logNlogN / denom : 0.0L;
        int useHLCorrInst = 0;
        long double hlCorrAvg = 1.0L;

        long double pairCount = 0.0;      // raw (or predicted-raw) ordered pair count
        long double pairCountNorm = 0.0;  // normalized C(n)

        // === model selection ===
        if (model == MODEL_EMPIRICAL) {
            uint64_t pairCountN = countRangedPairs(n, n_min, &current, prime_array, prime_array_end);
            if (pairCountN == ~0ULL) {
                fprintf(stderr,"Failed to count pairs at %" PRIu64 "\n", n);
                exitStatus = -1; break;
            }
            if (include_trivial && current > prime_array && current < prime_array_end && current[-1] == n) {
                pairCountN++;
            }
            pairCount = (long double)pairCountN;
            pairCountNorm = pairCount * norm;
        } else { // MODEL_HLA
            // twoSGB returns the predicted C(n) (normalized); NOT a raw count.
            pairCountNorm = twoSGB(n, prime_array, prime_array_len);
            if (pairCountNorm < 0.0) {
                fprintf(stderr,"Failed HL-A prediction at %" PRIu64 "\n", n);
                exitStatus = -1; 
                break;
            }

            if (norm < 0.0) { // defensive (shouldn’t happen with your delta logic)
                fprintf(stderr,"HL-A: non-positive norm at n=%" PRIu64 "\n", n);
                exitStatus = -1; 
                break;
            }
            // for small windows we calculate hlCorr for every value
            if(((agg_mode == AGG_PRIMORIAL) ? odd_primorial_minor : powDecade) < 10) {
                useHLCorrInst = 1;
                hlCorrAvg = hlCorr(n,delta);
                pairCountNorm *= hlCorrAvg;
            }

            int hasTrivial = 0;
            if(include_trivial) {
                // called to update the current pointer
                countRangedPairs(n, n, &current, prime_array, prime_array_end);
                hasTrivial = (current > prime_array && current < prime_array_end && current[-1] == n);
            }
            if(hasTrivial) {
                pairCount     = (norm > 0.5L)?(pairCountNorm / deltaL):1.0L;  // raw-count estimate for MIN/MAX/<COUNT>
                pairCountNorm = pairCount*norm;
            }
            else if(norm > 0.0) {
                pairCount     = pairCountNorm / norm;  // raw-count estimate for MIN/MAX/<COUNT>
            }
        }
        if(useHLCorrInst && n == 4 && compat_ver == VER_0_1_5) {
            pairCountTotal += pairCount/hlCorrAvg;
            pairCountTotalNorm += pairCountNorm/hlCorrAvg;
            hlCorrAvg = 1.0L;
        }
        else {
            pairCountTotal += pairCount;
            pairCountTotalNorm += pairCountNorm;
        }

        if (pairCount > pairCountMax || ! maxAt) {
            pairCountMax = pairCount;
            maxAtDelta = delta;
            maxAt = n;
        }
        if (pairCount < pairCountMin || ! minAt) {
            pairCountMin = pairCount;
            minAtDelta = delta;
            minAt = n;
        }
        if (pairCountNorm > pairCountMaxNorm || ! maxNormAt) {
            pairCountMaxNorm = pairCountNorm;
            maxNormAtDelta = delta;
            maxNormAt = n;
        }
        if (pairCountNorm < pairCountMinNorm || ! minNormAt) {
            pairCountMinNorm = pairCountNorm;
            minNormAtDelta = delta;
            minNormAt = n;
        }

        if (++n == nextN) {
            // Compute n_geom from actual edges (no drift)
            const uint64_t n0 = n_start;     // left edge (inclusive)
            const uint64_t n1 = n;           // right edge (exclusive)
            const long double n_geom = sqrtl((long double)n0 * (long double)((compat_ver != VER_0_1_5?n1-1:n1)));
            long double pairCountAvg = pairCountTotal/(n1 - n0);
            long double pairCountAvgNorm = pairCountTotalNorm/(n1 - n0);
            if(model == MODEL_HLA && ! useHLCorrInst) {
                const uint64_t n_geom_odd = (compat_ver != VER_0_1_5)?minPrefOdd(n_geom,n1-1):(1ULL|(uint64_t)floorl(n_geom));
                const uint64_t delta_odd = computeDelta(n_geom_odd,alpha,NULL);
                const uint64_t n_geom_even = (compat_ver  != VER_0_1_5)?maxPrefEven(n_geom,n0):(1ULL+n_geom_odd);
                const uint64_t delta_even = computeDelta(n_geom_even,alpha,NULL);
	        hlCorrAvg = 0.5L*(hlCorr(n_geom_even,delta_even)+hlCorr(n_geom_odd,delta_odd));
                pairCountAvg *= hlCorrAvg;
                pairCountAvgNorm *= hlCorrAvg;
	        pairCountMin *= hlCorr(minAt,minAtDelta);
	        pairCountMax *= hlCorr(maxAt,maxAtDelta);
	        pairCountMinNorm *= hlCorr(minNormAt,minNormAtDelta);
	        pairCountMaxNorm *= hlCorr(maxNormAt,maxNormAtDelta);
            }
            if (agg_mode == AGG_DECADE && compat_ver == VER_0_1_5) {
                if (model == MODEL_EMPIRICAL) {
                    printf("%d,%" PRIu64 ",%.0LF,%" PRIu64 ",%.0LF,%" PRIu64 ",%.6LF,%" PRIu64 ",%.6LF,%" PRIu64 ",%.6LF,%.6LF\n",
                        decade,
                        minAt, pairCountMin,
                        maxAt, pairCountMax,
                        minNormAt, pairCountMinNorm,
                        maxNormAt, pairCountMaxNorm,
                        ((uint64_t)floorl(n_geom))|(n > 10 ? 1ULL : 0ULL),
                        pairCountAvg,
                        pairCountAvgNorm
                    );
                }
                else {
                    printf("%d,%" PRIu64 ",%.6LF,%" PRIu64 ",%.6LF,%" PRIu64 ",%.8LF,%" PRIu64 ",%.8LF,%" PRIu64 ",%.8LF,%.8LF,%.8LF\n",
                        decade,
                        minAt, pairCountMin,
                        maxAt, pairCountMax,
                        minNormAt, pairCountMinNorm,
                        maxNormAt, pairCountMaxNorm,
                        ((uint64_t)floorl(n_geom))|(n > 10 ? 1ULL : 0ULL),
                        pairCountAvg,
                        pairCountAvgNorm,
                        hlCorrAvg
                    );
               }
             
            }else if (agg_mode == AGG_DECADE) {
                printf("%de%d,%" PRIu64 ",%.0LF,%" PRIu64 ",%.0LF,%" PRIu64 ",%.6LF,%" PRIu64 ",%.6LF,%.0LF,%.6LF,%.6LF\n",
                    (int)((n1 - 1) / powDecade),
                    decade,
                    minAt, pairCountMin,
                    maxAt, pairCountMax,
                    minNormAt, pairCountMinNorm,
                    maxNormAt, pairCountMaxNorm,
                    n_geom,
                    pairCountAvg,
                    pairCountAvgNorm
                );
            } else { // AGG_PRIMORIAL
                int isMajor = (n0%odd_primorial_major == 0);
                printf("(%d#)%.1f,%" PRIu64 ",%.0LF,%" PRIu64 ",%.0LF,%" PRIu64 ",%.6LF,%" PRIu64 ",%.6LF,%.0LF,%.6LF,%.6LF\n",
                    isMajor?odd_primorial_major:odd_primorial_minor,
                    ((double)(int)((n1 - 1) / (isMajor?prim_threshold_minor:prim_base)))*0.5,
                    minAt, pairCountMin,
                    maxAt, pairCountMax,
                    minNormAt, pairCountMinNorm,
                    maxNormAt, pairCountMaxNorm,
                    n_geom,
                    pairCountAvg,
                    pairCountAvgNorm
                );
            }
            fflush(stdout);
        
            // Advance bucket boundaries ONCE, per mode
            if (agg_mode != AGG_PRIMORIAL) {
                // If we just hit a decade boundary, grow decade
                if (n1 == nextDecade) {
                    decade++;
                    powDecade *= 10;
                    nextDecade *= 10;
                }
                nextN = n1 + powDecade;   // next right edge
            } else {
                // Grow primorial base if crossing threshold
                if ((prim_threshold_minor && n1 >= prim_threshold_minor)||(prim_threshold_major && n1 >= prim_threshold_major)) {
                    odd_primorial_base_and_next(n1, &prim_threshold_minor, &prim_threshold_major);
                    if (prim_threshold_minor <= prim_base) {
                        prim_threshold_minor = prim_base;
                        prim_threshold_major = prim_threshold_minor*odd_primorial_major;
                    }
                    else {
                        uint64_t dummy;
                        odd_primorial_base_and_next(prim_threshold_minor-1, &prim_base, &dummy);
                        odd_primorial_major = (int)(prim_threshold_minor / prim_base);
                        uint64_t prev;
                        odd_primorial_base_and_next(prim_base-1, &prev, &dummy);
                        odd_primorial_minor = (int)(prim_base / prev);
                    }
                }
                uint64_t right = next_multiple_ceiling(n1, prim_base);
                if (right == n1) {
                    right += prim_base;   // ensure progress
                }
                nextN = right;
            }
        
            if (n >= end_n) break;
        
            // Move left edge to previous right edge ONCE
            n_start = n1;
        
            // Reset per-bucket accumulators ONCE
            minAt             = 0;
            minAtDelta        = 0;
            minNormAtDelta    = 0;
            minNormAt         = 0;
            maxAt             = 0;
            maxNormAt         = 0;
            maxAtDelta        = 0;
            maxNormAtDelta    = 0;
            pairCountMin      = 0.0L;
            pairCountMax      = 0.0L;
            pairCountMinNorm  = 0.0L;
            pairCountMaxNorm  = 0.0L;
            pairCountTotal    = 0.0L;
            pairCountTotalNorm= 0.0L;
        }
    }

    cleanup:
        if (prime_array && prime_array != MAP_FAILED) {
            munmap(prime_array, st.st_size);
        }
        if (fd >= 0) {
            close(fd);
            fd = -1;
        }
        fflush(stdout);
        fflush(stderr);
        exit(exitStatus);
}

