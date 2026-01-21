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
#include <cfloat>
#include <cstdio>    // for std::vfprintf, std::fflush
#include "hlcorrstate.hpp"
#include "hlcorrinterp.hpp"

// Bound status classification
enum class BoundStatus {
    EXACT,      // measured == predicted (ratio == 1.0)
    EXPECTED,   // measured within expected range (ratio < 1.0 for max, ratio > 1.0 for min)
    VIOLATED,   // measured violates the bound (ratio > 1.0 for max, ratio < 1.0 for min)
    INVALID     // technical issues: sentinel values, division by zero, bad data
};

// Helper function to convert BoundStatus to string for output
inline const char* boundStatusToString(BoundStatus status) {
    switch (status) {
        case BoundStatus::EXACT:    return "EXACT";
        case BoundStatus::EXPECTED: return "EXPECTED";
        case BoundStatus::VIOLATED: return "VIOLATED";
        case BoundStatus::INVALID:  return "INVALID";
        default:                    return "UNKNOWN";
    }
}

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
    long double extra_first = 0.0L;
    long double extra_last = 0.0L;
    void putMinima(long double c, long double cBaseline, std::uint64_t n, std::uint64_t delta, long double hlCorr=1.0L);
    void putMinimaRatio(long double c_meas, long double c, long double cBaseline, std::uint64_t n, std::uint64_t delta, long double hlCorr=1.0L);
    void putMaxima(long double c, long double cBaseline, std::uint64_t n, std::uint64_t delta, long double hlCorr=1.0L);
    void putMaximaRatio(long double c_meas, long double c, long double cBaseline, std::uint64_t n, std::uint64_t delta, long double hlCorr=1.0L);
    void applyHLCorrStateMin(HLCorrState &state);
    void applyHLCorrStateMax(HLCorrState &state);
    void applyHLCorrStateMinRatio(HLCorrState &state);  // Compare ratios instead of raw values
    void applyHLCorrStateMaxRatio(HLCorrState &state);  // Compare ratios instead of raw values
    
    // Calculate ratio: extra / c (with zero-handling)
    // Returns ratio of extra_first/c_first or extra_last/c_last
    // Uses tolerance-based equality check to handle floating-point round-off errors
    long double getFirstRatio() const;
    long double getLastRatio() const;
    
    // Get lambda value (log of ratio) for this extrema
    // Note: first and last should have the same ratio (they may have different n values or raw c values)
    // Lambda uses the sign from the log function: lambda = logl(ratio)
    // This allows plotting to use the sign to determine color of the plot point
    // LDBL_MAX means prediction was 0 and measured was not 0 (valid for minimum, violated for maximum)
    // -LDBL_MAX or negative ratios: treated same as LDBL_MAX for now
    // ratio = 0 means measured was 0 and prediction was not 0
    inline long double getLambda() const {
        long double ratio = getFirstRatio();
        if (ratio == LDBL_MAX || ratio == -LDBL_MAX || ratio < 0.0L) {
            // Prediction = 0 and measured > 0, or negative ratio: treat as LDBL_MAX
            // This represents measured >> predicted
            return LDBL_MAX;  // Positive sentinel to indicate infinity
        }
        if (ratio == 0.0L) {
            // Measured = 0, prediction > 0: log(0) = -infinity
            return -LDBL_MAX;  // Negative sentinel
        }
        // ratio > 0: lambda = logl(ratio) (sign comes from log function)
        return std::logl(ratio);
    }
    
    // Get bound status for maximum bound (measured should be <= predicted)
    // Note: first and last should have the same ratio (they may have different n values or raw c values)
    // INVALID: technical issues (bad data - none currently)
    // VIOLATED: ratio > 1.0 or LDBL_MAX or negative (measured > predicted, bound is violated)
    // EXACT: ratio == 1.0
    // EXPECTED: ratio < 1.0 (measured < predicted, within bounds)
    inline BoundStatus getMaxBoundStatus() const {
        long double ratio = getFirstRatio();
        // LDBL_MAX, -LDBL_MAX, or negative ratios: treated same as LDBL_MAX (prediction = 0, measured > 0)
        // This violates the maximum bound
        if (ratio == LDBL_MAX || ratio == -LDBL_MAX || ratio < 0.0L) {
            return BoundStatus::VIOLATED;  // measured > 0 when predicted = 0, bound violated
        }
        // Handle c_meas = 0 case: ratio = 0, which is < 1.0, so EXPECTED (not a violation)
        if (ratio == 0.0L) {
            return BoundStatus::EXPECTED;  // measured = 0 < predicted, within bounds
        }
        if (ratio > 1.0L) {
            return BoundStatus::VIOLATED;  // measured > predicted, bound violated
        }
        if (ratio == 1.0L) {
            return BoundStatus::EXACT;
        }
        // 0 < ratio < 1.0L
        return BoundStatus::EXPECTED;
    }
    
    // Get bound status for minimum bound (measured should be >= predicted)
    // Note: first and last should have the same ratio (they may have different n values or raw c values)
    // INVALID: technical issues (bad data - none currently)
    // VIOLATED: ratio < 1.0 (measured < predicted, bound is violated)
    // EXACT: ratio == 1.0
    // EXPECTED: ratio > 1.0 or LDBL_MAX or negative (measured > predicted, within bounds)
    inline BoundStatus getMinBoundStatus() const {
        const long double abs_epsilon = 1e-8L;
        // For minimum bound: if prediction is <= epsilon (near zero or negative), 
        // any measured value satisfies the bound (measured >= predicted), so EXPECTED
        if (c_first <= abs_epsilon) {
            return BoundStatus::EXPECTED;
        }
        long double ratio = getFirstRatio();
        // LDBL_MAX, -LDBL_MAX, or negative ratios: treated same as LDBL_MAX (prediction = 0, measured > 0)
        // This is EXPECTED for minimum bound (measured > predicted, so we're above the minimum)
        if (ratio == LDBL_MAX || ratio == -LDBL_MAX || ratio < 0.0L) {
            return BoundStatus::EXPECTED;  // measured > 0 when predicted = 0, within bounds
        }
        // Handle c_meas = 0 case: prediction is positive (since we already handled <= epsilon above)
        // If measured is 0 and prediction is positive, then 0 < positive, so VIOLATED
        if (std::abs(ratio) <= abs_epsilon) {
            // ratio is effectively 0, meaning measured is effectively 0, and prediction is positive
            return BoundStatus::VIOLATED;  // measured (0) < predicted (positive), bound violated
        }
        if (ratio < 1.0L) {
            return BoundStatus::VIOLATED;  // measured < predicted, bound violated
        }
        if (ratio == 1.0L) {
            return BoundStatus::EXACT;
        }
        // ratio > 1.0L
        return BoundStatus::EXPECTED;
    }
