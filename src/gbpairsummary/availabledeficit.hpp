// availableDeficit - C++ wrapper for Chinese Remainder Theorem calculations
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

#ifndef AVAILABLE_DEFICIT_HPP
#define AVAILABLE_DEFICIT_HPP

#include <cstdint>
#include <cstddef>

/**
 * @brief C++ wrapper class for CRT-inspired prime deficit calculations
 * 
 * This class implements the Analytic Remainder Envelope R(δ,n) for calculating
 * effective small-prime deficits using a Chinese Remainder Theorem-inspired
 * approximation.
 * 
 * \section Mathematical Definition
 * 
 * The class computes the analytic remainder envelope R(δ,n) defined as:
 * 
 *   R(δ,n) = exp(base(δ,n) + δ·tail(δ,n))
 * 
 * where:
 * 
 * \subsection Base Term
 * 
 * The base contribution up to the fence prime p* is:
 * 
 *   base(δ,n) = Σ_{s ≤ p ≤ p*} log(p - 2)
 *               + Σ_{s ≤ p ≤ p*, p|n} [log(p - 1) - log(p - 2)]
 * 
 * The starting prime s depends on divisibility of n:
 *   - s = 3 if 3|n
 *   - s = 5 if 3∤n
 * 
 * This bifurcation corrects the systematic bias introduced by the p = 3 term
 * in the Goldbach grid.
 * 
 * \subsection Fence Index
 * 
 * The fence index k*(n,δ) is the largest integer such that:
 * 
 *   (R_{k*}^{(s)})² ≤ δ
 * 
 * where R_k^{(s)} = ∏_{p_i ≤ p_k, p_i ≥ s} (p_i - 1)
 * 
 * The corresponding prime is denoted p* = p_{k*}.
 * 
 * \subsection Tail Term
 * 
 * The tail term (for conservative bounds) is:
 * 
 *   tail(δ,n) = Σ_{i > k*} log(p_i - 2) / (R_i^{(s)})²
 * 
 * This sum converges extremely rapidly and changes only when k* increases.
 * 
 * \subsection Parameter Settings for Bounds Calculation
 * 
 * The conservative bounds calculation (as described in the mathematical
 * definition) uses the following parameter settings:
 * 
 *   - residue = 2 (Goldbach pairs)
 *   - allow_reductions = true (enables divisibility corrections)
 *   - use_short_interval = true (uses sqrt(δ) scaling)
 *   - residue_tail = 2 (for positive bounds) or 1 (for negative bounds)
 *   - allow_tail_reductions = false (tail is residue-independent for bounds)
 *   - tenting = false (no tenting correction for bounds)
 *   - prime_offset = 1 (products use (p-1) terms)
 * 
 * Other parameter settings are used for:
 *   - Predictive align calculations (different residue/tail settings)
 *   - Pointwise calculations (different interval scaling)
 *   - Experimental modes (various configurations for testing)
 * 
 * \subsection Asymptotic Behavior
 * 
 * Asymptotically:
 *   - e^{base(δ,n)} ≍ √δ / (log log δ)²
 *   - e^{δ·tail(δ,n)} = 1 + o(1)
 * 
 * so that R(δ,n) ~ √δ up to slowly varying logarithmic factors.
 * 
 * \subsection Remarks
 * 
 * - When 3|n, products and summations begin at p = 3; otherwise at p = 5.
 * - The fence index k*(n,δ) increases stepwise as δ crosses successive
 *   thresholds (R_k^{(s)})². Between thresholds, R(δ,n) is monotone
 *   increasing in δ.
 * - Each additional small prime divisor p ≤ p* of n multiplies R(δ,n) by
 *   (p-1)/(p-2) ≥ 1, preserving monotonicity with respect to local divisibility.
 */
