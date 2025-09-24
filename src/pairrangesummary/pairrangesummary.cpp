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
#include <cstdint>
#include <cinttypes>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <stdexcept>
#include <getopt.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <algorithm>

// C interop
extern "C" {
#include "libprime.h"
}

// C++ helper
#include "pairrange.hpp"

// ----- Strong enums -----
enum class AggMode   : int { Decade = 0, Primorial = 1 };

// ----- Usage -----
static void print_usage(const char* prog) {
    std::fprintf(stderr,
        "Usage: %s [OPTIONS] <prime_raw_file>\n"
        "\n"
        "Counts Goldbach pairs in a sliding window of half-width delta = floor(alpha*n).\n"
        "By default applies the Euler safety cap M(n)=ceil(((2n+1)-sqrt(8n+1))/2)-1.\n"
        "\n"
        "Positional:\n"
        "  prime_raw_file       Path to raw uint64_t prime array file\n"
        "\n"
        "Options:\n"
        "  --alpha=VAL          Window half-width multiplier (long double) in [0,1]. Default: 0.5\n"
        "  --trace=MODE         Trace aggregation: decade (default), primorial, or none\n"
        "  --dec-out=FILE       Write decade aggregation CSV to FILE (use \"-\" for stdout)\n"
        "  --prim-out=FILE      Write primorial aggregation CSV to FILE (use \"-\" for stdout)\n"
        "  --compat=VERSION     v0.1 (aka v0.1.5) or v0.2/current. Default: v0.2\n"
        "  --model=MODE         empirical (default) or hl-a\n"
        "  --start-n=N          Start n (uint64). Default: 4\n"
        "  --dec-start-n=N      Start n (uint64). Default: 4\n"
        "  --prim-start-n=N     Start n (uint64). Default: 6\n"
        "  --end-n=N            End n (uint64). Default: 5\n"
        "  --dec-end-n=N        End n (uint64). Default: 5\n"
        "  --prim-end-n=N       End n (uint64). Default: 5\n"
        "  --euler-cap          Enable Euler safety cap (default)\n"
        "  --no-euler-cap       Disable Euler safety cap\n"
        "  --append             Append output to existing files (no header output)\n"
        "  --no-append          Clobber existing file output the header line (default)\n"
        "  --include-trivial    Include the trivial pair (n,n) when n is prime (default: off)\n"
        "  --config-line        Prepend a commented provenance line to CSV output (opt-in)\n"
        "  -h, --help           Show this help and exit\n"
        "  -V, --version        Show version and exit\n",
        prog
    );
}

// ----- RAII mmap wrapper -----
class MMapU64 {
public:
    MMapU64() = default;
    MMapU64(const MMapU64&) = delete;
    MMapU64& operator=(const MMapU64&) = delete;

    ~MMapU64() {
        close();
    }

    void open_file(const char* path) {
        close();
        fd_ = ::open(path, O_RDONLY);
        if (fd_ < 0) {
            throw std::runtime_error("open failed");
        }
        if (::fstat(fd_, &st_) < 0) {
            throw std::runtime_error("fstat failed");
        }
        void* p = ::mmap(nullptr, st_.st_size, PROT_READ, MAP_PRIVATE, fd_, 0);
        if (p == MAP_FAILED) {
            throw std::runtime_error("mmap failed");
        }
        base_ = static_cast<std::uint64_t*>(p);
        len_  = (std::size_t)(st_.st_size / sizeof(std::uint64_t));
    }

    void close() {
        if (base_ && base_ != MAP_FAILED) {
            ::munmap(base_, st_.st_size);
            base_ = nullptr;
        }
        if (fd_ >= 0) {
            ::close(fd_); fd_ = -1;
        }
        len_ = 0;
    }

    std::uint64_t* begin() const {
        return base_;
    }
    std::uint64_t* end() const {
        return base_ ? (base_ + len_) : nullptr;
    }
    std::size_t size() const {
        return len_;
    }

private:
    int fd_ = -1;
    struct stat st_ {};
    std::uint64_t* base_ = nullptr;
    std::size_t len_ = 0;
};

// "%.6Lg" is compact (no forced fixed/scientific); tweak precision if you like.
static std::string fmt_alpha(long double a, int prec = 12) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*Lg", prec, a);
    return std::string(buf);
}

