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
    long double pairCountMinFirst = 0.0L;
    long double pairCountMinLast = 0.0L;
    long double pairCountMaxFirst = 0.0L;
    long double pairCountMaxLast = 0.0L;
    long double cMinFirst = 0.0L;
    long double cMinLast = 0.0L;
    long double cminus_of_n0First = 0.0L;
    long double cminusAsymp_of_n0First = 0.0L;
    long double cminus_of_n0Last = 0.0L;
    long double cminusAsymp_of_n0Last = 0.0L;
    long double cMaxFirst = 0.0L;
    long double cMaxLast = 0.0L;
    long double pairCountTotal = 0.0L;
    long double pairCountTotalNorm = 0.0L;
    long double pairCountAvg = 0.0L;
    long double c_of_n2First = 0.0L;
    long double c_of_n2Last = 0.0L;
    long double cminus_of_n2First = 0.0L;
    long double cminus_of_n2Last = 0.0L;
    long double cminusAsymp_of_n2First = 0.0L;
    long double cminusAsymp_of_n2Last = 0.0L;
    long double c_of_n3First = 0.0L;
    long double c_of_n3Last = 0.0L;
    long double cminus_of_n3First = 0.0L;
    long double cminus_of_n3Last = 0.0L;
    long double cminusAsymp_of_n3First = 0.0L;
    long double cminusAsymp_of_n3Last = 0.0L;
    long double cAvg = 0.0L;
    long double hlCorrAvg = 1.0L;

    std::uint64_t minAtFirst = 0;
    std::uint64_t minAtLast = 0;
    std::uint64_t n0First = 0;
    std::uint64_t n0Last = 0;
    std::uint64_t minAtDeltaFirst = 0;
    std::uint64_t minAtDeltaLast = 0;
    std::uint64_t delta_of_n0First = 0;
    std::uint64_t delta_of_n0Last = 0;
    std::uint64_t maxAtFirst = 0;
    std::uint64_t maxAtLast = 0;
    std::uint64_t n1First = 0;
    std::uint64_t n1Last = 0;
    std::uint64_t maxAtDeltaFirst = 0;
    std::uint64_t maxAtDeltaLast = 0;
    std::uint64_t delta_of_n1First = 0;
    std::uint64_t delta_of_n1Last = 0;
    std::uint64_t n2First = 0;
    std::uint64_t n2Last = 0;
    std::uint64_t n3First = 0;
    std::uint64_t n3Last = 0;

    void reset() {
        *this = GBLongIntervalSummary();
    }

    void aggregate(
        std::uint64_t n,
        std::uint64_t delta,
        long double cminus,
        long double cminusAsymp,
        bool useHLCorr
    ) {
        if (useHLCorrInst && useHLCorr && hlCorrAvg != 0.0L) {
            pairCountTotal     += pairCount     / hlCorrAvg;
            pairCountTotalNorm += c_of_n / hlCorrAvg;
            hlCorrAvg = 1.0L;
        } else {
            pairCountTotal     += pairCount;
            pairCountTotalNorm += c_of_n;
        }

        if (pairCount >= pairCountMaxLast || !maxAtLast) {
            if (pairCount > pairCountMaxFirst || !maxAtFirst) {
                pairCountMaxFirst = pairCount;
                maxAtDeltaFirst   = delta;
                maxAtFirst        = n;
            }
            pairCountMaxLast = pairCount;
            maxAtDeltaLast   = delta;
            maxAtLast        = n;
        }
        if (pairCount <= pairCountMinLast || !minAtLast) {
            if (pairCount < pairCountMinFirst || !minAtFirst) {
                pairCountMinFirst = pairCount;
                minAtDeltaFirst   = delta;
                minAtFirst        = n;
            }
            pairCountMinLast = pairCount;
            minAtDeltaLast   = delta;
            minAtLast        = n;
        }
        if (c_of_n >= cMaxLast || !n1Last) {
            if (c_of_n > cMaxFirst || !n1First) {
                cMaxFirst = c_of_n;
                delta_of_n1First = delta;
                n1First = n;
            }
            cMaxLast = c_of_n;
            delta_of_n1Last = delta;
            n1Last = n;
        }
        if (c_of_n <= cMinLast || !n0Last) {
            if(c_of_n < cMinFirst || !n0First) {
                cMinFirst = c_of_n;
                n0First = n;
                delta_of_n0First = delta;
                cminus_of_n0First = cminus;
                cminusAsymp_of_n0First = cminusAsymp;
            }
            cMinLast = c_of_n;
            cminus_of_n0Last = cminus;
            cminusAsymp_of_n0Last = cminusAsymp;
            delta_of_n0Last   = delta;
            n0Last        = n;
        }
        if (c_of_n-cminus <= c_of_n2Last-cminus_of_n2Last || !n2Last) {
            if (c_of_n-cminus < c_of_n2First-cminus_of_n2First || !n2First) {
                c_of_n2First = c_of_n;
                cminus_of_n2First = cminus;
                cminusAsymp_of_n2First = cminusAsymp;
                n2First = n;
            }
            c_of_n2Last = c_of_n;
            cminus_of_n2Last = cminus;
            cminusAsymp_of_n2Last = cminusAsymp;
            n2Last = n;
        }
        if (c_of_n-cminusAsymp <= c_of_n3Last-cminusAsymp_of_n3Last  || !n3Last) {
            if (c_of_n-cminusAsymp < c_of_n3First-cminusAsymp_of_n3First  || !n3First) {
                c_of_n3First = c_of_n;
                cminus_of_n3First = cminus;
                cminusAsymp_of_n3First = cminusAsymp;
                n3First = n;
            }
            c_of_n3Last = c_of_n;
            cminus_of_n3Last = cminus;
            cminusAsymp_of_n3Last = cminusAsymp;
            n3Last = n;
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
        pairCountMinFirst *= minState.eval(minAtFirst, minAtDeltaFirst);
        pairCountMinLast *= minState.eval(minAtLast, minAtDeltaLast);
        pairCountMaxFirst *= maxState.eval(maxAtFirst, maxAtDeltaFirst);
        pairCountMaxLast *= maxState.eval(maxAtLast, maxAtDeltaLast);
        cMinFirst *= minNormState.eval(n0First,delta_of_n0First);
        cMinLast *= minNormState.eval(n0Last,delta_of_n0Last);
        cMaxFirst *= maxNormState.eval(n1First,delta_of_n1First); 
        cMaxLast *= maxNormState.eval(n1Last,delta_of_n1Last); 
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

