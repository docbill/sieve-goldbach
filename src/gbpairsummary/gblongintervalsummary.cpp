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
    std::uint64_t preMertens,
    std::uint64_t preMertensAsymp
) {
    if(! interval.cps) {
        return;
    }
    // trivial sorting of 3 values
    // substituting a 0 for repeats
    uint64_t a = n0, b = n2, c = n3;
    if(b >= c) b = aToB(b,c);
    if(a >= b) a = aToB(a,b);
    if(b >= c) c = aToB(b,c);
    
    outputCpsLine(interval,a,alpha_n,decade,preMertens,preMertensAsymp);
    outputCpsLine(interval,b,alpha_n,decade,preMertens,preMertensAsymp);
    outputCpsLine(interval,c,alpha_n,decade,preMertens,preMertensAsymp);
}

// "%.6Lg" is compact (no forced fixed/scientific); tweak precision if you like.
static inline std::string fmt_preMertens(std::uint64_t preMertens) {
    char buf[64];
    buf[0] = 0;
    if(preMertens) {
        std::snprintf(buf, sizeof(buf), "%" PRIu64,preMertens);
    }
    return std::string(buf);
}


void GBLongIntervalSummary::outputCpsLine(
    GBLongInterval &interval,
    std::uint64_t n,
    long double alpha_n,
    int decade,
    std::uint64_t preMertens,
    std::uint64_t preMertensAsymp
) {
    if(n == 0 || ! interval.cps) {
        return;
    }
    double long deltaC = 0.0L;
    double long deltaCAsymp = 0.0L;
    if(n == n0) {
        deltaC = cMin - cminus_of_n0;
        deltaCAsymp = cMin - cminusAsymp_of_n0;
        if(decade < 0) {
            std::fprintf(interval.cps,
                "%" PRIu64 ",%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.12LF,%s,%s\n",
                n0,cMin,cminus_of_n0,deltaC,
                cminusAsymp_of_n0,deltaCAsymp,
                alpha_n,
                fmt_preMertens(preMertens).c_str(),fmt_preMertens(preMertensAsymp).c_str()
            );
        }
        else {
            std::fprintf(interval.cps,
                "%d,%" PRIu64 ",%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.6LF\n",
                decade,n0,cMin,cminus_of_n0,deltaC,
                cminusAsymp_of_n0,deltaCAsymp
            );
        }
    }
    else if(n == n2) {
        deltaC = c_of_n2 - cminus_of_n2;
        deltaCAsymp = c_of_n2 - cminusAsymp_of_n2;
        if(decade < 0) {
            std::fprintf(interval.cps,
                "%" PRIu64 ",%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.12LF,%s,%s\n",
                n2,c_of_n2,cminus_of_n2,deltaC,
                cminusAsymp_of_n2,deltaCAsymp,
                alpha_n,
                fmt_preMertens(preMertens).c_str(),fmt_preMertens(preMertensAsymp).c_str()
            );
        }
    }
    else if(n == n3) {
        deltaC = c_of_n3 - cminus_of_n3;
        deltaCAsymp = c_of_n3 - cminusAsymp_of_n3;
        if(decade < 0) {
            std::fprintf(interval.cps,
                "%" PRIu64 ",%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.12LF,%s,%s\n",
                n3,c_of_n3,cminus_of_n3,deltaC,
                cminusAsymp_of_n3,deltaCAsymp,
                alpha_n,
                fmt_preMertens(preMertens).c_str(),fmt_preMertens(preMertensAsymp).c_str()
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
        interval.deltaMertensAsymp = deltaC;
    }
}

