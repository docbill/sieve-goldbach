// gbrange - class for aggregating range interval counts
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
#ifndef GB_RANGE_HPP
#define GB_RANGE_HPP 1

#include <cstdio>
#include <cctype>
#include <cinttypes>
#include <vector>
#include <memory>

#include "gbwindow.hpp"
#include "hlcorrstate.hpp"
#include "gbdecade.hpp"
#include "gbprimorial.hpp"

enum class Model     : int { Empirical = 0, HLA = 1 };

class GBRange {
public:
    Model model = Model::Empirical;
    CompatVer compat_ver = CompatVer::Current;
    
    bool includeTrivial = false;
    int eulerCap = 1;

    GBDecade decAgg;
    GBPrimorial primAgg;

    std::vector<std::unique_ptr<GBWindow>> windows;

    EulerProductSeries product_series_left;

    void init(std::uint64_t* const _primeArray, const std::uint64_t* const _primeArrayEndend,size_t _primeArrayEndlen,int _eulerCap) {
        primeArray      = _primeArray;
        primeArrayEndend  = _primeArrayEndend;
        primeArrayEndlen  = _primeArrayEndlen;
        eulerCap = _eulerCap;

        product_series_left.init(primeArray,primeArrayEndend);
        for(auto &w : windows) {
            bool w_eulerCap = (eulerCap > 0) || (eulerCap < 0 && (compat_ver != CompatVer::V01x || w->alpha > 0.5L));
            w->init(primeArray,primeArrayEndend,w_eulerCap);
        }
        decReset(decAgg.left);
        primReset(primAgg.left);
    }

    void printHeaders();
    void printCpsSummaryHeaders();
    std::uint64_t decReset(std::uint64_t n_start);
    std::uint64_t primReset(std::uint64_t n_start);
    void outputFull(GBAggregate &agg,GBLongInterval &interval,bool useLegacy);
    void outputNorm(GBAggregate &agg,GBLongInterval &interval);
    void outputRaw(GBAggregate &agg,GBLongInterval &interval);
    void decOutputCpsSummary(GBWindow &w);
    void primOutputCpsSummary(GBWindow &w);
    int decInputCpsSummary(const char* filename);
    int primInputCpsSummary(const char* filename);

    int addRow( GBWindow &w, std::uint64_t n, std::uint64_t delta, const long double logN, const long double logNlogN, std::uint64_t empiricalPairCount, std::uint64_t trivialPairCount, long double twoSGB );

    int processRows();

private:
    std::uint64_t* primeArray = nullptr;
    const std::uint64_t* primeArrayEndend  = nullptr;
    std::size_t    primeArrayEndlen  = 0;
    HLCorrState primState, decState;

    void aggregate(
        GBWindow &window,
        std::uint64_t n,
        std::uint64_t delta,
        long double cminus,
        long double cminusAsymp
    ) {
        if(window.is_dec_active() && n >= decAgg.left && n < decAgg.right ) {
            const bool useHLCorr = (n == 4 && compat_ver == CompatVer::V01x);
            window.dec_aggregate(n, delta, cminus, cminusAsymp, useHLCorr);
        }
        if(window.is_prim_active()&& n >= primAgg.left && n < primAgg.right ) {
            window.prim_aggregate(n, delta, cminus, cminusAsymp);
        }
    }

    void calcAverage(GBWindow &window,GBLongInterval &interval,GBAggregate &agg,bool useLegacy);

    void dec_close();
    void prim_close();
    void summary_close();
};

#endif // GB_RANGE_HPP

