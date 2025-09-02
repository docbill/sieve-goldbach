// certifygbpairs - checks the gbpairs file for certified values
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

[[noreturn]] static void die(const std::string& s){
    std::cerr << "ERROR: " << s << "\n"; std::exit(1);
}

/*** small helpers ***/
static inline void rstrip_cr(std::string& s){
    if (!s.empty() && s.back() == '\r') s.pop_back();
}

/*** FNV-1a 64 for a lightweight row digest ***/
static inline u64 fnv1a64_update(u64 h, u64 v){
    static constexpr u64 P = 1099511628211ull;
    for (int i=0;i<8;++i){ unsigned char b = (v>>(8*i)) & 0xFF; h ^= b; h *= P; }
    return h;
}

/*** Args ***/
struct Args {
    std::string bitmap_path;
    std::string csv_path;
};
static Args parse_args(int argc, char** argv){
    Args a;
    for (int i=1;i<argc;++i){
        std::string s = argv[i];
        if (s=="--bitmap" && i+1<argc) a.bitmap_path = argv[++i];
        else if (s=="--file" && i+1<argc) a.csv_path = argv[++i];
        else die("Unknown/invalid arg: "+s+"  (use --bitmap <path> --file <csv>)");
    }
    if (a.bitmap_path.empty()) die("Provide --bitmap <odd-only prime bitmap>");
    if (a.csv_path.empty()) die("Provide --file <goldbach CSV>");
    return a;
}

/*** Odd-only bitmap oracle (bit k -> 3+2k, 1 = prime), backed by mmap ***/
class BitmapPrimeOracle {
    int fd_ = -1;
    size_t len_ = 0;
    const unsigned char* p_ = nullptr;
    u64 max_n_ = 1;

public:
    explicit BitmapPrimeOracle(const std::string& path){
        fd_ = ::open(path.c_str(), O_RDONLY);
        if (fd_ < 0) die("open(bitmap) failed: " + path);

        struct stat st{};
        if (::fstat(fd_, &st) != 0) { ::close(fd_); die("fstat(bitmap) failed"); }
        if (!S_ISREG(st.st_mode)) { ::close(fd_); die("bitmap is not a regular file"); }

        len_ = static_cast<size_t>(st.st_size);
        if (len_ > 0) {
            void* addr = ::mmap(nullptr, len_, PROT_READ, MAP_PRIVATE, fd_, 0);
            if (addr == MAP_FAILED) { ::close(fd_); die("mmap(bitmap) failed"); }
            p_ = static_cast<const unsigned char*>(addr);
#if defined(MADV_SEQUENTIAL)
            (void)::madvise(const_cast<unsigned char*>(p_), len_, MADV_SEQUENTIAL);
#endif
        }
        u64 bytes = static_cast<u64>(len_);
        u64 bits  = bytes * 8ull;
        max_n_ = (bits == 0ull) ? 1ull : (3ull + 2ull*(bits - 1ull));
    }

    ~BitmapPrimeOracle(){
        if (p_ && len_) ::munmap(const_cast<unsigned char*>(p_), len_);
        if (fd_ >= 0) ::close(fd_);
    }

    u64 max_n() const { return max_n_; }

    bool isPrime(u64 n) const {
        if (n == 2) return true;
        if (n < 2 || (n & 1ull) == 0ull) return false;
        if (n > max_n_) die("Bitmap does not cover n=" + std::to_string(n));
        u64 k = (n - 3ull) >> 1;            // bit index for odd n
        size_t byte = static_cast<size_t>(k >> 3);
        unsigned shift = (unsigned)(k & 7ull);
        if (byte >= len_) die("Bitmap bounds check failed (byte index)");
        return ((p_[byte] >> shift) & 1u) != 0u; // 1 = prime
    }
};

