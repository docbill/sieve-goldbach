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
#include <stdio.h>

// ---- small log cache up to 99 ----
static const long double LN_CACHE[100] = {
    // ln(0-9) // 0 is of course undefined, but we return 0.0L for convenience
    0.0L, 0.0L, 0.6931471805599453094L, 1.0986122886681096914L, 1.3862943611198906188L,
    1.6094379124341003746L, 1.7917594692280550008L, 1.9459101490553133051L, 2.0794415416798359283L, 2.1972245773362193828L,
    
    // ln(10-19)
    2.3025850929940456840L, 2.3978952727983705441L, 2.4849066497880003102L, 2.5649493574615367361L, 2.6390573296152586149L,
    2.7080502011022100660L, 2.7725887222397812377L, 2.8332133440562160802L, 2.8903717578961646922L, 2.9444389791664402350L,
    
    // ln(20-29)
    2.9957322735539909934L, 3.0445224377234229965L, 3.0910424533583158558L, 3.1354942159291496908L, 3.1780538303479456196L,
    3.2188758248682007492L, 3.2580965380214820470L, 3.2958368660043290742L, 3.3322045101752039233L, 3.3672958299864740272L,
    
    // ln(30-39)
    3.4011973816621553754L, 3.4339872044851462458L, 3.4657359027997265471L, 3.4965075614664802355L, 3.5263605246161613897L,
    3.5553480614894136797L, 3.5835189384561100016L, 3.6109179126442244444L, 3.6375861597263858774L, 3.6635616461296464274L,
    
    // ln(40-49)
    3.6888794541139363057L, 3.7135720667043080031L, 3.7376696182833683192L, 3.7612001156935624235L, 3.7841896339182611645L,
    3.8066624897703197574L, 3.8286413964890950000L, 3.8501476017100585868L, 3.8712010109078909291L, 3.8918202981106265870L,
    
    // ln(50-59)
    3.9120230054281460586L, 3.9318256327243257286L, 3.9512437185814274838L, 3.9702919135521218341L, 3.9889840465642745402L,
    4.0073331852324711998L, 4.0253516907351498778L, 4.0430512678345501514L, 4.0604430105464197753L, 4.0775374439057194505L,
    
    // ln(60-69)
    4.0943445622221006848L, 4.1108738641733113906L, 4.1271343850450914162L, 4.1431347263915326874L, 4.1588830833596718576L,
    4.1743872698956378097L, 4.1896547420264252631L, 4.2046926193909660597L, 4.2195077051761071428L, 4.2341065045972593988L,
    
    // ln(70-79)
    4.2484952420493593784L, 4.2626798770413151528L, 4.2766661190160552578L, 4.2904594411483911291L, 4.3040650932041702517L,
    4.3174881135363102755L, 4.3307333402863310698L, 4.3438054218536842113L, 4.3567088266895917179L, 4.3694478524670214952L,
    
    // ln(80-89)
    4.3820266346738811953L, 4.3944491546724387656L, 4.4067192472642533985L, 4.4188406077965983245L, 4.4308167988433133996L,
    4.4426512564903160608L, 4.4543472962535078625L, 4.4659081186545836786L, 4.4773368144782064604L, 4.4886363697321398383L,
    
    // ln(90-99)
    4.4998096703302650515L, 4.5108595065168497878L, 4.5217885770490406270L, 4.5325994931532563985L, 4.5432947822700038803L,
    4.5538768916005408346L, 4.5643481914678361102L, 4.5747109785033828221L, 4.5849674786705722577L, 4.5951198501345897122L
};

static inline long double ln_small_upto99(const uint64_t x) {
    if (x < (uint64_t)(sizeof(LN_CACHE)/sizeof(LN_CACHE[0]))){
        return LN_CACHE[x];
    }
    return logl((long double)x);
}

// if we use integer arithmetic, this will give spikes not observed in
// empirical results.  The reason being is we split an interval size S
// into small w parts.  If those are truncated, we sum of our short 
// intervals will be less than the full interval size.   The correct
// part to do integer math would be a floor on the (p-residue)^s term.
// However, this is faster and more conservative.
static inline long double expose_next_log_fast( const long double w, const uint64_t remainder, long double q) {
    const long double s = fmodl(w, q)/q;
    return s * ln_small_upto99(remainder);
}

static const uint64_t PRIMES[] = { 5ULL, 7ULL, 11ULL, 13ULL, 17ULL, 19ULL, 23ULL, 29ULL, 31ULL, 37ULL, 41ULL, 43ULL, 47ULL, 53ULL };

static const size_t PRIMES_COUNT = sizeof(PRIMES)/sizeof(PRIMES[0]);

static const uint64_t ODD_PRIMORIAL_U64 = 16294579238595022365ULL;

// Asymmetric remainder stepping, all unsigned
// m = p - r   (with 0 < r < p)

static inline uint64_t cap_tent(uint64_t n, uint64_t p, uint64_t r) {
    uint64_t m = p - r;                 // cap
    uint64_t t = (n + r) % p;           // phase with residue shift
    uint64_t k = t + 1;                 // 1..p
    return (k < m ? k : m);             // min(t+1, m)
}

