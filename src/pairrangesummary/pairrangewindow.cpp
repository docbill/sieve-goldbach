// pairrangewindow - class for aggregating windows
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
#include "pairrangewindow.hpp"

void PairRangeWindow::dec_close() {
    if(dec_out && dec_out != stdout) {
        std::fclose(dec_out);
        dec_out = nullptr;
    }
    dec_trace = nullptr;
}

void PairRangeWindow::prim_close() {
    if(prim_out && prim_out != stdout) {
        std::fclose(prim_out);
        prim_out = nullptr;
    }
    prim_trace = nullptr;
}

PairRangeWindow::~PairRangeWindow() {
    dec_close();
    prim_close();
}

