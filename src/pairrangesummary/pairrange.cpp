// pairinterval - class for aggregating interval counts
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
#include <cstdarg>   // for va_list, va_start, va_end, va_copy
#include <cstdio>    // for std::vfprintf, std::fflush
#include "pairrange.hpp"

// ----- Small helpers -----
static inline std::uint64_t log_floor_u64(std::uint64_t n, std::uint64_t base) {
    std::uint64_t k = 0;
    for (; n >= base; n /= base, ++k) {}
    return k;
}

static inline std::uint64_t ipow_u64(std::uint64_t base, std::uint64_t exp) {
    std::uint64_t p = 1;
    while (exp--) p *= base;
    return p;
}

// odd primes for primorial steps
static const std::uint32_t ODD_PRIMES[] = {
    3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97
};
static const std::size_t ODD_PRIMES_N = sizeof(ODD_PRIMES)/sizeof(ODD_PRIMES[0]);

static inline void odd_primorial_base_and_next(std::uint64_t n, std::uint64_t* P, std::uint64_t* P_next) {
    std::uint64_t p = 1, next = 0;
    for (std::size_t i = 0; i < ODD_PRIMES_N; ++i) {
        __uint128_t cand = (__uint128_t)p * (__uint128_t)ODD_PRIMES[i];
        if (cand > (__uint128_t)n) {
            if (cand <= (__uint128_t)UINT64_MAX) next = (std::uint64_t)cand;
            break;
        }
        p = (std::uint64_t)cand;
        if (i + 1 < ODD_PRIMES_N) {
            __uint128_t cand2 = (__uint128_t)p * (__uint128_t)ODD_PRIMES[i + 1];
            next = (cand2 <= (__uint128_t)UINT64_MAX) ? (std::uint64_t)cand2 : 0;
        } else {
            next = 0;
        }
    }
    if (p < 3 && n >= 3) p = 3;
    *P = p; *P_next = next;
}

static inline std::uint64_t next_multiple_ceiling(std::uint64_t n, std::uint64_t B) {
    if (B == 0) return n;
    std::uint64_t k = (n + B - 1) / B;
    return k * B;
}

static inline std::uint64_t maxPrefEven(long double value, std::uint64_t minValue) {
    std::uint64_t retval = (~1ULL) & (std::uint64_t)ceill(value);
    return (retval >= minValue) ? retval : minValue;
}

static inline std::uint64_t minPrefOdd(long double value, std::uint64_t maxValue) {
    std::uint64_t retval = 1ULL | (std::uint64_t)floorl(value);
    return (retval <= maxValue) ? retval : maxValue;
}

static inline std::uint64_t M_of_n(std::uint64_t n) {
    long double nd = (long double)n;
    long double val = ceill(((2.0L * nd + 1.0L) - sqrtl(8.0L * nd + 1.0L)) / 2.0L) - 1.0L;
    return (val < 0.0L) ? 0ULL : (std::uint64_t)val;
}


std::uint64_t PairRange::computeDelta(long double alpha,std::uint64_t n, int &euler_cap) {
    std::uint64_t delta = (std::uint64_t)floorl(alpha * (long double)n);
    if (euler_cap) {
        std::uint64_t cap = M_of_n(n);
        if (cap < 1ULL) {
            cap = 1ULL;
        }
        if (delta > cap) {
            delta = cap;
        }
        else if (delta + 1ULL <= cap) {
            euler_cap = 0;
            if (fabsl(alpha - 1.0L) < 1.0E-18L) {
                std::fprintf(stderr,
                    "FATAL: Euler cap bound violated at n=%" PRIu64
                    " (alpha=1): delta=%" PRIu64 " > M(n)=%" PRIu64 "\n",
                    n, delta, cap);
                return ~0ULL;
            }
        }
    }
    if (compat_ver != CompatVer::V015 || alpha > 0.5L) {
        std::uint64_t max_delta = (n > 3) ? (n - 3) : 1;
        if (delta > max_delta) {
            delta = max_delta;
        }
    }
    return delta;
}

// Printf to up to two FILE*s.
static inline void vfprintf_both(FILE* a, FILE* b, const char* fmt, va_list ap) {
    if (a) {
        va_list ap2;
        va_copy(ap2, ap);
        std::vfprintf(a, fmt, ap2); va_end(ap2);
        std::fflush(a);
    }
    if (b) {
        std::vfprintf(b, fmt, ap);
        std::fflush(b);
    }
}

