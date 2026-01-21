// gblongintervalsummary - class for aggregating windows
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

#include <cstdio>
#include <cinttypes>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <string>

#include "gblonginterval.hpp"

static inline uint64_t aToB(uint64_t a, uint64_t &b) {
    uint64_t retval=0;
    if(a > b) {
        retval = b;
        b = a;
    }
    return retval;
}

void GBLongIntervalSummary::outputCps(
    GBLongInterval &interval,
    long double alpha_n,
    int decade,
    std::uint64_t n_start,
    std::uint64_t preMertens,
    std::uint64_t preMertensAsymp
) {
    if(! interval.cps) {
        return;
    }
    // trivial sorting of 6 values
    // substituting a 0 for repeats, so a bubble sort is sufficient to detect repeats
    uint64_t a = cMinima.n_first, b = cMinima.n_last, c = n2First, d = n2Last, e = n3First, f = n3Last;
    // bubble sort first pass
    if(a >= b) a = aToB(a,b);
    if(b >= c) b = aToB(b,c);
    if(c >= d) c = aToB(c,d);
    if(d >= e) d = aToB(d,e);
    if(e >= f) e = aToB(e,f);

    // bubble sort second pass
    if(a >= b) a = aToB(a,b);
    if(b >= c) b = aToB(b,c);
    if(c >= d) c = aToB(c,d);
    if(d >= e) d = aToB(d,e);
    
    // bubble sort third pass
    if(a >= b) a = aToB(a,b);
    if(b >= c) b = aToB(b,c);
    if(c >= d) c = aToB(c,d);
    
    // bubble sort forth pass
    if(a >= b) a = aToB(a,b);
    if(b >= c) b = aToB(b,c);
    
    // bubble sort fifth  pass
    if(a >= b) a = aToB(a,b);
    
    outputCpsLine(interval,a,alpha_n,decade,n_start,preMertens,preMertensAsymp);
    outputCpsLine(interval,b,alpha_n,decade,n_start,preMertens,preMertensAsymp);
    outputCpsLine(interval,c,alpha_n,decade,n_start,preMertens,preMertensAsymp);
    outputCpsLine(interval,d,alpha_n,decade,n_start,preMertens,preMertensAsymp);
    outputCpsLine(interval,e,alpha_n,decade,n_start,preMertens,preMertensAsymp);
    outputCpsLine(interval,f,alpha_n,decade,n_start,preMertens,preMertensAsymp);
}

// "%.6Lg" is compact (no forced fixed/scientific); tweak precision if you like.
static inline std::string fmt_preMertens(std::uint64_t preMertens, std::uint64_t n_start) {
    char buf[64];
    buf[0] = 0;
    if(preMertens == 0L || preMertens >= n_start) {
        std::snprintf(buf, sizeof(buf), "%" PRIu64,preMertens);
    }
    return std::string(buf);
}

void ExtremaValues::putMinima(long double c, long double cBaseline, std::uint64_t n, std::uint64_t delta, long double hlCorr) {
    current = c+cBaseline;
    currentBaseline = cBaseline;
    // For alignment calculations: track any negative value (current <= 0.0L) or 
    // any value <= the current last value. This ensures we track the last occurrence
    // of any negative value, not just the absolute minimum.
    // NOTE: c_first/n_first should NOT be trusted for this extrema type as we're
    // tracking the last occurrence of any negative value, not the true minimum.
    // Only apply special logic when there's a non-zero baseline (alignment calculations)
    if ((cBaseline != 0.0L && current <= 0.0L) || current <= c_last  || !n_last) {
        if (current < c_first  || !n_first) {
            c_firstBaseline = currentBaseline;
            c_first = current;
            delta_first = delta;
            n_first = n;
            hlCorr_first = hlCorr;
        }
        c_lastBaseline = currentBaseline;
        c_last = current;  // Store current (c+cBaseline)
        delta_last = delta;
        n_last = n;
        hlCorr_last = hlCorr;
    }
}

