// libprime - declarations to use the libprime.a library
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

#ifndef PRIME_UTILS_H
#define PRIME_UTILS_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

extern FILE* pipeOpen(const char *cmdf,const char *filename,const char *opts);

static inline int is_odd_prime_fast(uint64_t n, const uint8_t* bitmap) {
    const uint64_t i = (n - 3) >> 1; // start at 3, only odds
    return bitmap[i >> 3] & (1 << (i & 7));
}

static inline int is_odd_prime(uint64_t n, const uint8_t* bitmap) {
    if (!(n & 1) || n < 3) return 0;
    const uint8_t i = (n - 3) >> 1; // start at 3, only odds
    return bitmap[i >> 3] & (1 << (i & 7));
}

// Count the number of Goldbach pairs for p+q=2n in the range [2,2n)
// Assumes: *current points to first prime > n, or a lower prime value in sorted buffer of primes.
// lowest: start of prime buffer
// highest: end of prime buffer (1 past last valid element)
extern int countPairs(uint64_t n, uint64_t **current, uint64_t *lowest, uint64_t *highest);

// Count the number of Goldbach pairs for p+q=2n in the range (n_min,2n-n_min) excluding 0.
// n_min: the minimum value to count.
// Assumes: *current points to first prime > n, or a lower prime value in sorted buffer of primes.
// lowest: start of prime buffer
// highest: end of prime buffer (1 past last valid element)
extern uint64_t countRangedPairs(uint64_t n, uint64_t n_min,uint64_t **current, uint64_t *lowest, uint64_t *highest);

// Resume counting the number of Goldbach pairs for p+q=2n in the range (n_min,2n-n_min) excluding 0.
// n_min: the minimum value to count.
// Assumes: *current points to first prime > n, or a lower prime value in sorted buffer of primes.
// lowest: start of prime buffer
// highest: end of prime buffer (1 past last valid element)
// lo: Pointer to the next lo prime to check
// hi: Pointer to the next hi prime to check
extern uint64_t countRangedPairsIter(uint64_t n, uint64_t n_min,uint64_t **current, uint64_t *lowest, uint64_t *highest,uint64_t **lo,uint64_t **hi);

// Find the minimum m > 0 value for Q_m (n-m)(n+m) for Goldbach pairs p+q=2n 
// primes: ascending array in [lowest, highest); *current is a moving cursor.
extern int findPair( uint64_t n, uint64_t **current, uint64_t *lowest, uint64_t *highest);

#include "twoSGB.h"
#include "calcBnormSymmetric.h"

#ifdef __cplusplus
}
#endif

#endif // PRIME_UTILS_H

