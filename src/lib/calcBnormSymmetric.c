// countBnormSymmetric - calculates Bnorm (used in earlier drafts)
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

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

#include "primeconstants.h"          /* defines KAPPA (and optionally TWIN_PRIME_C2, etc.) */
#include "calcBnormSymmetric.h"

/* ---------- internal helpers ---------- */

/* upper_bound: first pointer in [begin,end) with *ptr > x */
const uint64_t *primes_le_ptr(
    const uint64_t *lo,
    const uint64_t *hi,
    const uint64_t x)
{
    while (lo < hi) {
        const uint64_t *mid = lo + (hi - lo) / 2;
        if (*mid <= x) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return lo;
}

/* B_ref for a given y: product over odd primes ≤ y of (1 - 2/p), using cumprod. */
static inline double bref_from_y(
    uint64_t y,
    const uint64_t *odd_pr_begin,
    const uint64_t *odd_pr_end,
    const double *cum)
{
    if (y < 3ULL) {
        return 1.0;
    }
    uint64_t * pr_ptr = primes_le_ptr(odd_pr_begin, odd_pr_end, y);
    return (pr_ptr == odd_pr_begin) ? 1.0 : cum[pr_ptr-odd_pr_begin-1];
}

/* |I^{par}| for symmetric window ±1..±M with parity filter (n+m odd). */
static inline uint64_t calcIparSize(uint64_t n, uint64_t M)
{
    if ((M & 1ULL) == 0ULL) {
        return M;
    }
    if ((n & 1ULL) == 0ULL) {
        return M + 1ULL;
    }
    return M - 1ULL;
}

/* ---------- exported API ---------- */

double *build_cumprod_u64(const uint64_t *pr, size_t len)
{
    if (len == 0U) {
        return NULL;
    }
    double *cp = (double *)malloc(len * sizeof(double));
    if (cp == NULL) {
        return NULL;
    }

#if defined(BREF_BUILD_LOGKAHAN)
    /* Robust log-space build with Kahan compensation. */
    long double acc = 0.0L;
    long double c   = 0.0L;
    for (size_t i = 0U; i < len; ++i) {
        long double li = log1pl(-2.0L / (long double)pr[i]);  /* log(1 - 2/p) */
        long double y  = li - c;
        long double t  = acc + y;
        c   = (t - acc) - y;
        acc = t;
        cp[i] = (double)expl(acc);
    }
#else
    /* Simple multiplicative build (perfectly fine for current ranges). */
    double acc = 1.0;
    for (size_t i = 0U; i < len; ++i) {
        acc *= (1.0 - 2.0 / (double)pr[i]);
        cp[i] = acc;
    }
#endif
    return cp;
}

BnormOut calcBnormSymmetric(
    uint64_t n,
    uint64_t M,
    const uint64_t *odd_pr_begin,
    size_t plen,
    const double *cum)
{
    BnormOut out;
    out.bwin    = 0.0;
    out.bnorm   = 0.0;
    out.ipar_sz = calcIparSize(n, M);

    if (out.ipar_sz == 0ULL || plen == 0U || odd_pr_begin == NULL || cum == NULL) {
        return out;
    }

    /* Skip leading 2 if the slice starts at 2 (caller typically passes begin at 3). */
    if (*odd_pr_begin == 2ULL) {
        odd_pr_begin += 1;
        if (plen == 0U) {
            return out;
        }
        plen -= 1U;
    }

    const uint64_t *odd_pr_end = odd_pr_begin + plen;

    for (uint64_t k = 1ULL; k <= M; ++k) {
        bool keep = (((n + k) & 1ULL) == 1ULL);  /* keep only when n+k is odd */
        if (!keep) {
            continue;
        }
        uint64_t t = n + k;
        uint64_t y = (uint64_t)floor(sqrt((double)t));
        double b   = bref_from_y(y, odd_pr_begin, odd_pr_end, cum);
        out.bwin  += 2.0 * b;                    /* ±k both contribute */
    }

    double L = log((double)n);
    double scale = (L * L) / (4*KAPPA * (double)out.ipar_sz);
    out.bnorm = scale * out.bwin;

    return out;
}

