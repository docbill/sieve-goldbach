// validatepairrangesummary - limited validation of the pairrangesummary files
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

// SPDX-License-Identifier: GPL-3.0-or-later

#include <bits/stdc++.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using u64 = uint64_t;

/*** tiny utils ***/
[[noreturn]] static void die(const std::string& s){
    std::cerr << "ERROR: " << s << "\n"; std::exit(1);
}
static inline void rstrip_cr(std::string& s){ if(!s.empty() && s.back()=='\r') s.pop_back(); }

/*** exact 6-decimal helpers (to mirror %.6f output) ***/
static inline long long to_micro6(long double x){ return llround(x * 1000000.0L); }
static inline bool eq6(long double a, long double b){ return to_micro6(a) == to_micro6(b); }
static inline std::string fmt6(long double x){ std::ostringstream o; o.setf(std::ios::fixed); o<<std::setprecision(6)<<x; return o.str(); }

/*** args ***/
struct Args {
    std::string csv_path;
    std::string bitmap_path;   // optional (odd-only, 1 = prime)
    std::string raw_path;      // optional (uint64_le primes)
};
static Args parse_args(int argc, char** argv){
    Args a;
    for(int i=1;i<argc;++i){
        std::string s = argv[i];
        auto need = [&](bool ok){ if(!ok) die("Missing value after "+s); return std::string(argv[++i]); };
        if(s=="--file")      a.csv_path = need(i+1<argc);
        else if(s=="--bitmap") a.bitmap_path = need(i+1<argc);
        else if(s=="--raw")    a.raw_path    = need(i+1<argc);
        else die("Unknown arg: "+s+"  (use --file <csv> [--bitmap <bmp>] [--raw <bin>])");
    }
    if(a.csv_path.empty()) die("Provide --file <pairrangesummary.csv>");
    if(!a.bitmap_path.empty() ^ !a.raw_path.empty())
        die("Provide both --bitmap and --raw (or neither)");
    return a;
}

/*** mmap wrapper ***/
struct Mmap {
    int fd=-1; size_t len=0; const unsigned char* p=nullptr;
    void open_ro(const std::string& path){
        fd = ::open(path.c_str(), O_RDONLY);
        if(fd<0) die("open("+path+") failed");
        struct stat st{};
        if(::fstat(fd,&st)!=0){ ::close(fd); die("fstat("+path+") failed"); }
        len = (size_t)st.st_size;
        if(len){
            void* addr = ::mmap(nullptr, len, PROT_READ, MAP_PRIVATE, fd, 0);
            if(addr==MAP_FAILED){ ::close(fd); die("mmap("+path+") failed"); }
            p = (const unsigned char*)addr;
#if defined(MADV_SEQUENTIAL)
            (void)::madvise((void*)p, len, MADV_SEQUENTIAL);
#endif
        }
    }
    void close_unmap(){
        if(p && len) ::munmap((void*)p, len);
        if(fd>=0) ::close(fd);
        fd=-1; len=0; p=nullptr;
    }
    ~Mmap(){ close_unmap(); }
};

/*** odd-only bitmap oracle: bit k ↔ 3+2k, 1 = prime ***/
struct PrimeBitmap {
    Mmap m; u64 max_n=1;
    void load(const std::string& path){
        m.open_ro(path);
        u64 bits = (u64)m.len * 8ull;
        max_n = (bits==0? 1ull : (3ull + 2ull*(bits-1ull)));
    }
    bool covers(u64 n) const { return n <= max_n; }
    bool isPrime(u64 n) const {
        if(n==2) return true;
        if(n<2 || (n&1ull)==0ull) return false;
        if(!covers(n)) die("bitmap does not cover n="+std::to_string(n));
        u64 k = (n-3ull)>>1;
        size_t byte = (size_t)(k>>3);
        unsigned sh = (unsigned)(k & 7ull);
        return ((m.p[byte] >> sh) & 1u) != 0u;
    }
};

