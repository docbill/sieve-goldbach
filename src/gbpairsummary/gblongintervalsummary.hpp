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
    long double currentBaseline = 0.0L;
    long double c_first = 0.0L;
    long double c_last = 0.0L;
    long double c_firstBaseline = 0.0L;
    long double c_lastBaseline = 0.0L;
    std::uint64_t n_first = 0;
    std::uint64_t n_last = 0;
    std::uint64_t delta_first = 0;
    std::uint64_t delta_last = 0;
    long double hlCorr_first = 1.0L;
    long double hlCorr_last = 1.0L;
    void putMinima(long double c, long double cBaseline, std::uint64_t n, std::uint64_t delta, long double hlCorr=1.0L);
    void putMaxima(long double c, long double cBaseline, std::uint64_t n, std::uint64_t delta, long double hlCorr=1.0L);
    void applyHLCorrStateMin(HLCorrState &state);
    void applyHLCorrStateMax(HLCorrState &state);
private:
    void applyHLCorrFirst(long double hlCorr);
    void applyHLCorrLast(long double hlCorr);
};

class GBLongIntervalSummary {
public:
    bool useHLCorrInst = false;
    long double pairCount = 0.0L;
    long double c_of_n = 0.0L;
    ExtremaValues pairCountMinima;
    ExtremaValues pairCountMaxima;
    ExtremaValues pairCountAlignMaxima;
    ExtremaValues alignMinima;
    ExtremaValues alignMaxima;
    ExtremaValues boundMinima;
    ExtremaValues boundMaxima;
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
    long double currentJitter = 0.0L;
    long double jitterLast = 0.0L;
    long double jitterFirst = 0.0L;

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
        pairCountMaxima.putMaxima(pairCount, 0.0L, n, delta, hlCorrAvg);
        cMinima.putMinima(c_of_n, 0.0L, n, delta, hlCorrAvg);
        cMaxima.putMaxima(c_of_n, 0.0L, n, delta, hlCorrAvg);
        // Conservative bound: use raw values without HLCorr
        if (useHLCorrInst && useHLCorr && hlCorrAvg != 0.0L) {
            pairCountTotal     += pairCount     / hlCorrAvg;
            pairCountTotalNorm += c_of_n / hlCorrAvg;
            hlCorrAvg = 1.0L;
        } else {
            pairCountTotal     += pairCount;
            pairCountTotalNorm += c_of_n;
        }
        if(n == boundMinima.n_last) {
            jitterLast = currentJitter;
        }
        if(n == boundMinima.n_first) {
            jitterFirst = currentJitter;
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
        HLCorrState &alignMinNormState,
        HLCorrState &alignMaxNormState,
        HLCorrState &boundMinNormState,
        HLCorrState &boundMaxNormState
    ) {
        hlCorrAvg = 0.5L*(evenState(n_geom_even,delta_even)+oddState(n_geom_odd,delta_odd));
        pairCountAvg *= hlCorrAvg;
        cAvg *= hlCorrAvg;
        applyHLCorr(minState, maxState, minNormState, maxNormState, alignMinNormState, alignMaxNormState, boundMinNormState, boundMaxNormState);
    }
    
    void applyHLCorr(
        HLCorrState &minState,
        HLCorrState &maxState,
        HLCorrState &minNormState,
        HLCorrState &maxNormState,
        HLCorrState &alignMinNormState,
        HLCorrState &alignMaxNormState,
        HLCorrState &boundMinNormState,
        HLCorrState &boundMaxNormState
    ) {
        // Note: pairCountMinima should NOT call applyHLCorrStateMin because that method
        // has swapping logic designed for alignment calculations, not regular minimum tracking.
        // For pairCountMinima, we just need to apply the HLCorr to the c values without swapping n values.
        // However, when useHLCorrInst is false, the values have already been corrected during aggregation,
        // so we should NOT apply HLCorr correction again here.
        if(! useHLCorrInst) {
            pairCountMinima.applyHLCorrStateMin(minState);
        }
        pairCountMaxima.applyHLCorrStateMax(maxState);
        // Apply alignment-normalized correction when comparing pairCountAlignMaxima.current extrema
        // pairCountAlignMaxima.applyHLCorrStateMax(alignMinNormState);
        cMinima.applyHLCorrStateMin(minNormState);
        cMaxima.applyHLCorrStateMax(maxNormState);
        alignMinima.applyHLCorrStateMin(alignMinNormState);
        alignMaxima.applyHLCorrStateMax(alignMaxNormState);
        boundMinima.applyHLCorrStateMin(boundMinNormState);
        boundMaxima.applyHLCorrStateMax(boundMaxNormState);
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

