// eulerproductseries - class for calculating euler product series
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
#ifndef EULER_PRODUCT_SERIES_HPP
#define EULER_PRODUCT_SERIES_HPP 1
#include <cstdint>
#include <cinttypes>

class EulerProductSeries {
public:
    EulerProductSeries();
    EulerProductSeries(std::uint64_t * const primeArray, const std::uint64_t * const primeArrayEndend);
    EulerProductSeries(EulerProductSeries &product_series);

    void init(std::uint64_t * const primeArray, const std::uint64_t * const primeArrayEndend);

    void reset();

    // Product over odd primes p with p^2 <= n: PROD (p-2)/(p-1)
    long double operator()(std::uint64_t n) {
        if (n < n_left) {
            reset();
        }
        while (n > n_right) {
            advance_interval();
        }
        return result;
    }

private:
    std::uint64_t* prime_left = nullptr; // the position of 3 in the array
    const std::uint64_t * prime_end = nullptr; // last position in the array + 1
    std::uint64_t* prime_ptr   = nullptr; // the current position in the array

    std::uint64_t n_left = 0; // the first value of this interval
    std::uint64_t n_right = 0; // the last value of this interval
    std::uint64_t y_next = 0; // the sqrt of the start of the next interval
    long double result = 0; // the product series result for this interval
    bool warned_out = true; // indicates if we have warned about running out of primes

    void advance_interval();
};

#endif // EULER_PRODUCT_SERIES_HPP

