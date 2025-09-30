// gbwindow - class for aggregating windows
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
#include "gbwindow.hpp"

void inline close(FILE *&out) {
    if(out && out != stdout) {
        std::fclose(out);
        out = nullptr;
    }
}

GBWindow::GBWindow(long double a,EulerProductSeries &productSeries,CompatVer _compat_ver)
: alpha(a), alpha_n(a), compat_ver(_compat_ver), product_series_left(productSeries)
{}

void GBWindow::closeInterval(GBLongInterval &interval) {
    interval.active = 0;
    close(interval.out);
    close(interval.raw);
    close(interval.norm);
    close(interval.cps);
    interval.trace = nullptr;
}

GBWindow::~GBWindow() {
    closeInterval(dec);
    closeInterval(prim);
}

std::uint64_t GBWindow::computeDelta(long double n,long double &eulerCapAlpha) {
    std::uint64_t delta = (std::uint64_t)floorl(alpha *n);
    alpha_n = alpha;
    if(eulerCap) {
        if(eulerCapAlpha == 0.0L) {
            eulerCapAlpha = 1.0L+(0.5L-sqrtl(2.0L*n+0.25L))/n; // alpha(n) = ((2n+1)-sqrt(8n+1))/(2n)
        }
        long double val = ceill(eulerCapAlpha*n) - 1.0L;
        std::uint64_t cap = (val < 1.0L) ? 1ULL : (std::uint64_t)val;
        if (delta > cap) {
            delta = cap;
            alpha_n = eulerCapAlpha;
        }
    }
    if (compat_ver != CompatVer::V015 || alpha > 0.5L) {
        std::uint64_t max_delta = (n > 3) ? (n - 3) : 1;
        if (delta > max_delta) {
            delta = max_delta;
        }
    }
    return delta;
}

std::uint64_t GBWindow::computeDelta(long double n) {
    long double eulerCapAlpha = 0.0L;
    return computeDelta(n,eulerCapAlpha);
}

void GBWindow::updateN5percent(std::uint64_t n,std::uint64_t delta,long double logNlogN,long double diff,long double diffAsymp) {
    if(!n_5percent) {
        if( KPRODKPROD*(long double)delta < 400*logNlogN ) {
            return;
        }
        n_5percent = n;
        firstDiff = (preMertens <= n);
        firstDiffAsymp = (preMertensAsymp <= n);
    }
    if(nzeroStat && nzeroStat <= preMertens) {
        nzeroStat = 0;
        etaStat = 0.0L;
    }
    if(n > preMertens && (etaStat >= diff || ! nzeroStat)) {
        nzeroStat = n;
        etaStat = diff;
    }
    if(nzeroStatAsymp && nzeroStatAsymp <= preMertensAsymp) {
        nzeroStatAsymp = 0;
        etaStatAsymp = 0.0L;
    }
    if(n > preMertensAsymp && (etaStatAsymp >= diff || ! nzeroStatAsymp)) {
        nzeroStatAsymp = n;
        etaStatAsymp = diffAsymp;
    }
}