static inline long double getRatio(const long double c_meas, const long double current) {
    static const long double epsilon = 2e-8L;
    
    // Values are equal if: difference is within absolute tolerance OR within relative tolerance
    if (std::fabsl(c_meas - current) <= epsilon) {
        return 1.0L;
    }
    else if (current != 0.0L) {
        // current is not near zero, safe to divide
        return c_meas / current;
    }
    // Division by zero: current is effectively 0 (within tolerance) but c_meas is not
    // Use sentinel that preserves sign for correct comparison
    // Negative check is a safeguard - c_meas should never be < 0, but indicates math error if it is
    return (c_meas < 0.0L) ? -LDBL_MAX : LDBL_MAX;
}

long double ExtremaValues::getFirstRatio() const {
    return getRatio(extra_first, c_first);
}

long double ExtremaValues::getLastRatio() const {
    return getRatio(extra_last, c_last);
}

void ExtremaValues::putMinimaRatio(long double c_meas, long double c, long double cBaseline, std::uint64_t n, std::uint64_t delta, long double hlCorr) {
    current = c+cBaseline;
    currentBaseline = cBaseline;
    if(! n_last) {
        c_firstBaseline = currentBaseline;
        c_first = current;
        delta_first = delta;
        n_first = n;
        hlCorr_first = hlCorr;
        extra_first = c_meas;
        c_lastBaseline = currentBaseline;
        c_last = current;
        delta_last = delta;
        n_last = n;
        hlCorr_last = hlCorr;
        extra_last = c_meas;
        return;
    }
    // Calculate ratio: c_meas / current
    // Handle division by zero: use maximum long double as sentinel value that preserves sign
    // c_meas should never be negative, but we check for it as a safeguard to detect math errors
    // Use tolerance-based equality check to handle floating-point round-off errors
    const long double ratio = getRatio(c_meas, current);
    
    
    const long double r_last = getLastRatio();
    
    // Always track the minimum ratio value, including negative values.
    // This version does not have special handling for negative values with baseline,
    // it simply tracks the true minimum regardless of sign.
    if (ratio <= r_last) {
        const long double r_first = getFirstRatio();
        if (ratio < r_first) {
            c_firstBaseline = currentBaseline;
            c_first = current;
            delta_first = delta;
            n_first = n;
            hlCorr_first = hlCorr;
            extra_first = c_meas;
        }
        c_lastBaseline = currentBaseline;
        c_last = current;  // Store current (c+cBaseline)
        delta_last = delta;
        n_last = n;
        hlCorr_last = hlCorr;
        extra_last = c_meas;
    }
}

void ExtremaValues::putMaxima(long double c, long double cBaseline, std::uint64_t n, std::uint64_t delta, long double hlCorr) {
    current = c+cBaseline;
    currentBaseline = cBaseline;
    // Compare values directly for MAXIMUM
    if (current >= c_last  || !n_last) {
        if (current > c_first  || !n_first) {
            c_firstBaseline = currentBaseline;
            c_first = current;
            delta_first = delta;
            n_first = n;
            hlCorr_first = hlCorr;
        }
        c_lastBaseline = currentBaseline;
        c_last = current;
        delta_last = delta;
        n_last = n;
        hlCorr_last = hlCorr;
    }
}

void ExtremaValues::putMaximaRatio(long double c_meas, long double c, long double cBaseline, std::uint64_t n, std::uint64_t delta, long double hlCorr) {
    current = c+cBaseline;
    currentBaseline = cBaseline;
    if(! n_last) {
        c_firstBaseline = currentBaseline;
        c_first = current;
        delta_first = delta;
        n_first = n;
        hlCorr_first = hlCorr;
        extra_first = c_meas;
        c_lastBaseline = currentBaseline;
        c_last = current;
        delta_last = delta;
        n_last = n;
        hlCorr_last = hlCorr;
        extra_last = c_meas;
        return;
    }
    // Calculate ratio: c_meas / current
    // Handle division by zero: use maximum long double as sentinel value that preserves sign
    // c_meas should never be negative, but we check for it as a safeguard to detect math errors
    // Use tolerance-based equality check to handle floating-point round-off errors
    const long double ratio = getRatio(c_meas, current);
    
    const long double r_last = getLastRatio();
    
    // Track the maximum ratio value, including negative values.
    if (ratio >= r_last) {
        const long double r_first = getFirstRatio();
        if (ratio > r_first) {
            c_firstBaseline = currentBaseline;
            c_first = current;
            delta_first = delta;
            n_first = n;
            hlCorr_first = hlCorr;
            extra_first = c_meas;
        }
        c_lastBaseline = currentBaseline;
        c_last = current;  // Store current (c+cBaseline)
        delta_last = delta;
        n_last = n;
        hlCorr_last = hlCorr;
        extra_last = c_meas;
    }
}

