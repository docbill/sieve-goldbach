// gbdecade - class for aggregating range interval counts
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
#ifndef GB_DECADE_HPP
#define GB_DECADE_HPP 1

#include <math.h>
#include "gbaggregate.hpp"

class GBDecade : public GBAggregate {
public:
    int decade = 0;
    std::uint64_t threshold  = 10;
    FILE *cps_summary = nullptr;
    
    GBDecade();

    void reset(std::uint64_t &n_start,bool useLegacy) override;
};

#endif // GB_DECADE_HPP

