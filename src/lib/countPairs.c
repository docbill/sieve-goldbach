// countPairs - count the number of GB pairs in a window for a value of n
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

// Count the number of Goldbach pairs for p+q=2n in the range (n_min,2n-n_min) excluding 0.
// Assumes: *current points to first prime > n, or a lower prime value in sorted buffer of primes.
// lowest: start of prime buffer
// highest: end of prime buffer (1 past last valid element)
int countPairs( uint64_t n, uint64_t **current, uint64_t *lowest, uint64_t *highest) {
    int count = 0;
    uint64_t *lower = *current;
    uint64_t *higher = *current;
    const uint64_t twoN = n << 1;
    //printf("Start %" PRIu64 " %" PRIu64 "\n",*higher,n);
    for(;*(*current) < n; (*current)++);

    if (*higher == n) {
        // printf("%" PRIu64 " + %" PRIu64 " = %" PRIu64 "\n", n, n, twoN);
        higher = ++(*current);  // Advance the pointer to next prime after n
        count++;
    } else {
        // printf("Tried %" PRIu64 " + %" PRIu64 " > %" PRIu64 "\n", *higher, *higher, twoN);
        lower--;
    }
    if (higher >= highest) {
        return -1;
    }

    while (lower >= lowest) {
        //printf("-- %" PRIu64 " %" PRIu64 "\n",*lower,*higher);
        uint64_t sum = *lower + *higher;
        if (sum > twoN) {
            // printf("Tried %" PRIu64 " + %" PRIu64 " > %" PRIu64 "\n", *lower, *higher, twoN);
	    lower--;
	    continue;
        } else if (sum < twoN) {
            // printf("Tried %" PRIu64 " + %" PRIu64 " < %" PRIu64 "\n", *lower, *higher, twoN);
            higher++;
        } else {
            // printf("%" PRIu64 " + %" PRIu64 " = %" PRIu64 "\n", *lower, *higher, twoN);
            count += 2;
            lower--;
            higher++;
        }
        // Re-check bounds after increment
        if (higher >= highest) {
            return -1;
        }
    }

    return count;
}

