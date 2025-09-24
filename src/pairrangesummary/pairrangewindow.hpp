// pairrangewind - class for aggregating windows
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
#ifndef PAIR_RANGE_WINDOW_HPP
#define PAIR_RANGE_WINDOW_HPP 1

#include "pairinterval.hpp"

class PairRangeWindow {
public:
    long double alpha;
    long double hlCorrAvg = 1.0L;

    int need_euler_cap = 1;
    FILE *dec_out = nullptr;
    FILE *dec_trace = nullptr;
    FILE *prim_out = nullptr;
    FILE *prim_trace = nullptr;
    PairInterval prim_interval = PairInterval();
    PairInterval dec_interval = PairInterval();

    explicit PairRangeWindow(long double a) : alpha(a) {}
    ~PairRangeWindow();
    void dec_close();
    void prim_close();
    int is_dec_active() {
        return (dec_out || dec_trace);
    }
    int is_prim_active() {
        return (prim_out || prim_trace);
    }
};

#endif // PAIR_RANGE_WINDOW_HPP

