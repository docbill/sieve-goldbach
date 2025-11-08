// availableDeficit - calculates related to the Chinese Remainder Theorem
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

#include <cstdint>
#include <cstddef>
#include <cmath>
#include <climits>

#include "availabledeficit.hpp"

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

static const uint64_t PRIMES[] = {
    3ULL, 5ULL, 7ULL, 11ULL, 13ULL, 17ULL, 19ULL, 23ULL, 29ULL, 31ULL, 37ULL, 41ULL, 43ULL,
    47ULL, 53ULL, 59ULL, 61ULL, 67ULL, 71ULL, 73ULL, 79ULL, 83ULL, 89ULL, 97ULL,
    101ULL, 103ULL, 107ULL, 109ULL, 113ULL, 127ULL, 131ULL, 137ULL, 139ULL, 149ULL, 151ULL,
    157ULL, 163ULL, 167ULL, 173ULL, 179ULL, 181ULL, 191ULL, 193ULL, 197ULL, 199ULL, 211ULL
};

static const size_t PRIMES_COUNT = sizeof(PRIMES)/sizeof(PRIMES[0]);

// Asymmetric remainder stepping, all unsigned
// m = p - r   (with 0 < r < p)

static inline uint64_t cap_tent(uint64_t n, uint64_t p, uint64_t r) {
    uint64_t m = p - r;                 // cap
    uint64_t t = (n + r) % p;           // phase with residue shift
//    uint64_t t = n % p;                  // phase, no residue shift
    uint64_t k = t + 1;                 // 1..p
    return (k < m ? k : m);             // min(t+1, m)
//    return (n % (p-r)) + 1ULL;
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
long double AvailableDeficit::allowed_prime_deficit_internal(std::uint64_t n, std::uint64_t w_int)
{
    // const long double w = (long double)sqrtl((long double)w_int);
    // w_int = (std::uint64_t)floorl(w);

    // q_committed = product of committed primes (transition primes not included)
    // greedy estimate, we compute the smallest q with the maximum possibe contribution
    // Cache is valid when: n == n_prev AND w_int >= q_committed AND w_int < q_next
    // Recalculate when: n changes OR w_int < q_committed (shrunk) OR w_int >= q_next (grew past transition)
    // if(n != n_prev || w_int < q_committed || w_int >= q_next) {
    if(n != n_prev || w_int < q_committed*q_committed || w_int >= q_next*q_next) {
        // we include 3 only when allow_reductions && n%3 == 0, otherwise skip it (start at index 1)
        size_t i = (allow_reductions && n%3ULL == 0ULL) ? 0 : 1;
        // size_t i = 1;//(allow_reductions && n%3ULL == 0ULL) ? 0 : 1;
        const uint64_t p_max = 2ULL*n;
        sumlog = 0.0L;
        n_prev = n;
        q_committed = 1ULL;
        q_next = 1ULL;
        const std::uint64_t pm = PRIMES[PRIMES_COUNT-1]-prime_offset;
        // if(residue > 1ULL && w_int > pm*pm) {
        if(residue > 1ULL && w_int > pm) {
            size_t j = i;
            for (q_next = q_committed; i <  PRIMES_COUNT; ++i, q_committed = q_next) {
                q_next *= (PRIMES[i]-prime_offset);
                // Check if q_next exceeds UINT32_MAX before squaring to avoid overflow
                if ((q_next > UINT32_MAX) || ((q_next*q_next) > w_int)) {
                // if (q_next > w_int) {
                    break;
                }
            }
            // q = product of NONDIVIDING primes (transition primes included)
            // Note: j < i (not j <= i) because i points to the prime AFTER the break
            for (; j < i; ++j) {
                const uint64_t p = PRIMES[j];
                const uint64_t r = (allow_reductions && (n % p) == 0ULL) ? residue-1ULL : residue;
                sumlog += ln_small_upto99(tenting?cap_tent(n, p, r):(p-r));
            }
        }
        else {
            for (q_next = q_committed; i < PRIMES_COUNT; ++i, q_committed = q_next) {
                const uint64_t p = PRIMES[i];
                if((p-prime_offset)*(p-prime_offset) > w_int) {
                // if((p-prime_offset) > w_int) {
                    if(p >= p_max) {
                        // we have exceeded the window, so we return the sum of logs
                        return expl(sumlog);
                    }
                    // Set q_next to the next primorial that would exceed w_int
                    // This is p itself (or the product starting from beginning) if no primes were committed
                    if (q_committed == 1ULL) {
                        q_next = (p-prime_offset);  // Next transition is at the first prime we encounter
                    } else {
                        q_next = q_committed * (p-prime_offset);  // Next transition is current primorial * next prime
                    }
                    break;
                }
                const uint64_t r = (allow_reductions && (n % p) == 0ULL) ? residue-1ULL : residue;
                if (r == 0ULL) {
                    continue;
                }
                q_next *= p-prime_offset;
                // Check if q_next exceeds UINT32_MAX before squaring to avoid overflow
                if (( q_next > UINT32_MAX) || ((q_next*q_next) > w_int)) {
                // if (q_next > w_int) {
                    break;
                }
                sumlog += ln_small_upto99(tenting?cap_tent(n, p, r):(p-r));
            }
        }
        // The greed estimate was good, but only accounted for small primes and a single transition.
        // We'll estimate the contribution of smaller terms with more transitions.
        // exposure tail: extend q by subsequent NONDIVIDING primes; stop after exposure_count exposures
        // or when the next term contribution is < 1e-14 (precision limit for 8 decimal places on ~1e-5 values)
        // already exposed the transition prime
        tailfactor = 0.0L;
        int exposed = 0;
        for (long double q=(long double)q_committed; i < PRIMES_COUNT && exposed < exposure_count;++exposed) {
            const uint64_t p = PRIMES[i++];
            if(p > p_max) {
                break;
            }
            const uint64_t r = (allow_tail_reductions && (n % p) == 0ULL) ? residue_tail-1ULL : residue_tail;
            if (r == 0ULL) {
                continue;
            }
            q *= (long double)(p-prime_offset);
            const long double term = ln_small_upto99(tenting?cap_tent(n, p, r):(p-r)) / (use_short_interval ? q : q*q);
            tailfactor += term;
            if (term < 1e-14L) {
                break;
            }
        }
    }
    //const long double w = (long double)w_int;
    const long double w = use_short_interval ? sqrtl((long double)w_int) : (long double)w_int;
    return expl(sumlog + std::fmaxl(w, 1.0L) * tailfactor);
}

// with divisibility checks
long double AvailableDeficit::operator()(uint64_t n, std::uint64_t w_int, bool positive)
{
    const long double result = allowed_prime_deficit_internal(n, w_int);
    // // limit could be assuming what we are trying to verify, which is for large n, we cannot 
    // // produce enough deficity to violate Goldbach minimums or maximums.
    // long double result = fmaxl(w_in,1.0L);
    // if(w_int) {
    //     result = fmaxl(fminl(allowed_prime_deficit_internal(n, w_int, w_in), result), 1.0L);
    // }
    return (positive) ? result : -result;
}