static inline void fprintf_both(FILE* a, FILE* b, const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vfprintf_both(a, b, fmt, ap);
    va_end(ap);
}

static inline void fputs_both(const char* s, FILE* a, FILE* b) {
    if (a) {
        std::fputs(s, a);
    }
    if (b) {
       std::fputs(s, b);
    }
}
 
static void print_header(FILE *out1,FILE *out2,int useLegacy,Model model) {
    fputs_both(
        useLegacy
            ?(model == Model::Empirical
               ?"DECADE,MIN AT,MIN,MAX AT,MAX,n_0,C_min,n_1,C_max,n_geom,<COUNT>,C_avg\n"
               :"DECADE,MIN AT,MIN,MAX AT,MAX,n_0,Cpred_min,n_1,Cpred_max,N_geom,<COUNT>,Cpred_avg,HLCorr\n")
        : "START,minAt,G(minAt),maxAt,G(maxAt),n_0,C_min(n_0),n_1,C_max(n_1),n_geom,<COUNT>,C_avg\n",
        out1, out2);
}

void PairRange::print_headers() {
    for(auto &w : windows) {
        print_header(w->dec_out,w->dec_trace,(compat_ver == CompatVer::V015),model);
        print_header(w->prim_out,w->prim_trace,0,model);
    }
}

std::uint64_t PairRange::dec_reset(std::uint64_t n_start) {
    int need_reset = 0;
    for(auto &w : windows) {
        if(w->is_dec_active()) {
            need_reset = 1;
            w->dec_interval.reset();
        }
    }
    if(! need_reset) {
        return n_start;
    }
    if(n_start < dec_left) {
        n_start = dec_left;
    }
    decade      = (int)log_floor_u64(n_start, 10);
    dec_base   = ipow_u64(10, (std::uint64_t)decade);
    dec_right  = (n_start - (n_start % dec_base)) + dec_base;
    dec_threshold  = 10ULL * dec_base;
    dec_left = dec_right - dec_base;
    dec_n_geom = sqrtl((long double)dec_left * (long double)(compat_ver != CompatVer::V015 ? (dec_right - 1) : dec_right));
    if(dec_left >= dec_n_end) {
        dec_close();
    }
    return dec_left;
}

std::uint64_t PairRange::prim_reset(std::uint64_t n_start) {
    int need_reset = 0;
    for(auto &w : windows) {
        if(w->is_prim_active()) {
            need_reset = 1;
            w->prim_interval.reset();
        }
    }
    if(! need_reset) {
        return n_start;
    }
    if (n_start < prim_left) {
        n_start = prim_left;
    }
    odd_primorial_base_and_next(n_start, &prim_threshold_minor, &prim_threshold_major);
    if (prim_threshold_minor <= prim_base) {
        prim_threshold_minor = prim_base;
        prim_threshold_major = prim_threshold_minor * odd_primorial_major;
    } else {
        std::uint64_t dummy;
        odd_primorial_base_and_next(prim_threshold_minor - 1, &prim_base, &dummy);
        odd_primorial_major = (int)(prim_threshold_minor / prim_base);
        std::uint64_t prev;
        odd_primorial_base_and_next(prim_base - 1, &prev, &dummy);
        odd_primorial_minor = (int)(prim_base / prev);
    }
    prim_right = next_multiple_ceiling(n_start, prim_base);
    while (prim_right <= n_start) {
        prim_right += prim_base;
    }
    prim_left = prim_right - prim_base;
    prim_n_geom = sqrtl((long double)prim_left * (long double)(prim_right - 1));
    if(prim_left >= prim_n_end) {
        prim_close();
    }
    return prim_left;
}

void PairRange::dec_calc_average(PairRangeWindow &w, int applyHLCorr) {
    if(! w.is_dec_active()) {
        return;
    }
    PairInterval &dec_interval = w.dec_interval;
    dec_interval.pairCountAvg     = dec_interval.pairCountTotal     / (dec_right - dec_left);
    dec_interval.pairCountAvgNorm = dec_interval.pairCountTotalNorm / (dec_right - dec_left);
    if (applyHLCorr) {
        const std::uint64_t n_geom_odd  = (compat_ver == CompatVer::V015 ? ((1ULL | (std::uint64_t)floorl(dec_n_geom))) : minPrefOdd(dec_n_geom,dec_right - 1));
        const std::uint64_t delta_odd   = computeDelta(w.alpha,n_geom_odd);
        const std::uint64_t n_geom_even  = (compat_ver == CompatVer::V015 ? (1ULL + n_geom_odd) : maxPrefEven(dec_n_geom,dec_left));
        const std::uint64_t delta_even  = computeDelta(w.alpha,n_geom_even);
        dec_interval.applyHLCorr(n_geom_even, delta_even, n_geom_odd, delta_odd,
            decEvenCalc, decOddCalc, decMinCalc, decMaxCalc, decMinNormCalc, decMaxNormCalc );
    }
}
    
