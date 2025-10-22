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

void ExtremaValues::putMinima(long double c, std::uint64_t n, std::uint64_t delta, long double hlCorr) {
    current = c;
    
    // Compare values directly for MINIMUM
    if (c <= c_last  || !n_last) {
        if (c < c_first  || !n_first) {
            c_first = c;
            delta_first = delta;
            n_first = n;
            hlCorr_first = hlCorr;
        }
        c_last = c;
        delta_last = delta;
        n_last = n;
        hlCorr_last = hlCorr;
    }
}

void ExtremaValues::putMaxima(long double c, std::uint64_t n, std::uint64_t delta, long double hlCorr) {
    current = c;
    
    // Compare values directly for MAXIMUM
    if (c >= c_last  || !n_last) {
        if (c > c_first  || !n_first) {
            c_first = c;
            delta_first = delta;
            n_first = n;
            hlCorr_first = hlCorr;
        }
        c_last = c;
        delta_last = delta;
        n_last = n;
        hlCorr_last = hlCorr;
    }
}

void ExtremaValues::applyHLCorrFirst(long double hlCorr,long double c_firstBaseline) {
    if(hlCorr_first != 1.0L && hlCorr_first != 0.0L) {
        c_first = (c_first - c_firstBaseline) / hlCorr_first;  // Divide by OLD value
    }
    hlCorr_first = hlCorr;  // Update to NEW value
    if(hlCorr_first != 1.0L && hlCorr_first != 0.0L) {
        c_first = c_firstBaseline + (c_first * hlCorr_first);  // Multiply by NEW value
    }
}

void ExtremaValues::applyHLCorrLast(long double hlCorr,long double c_lastBaseline) {
    if(hlCorr_last != 1.0L && hlCorr_last != 0.0L) {
        c_last = (c_last - c_lastBaseline) / hlCorr_last;  // Divide by OLD value
    }
    hlCorr_last = hlCorr;  // Update to NEW value
    if(hlCorr_last != 1.0L && hlCorr_last != 0.0L) {
        c_last = c_lastBaseline + (c_last * hlCorr_last);  // Multiply by NEW value
    }
}

void ExtremaValues::applyHLCorrStateMin(HLCorrState &state,long double c_firstBaseline,long double c_lastBaseline) {
    applyHLCorrFirst(state(n_first, delta_first), c_firstBaseline);
    applyHLCorrLast(state(n_last, delta_last), c_lastBaseline);
    // Keep the minimum value
    if(c_last < c_first) {
        c_first = c_last;
        n_first = n_last;
        delta_first = delta_last;
        hlCorr_first = hlCorr_last;
    } else if(c_last > c_first) {
        c_last = c_first;
        n_last = n_first;
        delta_last = delta_first;
        hlCorr_last = hlCorr_first;
    }
}

void ExtremaValues::applyHLCorrStateMax(HLCorrState &state,long double c_firstBaseline,long double c_lastBaseline) {
    applyHLCorrFirst(state(n_first, delta_first), c_firstBaseline);
    applyHLCorrLast(state(n_last, delta_last), c_lastBaseline);
    // Keep the maximum value
    if(c_last > c_first) {
        c_first = c_last;
        n_first = n_last;
        delta_first = delta_last;
        hlCorr_first = hlCorr_last;
    } else if(c_last < c_first) {
        c_last = c_first;
        n_last = n_first;
        delta_last = delta_first;
        hlCorr_last = hlCorr_first;
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

