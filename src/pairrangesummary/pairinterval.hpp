// pairinterval - class for aggregating interval counts
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
#ifndef PAIR_INTERVAL_HPP
#define PAIR_INTERVAL_HPP 1

#include <cstdint>
#include <cmath>
#include "hlcorrstate.hpp"

// Your C library (declared C linkage)
extern "C" {
#include "libprime.h"
}

class PairInterval {
public:
    int useHLCorrInst = 0;
    long double pairCount = 0.0L;
    long double pairCountNorm = 0.0L;
    long double pairCountMin = 0.0L;
    long double pairCountMax = 0.0L;
    long double pairCountMinNorm = 0.0L;
    long double pairCountMaxNorm = 0.0L;
    long double pairCountTotal = 0.0L;
    long double pairCountTotalNorm = 0.0L;
    long double pairCountAvg = 0.0L;
    long double pairCountAvgNorm = 0.0L;
    long double hlCorrAvg = 1.0L;

    std::uint64_t minAt = 0;
    std::uint64_t minNormAt = 0;
    std::uint64_t minAtDelta = 0;
    std::uint64_t minNormAtDelta = 0;
    std::uint64_t maxAt = 0;
    std::uint64_t maxNormAt = 0;
    std::uint64_t maxAtDelta = 0;
    std::uint64_t maxNormAtDelta = 0;

    void reset() {
        *this = PairInterval();
    }

    void aggregate(
        std::uint64_t n,
        std::uint64_t delta,
        int useHLCorr,
        int firstMin
    ) {
        if (useHLCorrInst && useHLCorr && hlCorrAvg != 0.0L) {
            pairCountTotal     += pairCount     / hlCorrAvg;
            pairCountTotalNorm += pairCountNorm / hlCorrAvg;
            hlCorrAvg = 1.0L;
        } else {
            pairCountTotal     += pairCount;
            pairCountTotalNorm += pairCountNorm;
        }

        if (pairCount > pairCountMax || !maxAt) {
            pairCountMax = pairCount;
            maxAtDelta   = delta;
            maxAt        = n;
        }
        if ((firstMin?(pairCount < pairCountMin):(pairCount <= pairCountMin)) || !minAt) {
            pairCountMin = pairCount;
            minAtDelta   = delta;
            minAt        = n;
        }
        if (pairCountNorm > pairCountMaxNorm || !maxNormAt) {
            pairCountMaxNorm = pairCountNorm;
            maxNormAtDelta   = delta;
            maxNormAt        = n;
        }
        if ((firstMin?(pairCountNorm < pairCountMinNorm):(pairCountNorm <= pairCountMinNorm)) || !minNormAt) {
            pairCountMinNorm = pairCountNorm;
            minNormAtDelta   = delta;
            minNormAt        = n;
        }
    }

    void applyHLCorr(
        std::uint64_t n_geom_even,
        std::uint64_t delta_even,
        std::uint64_t n_geom_odd,
        std::uint64_t delta_odd,
        HLCorrState &evenState,
        HLCorrState &oddState,
        HLCorrState &minState,
        HLCorrState &maxState,
        HLCorrState &minNormState,
        HLCorrState &maxNormState
    ) {
        hlCorrAvg = 0.5L*(evenState.eval(n_geom_even,delta_even)+oddState.eval(n_geom_odd,delta_odd));
        pairCountAvg     *= hlCorrAvg;
        pairCountAvgNorm *= hlCorrAvg;
        pairCountMin     *= minState.eval(minAt, minAtDelta);
        pairCountMax     *= maxState.eval(maxAt, maxAtDelta);
        pairCountMinNorm *= minNormState.eval(minNormAt,minNormAtDelta);
        pairCountMaxNorm *= maxNormState.eval(maxNormAt,maxNormAtDelta);
    }
};

#endif // PAIR_INTERVAL_HPP

