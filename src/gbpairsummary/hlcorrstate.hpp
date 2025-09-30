// hlcorrcalc - for calculating the Hardy-Littlewood circle correction
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
#ifndef HLCORR_STATE_HPP
#define HLCORR_STATE_HPP 1

#include <cstdint>
#include <cmath>

// Your C library (declared C linkage)
extern "C" {
#include "libprime.h"
}

class HLCorrState {
private:
    long double invlogNlogN = 0.0L;
    long double invSum = 0.0L;
    long double sum = 0.0L;
    std::uint64_t n_prev=0;
    std::uint64_t delta_prev = 0;
    std::uint64_t m = 0;

public:
    void reset(std::uint64_t n) {
        n_prev = n;
        const long double logN = logl((long double)n);
        invlogNlogN = 1.0L / (logN * logN);
        invSum = 0.0L;
        sum = 0.0L;
        m = 1 + (n & 1ULL);
    }

    long double eval( std::uint64_t n, std::uint64_t delta) {
        if(n_prev != n || delta < delta_prev) {
            reset(n);
        }
        delta_prev = delta;
        for (; m <= delta; m += 2ULL) {
            sum    += 1.0L / (logl((long double)(n - m)) * logl((long double)(n + m)));
            invSum += invlogNlogN;
        }
        return (invSum > 0.0L) ? (sum / invSum) : 1.0L;
    }
};

static inline long double hlCorr(std::uint64_t n, std::uint64_t delta) {
    return HLCorrState().eval(n,delta);
}

#endif // HLCORR_STATE_HPP

