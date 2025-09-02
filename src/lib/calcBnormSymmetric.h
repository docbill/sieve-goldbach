// calcBnormSymmetric - headers for calculation Bnorm Symmetric
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

#ifndef CALC_BNORM_SYMMETRIC_H
#define CALC_BNORM_SYMMETRIC_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    double   bwin;
    double   bnorm;
    uint64_t ipar_sz;
} BnormOut;

const uint64_t *primes_le_ptr(
    const uint64_t *lo,
    const uint64_t *hi,
    const uint64_t x);

/* Build cumulative products over odd primes:
   cum[j] = ∏_{i=0..j} (1 - 2 / p_i), where pr[i] are odd primes (≥3). */
double *build_cumprod_u64(const uint64_t *pr, size_t len);

/* Compute Bwin and Bnorm for symmetric window m = ±1..±M with parity filter (n+m odd).
   odd_pr_begin points to the first odd prime (3), plen is the number of odd primes available,
   cum is the cumulative product array of the same length. */
BnormOut calcBnormSymmetric(
    uint64_t n,
    uint64_t M,
    const uint64_t *odd_pr_begin,
    size_t plen,
    const double *cum);

#endif /* CALC_BNORM_SYMMETRIC_H */