void ExtremaValues::applyHLCorrFirst(long double hlCorr) {
    c_first -= c_firstBaseline;
    if(hlCorr_first != 1.0L && hlCorr_first != 0.0L) {
        c_first /= hlCorr_first;  // Divide by OLD value
    }
    hlCorr_first = hlCorr;  // Update to NEW value
    if(hlCorr_first != 1.0L && hlCorr_first != 0.0L) {
        c_first *= hlCorr_first;  // Multiply by NEW value
    }
    c_first += c_firstBaseline;
}

void ExtremaValues::applyHLCorrLast(long double hlCorr) {
    c_last -= c_lastBaseline;
    if(hlCorr_last != 1.0L && hlCorr_last != 0.0L) {
        c_last /= hlCorr_last;  // Divide by OLD value
    }
    hlCorr_last = hlCorr;  // Update to NEW value
    if(hlCorr_last != 1.0L && hlCorr_last != 0.0L) {
        c_last *= hlCorr_last;  // Multiply by NEW value
    }
    c_last += c_lastBaseline;
}


void ExtremaValues::copyFirstToLast() {
    // Copy all first values to last, including extra_first to extra_last
    // This ensures associated data (like jitter) stays with the correct extrema
    c_last = c_first;
    c_lastBaseline = c_firstBaseline;
    n_last = n_first;
    delta_last = delta_first;
    hlCorr_last = hlCorr_first;
    extra_last = extra_first;
}

void ExtremaValues::copyLastToFirst() {
    // Copy all last values to first, including extra_last to extra_first
    // This ensures associated data (like jitter) stays with the correct extrema
    c_first = c_last;
    c_firstBaseline = c_lastBaseline;
    n_first = n_last;
    delta_first = delta_last;
    hlCorr_first = hlCorr_last;
    extra_first = extra_last;
}

void ExtremaValues::applyHLCorrStateMin(HLCorrState &state) {
    // Skip correction if no value was ever assigned (n_first == 0 indicates uninitialized)
    if(n_first == 0) {
        return;
    }
    applyHLCorrFirst(state(n_first, delta_first));
    applyHLCorrLast(state(n_last, delta_last));
    // For alignment calculations: after HLCorr correction, keep the minimum value.
    // The condition c_last <= 0.0L handles cases where the last value is still negative
    // after correction, ensuring we maintain proper minimum tracking.
    // NOTE: c_first/n_first should NOT be trusted for this extrema type.
    if(c_last <= 0.0L || c_last < c_first) {
        // Last is the minimum, copy it to first
        copyLastToFirst();
    } else if(c_last > c_first) {
        // First is the minimum, copy it to last
        copyFirstToLast();
    }
}

void ExtremaValues::applyHLCorrStateMax(HLCorrState &state) {
    // Skip correction if no value was ever assigned (n_first == 0 indicates uninitialized)
    if(n_first == 0) {
        return;
    }
    applyHLCorrFirst(state(n_first, delta_first));
    applyHLCorrLast(state(n_last, delta_last));
    // Keep the maximum value
    if(c_last > c_first) {
        // Last is the maximum, copy it to first
        copyLastToFirst();
    } else if(c_last < c_first) {
        // First is the maximum, copy it to last
        copyFirstToLast();
    }
}