private:
    void applyHLCorrFirst(long double hlCorr);
    void applyHLCorrLast(long double hlCorr);
    void copyFirstToLast();  // Copy all first values to last (including extra_first to extra_last)
    void copyLastToFirst();  // Copy all last values to first (including extra_last to extra_first)
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
    ExtremaValues boundRatioMinima;
    ExtremaValues boundRatioMaxima;
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
    long double &jitterLast = alignMinima.extra_last;
    long double &jitterFirst = alignMinima.extra_first;
    long double &boundRatioMinima_c_last = boundRatioMinima.extra_last;
    long double &boundRatioMinima_c_first = boundRatioMinima.extra_first;
    long double &boundRatioMaxima_c_last = boundRatioMaxima.extra_last;
    long double &boundRatioMaxima_c_first = boundRatioMaxima.extra_first;

    std::uint64_t n2First = 0;
    std::uint64_t n2Last = 0;
    std::uint64_t n3First = 0;
    std::uint64_t n3Last = 0;
    HLCorrInterpolator hlCorrEstimate;  // When HLCORR_USE_EXACT is set, this is HLCorrState

    void reset() {
        GBLongIntervalSummary temp;
        // Copy all non-reference members manually since references can't be reassigned
        useHLCorrInst = temp.useHLCorrInst;
        pairCount = temp.pairCount;
        c_of_n = temp.c_of_n;
        pairCountMinima = temp.pairCountMinima;
        pairCountMaxima = temp.pairCountMaxima;
        pairCountAlignMaxima = temp.pairCountAlignMaxima;
        alignMinima = temp.alignMinima;
        alignMaxima = temp.alignMaxima;
        boundMinima = temp.boundMinima;
        boundMaxima = temp.boundMaxima;
        boundRatioMinima = temp.boundRatioMinima;
        boundRatioMaxima = temp.boundRatioMaxima;
        cMinima = temp.cMinima;
        cMaxima = temp.cMaxima;
        cminus_of_n0First = temp.cminus_of_n0First;
        cminusAsymp_of_n0First = temp.cminusAsymp_of_n0First;
        cminus_of_n0Last = temp.cminus_of_n0Last;
        cminusAsymp_of_n0Last = temp.cminusAsymp_of_n0Last;
        pairCountTotal = temp.pairCountTotal;
        pairCountTotalNorm = temp.pairCountTotalNorm;
        pairCountAvg = temp.pairCountAvg;
        c_of_n2First = temp.c_of_n2First;
        c_of_n2Last = temp.c_of_n2Last;
        cminus_of_n2First = temp.cminus_of_n2First;
        cminus_of_n2Last = temp.cminus_of_n2Last;
        cminusAsymp_of_n2First = temp.cminusAsymp_of_n2First;
        cminusAsymp_of_n2Last = temp.cminusAsymp_of_n2Last;
        c_of_n3First = temp.c_of_n3First;
        c_of_n3Last = temp.c_of_n3Last;
        cminus_of_n3First = temp.cminus_of_n3First;
        cminus_of_n3Last = temp.cminus_of_n3Last;
        cminusAsymp_of_n3First = temp.cminusAsymp_of_n3First;
        cminusAsymp_of_n3Last = temp.cminusAsymp_of_n3Last;
        cAvg = temp.cAvg;
        hlCorrAvg = temp.hlCorrAvg;
        currentJitter = temp.currentJitter;
        // References are automatically updated when extrema are reset
        n2First = temp.n2First;
        n2Last = temp.n2Last;
        n3First = temp.n3First;
        n3Last = temp.n3Last;
        hlCorrEstimate = temp.hlCorrEstimate;
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
        if(n == alignMinima.n_last) {
            jitterLast = currentJitter;
        }
        if(n == alignMinima.n_first) {
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
        HLCorrState &boundMaxNormState,
        HLCorrState &boundDeltaMinNormState,
        HLCorrState &boundDeltaMaxNormState
    ) {
        hlCorrAvg = 0.5L*(evenState(n_geom_even,delta_even)+oddState(n_geom_odd,delta_odd));
        pairCountAvg *= hlCorrAvg;
        cAvg *= hlCorrAvg;
        applyHLCorr(minState, maxState, minNormState, maxNormState, alignMinNormState, alignMaxNormState, boundMinNormState, boundMaxNormState, boundDeltaMinNormState, boundDeltaMaxNormState);
    }
    
    void applyHLCorr(
        HLCorrState &minState,
        HLCorrState &maxState,
        HLCorrState &minNormState,
        HLCorrState &maxNormState,
        HLCorrState &alignMinNormState,
        HLCorrState &alignMaxNormState,
        HLCorrState &boundMinNormState,
        HLCorrState &boundMaxNormState,
        HLCorrState &boundDeltaMinNormState,
        HLCorrState &boundDeltaMaxNormState
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
        boundRatioMinima.applyHLCorrStateMinRatio(boundDeltaMinNormState);
        boundRatioMaxima.applyHLCorrStateMaxRatio(boundDeltaMaxNormState);
    }

    void outputCps(
        GBLongInterval &interval,
        long double alpha_n,
        int decade,
        std::uint64_t n_start,
        std::uint64_t preMertens,
        std::uint64_t preMertensAsymp
    );

    // v0.2.0: Output bound ratio data
    void outputBoundRatioMin(GBLongInterval &interval);
    void outputBoundRatioMax(GBLongInterval &interval);

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

