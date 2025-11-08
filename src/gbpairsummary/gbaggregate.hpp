// gbaggregate - class for aggregating range interval counts
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
#ifndef GB_AGGREGATE_HPP
#define GB_AGGREGATE_HPP 1

#include <cctype>
#include <cinttypes>
#include <string>

#include "hlcorrstate.hpp"
#include "hlcorrinterp.hpp"

class GBAggregate {
public:
    std::string label = "";
    std::uint64_t left = 4;
    std::uint64_t right = 5;
    std::uint64_t n_start = 0;
    std::uint64_t n_end = 5;
    std::uint64_t base = 1;
    long double n_geom = 0.0L;
    HLCorrState oddCalc, evenCalc, minCalc, maxCalc, minNormCalc, maxNormCalc;
    HLCorrState alignNormMinCalc, alignNormMaxCalc, boundNormMinCalc, boundNormMaxCalc;
    HLCorrState boundDeltaMinNormCalc, boundDeltaMaxNormCalc;

    GBAggregate();

    virtual void reset(std::uint64_t &n_start,bool useLegacy) = 0;
};

#endif // GB_AGGREGATE_HPP

