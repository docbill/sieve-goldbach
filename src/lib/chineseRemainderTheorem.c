// chineseRemainderTheorem - calculates related to the Chinese Remainder Theorem
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
#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include <limits.h>
#include "chineseRemainderTheorem.h"

// ---- small log cache up to 99 ----
static inline long double ln_small_upto99(unsigned x){
    if (x < 100u){
        static long double T[100];
        static int init = 0;
        if (!init){
            T[0] = 0.0L;
            for (unsigned i=1;i<100u;++i) {
                T[i] = logl((long double)i);
            }
            init = 1;
        }
        return T[x];
    }
    return logl((long double)x);
}

static inline long double ln_p_adjust(unsigned p, bool single_residue){
    const unsigned k = single_residue ? (p-1u) : (p-2u);
    return ln_small_upto99(k);
}

static inline long double expose_next_log_fast(
    uint64_t w, unsigned p_next, uint64_t q,
    bool single_residue)
{
    long double s = ((long double)(w % q))/(long double)q;
    return s * ln_p_adjust(p_next, single_residue);
}

static const unsigned PRIMES[] = { 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53 };

static const size_t PRIMES_COUNT = sizeof(PRIMES)/sizeof(PRIMES[0]);

static const long double ODD_PRIMORIAL_LD = 16294579238595022365.0L;

/*
 * ============================================================================
 *  EXPERIMENTAL: Unproven CRT-inspired approximation
 *  -------------------------------------------------
 *  WARNING: This is NOT a rigorous implementation of the Chinese Remainder
 *  Theorem (CRT). It is a CRT-inspired, first-order heuristic intended for
 *  exploratory use only. Do NOT rely on it for applications requiring
 *  mathematically proven correctness or tight worst-case guarantees.
 *
 *  Idea (informal):
 *    - Consider only primes p that do NOT divide n (p ∤ n); primes dividing n
 *      are treated as inert for this model.
 *    - For each such p, add either a full contribution log(p − ρ) when the
 *      window “spans” the threshold, or a fractional contribution proportional
 *      to the uncovered fraction of that modulus. (ρ ∈ {1,2} according to the
 *      single-/two-residue setting.)
 *    - The running modulus uses the product of previously considered
 *      non-dividing primes (“partial primorial”), not the full primorial.
 *
 *  Status:
 *    - This is a first-order approximation (a conjectural “order-1 term”).
 *    - Higher-order corrections (e.g., dropping a smaller prime to “squeeze in”
 *      a larger one) are NOT modeled here. Doing so naively would double-count
 *      and would require explicit inclusion–exclusion terms to correct.
 *    - Convergence of the associated series has NOT been proven. Empirical
 *      tests suggest reasonable means, but conservative bounds are required
 *      to account for worst-case deviations.
 *
 *  Practical guidance:
 *    - Treat results as heuristic estimates. Pair them with conservative lower/
 *      upper bounds when making claims.
 *    - For rigorous results, use established sieve/circle-method machinery
 *      or fundamental-lemma-based bounds.
 *
 *  Build-time opt-in:
 *    - Define EXPERIMENTAL_ESPD to enable this path explicitly.
 *
 *  Largest odd primorial that fits in uint64_t (3·5·…·53) is used as a natural
 *  cutoff for 64-bit arithmetic. This choice is purely pragmatic here and does
 *  not confer rigor.
 * ============================================================================
 */