/*** raw primes (uint64_le) ***/
struct RawPrimes {
    Mmap m; const u64* v=nullptr; size_t cnt=0;
    void load(const std::string& path){
        m.open_ro(path);
        if(m.len % sizeof(u64)) die("raw primes file size not multiple of 8");
        cnt = m.len/sizeof(u64);
        v = (const u64*)m.p;
    }
    // index of last prime <= x, or (size_t)-1 if none
    size_t upper_idx(u64 x) const {
        size_t lo=0, hi=cnt;
        while(lo<hi){
            size_t mid=(lo+hi)/2;
            if(v[mid] <= x) lo=mid+1; else hi=mid;
        }
        return (lo==0? (size_t)-1 : lo-1);
    }
};

/*** Count Goldbach pairs for 2n with |m|<=floor(n/2), m!=0
 *  Unique unordered pairs: p in [ceil(n/2), n), q = 2n - p prime; exclude p==n.
 ***/
static u64 goldbach_pairs_count_2n(u64 n, const RawPrimes& rp, const PrimeBitmap& pb){
    if(n<2) return 0;
    size_t ub = rp.upper_idx(n);              // last prime <= n
    if(ub==(size_t)-1) return 0;

    u64 lo_val = (n + 1)/2;                   // ceil(n/2)
    size_t lo=0, hi=ub+1;                     // lower_bound over v[0..ub]
    while(lo<hi){
        size_t mid=(lo+hi)/2;
        if(rp.v[mid] < lo_val) lo=mid+1; else hi=mid;
    }
    if(lo>ub) return 0;

    u64 c=0;
    for(size_t i=lo;i<=ub;++i){
        u64 p = rp.v[i];
        if(p==n) continue;                    // exclude m=0
        u64 q = 2*n - p;
        if(q>=2 && pb.isPrime(q)) ++c;
    }
    return 2*c;
}

/*** CSV helpers ***/
static std::vector<std::string> split_csv_simple(const std::string& s){
    std::vector<std::string> out;
    std::string cur; cur.reserve(64);
    for(size_t i=0;i<=s.size();++i){
        char ch = (i==s.size()? ',' : s[i]);
        if(ch==','){
            size_t a=0,b=cur.size();
            while(a<b && isspace((unsigned char)cur[a])) ++a;
            while(b>a && isspace((unsigned char)cur[b-1])) --b;
            out.emplace_back(cur.substr(a,b-a));
            cur.clear();
        }else cur.push_back(ch);
    }
    return out;
}

struct ColIdx {
    // we only REQUIRE these:
    int n0=-1, cmin=-1, n1=-1, cmax=-1, cavg=-1;
    // rich raw extrema (optional; parsed but UNUSED in checks per your note)
    int min_at=-1, MINv=-1, max_at=-1, MAXv=-1;
};

static ColIdx parse_header_and_get_indices(std::istream& in, size_t& header_ln){
    std::string line; header_ln = 0;
    while(std::getline(in,line)){
        ++header_ln; rstrip_cr(line);
        if(line.empty()) continue;
        if(line.size()>=3 &&
           (unsigned char)line[0]==0xEF && (unsigned char)line[1]==0xBB && (unsigned char)line[2]==0xBF)
            line.erase(0,3);
        break;
    }
    if(line.empty()) die("CSV appears empty (no header)");

    auto H = split_csv_simple(line);
    if(H.empty()) die("Empty header line");

    std::unordered_map<std::string,int> idx;
    for(int i=0;i<(int)H.size();++i) idx[H[i]] = i;

    auto need = [&](const char* name)->int{
        auto it = idx.find(name);
        if(it==idx.end()) die(std::string("Header missing column: ")+name);
        return it->second;
    };

    ColIdx ci;
    // required (support rich or minimal spellings)
    ci.n0   = idx.count("n_0")   ? need("n_0")   : need("n0");
    ci.cmin = idx.count("C_min") ? need("C_min") : need("Cmin");
    ci.n1   = idx.count("n_1")   ? need("n_1")   : need("n1");
    ci.cmax = idx.count("C_max") ? need("C_max") : need("Cmax");
    ci.cavg = idx.count("C_avg") ? need("C_avg") : need("Cavg");

    // optional raw-extrema columns
    if(idx.count("MIN AT")) ci.min_at = idx["MIN AT"];
    if(idx.count("MIN"))    ci.MINv   = idx["MIN"];
    if(idx.count("MAX AT")) ci.max_at = idx["MAX AT"];
    if(idx.count("MAX"))    ci.MAXv   = idx["MAX"];

    return ci;
}

