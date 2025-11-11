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

#include "eulerproductseries.hpp"

#include <cstdio>
#include <limits>

EulerProductSeries::EulerProductSeries() {}

EulerProductSeries::EulerProductSeries(std::uint64_t* const primeArray, const std::uint64_t* const primeArrayEndend) {
    init(primeArray,primeArrayEndend);
}

EulerProductSeries::EulerProductSeries(EulerProductSeries &productSeries) {
    init(productSeries.prime_left,productSeries.prime_end);
}

void EulerProductSeries::init(std::uint64_t* const primeArray, const std::uint64_t* const primeArrayEndend)
{
    prime_left = nullptr;
    prime_end = primeArrayEndend;
    if (primeArray && primeArrayEndend && primeArray < primeArrayEndend) {
        if (primeArray[0] == 3) {
            prime_left = primeArray;
        } else if (primeArray + 1 < primeArrayEndend && primeArray[1] == 3) {
            prime_left = primeArray + 1; // skip leading 2
        }
    }

    if (! prime_left) {
        std::fprintf(stderr, "Invalid prime array.\n");
    }
    reset();
}


void EulerProductSeries::reset() {
    warned_out = false;
    n_left = 1;
    prime_ptr = prime_left;
    if(prime_left == nullptr) {
        y_next = 0; // an invalid value
        n_right = std::numeric_limits<std::uint64_t>::max();
        result = 0;
    }
    else {
        y_next = 3;
        n_right = 8; // 3^2 - 1
        result = 1.0L;
    }
}

void EulerProductSeries::advance_interval() {
    n_left = n_right+1;

    const long double pm1 = static_cast<long double>(y_next - 1);
    const long double factor = (pm1 - 1.0L) / pm1; // 1 - 1/(p-1)

    const long double result_next = result * factor;
    if (result == result_next) {                     // precision plateau
        n_right = std::numeric_limits<std::uint64_t>::max();
        y_next = n_right; // overflow value
        return;
    }
    result = result_next;

    if (++prime_ptr >= prime_end) {
        if (!warned_out) {
            std::fprintf(stderr,
                "EulerProductSeries: need a prime > %" PRIu64 " for next p^2 threshold\n",
                y_next);
            warned_out = true;
        }
        n_right = std::numeric_limits<std::uint64_t>::max();
        y_next = n_right; // overflow value
        return;
    }
    y_next = *prime_ptr;
    if (y_next > std::numeric_limits<std::uint64_t>::max() / y_next) {
        y_next = n_right = std::numeric_limits<std::uint64_t>::max();
    } else {
        n_right = y_next * y_next - 1;
    }
}

