// countRangedPairs - for the specified window
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

// Count ordered Goldbach pairs p+q=2n with p>n_min (equivalently q>n_min),
// i.e. pairs in (n_min, 2n - n_min). Excludes the diagonal (n,n).
// primes: ascending array in [lowest, highest); *current is a moving cursor.
uint64_t countRangedPairs(
    uint64_t n,
    uint64_t n_min,
    uint64_t **current,
    uint64_t *lowest,
    uint64_t *highest
) {
    uint64_t *lo = NULL;
    uint64_t *hi = NULL;
    return countRangedPairsIter(n,n_min,current,lowest,highest,&lo,&hi);
}

// Count ordered Goldbach pairs p+q=2n with p>n_min (equivalently q>n_min),
// i.e. pairs in (n_min, 2n - n_min). Excludes the diagonal (n,n).
// primes: ascending array in [lowest, highest); *current is a moving cursor.
uint64_t countRangedPairsIter(
    uint64_t n,
    uint64_t n_min,
    uint64_t **current,
    uint64_t *lowest,
    uint64_t *highest,
    uint64_t **loPtr,
    uint64_t **hiPtr
) {
    const uint64_t twoN = n << 1;

    if(*hiPtr == NULL) {
        // Align hi to first prime > n (linear adjust around *current)
        *hiPtr = *current = seek_first_prime_gt_linear(n, *current, lowest, highest);
        if (*hiPtr >= highest) { // out of primes to the right
            *current = highest;
            return ~0ULL;
        }

        // loPtr is the prime just below hiPtr (<= n)
        if (*hiPtr == lowest) { // no smaller prime exists
            return 0;
        }
        *loPtr = *hiPtr - 1;
    }
    uint64_t count = 0;
    while (*loPtr >= lowest && **loPtr > n_min) {
        // overflow-safe comparison for **loPtr + **hiPtr ? twoN
        uint64_t need = twoN - **loPtr;
        if (**hiPtr > need) {
            --(*loPtr);                    // sum > 2n
        }
        else if (**hiPtr < need) {
            if(++(*hiPtr) >= highest) {
                return ~0ULL;
            }
        }
        else {
            // sum == 2n => one unordered pair, two ordered
            count += 2;
            --(*loPtr);
            if(++(*hiPtr) >= highest) {
                return ~0ULL;
            }
        }
    }
    return count;
}

