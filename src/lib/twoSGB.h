// twoSGB - used for twin prime research
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

#include "primeconstants.h"

// portable, fast odd-part extraction
static inline uint64_t oddPart(uint64_t n){
    if (n == 0ull || (n & 1ull)) return n;   // 0 or already odd

#if defined(__GNUC__) || defined(__clang__)
    return n >> __builtin_ctzll(n);          // maps to TZCNT/BSF + shift
#elif defined(_MSC_VER)
    unsigned long k;
    _BitScanForward64(&k, n);                // <intrin.h>, x64
    return n >> k;
#else
    // fallback: division by lowest-set-bit (always a power of two)
    return n / (n & (~n + 1ull));            // avoids using unary minus if you prefer
#endif
}

static inline double twoSGB(
    uint64_t n,
    const uint64_t *primes,
    size_t primes_len)
{
    const double base = 4.0 * TWIN_PRIME_C2;
    n = oddPart(n);
    if (n <= 1ull) return base;  // pure power of two or 1

    double s = base;
    uint64_t r = n;
    for (size_t i = 1; i < primes_len; ++i) {
        uint64_t p = primes[i];
        if (p * p > r) break;   // stop at sqrt(r)
        if (r % p == 0u) {
            if (p >= 3u) s *= (double)(p - 1) / (double)(p - 2);
            do { r /= p; } while (r % p == 0u);     // strip p^e
        }
    }
    if (r > 1ull && r >= 3ull) s *= (double)(r - 1) / (double)(r - 2);
    return s;
}