static bool parse_row_fields(const std::string& s, const ColIdx& ci,
                             u64& n0, long double& Cmin,
                             u64& n1, long double& Cmax,
                             long double& Cavg)
{
    auto V = split_csv_simple(s);
    auto get_u64 = [&](int k, u64& out)->bool{
        if(k<0 || k>=(int)V.size()) return false;
        char* e=nullptr; unsigned long long u = strtoull(V[k].c_str(), &e, 10);
        if(e!=V[k].c_str()+V[k].size()) return false;
        out = (u64)u; return true;
    };
    auto get_ld = [&](int k, long double& out)->bool{
        if(k<0 || k>=(int)V.size()) return false;
        char* e=nullptr; long double d = strtold(V[k].c_str(), &e);
        if(e!=V[k].c_str()+V[k].size()) return false;
        out = d; return true;
    };

    return get_u64(ci.n0, n0) && get_ld(ci.cmin, Cmin) &&
           get_u64(ci.n1, n1) && get_ld(ci.cmax, Cmax) &&
           get_ld(ci.cavg, Cavg);
}

/*** main ***/
int main(int argc, char** argv){
    std::ios::sync_with_stdio(false);
    Args a = parse_args(argc, argv);

    bool do_endpoints = (!a.bitmap_path.empty());
    PrimeBitmap pb; RawPrimes rp;
    if(do_endpoints){
        pb.load(a.bitmap_path);
        rp.load(a.raw_path);
    }

    std::ifstream in(a.csv_path);
    if(!in) die("Failed to open CSV: "+a.csv_path);

    size_t header_ln=0;
    ColIdx ci = parse_header_and_get_indices(in, header_ln);

    std::string line; size_t ln = header_ln;
    size_t rows = 0, checked = 0;

    while(std::getline(in, line)){
        ++ln; rstrip_cr(line);
        if(line.empty()) continue;

        u64 n0=0, n1=0;
        long double Cmin=0, Cmax=0, Cavg=0;
        if(!parse_row_fields(line, ci, n0, Cmin, n1, Cmax, Cavg))
            die("Bad CSV row at line "+std::to_string(ln)+": "+line);
        ++rows;

        // Sanity: C_min <= C_avg <= C_max at 6dp
        if(to_micro6(Cavg) < to_micro6(std::min(Cmin, Cmax)) ||
           to_micro6(Cavg) > to_micro6(std::max(Cmin, Cmax))){
            die("line "+std::to_string(ln)+": C_avg not within [C_min, C_max] at 6dp");
        }

        // Optional endpoint recomputation: n_0 is the location of C_min, n_1 of C_max
        if(do_endpoints){
            auto scale = [&](u64 n)->long double{
                if(n < 2) return 0.0L;
                long double ln_n = logl((long double)n);          // natural log
                long double M = floorl((long double)n / 2.0L);     // M(n) = floor(n/2)
                if(M <= 0.0L) return 0.0L;
                return (ln_n * ln_n) / M;
            };

            long double Cm0 = (long double) goldbach_pairs_count_2n(n0, rp, pb) * scale(n0);
            long double Cm1 = (long double) goldbach_pairs_count_2n(n1, rp, pb) * scale(n1);

            // n_0 corresponds to C_min, n_1 to C_max (by your definition)
            if(!eq6(Cm0, Cmin)){
                std::ostringstream oss;
                oss << "line " << ln << ": C_min mismatch at 6dp (endpoint n_0). "
                    << "expected=" << fmt6(Cm0) << " file=" << fmt6(Cmin)
                    << "  (n_0=" << n0 << ")";
                die(oss.str());
            }
            if(!eq6(Cm1, Cmax)){
                std::ostringstream oss;
                oss << "line " << ln << ": C_max mismatch at 6dp (endpoint n_1). "
                    << "expected=" << fmt6(Cm1) << " file=" << fmt6(Cmax)
                    << "  (n_1=" << n1 << ")";
                die(oss.str());
            }
            ++checked;
        }
    }

    if(rows==0) die("No data rows");

    std::cout << "OK: validated pairrangesummary file\n"
              << "rows=" << rows
              << "  checked=" << checked
              << (checked? " (endpoint-rescaled at 6dp)\n" : " (structure-only)\n");
    return 0;
}

