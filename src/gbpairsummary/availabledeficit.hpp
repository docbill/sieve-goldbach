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
 * This class provides a C++ interface to the Chinese Remainder Theorem-inspired
 * approximation for calculating effective small-prime deficits.
 */
class AvailableDeficit {
public:
    /**
     * @brief Constructor
     * 
     * @param _residue How many residues: typically 2 for Goldbach pairs, 1 for simple primes
     * @param _allow_reductions If true, allow reductions in the residues due to divisibility
     * @param _use_short_interval If true, use a short interval for the calculation
     * @param _residue_tail Residue count for the tail of the calculation (typically 2 for Goldbach pairs, 1 for simple primes)
     * @param _allow_tail_reductions If true, allow reductions in the tail of the calculation due to divisibility
     * @param _tenting If true, use tenting correction for extremas
     * @param _exposure_count Maximum number of exposure passes (typically 5)
     * @param _prime_offset A value to offset primes by
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
     * @brief Computes the effective small-prime deficit for an integer n
     * 
     * Computes the "effective small-prime deficit" for an integer n over a window
     * of width w_in, using a CRT-informed model with partial exposures.
     * 
     * @param n Input integer (>= 3 yields meaningful results)
     * @param w_int Window width (integer part)
     * @param positive If true, add to the total; if false, subtract
     * @return long double exp(sum of adjusted logs), i.e., the multiplicative deficit
     */
    long double operator()(uint64_t n, std::uint64_t w_int, bool positive);

private:
    std::uint64_t prime_offset;      ///< A value to offset primes by
    std::uint64_t residue;      ///< Residue count (typically 2 for Goldbach pairs, 1 for simple primes)
    std::uint64_t residue_tail;      ///< Residue count for the tail of the calculation (typically 2 for Goldbach pairs, 1 for simple primes)
    std::uint64_t n_prev = 0ULL; ///< Previous input integer
    std::uint64_t q_committed = 0ULL; ///< Product of committed primes (transition primes not included)
    std::uint64_t q_next = 0ULL; ///< Product of next prime (transition prime)
    long double sumlog = 0.0L; ///< Sum of logs
    bool use_short_interval; ///< Whether to use a short interval for the calculation
    bool allow_reductions; ///< Whether to allow reductions in the residues due to divisibility
    bool allow_tail_reductions; ///< Whether to allow reductions in the tail of the calculation
    bool tenting;          ///< Whether to use tenting correction for extremas
    int exposure_count;    ///< Maximum number of exposure passes
    long double tailfactor = 0.0L; ///< Tail factor

    /**
     * @brief Internal implementation of the deficit calculation
     * 
     * @param n Input integer
     * @param w_int Window size (integer part)
     * @return long double The calculated deficit
     */
    long double allowed_prime_deficit_internal(uint64_t n, uint64_t w_int);
};

#endif /* AVAILABLE_DEFICIT_HPP */