static std::string expand_alpha_template(const char* tmpl, long double alpha) {
    if (!tmpl) return {};
    std::string s(tmpl);
    const std::string key = "-=ALPHA=-";
    const std::string rep = fmt_alpha(alpha);  // e.g. "0.5" or "1"
    std::size_t pos = 0;
    while ((pos = s.find(key, pos)) != std::string::npos) {
        s.replace(pos, key.size(), rep);
        pos += rep.size();
    }
    return s;
}

// open from template; "-" means stdout; line-buffer for CSV
static FILE* open_stream_from_template(const char* tmpl, long double alpha, bool append) {
    if (!tmpl) return nullptr;
    std::string path = expand_alpha_template(tmpl, alpha);
    if (path == "-") return stdout;
    FILE* f = std::fopen(path.c_str(), append ? "a" : "w");
    if (!f) {
        std::perror(path.c_str());
        return nullptr;
    }
    std::setvbuf(f, nullptr, _IOLBF, 0);
    return f;
}

// ----- main -----
int main(int argc, char* argv[]) try {
    int exitStatus = 0;

    PairRange range = PairRange();
    std::uint64_t n_start_opt = 0;
    std::uint64_t n_end_opt = 0;
    int write_config = 0;
    const char* dec_out_path  = nullptr;
    const char* prim_out_path = nullptr;
    int append_to_file  = 0;
    long double alpha = 0.5L;
    FILE * dec_trace = nullptr;
    FILE * prim_trace = nullptr;
    std::vector<long double> alphas;

    static struct option long_opts[] = {
        {"alpha",           required_argument, 0,  0 },
        {"trace",           required_argument, 0,  0 },
        {"model",           required_argument, 0,  0 },
        {"dec-out",         required_argument, 0, 0},
        {"prim-out",        required_argument, 0, 0},
        {"n-start",         required_argument, 0,  0 },
        {"prim-n-start",    required_argument, 0,  0 },
        {"dec-n-start",     required_argument, 0,  0 },
        {"n-end",           required_argument, 0,  0 },
        {"prim-n-end",      required_argument, 0,  0 },
        {"dec-n-end",       required_argument, 0,  0 },
        {"compat",          required_argument, 0,  0 },
        {"euler-cap",       no_argument,       0,  0 },
        {"no-euler-cap",    no_argument,       0,  0 },
        {"append",          no_argument,       0,  0 },
        {"no-append",       no_argument,       0,  0 },
        {"include-trivial", no_argument,       0,  0 },
        {"config-line",     no_argument,       0,  0 },
        {"help",            no_argument,       0, 'h'},
        {"version",         no_argument,       0, 'V'},
        {0,0,0,0}
    };

    dec_trace = stdout;
    int opt, long_index = 0;
    while ((opt = getopt_long(argc, argv, "hV", long_opts, &long_index)) != -1) {
        switch (opt) {
            case 0: {
                const char* name = long_opts[long_index].name;
                if (!std::strcmp(name, "alpha")) {
                    char* endp = nullptr;
                    long double alpha = strtold(optarg, &endp);
                    if (!endp || *endp != '\0' || !(alpha >= 0.0L && alpha <= 1.0L)) {
                        std::fprintf(stderr, "Error: --alpha must be a number in [0,1]\n");
                        return 1;
                    }
                    alphas.push_back(alpha);
                } else if (!std::strcmp(name, "n-start")) {
                    char* endp = nullptr;
                    unsigned long long tmp = strtoull(optarg, &endp, 10);
                    if (!endp || *endp != '\0' || tmp < 4ULL) {
                        std::fprintf(stderr, "Error: --start-n must be an integer >= 4\n");
                        return 1;
                    }
                    range.prim_left = range.dec_left = n_start_opt = (std::uint64_t)tmp;
                 } else if (!std::strcmp(name, "dec-out")) {
                    dec_out_path = optarg;           // "-" means stdout
                 } else if (!std::strcmp(name, "prim-out")) {
                     prim_out_path = optarg;          // "-" means stdout
                 } else if (!std::strcmp(name, "dec-n-start")) {
                    char* endp = nullptr;
                    unsigned long long tmp = strtoull(optarg, &endp, 10);
                    if (!endp || *endp != '\0' || tmp < 4ULL) {
                        std::fprintf(stderr, "Error: --dec-start-n must be an integer >= 4\n");
                        return 1;
                    }
                    range.dec_left = (std::uint64_t)tmp;
                    if( n_start_opt == 0 || range.dec_left < n_start_opt) {
                        n_start_opt = range.dec_left;
                    }
                } else if (!std::strcmp(name, "prim-n-start")) {
                    char* endp = nullptr;
                    unsigned long long tmp = strtoull(optarg, &endp, 10);
                    if (!endp || *endp != '\0' || tmp < 6ULL) {
                        std::fprintf(stderr, "Error: --prim-start-n must be an integer >= 6\n");
                        return 1;
                    }
                    range.prim_left = (std::uint64_t)tmp;
                    if( n_start_opt == 0 || range.prim_left < n_start_opt) {
                        n_start_opt = range.prim_left;
                    }
                } else if (!std::strcmp(name, "n-end")) {
                    char* endp = nullptr;
                    unsigned long long tmp = strtoull(optarg, &endp, 10);
                    if (!endp || *endp != '\0' || tmp < 5ULL) {
                        std::fprintf(stderr, "Error: --dec-end-n must be an integer >= 5\n");
                        return 1;
                    }
                    range.prim_n_end = range.dec_n_end = n_end_opt = (std::uint64_t)tmp;
                } else if (!std::strcmp(name, "dec-n-end")) {
                    char* endp = nullptr;
                    unsigned long long tmp = strtoull(optarg, &endp, 10);
                    if (!endp || *endp != '\0' || tmp < 5ULL) {
                        std::fprintf(stderr, "Error: --dec-end-n must be an integer >= 5\n");
                        return 1;
                    }
                    range.dec_n_end = (std::uint64_t)tmp;
                    if( n_end_opt == 0 || range.dec_n_end < n_end_opt) {
                        n_end_opt = range.dec_n_end;
                    }
                } else if (!std::strcmp(name, "prim-n-end")) {
                    char* endp = nullptr;
                    unsigned long long tmp = strtoull(optarg, &endp, 10);
                    if (!endp || *endp != '\0' || tmp < 4ULL) {
                        std::fprintf(stderr, "Error: --prim-end-n must be an integer >= 9\n");
                        return 1;
                    }
                    range.prim_n_end = (std::uint64_t)tmp;
                    if( n_end_opt == 0 || range.prim_n_end < n_end_opt) {
                        n_end_opt = range.prim_n_end;
                    }
                } else if (!std::strcmp(name, "compat")) {
                    if (!std::strcmp(optarg, "v0.1") || !std::strncmp(optarg, "v0.1.5", 6)) {
                        if (alpha <= 0.5L) range.euler_cap = 0;
                        range.compat_ver = CompatVer::V015;
                    } else if (!std::strcmp(optarg,"current") || !std::strcmp(optarg,"v0.2") || !std::strncmp(optarg,"v0.2.",5)) {
                        range.compat_ver = CompatVer::Current;
                    } else {
                        std::fprintf(stderr, "Error: unrecognized compatibility version.\n");
                        return 1;
                    }
                } else if (!std::strcmp(name, "euler-cap")) {
                    range.euler_cap = 1;
                } else if (!std::strcmp(name, "no-euler-cap")) {
                    range.euler_cap = 0;
                } else if (!std::strcmp(name, "append")) {
                    append_to_file = 1;
                } else if (!std::strcmp(name, "no-append")) {
                    append_to_file = 2;
                } else if (!std::strcmp(name, "include-trivial")) {
                    range.include_trivial = 1;
                } else if (!std::strcmp(name, "config-line")) {
                    write_config = 1;
                } else if (!std::strcmp(name, "trace")) {
                    if (!strcasecmp(optarg, "decade")) {
                        dec_trace = stdout;
                        prim_trace = nullptr;
                    }
                    else if (!strcasecmp(optarg, "primorial")) {
                        prim_trace = stdout;
                        dec_trace = nullptr;
                    }
                    else if (!strcasecmp(optarg, "none")) {
                        prim_trace = nullptr;
                        dec_trace = nullptr;
                    }
                    else {
                        std::fprintf(stderr, "Error: --trace must be 'decade', 'primorial', or 'none'\n");
                        return 1;
                    }
                } else if (!std::strcmp(name, "model")) {
                    if (!strcasecmp(optarg, "empirical")) {
                        range.model = Model::Empirical;
                    }
                    else if (!strcasecmp(optarg, "hl-a") || !strcasecmp(optarg, "hla")) {
                        range.model = Model::HLA;
                    }
                    else {
                        std::fprintf(stderr, "Error: --model must be empirical or hl-a\n");
                        return 1;
                    }
                }
            } break;
            case 'h': print_usage(argv[0]); return 0;
            case 'V': std::fprintf(stderr, "pairrangesummary (GPL-3.0-or-later) v0.2.0\n"); return 0;
            default : print_usage(argv[0]); return 1;
        }
    }
    if (optind + 1 != argc) {
        print_usage(argv[0]);
        return 1;
    }

    const char* prime_file = argv[optind];



    if (n_end_opt <= n_start_opt) {
        std::fprintf(stderr, "Error: *-n-end (%" PRIu64 ") must be > *-n-start (%" PRIu64 ")\n", n_end_opt, n_start_opt);
        return 1;
    }
    if (write_config) {
        std::fprintf(stdout,
            "# alpha=%.18Lg euler_cap=%d include_trivial=%d start_n=%" PRIu64
            " n_end=%" PRIu64 " model=%s\n",
            range.alpha, range.euler_cap, range.include_trivial, n_start_opt, n_end_opt,
            (range.model == Model::Empirical ? "empirical" : "hl-a"));
    }

    // Map primes with RAII
    MMapU64 primes;
    try {
        primes.open_file(prime_file);
    } catch (const std::exception& e) {
        std::perror(e.what());
        return 1;
    }
    std::uint64_t* prime_array      = primes.begin();
    std::uint64_t* prime_array_end  = primes.end();
    std::size_t    prime_array_len  = primes.size();
    std::uint64_t* current          = prime_array;
    
    // sort low → high and (optionally) dedupe exact repeats
    std::sort(alphas.begin(), alphas.end());
    alphas.erase(std::unique(alphas.begin(), alphas.end()), alphas.end());
    
    // default if none specified
    if (alphas.empty()) {
        alphas.push_back(0.5L);
    }
    
    for(auto &alpha : alphas) {
        range.windows.push_back(std::make_unique<PairRangeWindow>(alpha));
    }
    int need_trace = ! (dec_trace && prim_trace);
    for(auto &w : range.windows) {
        // Respect --trace default behavior: if neither path is given, keep current behavior.
        if (dec_out_path)  {
            w->dec_out  = open_stream_from_template(dec_out_path, w->alpha, append_to_file);
            need_trace = 0;
        }
        if (prim_out_path) {
            w->prim_out  = open_stream_from_template(prim_out_path, w->alpha, append_to_file);
            need_trace = 0;
        }
        if ((dec_out_path  && !w->dec_out) || (prim_out_path && !w->prim_out)) {
            return 1;
        }
    }
    if(need_trace) {
        dec_trace = stdout;
    }
    int prim_is_active = 0;
    int dec_is_active = 0;
    for(auto &w : range.windows) {
        w->dec_trace = dec_trace;
        w->prim_trace = prim_trace;
        break;
    }
    for(auto &w : range.windows) {
        dec_is_active = dec_is_active || w->is_dec_active();
        prim_is_active = prim_is_active || w->is_prim_active();
    }

    if (! append_to_file) {
        range.print_headers();
    }

    range.dec_reset(range.dec_left);
    range.prim_reset(range.prim_left);
    std::uint64_t n_start = prim_is_active
	? ((dec_is_active && range.dec_left < range.prim_left)?range.dec_left:range.prim_left)
	: range.dec_left;
    std::uint64_t n_end = prim_is_active
	? ((dec_is_active && range.dec_n_end > range.prim_n_end)?range.dec_n_end:range.prim_n_end)
	: range.dec_n_end;

    for(auto & w : range.windows) {
        w->need_euler_cap = range.euler_cap;
    }

    for (std::uint64_t n = n_start; n < n_end; ) {
        const long double twoSGB_n = (range.model == Model::Empirical ? 0.0L : (long double)twoSGB(n, prime_array, prime_array_len));
        if (twoSGB_n < 0.0L) {
            std::fprintf(stderr, "Failed HL-A prediction at %" PRIu64 "\n", n);
            return -1;
        }
        int need_trivial = range.include_trivial;
        std::uint64_t pc = 0;
        // we use pointers here, so we know where we left off.
        std::uint64_t *lo = nullptr;
        std::uint64_t *hi = nullptr;
        // Here is where we add a loop if we needed to support multiple windows,
        // as twoSGB_n is alpha independant and does not need to be recomputed.
        int need_dec_reset = 0;
        int need_prim_reset = 0;
        long double logNlogN = 0.0L;
        for(auto & w : range.windows) {
            std::uint64_t delta = range.computeDelta(w->alpha, n, w->need_euler_cap);
            if (delta == ~0ULL) {
                return 2;
            }
            if (range.model == Model::Empirical) {
                std::uint64_t _pc = countRangedPairsIter(n, n - delta - 1, &current, prime_array, prime_array_end, &lo, &hi);
                if (_pc == ~0ULL) {
                    std::fprintf(stderr, "Failed to count pairs at %" PRIu64 "\n", n);
                    return -1;
                }
                if (need_trivial && current > prime_array && current < prime_array_end && current[-1] == n) {
                    pc += 1ULL+_pc;
                    need_trivial = 0;
                }
                else {
                    pc += _pc;
                }
            }
            else if (need_trivial) {
                need_trivial = 0;
                // simply called to position the current pointer
                countRangedPairs(n, n, &current, prime_array, prime_array_end);
                if (current > prime_array && current < prime_array_end && current[-1] == n) {
                    pc = 1;
                }
            }
            if(logNlogN == 0.0L) {
                const long double logN = logl((long double)n); 
                logNlogN = logN*logN;
            }
            int retval = range.addRow(*w, n, delta, logNlogN, pc, twoSGB_n);
            if(retval != 0) {
                return retval;
            }
        }
        n++;
        for(auto & w : range.windows) {
            if (w->is_dec_active() && n == range.dec_right) {
                //bcr fprintf(stderr,"Output Dec %" PRIu64 ",%0.6LF,%" PRIu64 ",%d,%d,%d,%" PRIu64 ",%" PRIu64 "\n",n,w->alpha,range.dec_right,w->is_dec_active(),w->dec_out != nullptr,w->dec_trace != nullptr,range.dec_left);
                //bcr fprintf(stderr,"xxOutput Prim %" PRIu64 ",%0.6LF,%" PRIu64 ",%d,%d,%d,%" PRIu64 ",%" PRIu64 "\n",n,w->alpha,range.prim_right,w->is_prim_active(),w->prim_out != nullptr,w->prim_trace != nullptr,range.prim_left);
                range.dec_calc_average(*w,range.model == Model::HLA && ! w->dec_interval.useHLCorrInst);
                range.dec_output_aggregate(*w);
                need_dec_reset = 1;
            }
            if (w->is_prim_active() && n == range.prim_right) {
                //bcr fprintf(stderr,"Output Prim %" PRIu64 ",%0.6LF,%" PRIu64 ",%d,%d,%d,%" PRIu64 ",%" PRIu64 "\n",n,w->alpha,range.prim_right,w->is_prim_active(),w->prim_out != nullptr,w->prim_trace != nullptr,range.prim_left);
                //bcr fprintf(stderr,"xxOutput Dec %" PRIu64 ",%0.6LF,%" PRIu64 ",%d,%d,%d,%" PRIu64 ",%" PRIu64 "\n",n,w->alpha,range.dec_right,w->is_dec_active(),w->dec_out != nullptr,w->dec_trace != nullptr,range.dec_left);
                range.prim_calc_average(*w,range.model == Model::HLA && ! w->prim_interval.useHLCorrInst);
                range.prim_output_aggregate(*w);
                need_prim_reset = 1;
            }
        }
        if(need_dec_reset) {
            range.dec_reset(range.dec_right);
        }
        if(need_prim_reset) {
            range.prim_reset(range.prim_right);
        }
    }
    return exitStatus;

} catch (const std::exception& e) {
    std::fprintf(stderr, "Unhandled exception: %s\n", e.what());
    return 1;
}