// with divisibility checks
static long double allowed_prime_deficit_internal(uint64_t n, long double w_in, bool single_residue, size_t start_index)
{
    if (w_in < 3.0L) {
        return (w_in < 1.0L) ? 0.0L : 1.0L;
    }

    long double sumlog = 0.0L;

    // If w spans 3·5·…·53, every nondividing prime fully contributes
    if (w_in >= ODD_PRIMORIAL_LD) {
        for (size_t i = 0; i < PRIMES_COUNT; ++i) {
            unsigned p = PRIMES[i];
            if ((n % (uint64_t)p) != 0ULL)
                sumlog += ln_p_adjust(p, single_residue);
        }
        return expl(sumlog);
    }

    const uint64_t w_int = (uint64_t)floorl(w_in);

    // q = product of NONDIVIDING primes (transition primes included)
    uint64_t q = 1ULL;
    // q_committed = product of committed primes (transition primes not included)
    uint64_t q_committed = 1ULL;

    // greedy estimate, we compute the smallest q with the maximum possibe contribution
    size_t i = start_index;
    for (; i < PRIMES_COUNT; ++i, q_committed = q) {
        const unsigned p = PRIMES[i];
        if(p > w_int) {
            return expl(sumlog);
        }
        // ignore primes dividing n
        if ((n % (uint64_t)p) == 0ULL) {
            continue;
        }

        q *= (uint64_t)p;                // commit (transition prime is committed too)

        if (w_int < q) {
            // not spanned → exposure at current q (which already includes p)
            sumlog += expose_next_log_fast(w_int, p, q, single_residue);
            break;
        }
        // fully spanned → full contribution
        sumlog += ln_p_adjust(p, single_residue);
    }
    start_index = i;
    // The greed estimate was good, but only accounted for small primes and a single transition.
    // We'll estimate the contribution of smaller terms with more transitions.
    // exposure tail: extend q by subsequent NONDIVIDING primes; stop after 5 exposures total
    // already exposed the transition prime
    for (int exposed = 1; ++i < PRIMES_COUNT && exposed < 5;) {
        const unsigned p = PRIMES[i];
        if(p > w_int) {
            break;
        }
        if ((n % (uint64_t)p) == 0ULL) {
            continue;  // skip divisors of n
        }
        q *= (uint64_t)p;
        sumlog += expose_next_log_fast(w_int, p, q, single_residue);
        ++exposed;
    }
    return expl(sumlog);
    // We have shifted all the committed primes remainders into the interval q_committed.  
    // However non-committed primes may still contribute in the unused portion of the interval.
    // I am not sure if this is redundant with having the transition primes already accounted for.
    // Maybe we should only account for the transition primes once, and then only account for the non-committed primes.
    // But this is a more conservative estimate on the maximum possible contribution.
    // When I do a formal proof, I should learn if this is overly conservative.
    //return expl(sumlog)+allowed_prime_deficit_internal(n, w_int-q_committed, single_residue, start_index);
}

// with divisibility checks
long double allowed_prime_deficit(uint64_t n, long double w_in, bool single_residue)
{
    return allowed_prime_deficit_internal(n, w_in, single_residue, 0);
}




/*
 * ============================================================================
 *  REFERENCE IMPLEMENTATION: Exact Chinese Remainder Theorem (CRT)
 *  --------------------------------------------------------------------------
 *  This is a standard, proven implementation of the classical CRT:
 *
 *    Given pairwise coprime moduli m1, m2, ..., mk and remainders r1, r2, ...,
 *    rk, find x such that:
 *        x ≡ ri (mod mi)   for all i,
 *    and 0 ≤ x < M where M = m1*m2*...*mk.
 *
 *  Algorithm (constructive proof):
 *    1. Compute M = ∏ mi
 *    2. For each i:
 *         Mi = M / mi
 *         yi = Mi^{-1} (mod mi)
 *    3. Return x = Σ (ri * Mi * yi) mod M
 *
 *  This implementation is mathematically rigorous and exact.
 *  Use it for validation and comparison with the experimental CRT-inspired
 *  heuristic above.
 *
 *  Limitations:
 *    - Requires all moduli to be pairwise coprime.
 *    - Uses 64-bit arithmetic; overflows if M exceeds 2^63−1.
 *      For large moduli, use a big-integer or arbitrary-precision library.
 *
 *  Returns:
 *    x  (0 ≤ x < M) — the unique solution modulo M,
 *    or −1 if no inverse exists (i.e., moduli not coprime or invalid input).
 * ============================================================================
 */

long long extended_gcd(long long a, long long b, long long *x, long long *y) {
    if (a == 0) {
        *x = 0;
        *y = 1;
        return b;
    }
    long long x1, y1;
    long long gcd = extended_gcd(b % a, a, &x1, &y1);
    *x = y1 - (b / a) * x1;
    *y = x1;
    return gcd;
}

long long mod_inverse(long long a, long long m) {
    long long x, y;
    long long g = extended_gcd(a, m, &x, &y);
    if (g != 1) return -1; // no inverse
    x %= m;
    if (x < 0) x += m;
    return x;
}

long long exact_chinese_remainder_theorem(
    const long long *remainders,
    const long long *moduli,
    int count
) {
    if (!remainders || !moduli || count <= 0) return -1;

    long long M = 1;
    for (int i = 0; i < count; i++) {
        if (moduli[i] <= 0) return -1;
         // crude overflow guard
        if (M > LLONG_MAX / moduli[i]) return -1;
        M *= moduli[i];
    }

    long long result = 0;
    for (int i = 0; i < count; i++) {
        long long mi = moduli[i];
        long long ri = remainders[i] % mi;
        if (ri < 0) ri += mi;

        long long Mi = M / mi;
        long long inv = mod_inverse(Mi % mi, mi);
        if (inv == -1) return -1;

        // accumulate safely modulo M
        __int128 term = (__int128)ri * Mi * inv;
        result = (result + (long long)(term % M)) % M;
    }

    if (result < 0) result += M;
    return result;
}