static inline uint64_t min_u64(uint64_t a, uint64_t b) {
    return (a < b) ? a : b;
}
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
static long double allowed_prime_deficit_internal(uint64_t n, uint64_t *w_intptr, long double w, uint64_t residue, bool tenting, size_t *prime_pos_ptr, int exposure_count)
{
    const uint64_t w_int = *w_intptr;

    long double sumlog = 0.0L;
    // If w spans 3·5·…·53, every nondividing prime fully contributes
    if (w_int >= ODD_PRIMORIAL_U64) {
        for (size_t i = 0; i < PRIMES_COUNT; ++i) {
            const uint64_t p = PRIMES[i];
            if ((n % p) != 0ULL)
                sumlog += ln_small_upto99(p-residue);
        }
        return expl(sumlog);
    }


    int exposed = 0;
    // q_committed = product of committed primes (transition primes not included)
    uint64_t q_committed = 1ULL;
    const uint64_t p_max = 2ULL*n;
    // greedy estimate, we compute the smallest q with the maximum possibe contribution
    size_t i = *prime_pos_ptr;
    // q = product of NONDIVIDING primes (transition primes included)
    for (uint64_t q = q_committed; i < PRIMES_COUNT; ++i, q_committed = q) {
        const uint64_t p = PRIMES[i];
        if(p > w_int) {
            if(p >= p_max) {
                // we have exceeded the window, so we return the sum of logs
                // and set the window to 0
                *prime_pos_ptr = PRIMES_COUNT;
                *w_intptr = 0ULL;
                return expl(sumlog);
            }
            break;
        }
        uint64_t r = residue;
        // Ignore primes dividing n when negative. As per CRT this would not contribue, and 
        // we are greedly trying to produce the smallest interval with the largest product.
        // However, we know that these primes will contribute when positive with a reduced residue.
        if ((n % p) == 0ULL) {
            if(r-- <= 1ULL) {
                continue;
            }
        }
#if 0
        q *= p;
        if ( q > w_int) {
            break;
        }
        // fully spanned → full contribution
        sumlog += ln_small_upto99(p - r);
#else 
        q *= p;
        if ( q > w_int) {
            break;
        }
        if(! tenting) {
            // this is more conservative, finding absolute bounds.
            sumlog += ln_small_upto99(p-r);
        }
        else {
            // this is for finding constructive extremas, for tracing maximums
            sumlog += ln_small_upto99(cap_tent(n, p, r));
            //sumlog += ln_small_upto99(min_u64(p-r, (n%p)+1ULL));
        }
//        sumlog += ln_small_upto99(p - r - ((n+r) % (p-r)));
//        long double remainder = (long double)(p-r) - fmodl(w, (long double)(p-r));
//        if(remainder == 0.0L) {
//            continue;
//        }
//        q *= p;
//        if ( q >= w) {
//            break;
//        }
//        sumlog += logl(remainder);
#endif
    }
    *prime_pos_ptr = i;
    // The greed estimate was good, but only accounted for small primes and a single transition.
    // We'll estimate the contribution of smaller terms with more transitions.
    // exposure tail: extend q by subsequent NONDIVIDING primes; stop after 5 exposures total
    // already exposed the transition prime
    for (uint64_t q=q_committed; i < PRIMES_COUNT && exposed < exposure_count;++exposed) {
        const uint64_t p = PRIMES[i++];
        if(p > p_max) {
            break;
        }
        uint64_t r = residue;
        if ((n % p) == 0ULL) {
            if(r-- <= 1ULL) {
                continue;
            }
        }
        if(! tenting) {
            sumlog += expose_next_log_fast(w, (long double)(p-r), (q*=p));
        }
        else {
            sumlog += expose_next_log_fast(w, (long double)(cap_tent(n, p, r)), (q*=p));
            //sumlog += expose_next_log_fast(w, (long double)(min_u64(p-r, (n%p)+1ULL)), (q*=p));
        }
    }
    *w_intptr = w_int - q_committed;
    return expl(sumlog);
}

#ifdef CHINESE_REMAINDER_THEOREM_EXPERIMENTAL
// This is probably not a good idea, but it is here for reference.
long double allowed_prime_deficit_expirimental(uint64_t n, long double w_in, uint64_t residue, bool positive)
{
    uint64_t w_int = (uint64_t)floorl(w_in);
    if(! w_int) {
        // if the window is only a fractional size, we can assume a single random prime can cancel it fully.
        return w_in;
    }
    long double result = 0.0L;
    for(size_t prime_pos = 0;prime_pos < PRIMES_COUNT;) {
        result += allowed_prime_deficit_internal(n, &w_int, w_in, residue, &prime_pos, 0);
        // We have shifted all the committed primes remainders into the interval q_committed. 
        // However non-committed primes may still contribute in the unused portion of the interval.
        // We allow additional iterations to account for this. A negative value means no limit.
        // Looping is not really supported by CRT, but it is worth trying.
    }
    return (positive) ? min_ld(result,w_in) : -min_ld(result,w_in);
}
#endif

// with divisibility checks
long double allowed_prime_deficit(uint64_t n, long double w_in, uint64_t residue, bool positive, bool tenting, int exposure_count)
{
    uint64_t w_int = (uint64_t)floorl(w_in);
    // maximum number of odd values in the window.  This is the maximum possible value.
    long double result = fmaxl(w_in,1.0L);
    if(w_int) {
        size_t prime_pos = 0;
        result = fmaxl(fminl(allowed_prime_deficit_internal(n, &w_int, w_in, residue, tenting, &prime_pos, exposure_count), result), 1.0L);
    }
    return (positive) ? result : -result;
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
