// certifyprimes - checks the prime files for certified values
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
using u64 = uint64_t;
using u128 = __uint128_t;

struct Args {
    std::string path;
    bool binary = false;
    bool text = false;
    bool big_endian = false;
    bool bitmap = false;
    bool bitmap_include2 = false; // set true if your bitmap has a bit for 2
    u64 segment = 4'000'000ULL; // numbers per segment
};

static void die(const std::string& msg) {
    std::cerr << "ERROR: " << msg << "\n";
    std::exit(1);
}

// FNV-1a 64-bit (sequence checksum over values read)
static inline u64 fnv1a64_update(u64 h, u64 v) {
    // hash the 8-byte little-endian representation of v to be format-agnostic
    static constexpr u64 FNV_PRIME = 1099511628211ull;
    static constexpr u64 FNV_OFFSET = 1469598103934665603ull;
    (void)FNV_OFFSET; // already provided starting seed by caller
    for (int i = 0; i < 8; ++i) {
        unsigned char b = static_cast<unsigned char>((v >> (8*i)) & 0xFF);
        h ^= b;
        h *= FNV_PRIME;
    }
    return h;
}

static Args parse_args(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        if (s == "--file" && i+1 < argc) { a.path = argv[++i]; }
        else if (s == "--binary") { a.binary = true; }
        else if (s == "--text")   { a.text = true; }
        else if (s == "--big")    { a.big_endian = true; }
	else if (s == "--bitmap") { a.bitmap = true; }
        else if (s == "--bitmap-include2") { a.bitmap_include2 = true; }
        else if (s == "--segment" && i+1 < argc) {
            a.segment = std::stoull(argv[++i]);
            if (a.segment < 1'000'000ULL) a.segment = 1'000'000ULL;
        } else {
            die("Unknown/invalid arg: " + s);
        }
    }
    if (a.path.empty()) die("Provide --file <path>");
    int modes = (a.binary?1:0) + (a.text?1:0) + (a.bitmap?1:0);
    if (modes != 1) {
        die("Select exactly one: --binary, --text, or --bitmap");
    }
    return a;
}

// Given file size in bytes for an odd-only bitmap (bit k -> 3+2k), compute max N
static inline u64 bitmap_max_n_from_bytes(u64 bytes) {
    u64 bits = bytes * 8ull;
    return (bits == 0ull) ? 1ull : (3ull + 2ull*(bits - 1ull));
}

// Count set bits in a byte (for quick stats)
static inline unsigned popcount8(unsigned char b) {
    // builtin is fine; fallback is trivial if needed
    return __builtin_popcount((unsigned)b);
}

// Reader that yields next u64 value from text or binary
struct SeqReader {
    std::ifstream in;
    bool is_binary, big;
    SeqReader(const std::string& path, bool binary, bool big_endian)
    : is_binary(binary), big(big_endian) {
        std::ios::openmode mode = std::ios::in | (binary ? std::ios::binary : (std::ios::openmode)0);
        in.open(path, mode);
        if (!in) die("Failed to open file: " + path);
    }
    bool next(u64& v) {
        if (is_binary) {
            unsigned char buf[8];
            if (!in.read(reinterpret_cast<char*>(buf), 8)) return false;
            if (big) {
                v = 0;
                for (int i = 0; i < 8; ++i) v = (v << 8) | buf[i];
            } else {
                v = 0;
                for (int i = 7; i >= 0; --i) v = (v << 8) | buf[i];
            }
            return true;
        } else {
            // text
            unsigned long long x;
            if (!(in >> x)) return false;
            v = static_cast<u64>(x);
            return true;
        }
    }
};

// Simple sieve to get all base primes up to limit (inclusive)
static void sieve_small(u64 limit, std::vector<u64>& primes) {
    if (limit < 2) { primes.clear(); return; }
    std::vector<bool> is_prime(limit + 1, true);
    is_prime[0] = false; is_prime[1] = false;
    for (u64 p = 2; p * p <= limit; ++p) {
        if (is_prime[p]) {
            for (u64 q = p*p; q <= limit; q += p) is_prime[q] = false;
        }
    }
    primes.clear();
    for (u64 n = 2; n <= limit; ++n) if (is_prime[n]) primes.push_back(n);
}