class AvailableDeficit {
public:
    /**
     * @brief Constructor
     * 
     * Constructs an AvailableDeficit calculator with specified parameters.
     * 
     * \note For conservative bounds (matching the mathematical definition):
     *   - residue = 2, allow_reductions = true, use_short_interval = true
     *   - residue_tail = 2 (positive) or 1 (negative), allow_tail_reductions = false
     *   - tenting = false, prime_offset = 1
     * 
     * @param _residue How many residues: typically 2 for Goldbach pairs, 1 for simple primes.
     *                 For bounds: use 2.
     * @param _allow_reductions If true, allow reductions in residues due to divisibility.
     *                          When n mod p = 0, uses (residue-1) instead of residue.
     *                          For bounds: use true.
     * @param _use_short_interval If true, uses sqrt(δ) scaling in the tail term.
     *                           For bounds: use true.
     * @param _residue_tail Residue count for the tail calculation.
     *                      For conservative bounds: use 2 (positive) or 1 (negative).
     * @param _allow_tail_reductions If true, allows residue reductions in tail terms.
     *                               For conservative bounds: use false (tail is residue-independent).
     * @param _tenting If true, uses tenting correction for extrema calculations.
     *                For conservative bounds: use false.
     * @param _exposure_count Maximum number of exposure passes in tail calculation (typically 5-20).
     * @param _prime_offset Offset applied to primes in products: typically 1 for bounds (giving (p-1) terms).
     *                     For bounds: use 1.
     */
    AvailableDeficit(
        std::uint64_t _residue,
        bool _allow_reductions,
        bool _use_short_interval,
        std::uint64_t _residue_tail,
        bool _allow_tail_reductions,
        bool _tenting,
        int _exposure_count,
        std::uint64_t _prime_offset
    )
    : prime_offset(_prime_offset),
        residue(_residue),
        residue_tail(_residue_tail), 
        use_short_interval(_use_short_interval), 
        allow_reductions(_allow_reductions), 
        allow_tail_reductions(_allow_tail_reductions),
        tenting(_tenting),
        exposure_count(_exposure_count)
    {}

    /**
     * @brief Computes the analytic remainder envelope R(δ,n)
     * 
     * Computes the effective small-prime deficit for an integer n over a window
     * of width w_int (corresponding to δ in the mathematical definition).
     * 
     * The calculation uses cached values when n and w_int are within the
     * valid range (n unchanged and q_committed² ≤ w_int < q_next²).
     * 
     * \note For conservative bounds, this computes:
     *   R(δ,n) = exp(base(δ,n) + δ·tail(δ,n))
     * 
     * where δ corresponds to w_int and the base/tail terms are computed
     * according to the fence index k*(n,δ).
     * 
     * @param n Input integer (>= 3 yields meaningful results)
     * @param w_int Window width parameter δ (integer part). For short intervals,
     *              this represents 2δ in the mathematical notation.
     * @param positive If true, returns +R(δ,n); if false, returns -R(δ,n)
     * @return long double The analytic remainder envelope R(δ,n) (or its negative)
     */
    long double operator()(uint64_t n, std::uint64_t w_int, bool positive);

private:
    std::uint64_t prime_offset;      ///< Offset applied to primes in products (typically 1 for bounds, giving (p-1) terms)
    std::uint64_t residue;            ///< Residue count: 2 for Goldbach pairs, 1 for simple primes. For bounds: use 2.
    std::uint64_t residue_tail;       ///< Residue count for tail terms. For conservative bounds: 2 (positive) or 1 (negative).
    std::uint64_t n_prev = 0ULL;      ///< Previous input integer n (for caching)
    std::uint64_t q_committed = 0ULL; ///< Product R_{k*}^{(s)} of committed primes up to fence index k*
    std::uint64_t q_next = 0ULL;      ///< Next transition point: product that would exceed the fence
    long double sumlog = 0.0L;        ///< Accumulated sum: base(δ,n) = Σ log(p-r) terms
    bool use_short_interval;         ///< If true, uses sqrt(δ) scaling in tail. For bounds: use true.
    bool allow_reductions;            ///< If true, uses (residue-1) when p|n. For bounds: use true.
    bool allow_tail_reductions;       ///< If true, allows residue reductions in tail. For bounds: use false.
    bool tenting;                     ///< If true, applies tenting correction. For bounds: use false.
    int exposure_count;               ///< Maximum number of tail exposure passes (typically 5-20)
    long double tailfactor = 0.0L;   ///< Accumulated tail(δ,n) = Σ log(p-r) / (R_i^{(s)})²

    /**
     * @brief Internal implementation of R(δ,n) calculation
     * 
     * Implements the analytic remainder envelope calculation:
     *   R(δ,n) = exp(base(δ,n) + δ·tail(δ,n))
     * 
     * The calculation:
     * 1. Determines the starting prime s (3 if 3|n, else 5)
     * 2. Finds the fence index k* such that (R_{k*}^{(s)})² ≤ w_int
     * 3. Computes base(δ,n) = Σ log(p-2) + Σ_{p|n} [log(p-1) - log(p-2)]
     * 4. Computes tail(δ,n) = Σ_{i>k*} log(p_i-r) / (R_i^{(s)})²
     * 5. Returns exp(sumlog + w·tailfactor) where w = sqrt(w_int) for short intervals
     * 
     * Uses caching to avoid recomputation when n and w_int are within valid ranges.
     * 
     * @param n Input integer
     * @param w_int Window parameter δ (integer part). For short intervals, represents 2δ.
     * @return long double The analytic remainder envelope R(δ,n)
     */
    long double allowed_prime_deficit_internal(uint64_t n, uint64_t w_int);
};

#endif /* AVAILABLE_DEFICIT_HPP */
