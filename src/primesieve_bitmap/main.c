// primesieve_bitmap - creates a bitmap of odd primes
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define SEGMENT_SIZE 32768  // ~32 KB

void write_bitmap(uint8_t *flags, size_t count_bits, FILE *out) {
    size_t count_bytes = (count_bits + 7) / 8;
    fwrite(flags, 1, count_bytes, out);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <limit>\n", argv[0]);
        return 1;
    }

    uint64_t limit = strtoull(argv[1], NULL, 10);
    if (limit < 2) {
        fprintf(stderr, "Limit must be >= 2\n");
        return 1;
    }
    uint64_t sqrt_limit = (uint64_t)sqrt((double)limit) + 1;
    uint8_t *base_primes = calloc((sqrt_limit + 1) >> 1, sizeof(uint8_t));
    if (!base_primes) {
        fprintf(stderr, "Failed to allocate base_primes\n");
        return 1;
    }

    // Sieve small primes up to sqrt(limit)
    for (uint64_t i = 0; (2*i+3)*(2*i+3) <= sqrt_limit; ++i) {
        if (!(base_primes[i >> 3] & (1 << (i & 7)))) {
            uint64_t p = (i << 1) + 3;
            uint64_t jmax = (sqrt_limit - 1) >> 1;
            for (uint64_t j = (p * p - 3) >> 1; j < jmax; j += p) {
                base_primes[j >> 3] |= (1 << (j & 7));  // <<< FIXED
            }
        }
    }

    uint8_t *flags = calloc((SEGMENT_SIZE + 7) / 8, sizeof(uint8_t));
    if (!flags) {
        fprintf(stderr, "Failed to allocate flags\n");
        free(base_primes);
        return 1;
    }

    FILE *out = stdout;

    for (uint64_t low = 3; low <= limit; low += 2 * SEGMENT_SIZE) {
        uint64_t high = low + 2 * SEGMENT_SIZE - 1;
        uint64_t segment_len = (high - low) / 2 + 1;

        memset(flags, 0, (segment_len + 7)/8);

        for (uint64_t i = 0; (2*i+3)*(2*i+3) <= high; ++i) {
            if (!(base_primes[i >> 3] & (1 << (i & 7)))) {
                uint64_t p = (i<<1) + 3;
		if(p&0xffffffff00000000ULL) {
		    break;
		}
                uint64_t j = p * p;

                if (j < low) {
                    j = ((low + p - 1) / p) * p;
                    if ((j & 1) == 0) j += p;
                }

                for (uint64_t jstep=(p<<1); j <= high; j += jstep) {
                    uint64_t idx = (j - low) >> 1;
                    flags[idx >> 3] |= (1 << (idx & 7));
                }
            }
        }

        // Invert: 1 = prime
        for (size_t i = 0; i < (segment_len + 7)/8; ++i) {
            flags[i] = ~flags[i];
	}

        write_bitmap(flags, segment_len, out);
    }

    free(flags);
    free(base_primes);
    return 0;
}