// Verify an odd-only bitmap file against a segmented sieve.
// Assumes 1=prime, 0=composite in the file (your encoder inverts marks).
// If include2==true, the very first bit corresponds to 2; otherwise the first
// bit corresponds to 3 and we ignore 2 here (but still check via sieve).
static void verify_bitmap_file(const std::string& path, bool include2) {
    // 1) Get file size and range
    std::ifstream in(path, std::ios::binary);
    if (!in) die("Failed to open bitmap: " + path);
    in.seekg(0, std::ios::end);
    std::streamoff sz = in.tellg();
    if (sz < 0) die("Failed to stat bitmap: " + path);
    u64 bytes = static_cast<u64>(sz);
    in.seekg(0, std::ios::beg);

    // If include2, the mapping is: bit0 -> 2, bit1 -> 3, bit2 -> 5, etc.
    // If odd-only, mapping is: bit0 -> 3, bit1 -> 5, ...
    // We'll verify odd domain bit-for-bit; for include2 we additionally verify the very first bit equals (2 is prime).
    u64 max_n = include2
        ? (bytes == 0 ? 1ull : (2ull + 1ull*(bytes*8ull - 1ull))) // (bit k -> 2 + k), rarely used
        : bitmap_max_n_from_bytes(bytes);                          // (bit k -> 3 + 2k)

    if (!include2 && max_n < 3) {
        // Empty odd domain; trivially OK (but odd)
        std::cout << "OK: verified odd-only bitmap (empty)\n"
                  << "bytes=" << bytes << "  max_n=" << max_n << "  primes_bits=0"
                  << "  fnv1a64=0x" << std::hex << 1469598103934665603ull << std::dec << "\n";
        return;
    }

    // 2) Prepare a segmented sieve over [lo..max_n] on the odd integers (and handle 2 separately)
    const u64 SEG_ODDS = 1ull << 20; // 1M odds per segment (~125KB bitmap slice) - tweak if desired
    u64 fnv = 1469598103934665603ull;
    u64 primes_bits = 0; // number of '1' bits (for stats)
    u64 last_prime = 2;

    // Verify the '2' bit if present at the head of the file
    if (include2 && bytes > 0) {
        unsigned char first;
        if (!in.read(reinterpret_cast<char*>(&first), 1)) die("Bitmap read error at first byte");
        // bit0 corresponds to 2
        bool bit2 = (first & 0x01) != 0;
        if (!bit2) die("Bitmap mismatch: bit for 2 is 0 but 2 is prime.");
        primes_bits += popcount8(first); // we'll hash/count this byte now,
        fnv = fnv1a64_update(fnv, (u64)first); // for bitmap we can fold bytes too; reuse fnv routine byte-wise below
        // Rewind to after first byte; the rest of pipeline expects odd-only alignment.
        // For simplicity, handle include2 by verifying bit 0 then continuing with odd-only from 3.
        // That means the remaining bytes represent bits starting at 3+2*k with an offset of 7 bits in the first byte.
        // To keep this minimal, we only support odd-only hereafter. If you truly need include2,
        // it's cleaner to store odd-only bitmaps. Otherwise, expand this to handle the shifted first byte.
        die("Current implementation supports odd-only bitmaps cleanly; please provide odd-only bitmap (no include2).");
    }

    // 3) Odd-only verification: process file in slices aligned to segment odds
    // We'll read exactly the number of bytes covering the current odd segment: (odds_this_seg+7)/8
    std::vector<char> buf;
    buf.reserve((SEG_ODDS + 7) / 8);

    // Precompute base primes up to sqrt(max_n) each iteration segment on demand (reuse your sieve_small)
    // We'll loop over segments [lo..hi] of odds:
    for (u64 lo = 3; lo <= max_n; ) {
        u64 hi = lo + 2*(SEG_ODDS - 1);
        if (hi > max_n) hi = max_n;
        u64 odds = ((hi - lo) >> 1) + 1;                  // number of odd candidates in this segment
        size_t need_bytes = (size_t)((odds + 7) / 8);
        buf.resize(need_bytes);

        // Read the next chunk of bitmap bytes
        if (!in.read(buf.data(), need_bytes)) die("Bitmap read error (unexpected EOF).");

        // Hash bytes for a cheap integrity fingerprint; also count 1-bits for stats
        for (size_t i = 0; i < need_bytes; ++i) {
            unsigned char b = (unsigned char)buf[i];
            // fold byte into FNV: reuse the u64 routine by feeding 8 copies is overkill;
            // instead, just xor+multiply per byte like we do for 64-bit little-endian:
            // (We can call fnv over the 8 little-endian bytes of 'b', but simpler is inline:)
            static constexpr u64 FNV_PRIME = 1099511628211ull;
            fnv ^= (u64)b;
            fnv *= FNV_PRIME;

            primes_bits += popcount8(b);
        }

        // Build prime mask for [lo..hi] with segmented sieve (odds only)
        //  - mark[i]=1 for prime, 0 for composite (match bitmap convention)
        std::vector<char> mark(odds, 1);
        long double dhi = (long double)hi;
        u64 need = (u64)std::floor(std::sqrt(dhi));
        if (need < 2) need = 2;
        std::vector<u64> base;
        sieve_small(need, base);
        for (u64 p : base) {
            if (p == 2) {
                // cross off even multiples; no effect on odd domain
                continue;
            }
            u128 pp = (u128)p * (u128)p;
            u64 start = (u64)pp;
            if (start < lo) {
                start = ((lo + p - 1) / p) * p;
            }
            if ((start & 1ull) == 0ull) start += p; // ensure odd start
            for (u64 m = start; m <= hi; m += (p << 1)) {
                // m is odd composite
                mark[(m - lo) >> 1] = 0;
            }
        }

        // Compare bitmap bits to 'mark' bit-by-bit
        for (u64 i = 0; i < odds; ++i) {
            unsigned char byte = (unsigned char)buf[(size_t)(i >> 3)];
            unsigned shift = (unsigned)(i & 7ull);
            bool bit = ((byte >> shift) & 1u) != 0u;
            bool should_be_prime = (mark[i] != 0);
            if (bit != should_be_prime) {
                u64 n = lo + (i << 1); // the odd value this bit represents
                std::ostringstream oss;
                oss << "Bitmap mismatch at n=" << n
                    << " (bit=" << bit << ", sieve=" << should_be_prime << ")";
                die(oss.str());
            }
            if (should_be_prime) last_prime = lo + (i << 1);
        }

        // next segment
        if (hi == std::numeric_limits<u64>::max()) break;
        if (hi >= max_n) break;
        lo = hi + 2; // next odd after hi
    }

    // Ensure we didn't leave any unread bytes (should be exact)
    if (in.peek() != std::char_traits<char>::eof()) die("Bitmap has trailing data beyond expected range.");

    std::cout << "OK: verified odd-only prime bitmap from 3.." << max_n << "\n"
              << "bytes=" << bytes
              << "  primes_bits=" << primes_bits
              << "  last=" << last_prime
              << "  fnv1a64=0x" << std::hex << std::setw(16) << std::setfill('0') << fnv << std::dec
              << "\n";
}


