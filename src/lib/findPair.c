// findPairs - find the lowest m != 0 of goldbach pairs
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

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "libprime.h"

// Move cur to the first prime > n using linear adjustment.
// lowest .. highest is a half-open range [lowest, highest).
static inline uint64_t* seek_first_prime_gt_linear(
    uint64_t n,
    uint64_t *cur,
    uint64_t *lowest,
    uint64_t *highest
) {
    if (cur <= lowest) {
        cur = lowest;
    }
    if (cur >= highest) { // clamp inside
        cur = highest - 1;
    }

    if (*cur <= n) {
        // move forward until strictly greater than n
        for(;cur < highest && *cur <= n; cur++);
        // cur may equal highest (past-the-end) here
    } else {
        // move backward while previous is still > n
        for(;cur > lowest && *(cur - 1) > n;cur--);
    }
    return cur;
}

// Find the minimum m > 0 value for Q_m (n-m)(n+m) for Goldbach pairs p+q=2n 
// primes: ascending array in [lowest, highest); *current is a moving cursor.
int findPair(
    uint64_t n,
    uint64_t **current,
    uint64_t *lowest,
    uint64_t *highest
) {
    const uint64_t twoN = n << 1;
    const uint64_t n_min = 2;

    // Align hi to first prime > n (linear adjust around *current)
    uint64_t *hi = seek_first_prime_gt_linear(n, *current, lowest, highest);
    if (hi >= highest) { // out of primes to the right
        *current = highest;
        return -1;
    }

    // lo is the prime just below hi (<= n)
    if (hi == lowest) { // no smaller prime exists
        return 0;
    }
    uint64_t *lo = hi - 1;

    while (lo >= lowest && *lo > n_min) {
        // overflow-safe comparison for *lo + *hi ? twoN
        uint64_t need = twoN - *lo;
        if (*hi > need) {
            --lo;                    // sum > 2n
        }
        else if (*hi < need) {
            ++hi;                    // sum < 2n
            if (hi >= highest) {
                return -1;
            }
        }
        else {
            // we found a pair, return the m value.
            return n-lo[0];
        }
    }
    return (n == (*current)[-1])?0:-1;
}