/*** CSV split: 4 unsigned integer columns, no quotes ***/
static bool split4(const std::string& s, u64& c2N, u64& cNm, u64& cNp, u64& c2M){
    int field=0; size_t i=0, n=s.size();
    std::string tok; tok.reserve(32);
    u64 vals[4]; int got=0;
    while (i<=n){
        char ch = (i==n)? ',' : s[i];
        if (ch==','){
            size_t a=0,b=tok.size();
            while (a<b && isspace((unsigned char)tok[a])) ++a;
            while (b>a && isspace((unsigned char)tok[b-1])) --b;
            if (b<=a) return false;
            char* end=nullptr;
            unsigned long long v = strtoull(tok.c_str()+a, &end, 10);
            if (end != tok.c_str()+b) return false;
            if (got<4) vals[got++] = (u64)v;
            tok.clear(); ++field;
            if (field>4) return false;
        } else {
            tok.push_back(ch);
        }
        ++i;
    }
    if (got!=4) return false;
    c2N=vals[0]; cNm=vals[1]; cNp=vals[2]; c2M=vals[3];
    return true;
}

/*** Main verification ***/
int main(int argc, char** argv){
    std::ios::sync_with_stdio(false);
    Args a = parse_args(argc, argv);

    BitmapPrimeOracle P(a.bitmap_path);

    std::ifstream in(a.csv_path);
    if (!in) die("Failed to open CSV: " + a.csv_path);

    std::string line;

    // Read first non-empty line (strip possible UTF-8 BOM + CR) and validate header
    while (std::getline(in, line)) {
        rstrip_cr(line);
        if (line.empty()) continue;
        // strip BOM if present
        if (line.size() >= 3 &&
            (unsigned char)line[0]==0xEF && (unsigned char)line[1]==0xBB && (unsigned char)line[2]==0xBF) {
            line.erase(0,3);
        }
        break;
    }
    if (line.empty()) die("CSV appears empty");
    if (line != "2N,N-M,N+M,2M") {
        die("Unexpected header line: \"" + line + "\" (expected \"2N,N-M,N+M,2M\")");
    }

    u64 expected2N = 0;
    bool any = false;
    u64 count = 0, last2N = 0;
    u64 fnv = 1469598103934665603ull;

    while (std::getline(in, line)){
        rstrip_cr(line);
        if (line.empty()) continue;

        u64 c2N, cNm, cNp, c2M;
        if (!split4(line, c2N, cNm, cNp, c2M))
            die("Bad CSV row: " + line);

        // 2N: even and strictly consecutive (no gaps)
        if ((c2N & 1ull) != 0ull)
            die("2N is not even at row with 2N=" + std::to_string(c2N));
        if (!any) { expected2N = c2N; any = true; }
        if (c2N != expected2N)
            die("Gap/out-of-order 2N: expected " + std::to_string(expected2N) +
                " got " + std::to_string(c2N));

        // Sum/diff identities
        if (cNm + cNp != c2N)
            die("Sum check failed: (N-M)+(N+M) != 2N at 2N=" + std::to_string(c2N));
        if (cNp < cNm)
            die("Order check failed: N+M < N-M at 2N=" + std::to_string(c2N));
        if (cNp - cNm != c2M)
            die("Diff check failed: (N+M)-(N-M) != 2M at 2N=" + std::to_string(c2N));

        // Primality via mmap’d bitmap oracle
        if (!P.isPrime(cNm))
            die("N-M is not prime at 2N=" + std::to_string(c2N) +
                " (N-M=" + std::to_string(cNm) + ")");
        if (!P.isPrime(cNp))
            die("N+M is not prime at 2N=" + std::to_string(c2N) +
                " (N+M=" + std::to_string(cNp) + ")");

        // Cert stats
        ++count; last2N = c2N;
        fnv = fnv1a64_update(fnv, c2N);
        fnv = fnv1a64_update(fnv, cNm);
        fnv = fnv1a64_update(fnv, cNp);
        fnv = fnv1a64_update(fnv, c2M);

        expected2N += 2;
    }

    if (!any) die("No data rows found.");

    std::cout << "OK: verified Goldbach pairs from 2N="
              << (last2N - (count-1)*2) << " .. " << last2N << "\n"
              << "rows=" << count
              << "  last2N=" << last2N
              << "  fnv1a64=0x" << std::hex << std::setw(16) << std::setfill('0')
              << fnv << std::dec << "\n";
    return 0;
}

