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
#include "hlcorrinterp.hpp"

// Your C library (declared C linkage)
extern "C" {
#include "libprime.h"
}

class GBLongInterval;

// Helper class to group first/last extrema values
class ExtremaValues {
public:
    long double current = 0.0L;
    long double c_first = 0.0L;
    long double c_last = 0.0L;
    std::uint64_t n_first = 0;
    std::uint64_t n_last = 0;
    std::uint64_t delta_first = 0;
    std::uint64_t delta_last = 0;
    long double hlCorr_first = 1.0L;
    long double hlCorr_last = 1.0L;
    void putMinima(long double c, std::uint64_t n, std::uint64_t delta, long double hlCorr=1.0L);
    void putMaxima(long double c, std::uint64_t n, std::uint64_t delta, long double hlCorr=1.0L);
    void applyHLCorrStateMin(HLCorrState &state,long double c_firstBaseline=0.0L,long double c_lastBaseline=0.0L);
    void applyHLCorrStateMax(HLCorrState &state,long double c_firstBaseline=0.0L,long double c_lastBaseline=0.0L);
private:
    void applyHLCorrFirst(long double hlCorr,long double c_firstBaseline=0.0L);
    void applyHLCorrLast(long double hlCorr,long double c_lastBaseline=0.0L);
};

class GBLongIntervalSummary {
public:
    bool useHLCorrInst = false;
    long double pairCount = 0.0L;
    long double c_of_n = 0.0L;
    long double c_alignFirst = 0.0L;
    long double c_alignLast = 0.0L;
    ExtremaValues pairCountMinima;
    ExtremaValues pairCountMaxima;
    ExtremaValues pairCountAlignMaxima;
    ExtremaValues alignMinima;
    ExtremaValues alignNoHLCorrMinima;
    ExtremaValues cMinima;
    ExtremaValues cMaxima;
    long double cminus_of_n0First = 0.0L;
    long double cminusAsymp_of_n0First = 0.0L;
    long double cminus_of_n0Last = 0.0L;
    long double cminusAsymp_of_n0Last = 0.0L;
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

    std::uint64_t n2First = 0;
    std::uint64_t n2Last = 0;
    std::uint64_t n3First = 0;
    std::uint64_t n3Last = 0;
    HLCorrInterpolator hlCorrEstimate;

    void reset() {
        GBLongIntervalSummary temp;
        *this = temp;
    }

    void aggregate(
        std::uint64_t n,
        std::uint64_t delta,
        long double cminus,
        long double cminusAsymp,
        bool useHLCorr
    ) {
        // Store the current hlCorrAvg value that's embedded in the data
        pairCountMinima.putMinima(pairCountMinima.current, n, delta);
        pairCountMaxima.putMaxima(pairCount, n, delta, hlCorrAvg);
        pairCountAlignMaxima.putMaxima(pairCountAlignMaxima.current, n, delta, hlCorrAvg);
        cMinima.putMinima(c_of_n, n, delta, hlCorrAvg);
        cMaxima.putMaxima(c_of_n, n, delta, hlCorrAvg);
        alignMinima.putMinima(alignMinima.current, n, delta, hlCorrAvg);
        // Conservative bound: use raw values without HLCorr
        alignNoHLCorrMinima.putMinima(alignNoHLCorrMinima.current, n, delta);
        if(alignMinima.c_first == n) {
            c_alignFirst = c_of_n - alignMinima.c_first;
        }
        if(alignMinima.c_last == n) {
            c_alignLast = c_of_n - alignMinima.c_last;
        }
        if (useHLCorrInst && useHLCorr && hlCorrAvg != 0.0L) {
            pairCountTotal     += pairCount     / hlCorrAvg;
            pairCountTotalNorm += c_of_n / hlCorrAvg;
            hlCorrAvg = 1.0L;
        } else {
            pairCountTotal     += pairCount;
            pairCountTotalNorm += c_of_n;
        }

        if(n == cMinima.n_last) {
            if(n == cMinima.n_first) {
                cminus_of_n0First = cminus;
                cminusAsymp_of_n0First = cminusAsymp;
            }
            cminus_of_n0Last = cminus;
            cminusAsymp_of_n0Last = cminusAsymp;
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
        HLCorrState &maxNormState,
        HLCorrState &alignNormState
    ) {
        hlCorrAvg = 0.5L*(evenState(n_geom_even,delta_even)+oddState(n_geom_odd,delta_odd));
        pairCountAvg *= hlCorrAvg;
        cAvg *= hlCorrAvg;
        applyHLCorr(minState, maxState, minNormState, maxNormState, alignNormState);
    }
    
    void applyHLCorr(
        HLCorrState &minState,
        HLCorrState &maxState,
        HLCorrState &minNormState,
        HLCorrState &maxNormState,
        HLCorrState &alignNormState
    ) {
        if(! useHLCorrInst) {
            pairCountMinima.applyHLCorrStateMin(minState);
        }
        pairCountMaxima.applyHLCorrStateMax(maxState);
        // Apply alignment-normalized correction when comparing pairCountAlignMaxima.current extrema
        // pairCountAlignMaxima.applyHLCorrStateMax(alignNormState);
        cMinima.applyHLCorrStateMin(minNormState);
        cMaxima.applyHLCorrStateMax(maxNormState);
        alignMinima.applyHLCorrStateMin(alignNormState, c_alignFirst, c_alignLast);
        // Conservative bound: do NOT apply HLCorr (already using raw values)
        // alignNoHLCorrMinima.applyHLCorrStateMin(alignNormState, c_alignFirst, c_alignLast);
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

