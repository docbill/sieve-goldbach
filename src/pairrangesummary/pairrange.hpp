// pairrange - class for aggregating range interval counts
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
#ifndef PAIR_RANGE_HPP
#define PAIR_RANGE_HPP 1

#include <cstdio>
#include <cctype>
#include <cinttypes>
#include <vector>
#include <memory>

#include "pairrangewindow.hpp"

enum class Model     : int { Empirical = 0, HLA = 1 };
enum class CompatVer : int { V015 = 0, Current = 1 };

class PairRange {
public:
    Model model = Model::Empirical;
    CompatVer compat_ver = CompatVer::Current;
    int euler_cap = 1;
    int include_trivial = 0;
    long double alpha = 0.5;

    int decade = 0;
    std::uint64_t dec_left = 4;
    std::uint64_t dec_right  = 5;
    std::uint64_t dec_n_end   = 5;
    std::uint64_t dec_threshold  = 10;
    std::uint64_t dec_base   = 1;
    long double dec_n_geom    = 0.0L;

    std::vector<std::unique_ptr<PairRangeWindow>> windows;

    std::uint64_t prim_left = 6;
    std::uint64_t prim_right  = 9;
    std::uint64_t prim_n_end   = 9;
    std::uint64_t prim_base = 3;
    std::uint64_t prim_threshold_major = 0;
    std::uint64_t prim_threshold_minor = 0;
    int odd_primorial_major = 5;
    int odd_primorial_minor = 3;
    long double prim_n_geom = 0.0L;

    HLCorrState decOddCalc, decEvenCalc, decMinCalc, decMaxCalc, decMinNormCalc, decMaxNormCalc;
    HLCorrState primOddCalc, primEvenCalc, primMinCalc, primMaxCalc, primMinNormCalc, primMaxNormCalc;
   
    void print_headers();
    std::uint64_t dec_reset(std::uint64_t n_start);
    std::uint64_t prim_reset(std::uint64_t n_start);
    
    void dec_calc_average(PairRangeWindow &window, int applyHLCorr);
    void prim_calc_average(PairRangeWindow &window, int applyHLCorr);
    void dec_output_aggregate(PairRangeWindow &window);
    void prim_output_aggregate(PairRangeWindow &window);
    std::uint64_t computeDelta(long double alpha,std::uint64_t n, int &euler_cap);

    std::uint64_t computeDelta(long double alpha,std::uint64_t n) {
        int _euler_cap = euler_cap && (compat_ver != CompatVer::V015);
        return computeDelta(alpha,n,_euler_cap);
    }

    int addRow( PairRangeWindow &w, std::uint64_t n, std::uint64_t delta, const long double logNlogN, std::uint64_t pc, long double twoSGB );

private:
    void aggregate(
        PairRangeWindow &window,
        std::uint64_t n,
        std::uint64_t delta
    ) {
        if(window.is_dec_active() && n >= dec_left && n < dec_right ) {
            const int firstMin = (compat_ver == CompatVer::V015);
            const int useHLCorr = (n == 4 && compat_ver == CompatVer::V015);
            window.dec_interval.aggregate(n, delta, useHLCorr,firstMin);
        }
        if(window.is_prim_active()&& n >= prim_left && n < prim_right ) {
            window.prim_interval.aggregate(n, delta, 0, 0);
        }
    }

    void dec_close();
    void prim_close();
};

#endif // PAIR_RANGE_HPP