void PairRange::prim_calc_average(PairRangeWindow &w,int applyHLCorr) {
    if(! w.is_prim_active()) {
        return;
    }
    PairInterval &prim_interval = w.prim_interval;
    prim_interval.pairCountAvg     = prim_interval.pairCountTotal     / (prim_right - prim_left);
    prim_interval.pairCountAvgNorm = prim_interval.pairCountTotalNorm / (prim_right - prim_left);
    if (applyHLCorr) {
        const std::uint64_t n_geom_odd  = minPrefOdd(prim_n_geom,prim_right - 1);
        const std::uint64_t delta_odd   = computeDelta(w.alpha,n_geom_odd);
        const std::uint64_t n_geom_even  = maxPrefEven(prim_n_geom,prim_left);
        const std::uint64_t delta_even  = computeDelta(w.alpha,n_geom_even);
        prim_interval.applyHLCorr(n_geom_even, delta_even, n_geom_odd, delta_odd,
            primEvenCalc, primOddCalc, primMinCalc, primMaxCalc, primMinNormCalc, primMaxNormCalc );
    }
}
    
void PairRange::dec_output_aggregate(PairRangeWindow &w) {
    if(! w.is_dec_active()) {
        return;
    }
    PairInterval &dec_interval = w.dec_interval;
    if(compat_ver != CompatVer::V015) {
        fprintf_both(w.dec_out,w.dec_trace,
            (model == Model::Empirical)
		? "%de%d,%" PRIu64 ",%.0Lf,%" PRIu64 ",%.0Lf,%" PRIu64 ",%.9Lf,%" PRIu64 ",%.6Lf,%.0Lf,%.6Lf,%.9Lf\n"
		: "%de%d,%" PRIu64 ",%.3Lf,%" PRIu64 ",%.3Lf,%" PRIu64 ",%.9Lf,%" PRIu64 ",%.6Lf,%.0Lf,%.6Lf,%.9Lf\n",
            (int)((dec_right - 1) / dec_base),
            decade,
            dec_interval.minAt,     dec_interval.pairCountMin,
            dec_interval.maxAt,     dec_interval.pairCountMax,
            dec_interval.minNormAt, dec_interval.pairCountMinNorm,
            dec_interval.maxNormAt, dec_interval.pairCountMaxNorm,
            dec_n_geom,
            dec_interval.pairCountAvg,
            dec_interval.pairCountAvgNorm
        );
        return;
    }
    if(model == Model::Empirical) {
        fprintf_both(w.dec_out,w.dec_trace,"%d,%" PRIu64 ",%.0Lf,%" PRIu64 ",%.0Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf,%.6Lf\n",
            decade,
            dec_interval.minAt,     dec_interval.pairCountMin,
            dec_interval.maxAt,     dec_interval.pairCountMax,
            dec_interval.minNormAt, dec_interval.pairCountMinNorm,
            dec_interval.maxNormAt, dec_interval.pairCountMaxNorm,
            ((std::uint64_t)floorl(dec_n_geom)) | (dec_n_geom >= 10L ? 1ULL : 0ULL),
            dec_interval.pairCountAvg,
            dec_interval.pairCountAvgNorm
        );
        return;
    }
    fprintf_both(w.dec_out,w.dec_trace,"%d,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.8Lf,%" PRIu64 ",%.8Lf,%" PRIu64 ",%.8Lf,%.8Lf,%.8Lf\n",
        decade,
        dec_interval.minAt,     dec_interval.pairCountMin,
        dec_interval.maxAt,     dec_interval.pairCountMax,
        dec_interval.minNormAt, dec_interval.pairCountMinNorm,
        dec_interval.maxNormAt, dec_interval.pairCountMaxNorm,
        ((std::uint64_t)floorl(dec_n_geom)) | (dec_n_geom >= 10L ? 1ULL : 0ULL),
        dec_interval.pairCountAvg,
        dec_interval.pairCountAvgNorm,
        dec_interval.hlCorrAvg
    );
}