// Segmented sieve prime generator
struct SegPrimeGen {
    u64 seg_size;
    u64 lo = 0, hi = 0;
    u64 cursor = 0; // current index in segment
    std::vector<char> mark;
    std::vector<u64> base_primes;
    bool first = true;

    SegPrimeGen(u64 segment_numbers) : seg_size(segment_numbers) {
        if (seg_size < 1'000'000ULL) seg_size = 1'000'000ULL;
    }

    // Prepare next segment [lo, hi]
    void next_segment() {
        if (first) {
            lo = 2;
            // ADD THIS:
            hi = (seg_size >= 2) ? (lo + seg_size - 1) : 2;
            first = false;
        } else if (hi >= std::numeric_limits<u64>::max() - seg_size) {
            lo = hi + 1;
            hi = std::numeric_limits<u64>::max();
        } else {
            lo = hi + 1;
            hi = lo + seg_size - 1;
            if (hi < lo) hi = std::numeric_limits<u64>::max();
        }
        // Extend base primes up to sqrt(hi)
        long double dhi = static_cast<long double>(hi);
        u64 need = (u64)std::floor(std::sqrt(dhi));
        if (need < 2) need = 2;
        std::vector<u64> new_base;
        sieve_small(need, new_base);
        base_primes.swap(new_base);

        // mark primes in [lo,hi]
        u64 width = (hi >= lo) ? (hi - lo + 1) : 0;
        mark.assign(width, 1);
        if (width == 0) return;

        // Handle 0/1 if segment starts at 0 or 1
        if (lo == 0) { if (width > 0) mark[0] = 0; if (width > 1) mark[1] = 0; }
        else if (lo == 1) { mark[0] = 0; }

	for (u64 p : base_primes) {
            u128 pp128 = (u128)p * (u128)p;
            u64 pp = (pp128 > (u128)std::numeric_limits<u64>::max())
                ? std::numeric_limits<u64>::max()
                : (u64)pp128;

            // First multiple of p in [lo, hi]
            u64 start = (lo + p - 1) / p * p;
            if (start < pp) start = pp;  // do not start before p*p

            if (start > hi) continue;
            for (u64 m = start; m <= hi; m += p) {
                mark[m - lo] = 0;
            }
        }

        cursor = 0;
    }