void ExtremaValues::applyHLCorrStateMinRatio(HLCorrState &state) {
    // Skip correction if no value was ever assigned (n_first == 0 indicates uninitialized)
    if(n_first == 0) {
        return;
    }
    applyHLCorrFirst(state(n_first, delta_first));
    applyHLCorrLast(state(n_last, delta_last));
    // After HLCorr correction, compare ratios to determine the minimum
    long double r_last = getLastRatio();
    long double r_first = getFirstRatio();
    // For alignment calculations: after HLCorr correction, keep the minimum ratio.
    // The condition r_last <= 0.0L handles cases where the last ratio is still negative
    // after correction, ensuring we maintain proper minimum tracking.
    if(r_last <= 0.0L || r_last < r_first) {
        // Last ratio is the minimum, copy it to first
        copyLastToFirst();
    } else if(r_last > r_first) {
        // First ratio is the minimum, copy it to last
        copyFirstToLast();
    }
}

void ExtremaValues::applyHLCorrStateMaxRatio(HLCorrState &state) {
    // Skip correction if no value was ever assigned (n_first == 0 indicates uninitialized)
    if(n_first == 0) {
        return;
    }
    applyHLCorrFirst(state(n_first, delta_first));
    applyHLCorrLast(state(n_last, delta_last));
    // After HLCorr correction, compare ratios to determine the maximum
    long double r_last = getLastRatio();
    long double r_first = getFirstRatio();
    // Keep the maximum ratio value
    if(r_last > r_first) {
        // Last ratio is the maximum, copy it to first
        copyLastToFirst();
    } else if(r_last < r_first) {
        // First ratio is the maximum, copy it to last
        copyFirstToLast();
    }
}

