// gbprimorial - class for aggregating range interval counts
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
#ifndef GB_PRIMORIAL_HPP
#define GB_PRIMORIAL_HPP 1

#include "gbaggregate.hpp"

class GBPrimorial : public GBAggregate {
public:
    std::uint64_t thresholdMajor = 3;
    std::uint64_t thresholdMinor = 1;
    int major = 3;
    int minor = 1;
    FILE *cps_summary = nullptr;

    GBPrimorial();
    
    void reset(std::uint64_t &n_start,bool inclusiveInterval) override;
};

#endif // GB_PRIMORIAL_HPP

