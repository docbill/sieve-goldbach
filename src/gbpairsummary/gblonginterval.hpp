// gbwindowwindowInterval - class for window windowInterval
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
#ifndef GB_LONG_INTERVAL_HPP
#define GB_LONG_INTERVAL_HPP 1

#include <math.h>

#include "gblongintervalsummary.hpp"

class GBLongInterval {
public:
    int active = -1;
    std::uint64_t nstar = 0;
    long double deltaMertens = 0.0L;
    std::uint64_t nstarAsymp = 0;
    long double deltaMertensAsymp = 0.0L;
    FILE *out = nullptr;
    FILE *raw = nullptr;
    FILE *norm = nullptr;
    FILE *trace = nullptr;
    FILE *cps = nullptr;
    FILE *boundRatioMin = nullptr;  // v0.2.0: bound ratio minimum output
    FILE *boundRatioMax = nullptr;   // v0.2.0: bound ratio maximum output
    GBLongIntervalSummary summary = GBLongIntervalSummary();
};

#endif // GB_LONG_INTERVAL_HPP