void GBLongIntervalSummary::outputCpsLine(
    GBLongInterval &interval,
    std::uint64_t n,
    long double alpha_n,
    int decade,
    std::uint64_t n_start,
    std::uint64_t preMertens,
    std::uint64_t preMertensAsymp
) {
    if(n == 0 || ! interval.cps) {
        return;
    }
    if(decade >= 0 && alpha_n == 0.5L && n == 19) {
        // attepmt to reproduce the v0.1.5 output, which did not have a n0Last variable
        return;
    }
    double long deltaC = 0.0L;
    double long deltaCAsymp = 0.0L;
    if(n == cMinima.n_first) {
        deltaC = cMinima.c_first - cminus_of_n0First;
        deltaCAsymp = cMinima.c_first - cminusAsymp_of_n0First;
        if(decade < 0) {
            std::fprintf(interval.cps,
                "%" PRIu64 ",%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.6LF,%s,%s,%0.12LF\n",
                cMinima.n_first,cMinima.c_first,cminus_of_n0First,deltaC,
                cminusAsymp_of_n0First,deltaCAsymp,
                fmt_preMertens(preMertens,n_start).c_str(),fmt_preMertens(preMertensAsymp,n_start).c_str(),
                alpha_n);
        }
        else {
            std::fprintf(interval.cps,
                "%d,%" PRIu64 ",%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.6LF\n",
                decade,cMinima.n_first,cMinima.c_first,cminus_of_n0First,deltaC,
                cminusAsymp_of_n0First,deltaCAsymp
            );
        }
    }
    else if(n == cMinima.n_last) {
        deltaC = cMinima.c_last - cminus_of_n0Last;
        deltaCAsymp = cMinima.c_last - cminusAsymp_of_n0Last;
        if(decade < 0) {
            std::fprintf(interval.cps,
                "%" PRIu64 ",%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.6LF,%s,%s,%0.12LF\n",
                cMinima.n_last,cMinima.c_last,cminus_of_n0Last,deltaC,
                cminusAsymp_of_n0Last,deltaCAsymp,
                fmt_preMertens(preMertens,n_start).c_str(),fmt_preMertens(preMertensAsymp,n_start).c_str(),
                alpha_n);
        }
        else {
            std::fprintf(interval.cps,
                "%d,%" PRIu64 ",%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.6LF\n",
                decade,cMinima.n_last,cMinima.c_last,cminus_of_n0Last,deltaC,
                cminusAsymp_of_n0Last,deltaCAsymp
            );
        }
    }
    else if(n == n2First) {
        deltaC = c_of_n2First - cminus_of_n2First;
        deltaCAsymp = c_of_n2First - cminusAsymp_of_n2First;
        if(decade < 0) {
            std::fprintf(interval.cps,
                "%" PRIu64 ",%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.6LF,%s,%s,%0.12LF\n",
                n2First,c_of_n2First,cminus_of_n2First,deltaC,
                cminusAsymp_of_n2First,deltaCAsymp,
                fmt_preMertens(preMertens,n_start).c_str(),fmt_preMertens(preMertensAsymp,n_start).c_str(),
                alpha_n
            );
        }
    }
    else if(n == n2Last) {
        deltaC = c_of_n2Last - cminus_of_n2Last;
        deltaCAsymp = c_of_n2Last - cminusAsymp_of_n2Last;
        if(decade < 0) {
            std::fprintf(interval.cps,
                "%" PRIu64 ",%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.6LF,%s,%s,%0.12LF\n",
                n2Last,c_of_n2Last,cminus_of_n2Last,deltaC,
                cminusAsymp_of_n2Last,deltaCAsymp,
                fmt_preMertens(preMertens,n_start).c_str(),fmt_preMertens(preMertensAsymp,n_start).c_str(),
                alpha_n
            );
        }
    }
    else if(n == n3First) {
        deltaC = c_of_n3First - cminus_of_n3First;
        deltaCAsymp = c_of_n3First - cminusAsymp_of_n3First;
        if(decade < 0) {
            std::fprintf(interval.cps,
                "%" PRIu64 ",%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.6LF,%s,%s,%0.12LF\n",
                n3First,c_of_n3First,cminus_of_n3First,deltaC,
                cminusAsymp_of_n3First,deltaCAsymp,
                fmt_preMertens(preMertens,n_start).c_str(),fmt_preMertens(preMertensAsymp,n_start).c_str(),
                alpha_n
            );
        }
    }
    else if(n == n3Last) {
        deltaC = c_of_n3Last - cminus_of_n3Last;
        deltaCAsymp = c_of_n3Last - cminusAsymp_of_n3Last;
        if(decade < 0) {
            std::fprintf(interval.cps,
                "%" PRIu64 ",%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.6LF,%s,%s,%0.12LF\n",
                n3Last,c_of_n3Last,cminus_of_n3Last,deltaC,
                cminusAsymp_of_n3Last,deltaCAsymp,
                fmt_preMertens(preMertens,n_start).c_str(),fmt_preMertens(preMertensAsymp,n_start).c_str(),
                alpha_n
            );
        }
    }
    else {
        return;
    }
    if(deltaC <= 0.0L) {
        interval.nstar = 0;
        interval.deltaMertens = deltaC;
    }
    else if(interval.nstar <= preMertens && n > preMertens) {
        interval.nstar = n;
        interval.deltaMertens = deltaC;
    }
    if(deltaCAsymp <= 0.0L) {
        interval.nstarAsymp = 0;
        interval.deltaMertensAsymp = deltaCAsymp;
    }
    else if(interval.nstarAsymp <= preMertensAsymp && n > preMertensAsymp) {
        interval.nstarAsymp = n;
        interval.deltaMertensAsymp = deltaCAsymp;
    }
}

