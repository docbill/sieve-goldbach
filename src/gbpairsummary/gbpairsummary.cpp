
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
#include "gbrange.hpp"

// ----- Strong enums -----
enum class AggMode   : int { Decade = 0, Primorial = 1 };

static const std::string ALPHA_KEY = "-=ALPHA=-";
static const std::string FORMAT_KEY = "-=FORMAT=-";

// ----- Usage -----
static void print_usage(const char* prog) {
    std::fprintf(stderr,
        "Usage: %s [OPTIONS] <prime_raw_file>\n"
        "\n"
        "Counts Goldbach pairs in a sliding window of half-width delta = floor(alpha(n)*n).\n"
        "By default applies the Euler safety cap M(n)=ceil(((2n+1)-sqrt(8n+1))/2)-1.\n"
        "\n"
        "Positional:\n"
        "  prime_raw_file       Path to raw uint64_t prime array file\n"
        "\n"
        "Options:\n"
        "  --alpha=VAL          Window half-width multiplier (long double) in [0,1]. Default: 0.5\n"
        "  --trace=MODE         Trace aggregation: decade (default), primorial, or none\n"
        "  --dec-out=FILE       Write decade aggregation CSV to FILE (use \"-\" for stdout)\n"
        "  --dec-raw=FILE       Write decade unnormalized aggregation CSV to FILE (use \"-\" for stdout)\n"
        "  --dec-norm=FILE      Write decade normaziled aggregation CSV to FILE (use \"-\" for stdout)\n"
        "  --dec-cps=FILE       Write decade cps CSV to FILE (use \"-\" for stdout)\n"
        "  --dec-cps-summary=FILE  Write a summary cps values to a text file (use \"-\" for stdout)\n"
        "  --dec-cps-summary-resume=FILE Resume processing by reading previous cps summary from FILE\n"
        "  --prim-out=FILE      Write primorial aggregation CSV to FILE (use \"-\" for stdout)\n"
        "  --prim-raw=FILE      Write primorial unnormalized aggregation CSV to FILE (use \"-\" for stdout)\n"
        "  --prim-norm=FILE     Write primorial normalized aggregation CSV to FILE (use \"-\" for stdout)\n"
        "  --prim-cps=FILE      Write primorial cps CSV to FILE (use \"-\" for stdout)\n"
        "  --prim-cps-summary=FILE Write a summary cps values to a text file (use \"-\" for stdout)\n"
        "  --prim-cps-summary-resume=FILE Resume processing by reading previous cps summary from FILE\n"
        "  --compat=VERSION     v0.1 (aka v0.1.5) or v0.2/current. Default: v0.2\n"
        "  --model=MODE         empirical (default) or hl-a\n"
        "  --start-n=N          Start n (uint64). Default: 4\n"
        "  --dec-start-n=N      Start n (uint64). Default: 4\n"
        "  --prim-start-n=N     Start n (uint64). Default: 6\n"
        "  --end-n=N            End n (uint64). Default: 5\n"
        "  --dec-end-n=N        End n (uint64). Default: 5\n"
        "  --prim-end-n=N       End n (uint64). Default: 9\n"
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

static int containsKey(const char * tmpl,const std::string &key) {
    if (!tmpl) return 0;
    std::string s(tmpl);
    return (s.find(key,0) != std::string::npos);
}

static inline std::string replace_all(std::string &retval,const std::string &key, const std::string rep) {
    std::size_t pos = 0;
    while ((pos = retval.find(key, pos)) != std::string::npos) {
        retval.replace(pos, key.size(), rep);
        pos += rep.size();
    }
    return retval;
}

static std::string expand_template(const char* tmpl, long double alpha, const char *format) {
    if (! tmpl) {
        return {};
    }
    std::string s = tmpl;
    s = replace_all(s,ALPHA_KEY,fmt_alpha(alpha));
    if(format) {
        const std::string sformat = format;
        s = replace_all(s,FORMAT_KEY,sformat);
    }
    return s;
}

// open from template; "-" means stdout; line-buffer for CSV
static FILE* open_stream_from_template(const char* tmpl, long double alpha, const char* format, bool append) {
    if (!tmpl) return nullptr;
    std::string path = expand_template(tmpl, alpha, format);
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

    GBRange range = GBRange();
    std::uint64_t n_start_opt = 0;
    std::uint64_t n_end_opt = 0;
    bool write_config = false;
    const char* dec_out_path  = nullptr;
    const char* dec_raw_path  = nullptr;
    const char* dec_norm_path  = nullptr;
    const char* dec_cps_path  = nullptr;
    const char* dec_cps_summary_path  = nullptr;
    const char* dec_cps_summary_resume_path = nullptr;
    const char* dec_boundRatioMin_path = nullptr;  // v0.2.0: bound ratio minimum output
    const char* dec_boundRatioMax_path = nullptr;  // v0.2.0: bound ratio maximum output
    const char* prim_out_path = nullptr;
    const char* prim_raw_path = nullptr;
    const char* prim_norm_path = nullptr;
    const char* prim_cps_path = nullptr;
    const char* prim_cps_summary_path = nullptr;
    const char* prim_cps_summary_resume_path = nullptr;
    const char* prim_boundRatioMin_path = nullptr;  // v0.2.0: bound ratio minimum output
    const char* prim_boundRatioMax_path = nullptr;  // v0.2.0: bound ratio maximum output
    bool append_to_file  = false;
    std::vector<long double> alphas;
    FILE * dec_trace = nullptr;
    FILE * prim_trace = nullptr;
    int eulerCap = -1;

    static struct option long_opts[] = {
        {"alpha",           required_argument, 0,  0 },
        {"trace",           required_argument, 0,  0 },
        {"model",           required_argument, 0,  0 },
        {"dec-out",         required_argument, 0,  0 },
        {"dec-raw",         required_argument, 0,  0 },
        {"dec-norm",        required_argument, 0,  0 },
        {"dec-cps",         required_argument, 0,  0 },
        {"dec-cps-summary", required_argument, 0,  0 },
        {"dec-cps-summary-resume", required_argument, 0,  0 },
        {"dec-bound-ratio-min", required_argument, 0,  0 },  // v0.2.0
        {"dec-bound-ratio-max", required_argument, 0,  0 },  // v0.2.0
        {"prim-out",        required_argument, 0,  0 },
        {"prim-raw",        required_argument, 0,  0 },
        {"prim-norm",       required_argument, 0,  0 },
        {"prim-cps",        required_argument, 0,  0 },
        {"prim-cps-summary",required_argument, 0,  0 },
        {"prim-cps-summary-resume",required_argument, 0,  0 },
        {"prim-bound-ratio-min", required_argument, 0,  0 },  // v0.2.0
        {"prim-bound-ratio-max", required_argument, 0,  0 },  // v0.2.0
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
                }
                else if (!std::strcmp(name, "n-start")) {
                    char* endp = nullptr;
                    unsigned long long tmp = strtoull(optarg, &endp, 10);
                    if (!endp || *endp != '\0' || tmp < 4ULL) {
                        std::fprintf(stderr, "Error: --start-n must be an integer >= 4\n");
                        return 1;
                    }
                    range.primAgg.left = range.decAgg.left = n_start_opt = (std::uint64_t)tmp;
                }
                else if (!std::strcmp(name, "dec-out")) {
                    dec_out_path = optarg; // "-" means stdout
                }
                else if (!std::strcmp(name, "dec-raw")) {
                    dec_raw_path = optarg; // "-" means stdout
                }
                else if (!std::strcmp(name, "dec-norm")) {
                    dec_norm_path = optarg; // "-" means stdout
                }
                else if (!std::strcmp(name, "dec-cps")) {
                    dec_cps_path = optarg; // "-" means stdout
                }
                else if (!std::strcmp(name, "dec-cps-summary")) {
                    dec_cps_summary_path = optarg; // "-" means stdout
                }
                else if (!std::strcmp(name, "dec-cps-summary-resume")) {
                    dec_cps_summary_resume_path = optarg;
                }
                else if (!std::strcmp(name, "dec-bound-ratio-min")) {
                    dec_boundRatioMin_path = optarg;  // v0.2.0
                }
                else if (!std::strcmp(name, "dec-bound-ratio-max")) {
                    dec_boundRatioMax_path = optarg;  // v0.2.0
                }
                else if (!std::strcmp(name, "prim-out")) {
                    prim_out_path = optarg; // "-" means stdout
                }
                else if (!std::strcmp(name, "prim-raw")) {
                    prim_raw_path = optarg; // "-" means stdout
                }
                else if (!std::strcmp(name, "prim-norm")) {
                    prim_norm_path = optarg; // "-" means stdout
                }
                else if (!std::strcmp(name, "prim-cps")) {
                    prim_cps_path = optarg; // "-" means stdout
                }
                else if (!std::strcmp(name, "prim-cps-summary")) {
                    prim_cps_summary_path = optarg; // "-" means stdout
                }
                else if (!std::strcmp(name, "prim-cps-summary-resume")) {
                    prim_cps_summary_resume_path = optarg;
                }
                else if (!std::strcmp(name, "prim-bound-ratio-min")) {
                    prim_boundRatioMin_path = optarg;  // v0.2.0
                }
                else if (!std::strcmp(name, "prim-bound-ratio-max")) {
                    prim_boundRatioMax_path = optarg;  // v0.2.0
                }
                else if (!std::strcmp(name, "dec-n-start")) {
                    char* endp = nullptr;
                    unsigned long long tmp = strtoull(optarg, &endp, 10);
                    if (!endp || *endp != '\0' || tmp < 4ULL) {
                        std::fprintf(stderr, "Error: --dec-start-n must be an integer >= 4\n");
                        return 1;
                    }
                    range.decAgg.left = (std::uint64_t)tmp;
                    if( n_start_opt == 0 || range.decAgg.left < n_start_opt) {
                        n_start_opt = range.decAgg.left;
                    }
                }
                else if (!std::strcmp(name, "prim-n-start")) {
                    char* endp = nullptr;
                    unsigned long long tmp = strtoull(optarg, &endp, 10);
                    if (!endp || *endp != '\0' || tmp < 4ULL) {
                        std::fprintf(stderr, "Error: --prim-start-n must be an integer >= 4\n");
                        return 1;
                    }
                    range.primAgg.left = (std::uint64_t)tmp;
                    if( n_start_opt == 0 || range.primAgg.left < n_start_opt) {
                        n_start_opt = range.primAgg.left;
                    }
                }
                else if (!std::strcmp(name, "n-end")) {
                    char* endp = nullptr;
                    unsigned long long tmp = strtoull(optarg, &endp, 10);
                    if (!endp || *endp != '\0' || tmp < 5ULL) {
                        std::fprintf(stderr, "Error: --dec-end-n must be an integer >= 5\n");
                        return 1;
                    }
                    range.primAgg.n_end = range.decAgg.n_end = n_end_opt = (std::uint64_t)tmp;
                }
                else if (!std::strcmp(name, "dec-n-end")) {
                    char* endp = nullptr;
                    unsigned long long tmp = strtoull(optarg, &endp, 10);
                    if (!endp || *endp != '\0' || tmp < 5ULL) {
                        std::fprintf(stderr, "Error: --dec-end-n must be an integer >= 5\n");
                        return 1;
                    }
                    range.decAgg.n_end = (std::uint64_t)tmp;
                    if( n_end_opt == 0 || range.decAgg.n_end < n_end_opt) {
                        n_end_opt = range.decAgg.n_end;
                    }
                }
                else if (!std::strcmp(name, "prim-n-end")) {
                    char* endp = nullptr;
                    unsigned long long tmp = strtoull(optarg, &endp, 10);
                    if (!endp || *endp != '\0' || tmp < 4ULL) {
                        std::fprintf(stderr, "Error: --prim-end-n must be an integer >= 9\n");
                        return 1;
                    }
                    range.primAgg.n_end = (std::uint64_t)tmp;
                    if( n_end_opt == 0 || range.primAgg.n_end < n_end_opt) {
                        n_end_opt = range.primAgg.n_end;
                    }
                }
                else if (!std::strcmp(name, "compat")) {
                    if (!std::strcmp(optarg, "v0.1") || !std::strncmp(optarg, "v0.1.5", 6)) {
                        range.compat_ver = CompatVer::V015;
                    } else if (!std::strcmp(optarg,"current") || !std::strcmp(optarg,"v0.2") || !std::strncmp(optarg,"v0.2.",5)) {
                        range.compat_ver = CompatVer::Current;
                    } else {
                        std::fprintf(stderr, "Error: unrecognized compatibility version.\n");
                        return 1;
                    }
                }
                else if (!std::strcmp(name, "euler-cap")) {
                    eulerCap = 1;
                }
                else if (!std::strcmp(name, "no-euler-cap")) {
                    eulerCap = 0;
                }
                else if (!std::strcmp(name, "append")) {
                    append_to_file = true;
                }
                else if (!std::strcmp(name, "no-append")) {
                    append_to_file = false;
                }
                else if (!std::strcmp(name, "include-trivial")) {
                    range.includeTrivial = 1;
                }
                else if (!std::strcmp(name, "config-line")) {
                    write_config = true;
                }
                else if (!std::strcmp(name, "trace")) {
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
                }
                else if (!std::strcmp(name, "model")) {
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
            case 'V': std::fprintf(stderr, "gbpairsummary (GPL-3.0-or-later) v0.2.0\n"); return 0;
            default : print_usage(argv[0]); return 1;
        }
    }
    if (optind + 1 != argc) {
        print_usage(argv[0]);
        return 1;
    }

    const char* prime_file = argv[optind];

    if (n_end_opt > 0 && n_end_opt <= n_start_opt) {
        std::fprintf(stderr, "Error: *-n-end (%" PRIu64 ") must be > *-n-start (%" PRIu64 ")\n", n_end_opt, n_start_opt);
        return 1;
    }
    if (write_config) {
        std::fprintf(stdout,
            "# eulerCap=%d includeTrivial=%d start_n=%" PRIu64
            " n_end=%" PRIu64 " model=%s\n",
            range.eulerCap, range.includeTrivial, n_start_opt, n_end_opt,
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
   
    // sort low → high and (optionally) dedupe exact repeats
    std::sort(alphas.begin(), alphas.end());
    alphas.erase(std::unique(alphas.begin(), alphas.end()), alphas.end());
    
    // default if none specified
    if (alphas.empty()) {
        alphas.push_back(0.5L);
    }

    if(dec_out_path && containsKey(dec_out_path,FORMAT_KEY)) {
        if(! dec_raw_path) {
            dec_raw_path = dec_out_path;
        }
        if(! dec_norm_path) {
            dec_norm_path = dec_out_path;
        }
        if(range.model == Model::Empirical && ! dec_cps_path) {
            dec_cps_path = dec_out_path;
        }
    }
    if(prim_out_path && containsKey(prim_out_path,FORMAT_KEY)) {
        if(! prim_raw_path) {
            prim_raw_path = prim_out_path;
        }
        if(! prim_norm_path) {
            prim_norm_path = prim_out_path;
        }
        if(range.model == Model::Empirical) {
            if(! prim_cps_path) {
                prim_cps_path = prim_out_path;
            }
        }
        else if(dec_cps_summary_path || prim_cps_summary_path || prim_cps_path) {
            fprintf(stderr,"Empirical model required for cps output\n");
            return 1;
        }
    }
    
    for(auto &alpha : alphas) {
        range.windows.push_back(std::make_unique<GBWindow>(alpha,range.product_series_left,range.compat_ver));
    }
    if(alphas.size() > 1) {
        if( ! (dec_out_path || dec_raw_path || dec_norm_path || dec_cps_path || prim_out_path || prim_raw_path || prim_norm_path || prim_cps_path)) {
            std::fprintf(stderr, "Multiple alpha values are not support to trace output.\n");
            return 1;
        }
        if(dec_out_path && !containsKey(dec_out_path,ALPHA_KEY)) {
            std::fprintf(stderr, "The %s macro required with multiple alpha values. --dec-out=%s\n", ALPHA_KEY.c_str(), dec_out_path);
            return 1;
        }
        if(dec_raw_path && !containsKey(dec_raw_path,ALPHA_KEY)) {
            std::fprintf(stderr, "The %s macro required with multiple alpha values. --dec-raw=%s\n", ALPHA_KEY.c_str(), dec_raw_path);
            return 1;
        }
        if(dec_norm_path && !containsKey(dec_norm_path,ALPHA_KEY)) {
            std::fprintf(stderr, "The %s macro required with multiple alpha values. --dec-norm=%s\n", ALPHA_KEY.c_str(), dec_norm_path);
            return 1;
        }
        if(dec_cps_path && !containsKey(dec_cps_path,ALPHA_KEY)) {
            std::fprintf(stderr, "The %s macro required with multiple alpha values. --dec-cps=%s\n", ALPHA_KEY.c_str(), dec_cps_path);
            return 1;
        }
        if(prim_out_path && !containsKey(prim_out_path,ALPHA_KEY)) {
            std::fprintf(stderr, "The %s macro required with multiple alpha values. --prim-out=%s\n", ALPHA_KEY.c_str(), prim_out_path);
            return 1;
        }
        if(prim_raw_path && !containsKey(prim_raw_path,ALPHA_KEY)) {
            std::fprintf(stderr, "The %s macro required with multiple alpha values. --prim-raw=%s\n", ALPHA_KEY.c_str(), prim_raw_path);
            return 1;
        }
        if(prim_norm_path && !containsKey(prim_norm_path,ALPHA_KEY)) {
            std::fprintf(stderr, "The %s macro required with multiple alpha values. --prim-norm=%s\n", ALPHA_KEY.c_str(), prim_norm_path);
            return 1;
        }
        if(prim_cps_path && !containsKey(prim_cps_path,ALPHA_KEY)) {
            std::fprintf(stderr, "The %s macro required with multiple alpha values. --prim-cps=%s\n", ALPHA_KEY.c_str(), prim_cps_path);
            return 1;
        }
        if(dec_boundRatioMin_path && !containsKey(dec_boundRatioMin_path,ALPHA_KEY)) {
            std::fprintf(stderr, "The %s macro required with multiple alpha values. --dec-bound-ratio-min=%s\n", ALPHA_KEY.c_str(), dec_boundRatioMin_path);
            return 1;
        }
        if(dec_boundRatioMax_path && !containsKey(dec_boundRatioMax_path,ALPHA_KEY)) {
            std::fprintf(stderr, "The %s macro required with multiple alpha values. --dec-bound-ratio-max=%s\n", ALPHA_KEY.c_str(), dec_boundRatioMax_path);
            return 1;
        }
        if(prim_boundRatioMin_path && !containsKey(prim_boundRatioMin_path,ALPHA_KEY)) {
            std::fprintf(stderr, "The %s macro required with multiple alpha values. --prim-bound-ratio-min=%s\n", ALPHA_KEY.c_str(), prim_boundRatioMin_path);
            return 1;
        }
        if(prim_boundRatioMax_path && !containsKey(prim_boundRatioMax_path,ALPHA_KEY)) {
            std::fprintf(stderr, "The %s macro required with multiple alpha values. --prim-bound-ratio-max=%s\n", ALPHA_KEY.c_str(), prim_boundRatioMax_path);
            return 1;
        }
    }
    bool need_trace = ! (dec_trace && prim_trace);
    for(auto &w : range.windows) {
        // Respect --trace default behavior: if neither path is given, keep current behavior.
        if (dec_out_path)  {
            w->dec.out  = open_stream_from_template(dec_out_path, w->alpha, "full", append_to_file);
            if(! w->dec.out) {
                return 1;
            }
            need_trace = false;
        }
        if (dec_raw_path)  {
            w->dec.raw  = open_stream_from_template(dec_raw_path, w->alpha, "raw", append_to_file);
            if(! w->dec.raw) {
                return 1;
            }
            need_trace = false;
        }
        if (dec_norm_path)  {
            w->dec.norm  = open_stream_from_template(dec_out_path, w->alpha, "norm", append_to_file);
            if(! w->dec.norm) {
                return 1;
            }
            need_trace = false;
        }
        if (dec_cps_path)  {
            w->dec.cps  = open_stream_from_template(dec_out_path, w->alpha, "cps", append_to_file);
            if(! w->dec.cps) {
                return 1;
            }
            need_trace = false;
        }
        if (dec_cps_summary_path) {
            range.decAgg.cps_summary  = open_stream_from_template(dec_cps_summary_path, w->alpha, "cps-summary", false);
            if(! range.decAgg.cps_summary) {
                return 1;
            }
        }
        if (dec_boundRatioMin_path) {
            w->dec.boundRatioMin = open_stream_from_template(dec_boundRatioMin_path, w->alpha, "bound-ratio-min", append_to_file);
            if(! w->dec.boundRatioMin) {
                return 1;
            }
        }
        if (dec_boundRatioMax_path) {
            w->dec.boundRatioMax = open_stream_from_template(dec_boundRatioMax_path, w->alpha, "bound-ratio-max", append_to_file);
            if(! w->dec.boundRatioMax) {
                return 1;
            }
        }
        if (prim_out_path) {
            w->prim.out  = open_stream_from_template(prim_out_path, w->alpha, "full", append_to_file);
            if(! w->prim.out) {
                return 1;
            }
            need_trace = false;
        }
        if (prim_raw_path) {
            w->prim.raw  = open_stream_from_template(prim_out_path, w->alpha, "raw", append_to_file);
            if(! w->prim.raw) {
                return 1;
            }
            need_trace = false;
        }
        if (prim_norm_path) {
            w->prim.norm  = open_stream_from_template(prim_out_path, w->alpha, "norm", append_to_file);
            if(! w->prim.norm) {
                return 1;
            }
            need_trace = false;
        }
        if (prim_cps_path) {
            w->prim.cps  = open_stream_from_template(prim_out_path, w->alpha, "cps", append_to_file);
            if(! w->prim.cps) {
                return 1;
            }
            need_trace = false;
        }
        if (prim_cps_summary_path) {
            range.primAgg.cps_summary  = open_stream_from_template(prim_cps_summary_path, w->alpha, "cps-summary", false);
            if(! range.primAgg.cps_summary) {
                return 1;
            }
        }
        if (prim_boundRatioMin_path) {
            w->prim.boundRatioMin = open_stream_from_template(prim_boundRatioMin_path, w->alpha, "bound-ratio-min", append_to_file);
            if(! w->prim.boundRatioMin) {
                return 1;
            }
        }
        if (prim_boundRatioMax_path) {
            w->prim.boundRatioMax = open_stream_from_template(prim_boundRatioMax_path, w->alpha, "bound-ratio-max", append_to_file);
            if(! w->prim.boundRatioMax) {
                return 1;
            }
        }
    }
    if(need_trace) {
        dec_trace = stdout;
    }
 
    for(auto &w : range.windows) {
        w->dec.trace = dec_trace;
        w->prim.trace = prim_trace;
        break;
    }
    
    range.init(primes.begin(),primes.end(),primes.size(),eulerCap);

    if (! append_to_file) {
        range.printHeaders();
    }

    // Resume from previous cps summary if specified
    if (dec_cps_summary_resume_path) {
        int resume_result = range.decInputCpsSummary(dec_cps_summary_resume_path);
        if (resume_result != 0) {
            std::fprintf(stderr, "Error: Failed to resume decade from %s (return code: %d)\n", 
                dec_cps_summary_resume_path, resume_result);
            return -1;
        }
    }
    if (prim_cps_summary_resume_path) {
        int resume_result = range.primInputCpsSummary(prim_cps_summary_resume_path);
        if (resume_result != 0) {
            std::fprintf(stderr, "Error: Failed to resume primorial from %s (return code: %d)\n", 
                prim_cps_summary_resume_path, resume_result);
            return -1;
        }
    }
    range.printCpsSummaryHeaders();
    range.processRows();
    
    return exitStatus;

} catch (const std::exception& e) {
    std::fprintf(stderr, "Unhandled exception: %s\n", e.what());
    return 1;
}