    // Yield next prime; returns false only if we somehow run out (shouldn't happen practically)
    bool next(u64& p) {
        while (true) {
            if (mark.empty() || cursor >= mark.size()) {
                next_segment();
                if (mark.empty()) return false;
            }
            while (cursor < mark.size()) {
                if (mark[cursor]) {
                    p = lo + cursor;
                    ++cursor;
                    return true;
                }
                ++cursor;
            }
        }
    }
};

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Args a = parse_args(argc, argv);
    if (a.bitmap) {
        // Certify bitmap file independently (no raw prime file needed)
        verify_bitmap_file(a.path, a.bitmap_include2);
        return 0;
    }
    SeqReader reader(a.path, a.binary, a.big_endian);
    SegPrimeGen gen(a.segment);

    u64 file_val;
//    u64 expected;
    u64 count = 0;
    u64 last = 0;
    u64 fnv = 1469598103934665603ull; // FNV-1a offset basis
    u64 segments = 0;

    // We drive generation by the file: for each file value, generate the next prime and compare.
    // Also count how many segments were built (for info).
    // We'll count segments by incrementing in SegPrimeGen::next_segment()—quick hack:
    // track hi changes
    u64 prev_hi = 0;

    // tiny wrapper: detect when a new segment starts
    auto next_prime = [&](u64& p)->bool{
        bool ok = gen.next(p);
        if (!ok) return false;
        if (gen.hi != prev_hi) { ++segments; prev_hi = gen.hi; }
        return true;
    };

    // Start consuming
    if (!reader.next(file_val)) die("File is empty; expected at least the prime 2.");

    // Main loop: for every value in the file, the next generated prime must match it.
    while (true) {
        u64 p;
        if (!next_prime(p)) die("Internal error: prime generator exhausted.");
        if (p != file_val) {
            std::ostringstream oss;
            if (p < file_val) {
                oss << "Mismatch: expected prime " << p << " but file has " << file_val
                    << " (file skipped at least one prime).";
            } else {
                oss << "Mismatch: file has non-prime or out-of-order value " << file_val
                    << " (expected " << p << ").";
            }
            die(oss.str());
        }
        // matched one
        ++count;
        last = p;
        fnv = fnv1a64_update(fnv, p);

        // advance file; break on EOF (success)
        if (!reader.next(file_val)) break;
    }

    // Success: the file exactly matches the first 'count' primes (i.e., all primes up to 'last').
    std::cout << "OK: verified complete prime sequence from 2.." << last << "\n"
              << "count=" << count
              << "  last=" << last
              << "  fnv1a64=0x" << std::hex << std::setw(16) << std::setfill('0') << fnv << std::dec
              << "  segments=" << segments
              << "\n";
    return 0;
}