void GBLongIntervalSummary::outputBoundRatioMin(GBLongInterval &interval) {
    if(! interval.boundRatioMin || boundRatioMinima.n_first == 0) {
        return;
    }
    // TODO: This function already includes both ratio and lambda - good example to follow for other outputs
    long double ratio = boundRatioMinima.getFirstRatio();
    long double lambda = boundRatioMinima.getLambda();
    BoundStatus status = boundRatioMinima.getMinBoundStatus();
    
    // Output ratio as empty string if it's an invalid sentinel value (LDBL_MAX or -LDBL_MAX)
    char ratio_buf[64] = "";
    if(ratio != LDBL_MAX && ratio != -LDBL_MAX && std::isfinite(ratio)) {
        std::snprintf(ratio_buf, sizeof(ratio_buf), "%.8Lf", ratio);
    }
    
    // Output lambda as empty string if it's an invalid sentinel value (LDBL_MAX or -LDBL_MAX)
    // Note: std::isfinite(LDBL_MAX) returns true because LDBL_MAX is finite, so we check explicitly
    char lambda_buf[64] = "";
    if(lambda != LDBL_MAX && lambda != -LDBL_MAX && std::isfinite(lambda)) {
        std::snprintf(lambda_buf, sizeof(lambda_buf), "%.8Lf", lambda);
    }

    // Output c_first and baseline_first as empty string if they're invalid sentinel values
    // These indicate norm=0 (delta=0) cases where normalization is undefined
    char first_buf[64] = "";
    char baseline_buf[64] = "";
    long double c_first = boundRatioMinima.c_first;
    long double baseline = boundRatioMinima.c_firstBaseline;
    // Check for sentinel values: LDBL_MIN, LDBL_MAX, or -LDBL_MAX
    // Note: std::isfinite(LDBL_MAX) returns true, so we must check explicitly
    if(c_first != LDBL_MAX && c_first != -LDBL_MAX && std::isfinite(c_first)) {
        std::snprintf(first_buf, sizeof(first_buf), "%.8Lf", c_first);
    }
    if(baseline != LDBL_MAX && baseline != -LDBL_MAX && std::isfinite(baseline)) {
        std::snprintf(baseline_buf, sizeof(baseline_buf), "%.8Lf", baseline);
    }
    
    std::fprintf(interval.boundRatioMin,
        "%" PRIu64 ",%s,%s,%s,%.8Lf,%s,%s\n",
        boundRatioMinima.n_first,
        ratio_buf,
        first_buf,
        baseline_buf,
        boundRatioMinima.extra_first,  // c_of_n_first
        lambda_buf,
        boundStatusToString(status)
    );
}

void GBLongIntervalSummary::outputBoundRatioMax(GBLongInterval &interval) {
    if(! interval.boundRatioMax || boundRatioMaxima.n_first == 0) {
        return;
    }
    // TODO: This function already includes both ratio and lambda - good example to follow for other outputs
    long double ratio = boundRatioMaxima.getFirstRatio();
    long double lambda = boundRatioMaxima.getLambda();
    BoundStatus status = boundRatioMaxima.getMaxBoundStatus();
    
    // Output ratio as empty string if it's an invalid sentinel value (LDBL_MAX or -LDBL_MAX)
    char ratio_buf[64] = "";
    if(ratio != LDBL_MAX && ratio != -LDBL_MAX && std::isfinite(ratio)) {
        std::snprintf(ratio_buf, sizeof(ratio_buf), "%.8Lf", ratio);
    }
    
    // Output lambda as empty string if it's an invalid sentinel value (LDBL_MAX or -LDBL_MAX)
    // Note: std::isfinite(LDBL_MAX) returns true because LDBL_MAX is finite, so we check explicitly
    char lambda_buf[64] = "";
    if(lambda != LDBL_MAX && lambda != -LDBL_MAX && std::isfinite(lambda)) {
        std::snprintf(lambda_buf, sizeof(lambda_buf), "%.8Lf", lambda);
    }
    
    // Output c_first and baseline_first as empty string if they're invalid sentinel values
    // These indicate norm=0 (delta=0) cases where normalization is undefined
    char first_buf[64] = "";
    char baseline_buf[64] = "";
    long double c_first = boundRatioMaxima.c_first;
    long double baseline = boundRatioMaxima.c_firstBaseline;
    // Check for sentinel values: LDBL_MIN, LDBL_MAX, or -LDBL_MAX
    // Note: std::isfinite(LDBL_MAX) returns true, so we must check explicitly
    if(c_first != LDBL_MAX && c_first != -LDBL_MAX && std::isfinite(c_first)) {
        std::snprintf(first_buf, sizeof(first_buf), "%.8Lf", c_first);
    }
    if(baseline != LDBL_MAX && baseline != -LDBL_MAX && std::isfinite(baseline)) {
        std::snprintf(baseline_buf, sizeof(baseline_buf), "%.8Lf", baseline);
    }
    
    std::fprintf(interval.boundRatioMax,
        "%" PRIu64 ",%s,%s,%s,%.8Lf,%s,%s\n",
        boundRatioMaxima.n_first,
        ratio_buf,
        first_buf,
        baseline_buf,
        boundRatioMaxima.extra_first,  // c_of_n_first
        lambda_buf,
        boundStatusToString(status)
    );
}

