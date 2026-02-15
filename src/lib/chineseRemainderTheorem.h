// chineseRemainderTheorem - headers for calculation of the Chinese Remainder Theorem
// Reference-only: retained for comparison, not used by current code paths.
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
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHINESE_REMAINDER_THEOREM_H
#define CHINESE_REMAINDER_THEOREM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----- Visibility / export control (override as needed) -----
#if !defined(CRT_API)
#  if defined(_WIN32) || defined(__CYGWIN__)
#    ifdef CRT_BUILD
#      define CRT_API __declspec(dllexport)
#    else
#      define CRT_API __declspec(dllimport)
#    endif
#  elif defined(__GNUC__) || defined(__clang__)
#    define CRT_API __attribute__((visibility("default")))
#  else
#    define CRT_API
#  endif
#endif

// -----------------------------------------------------------------------------
// allowed_prime_deficit
//
// Computes the "effective small-prime deficit" for an integer n over a window
// of width w_in, using a CRT-informed model with partial exposures. Internally,
// it commits full contributions up to the largest odd primorial that fits in
// uint64_t (3·5·7·…·53) and performs a single fractional-exposure pass if the
// window doesn't span a prime's threshold.
//
// Parameters:
//   n              : input integer (>= 3 yields meaningful results)
//   w_in           : window width (long double); only floor(w_in) is used
//   residue        : how many residues :  typically 2 residues for goldbach pairs, 1 for simple primes
//   positive       : if true, if the remainder is add to the total, false if subtracted
//   exposure_count : maximum number of exposure passes (typically 5)
//   allow_iterations : maximum number of iterations (-1 for unlimited)
//
// Returns:
//   long double exp(sum of adjusted logs), i.e., the multiplicative deficit.
// -----------------------------------------------------------------------------
CRT_API long double
allowed_prime_deficit(uint64_t n, long double w_in, uint64_t residue, bool positive, bool tenting, int exposure_count);

// this is an alternative way to try and calculate exposure terms -- not even heiristically verified.
#ifdef CHINESE_REMAINDER_THEOREM_EXPERIMENTAL
CRT_API long double
allowed_prime_deficit_expirimental(uint64_t n, long double w_in, uint64_t residue, bool positive);
#endif

// -----------------------------------------------------------------------------
// Reference implementation: Exact Chinese Remainder Theorem
//
// These functions provide a standard, proven implementation of the Chinese
// Remainder Theorem for comparison with the experimental method above.
// Use these for applications requiring mathematical rigor and proven correctness.
// -----------------------------------------------------------------------------

// Exact Chinese Remainder Theorem implementation
// Returns the unique solution x mod M where M = product of all moduli
// Returns -1 if no solution exists (moduli not pairwise coprime)
CRT_API long long
exact_chinese_remainder_theorem(const long long *remainders,
                                 const long long *moduli, 
                                 int count);

// Helper function to find modular inverse using extended Euclidean algorithm
CRT_API long long
mod_inverse(long long a, long long m);

// Extended Euclidean algorithm
CRT_API long long
extended_gcd(long long a, long long b, long long *x, long long *y);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* CHINESE_REMAINDER_THEOREM_H */
