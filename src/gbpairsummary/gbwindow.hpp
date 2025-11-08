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
#ifndef GB_WINDOW_HPP
#define GB_WINDOW_HPP 1

#include <math.h>

#include "gblonginterval.hpp"
#include "eulerproductseries.hpp"

enum class CompatVer : int { V015 = 0, Current = 1 };

class GBWindow {
public:
    const long double alpha;
    long double alpha_n;
    CompatVer compat_ver;
    std::uint64_t n_5percent = 0;
    std::uint64_t nzeroStat = 0;
    std::uint64_t nzeroStatAsymp = 0;
    long double etaStat = 0.0L;
    long double etaStatAsymp = 0.0L;
    long double hlCorrAvg = 1.0L;
    std::uint64_t preMertens = 0;
    std::uint64_t preMertensAsymp = 0;

    bool eulerCap = true;
    GBLongInterval dec;
    GBLongInterval prim;

    GBWindow(long double a,EulerProductSeries &productSeries,CompatVer compat_ver);

    void init(std::uint64_t* const prime_array, const std::uint64_t* const prime_array_end,bool _eulerCap) {
        eulerCap = _eulerCap;
        product_series_right.init(prime_array,prime_array_end);
        dec.active = (dec.out || dec.trace || dec.raw || dec.norm || dec.cps || dec.boundRatioMin || dec.boundRatioMax);
        prim.active = (prim.out || prim.trace || prim.raw || prim.norm || prim.cps || prim.boundRatioMin || prim.boundRatioMax);
    }

    ~GBWindow();

    std::uint64_t computeDelta(long double n);
    std::uint64_t computeDelta(long double n,long double &eulerCapAlpha);

    void closeInterval(GBLongInterval &interval);

    bool is_dec_active() {
        return dec.active;
    }

    bool is_prim_active() {
        return prim.active;
    }

    long double calcCminus(std::uint64_t n, std::uint64_t delta,long double logNlogN) {
        return logNlogN*product_series_left(n)*product_series_right(n+delta);
    }

    long double calcCminusAsymp(long double logN) {
        return KPRODKPROD*logN/(logl(1.0L+alpha_n)+logN);
    }

    void checkCrossing(std::uint64_t n,bool leCminus) {
        if(leCminus) {
            preMertens = n;
            nzeroStat = prim.nstar = dec.nstar = 0;
            prim.deltaMertens = dec.deltaMertens = etaStat = 0.0L;
        }
    }

    void checkCrossingAsymp(std::uint64_t n,bool leCminusAsymp) {
        if(leCminusAsymp) {
            preMertensAsymp = n;
            nzeroStatAsymp = prim.nstarAsymp = dec.nstarAsymp = 0;
            prim.deltaMertensAsymp = dec.deltaMertensAsymp = etaStatAsymp = 0.0L;
        }
    }

    void updateN5percent(std::uint64_t n,std::uint64_t delta,long double logNlogN,long double diff,long double diffAsymp);

    void dec_aggregate(
        std::uint64_t n,
        std::uint64_t delta,
        long double cminus,
        long double cminusAsymp,
        bool useHLCorr
    ) {
        dec.summary.aggregate(n,delta,cminus,cminusAsymp,useHLCorr);
    }
    
    void prim_aggregate(
        std::uint64_t n,
        std::uint64_t delta,
        long double cminus,
        long double cminusAsymp
    ) {
        prim.summary.aggregate(n,delta,cminus,cminusAsymp,false);
    }

private:
    EulerProductSeries &product_series_left;
    EulerProductSeries product_series_right;
};

#endif // GB_WINDOW_HPP

