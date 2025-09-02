// calculate2SGBRange - headers for HL-A calculations
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

// Compute 2*S_GB(2n) for a contiguous range of n, using a given prime list.
// out[i] gets 2*S_GB(2*(n0+i)) = 4*C2 * Π_{p| (n0+i), p>=3} (p-1)/(p-2).
// Requirements:
//   - primes[] is sorted ascending and begins with 2
//   - primes covers at least all primes <= (n0+len-1)
//   - out[] is preallocated with size >= len
// Complexity: ~ len * log log (n0+len) multiplications.

#include <stdint.h>
#include <stddef.h>

#ifndef TWIN_PRIME_C2
#define TWIN_PRIME_C2 0.6601618158468695739278121100145557784 /* high-prec C2 */
#endif

static inline void
compute2SGBrange(
     const uint32_t *primes,
     size_t primes_len,
     uint64_t n0,
     double *out,
     size_t len
) {
    const double base = 4.0 * TWIN_PRIME_C2;  // 2 * (2*C2)
    // init: value when odd part has no odd prime factors
    for (size_t i = 0; i < len; ++i) out[i] = base;

    if (len == 0 || primes_len == 0) return;

    const uint64_t end = n0 + (len ? (uint64_t)len - 1ull : 0ull);

    for (size_t ip = 0; ip < primes_len; ++ip) {
        uint32_t p = primes[ip];
        if (p <= 2u) continue;                 // ignore p=2
        if ((uint64_t)p > end) break;          // no multiples in [n0, end]

        // factor to apply once for each n divisible by p
        const double fac = (double)(p - 1) / (double)(p - 2);

        // first multiple of p in [n0, end]
        uint64_t q = (n0 + p - 1) / p;         // ceil(n0/p)

        // sweep multiples
        for (uint64_t m = q *(uint64_t)p; m <= end; m += (uint64_t)p) {
            out[m - n0] *= fac;                // multiply once per distinct prime divisor
        }
    }
}

