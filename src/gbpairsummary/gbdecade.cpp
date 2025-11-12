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

#include <math.h>
#include <string.h>
#include "gbdecade.hpp"

static inline std::uint64_t log_floor_u64(std::uint64_t n, std::uint64_t base) {
    std::uint64_t k = 0;
    for (; n >= base; n /= base, ++k) {}
    return k;
}

static inline std::uint64_t ipow_u64(std::uint64_t base, std::uint64_t exp) {
    std::uint64_t p = 1;
    while (exp--) p *= base;
    return p;
}

GBDecade::GBDecade() {}

void GBDecade::reset(std::uint64_t &n_start,bool useLegacy) {
    if(n_start < left) {
        n_start = left;
    }
    decade      = (int)log_floor_u64(n_start, 10);
    base   = ipow_u64(10, (std::uint64_t)decade);
    right  = (n_start - (n_start % base)) + base;
    threshold  = 10ULL * base;
    left = right - base;
    if(! this->n_start) {
        this->n_start = left;
    }
    n_geom = sqrtl((long double)left * (long double)(useLegacy?right:(right-1)));
    char buf[64];
    if(useLegacy) {
        snprintf(buf,sizeof(buf),"%d",decade);
    }
    else { 
        snprintf(buf,sizeof(buf),"%de%d",(int)((right - 1) / base),decade);
    }
    label = std::string(buf);
}

