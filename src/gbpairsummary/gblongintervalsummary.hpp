// gblongintervalsummary - class for aggregating windowInterval counts
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
#ifndef GB_LONG_INTERVAL_SUMMARY_HPP
#define GB_LONG_INTERVAL_SUMMARY_HPP 1

#include <cstdint>
#include <cmath>
#include <cstdio>    // for std::vfprintf, std::fflush
#include "hlcorrstate.hpp"

// Your C library (declared C linkage)
extern "C" {
#include "libprime.h"
}

class GBLongInterval;

class GBLongIntervalSummary {
public:
    bool useHLCorrInst = false;
    long double pairCount = 0.0L;
    long double c_of_n = 0.0L;
    long double pairCountMin = 0.0L;
    long double pairCountMax = 0.0L;
    long double cMin = 0.0L;
    long double cminus_of_n0 = 0.0L;
    long double cminusAsymp_of_n0 = 0.0L;
    long double cMax = 0.0L;
    long double pairCountTotal = 0.0L;
    long double pairCountTotalNorm = 0.0L;
    long double pairCountAvg = 0.0L;
    long double c_of_n2 = 0.0L;
    long double cminus_of_n2 = 0.0L;
    long double cminusAsymp_of_n2 = 0.0L;
    long double c_of_n3 = 0.0L;
    long double cminus_of_n3 = 0.0L;
    long double cminusAsymp_of_n3 = 0.0L;
    long double cAvg = 0.0L;
    long double hlCorrAvg = 1.0L;

    std::uint64_t minAt = 0;
    std::uint64_t n0 = 0;
    std::uint64_t minAtDelta = 0;
    std::uint64_t delta_of_n0 = 0;
    std::uint64_t maxAt = 0;
    std::uint64_t n1 = 0;
    std::uint64_t maxAtDelta = 0;
    std::uint64_t delta_of_n1 = 0;
    std::uint64_t n2 = 0;
    std::uint64_t n3 = 0;

    void reset() {
        *this = GBLongIntervalSummary();
    }

    void aggregate(
        std::uint64_t n,
        std::uint64_t delta,
        long double cminus,
        long double cminusAsymp,
        bool useHLCorr,
        bool firstMin,
        bool firstDiff,
        bool firstDiffAsymp
    ) {
        if (useHLCorrInst && useHLCorr && hlCorrAvg != 0.0L) {
            pairCountTotal     += pairCount     / hlCorrAvg;
            pairCountTotalNorm += c_of_n / hlCorrAvg;
            hlCorrAvg = 1.0L;
        } else {
            pairCountTotal     += pairCount;
            pairCountTotalNorm += c_of_n;
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
        if (c_of_n > cMax || !n1) {
            cMax = c_of_n;
            delta_of_n1   = delta;
            n1 = n;
        }
        if ((firstMin?(c_of_n < cMin):(c_of_n <= cMin)) || !n0) {
            cMin = c_of_n;
            cminus_of_n0 = cminus;
            cminusAsymp_of_n0 = cminusAsymp;
            delta_of_n0   = delta;
            n0        = n;
        }
        if ((firstDiff?(c_of_n-cminus < c_of_n2-cminus_of_n2 ):(c_of_n-cminus <= c_of_n2-cminus_of_n2 )) || !n2) {
            c_of_n2 = c_of_n;
            cminus_of_n2 = cminus;
            cminusAsymp_of_n2 = cminusAsymp;
            n2 = n;
        }
        if ((firstDiffAsymp?(c_of_n-cminusAsymp < c_of_n3-cminusAsymp_of_n3 ): (c_of_n-cminusAsymp <= c_of_n3-cminusAsymp_of_n3 )) || !n3) {
            c_of_n3 = c_of_n;
            cminus_of_n3 = cminus;
            cminusAsymp_of_n3 = cminusAsymp;
            n3 = n;
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
        cAvg *= hlCorrAvg;
        pairCountMin     *= minState.eval(minAt, minAtDelta);
        pairCountMax     *= maxState.eval(maxAt, maxAtDelta);
        cMin *= minNormState.eval(n0,delta_of_n0);
        cMax *= maxNormState.eval(n1,delta_of_n1);
    }

    void outputCps(
        GBLongInterval &interval,
        long double alpha_n,
        int decade,
        std::uint64_t n_start,
        std::uint64_t preMertens,
        std::uint64_t preMertensAsymp
    );

private:
    void outputCpsLine(
        class GBLongInterval &interval,
        std::uint64_t n,
        long double alpha_n,
        int decade,
        std::uint64_t n_start,
        std::uint64_t preMertens,
        std::uint64_t preMertensAsymp
    );
};

#endif // GB_LONG_INTERVAL_SUMMARY_HPP