void PairRange::prim_output_aggregate(PairRangeWindow &w) {
    if(! w.is_prim_active()) {
        return;
    }
    PairInterval &prim_interval = w.prim_interval;
    int isMajor = (prim_left % odd_primorial_major == 0);
    fprintf_both(w.prim_out,w.prim_trace,
        (model == Model::Empirical)
            ? "(%d#)%.1f,%" PRIu64 ",%.0Lf,%" PRIu64 ",%.0Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf,%.0Lf,%.6Lf,%.9Lf\n"
            : "(%d#)%.1f,%" PRIu64 ",%.3Lf,%" PRIu64 ",%.3Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf,%.0Lf,%.6Lf,%.9Lf\n",
        isMajor ? odd_primorial_major : odd_primorial_minor,
        ((double)(int)((prim_right - 1) / (isMajor ? prim_threshold_minor : prim_base))) * 0.5,
        prim_interval.minAt,     prim_interval.pairCountMin,
        prim_interval.maxAt,     prim_interval.pairCountMax,
        prim_interval.minNormAt, prim_interval.pairCountMinNorm,
        prim_interval.maxNormAt, prim_interval.pairCountMaxNorm,
        prim_n_geom,
        prim_interval.pairCountAvg,
        prim_interval.pairCountAvgNorm
    );
}

int PairRange::addRow(
    PairRangeWindow &w,
    std::uint64_t n,
    std::uint64_t delta,
    const long double logNlogN,
    std::uint64_t pc,
    long double twoSGB
) {
    PairInterval &prim_interval = w.prim_interval;
    PairInterval &dec_interval = w.dec_interval;

    const long double deltaL = (long double)delta;
    const long double denom = (include_trivial ? 0.5L : 0.0L) + deltaL;
    const long double norm  = (denom > 0.0L) ? (logNlogN / denom) : 0.0L;

    if (norm < 0.0L) {
        std::fprintf(stderr, "HL-A: non-positive norm at %" PRIu64 "\n", n);
        return -1;
    }

    prim_interval.useHLCorrInst = dec_interval.useHLCorrInst = 0;

    if (model == Model::Empirical) {
        long double pairCount = (long double)pc;
        prim_interval.pairCount = dec_interval.pairCount = pairCount;
        prim_interval.pairCountNorm = dec_interval.pairCountNorm = pairCount * norm;
    } else { // HLA
        prim_interval.pairCount = dec_interval.pairCount = 0.0L;
        prim_interval.pairCountNorm = dec_interval.pairCountNorm = 0.0L;

        prim_interval.hlCorrAvg = dec_interval.hlCorrAvg = 1.0L;
        long double hlCorrAvg = 0.0L;
        if(w.is_prim_active()) {
            if(odd_primorial_minor < 5) {
                prim_interval.useHLCorrInst = 1;
                prim_interval.hlCorrAvg = hlCorrAvg = hlCorr(n,delta);
                prim_interval.pairCountNorm = twoSGB*hlCorrAvg;
            }
            else {
                prim_interval.pairCountNorm = twoSGB;
            }
            if (pc) {
                prim_interval.pairCount  = (norm > 0.5L) ? (prim_interval.pairCountNorm / deltaL) : 1.0L;
                prim_interval.pairCountNorm = prim_interval.pairCount * norm;
            } else if (norm > 0.0L) {
                prim_interval.pairCount = prim_interval.pairCountNorm / norm;
            }
        }
        if(w.is_dec_active()) {
            if (dec_base < 10) {
                dec_interval.useHLCorrInst  = 1;
                if(hlCorrAvg == 0.0L) {
                    hlCorrAvg = hlCorr(n,delta);
                } 
                dec_interval.hlCorrAvg = hlCorrAvg;
                dec_interval.pairCountNorm = twoSGB*hlCorrAvg;
            }
            else {
                dec_interval.pairCountNorm = twoSGB;
            }
            if (pc) {
                dec_interval.pairCount  = (norm > 0.5L) ? (dec_interval.pairCountNorm / deltaL) : 1.0L;
                dec_interval.pairCountNorm = dec_interval.pairCount * norm;
            } else if (norm > 0.0L) {
                dec_interval.pairCount = dec_interval.pairCountNorm / norm;
            }
        }
    }
    aggregate(w, n, delta);
    return 0;
}

void PairRange::prim_close() {
    prim_right = 0;
    for(auto &w : windows) {
        w->prim_close();
    }
}

void PairRange::dec_close() {
    dec_right = 0;
    for(auto &w : windows) {
        w->dec_close();
    }
}


