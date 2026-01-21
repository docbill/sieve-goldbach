// gbpsi - class for aggregating primorial short interval counts
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

#include "gbpsi.hpp"

// odd primes for primorial steps
static const std::uint32_t ODD_PRIMES[] = {
    3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97
};
static const std::size_t ODD_PRIMES_N = sizeof(ODD_PRIMES)/sizeof(ODD_PRIMES[0]);

GBPSI::GBPSI() {}

void GBPSI::reset(std::uint64_t &n_start,bool) {
    if (n_start < left) {
        n_start = left;
    }
    for(right = n_start + base;2*right > q_next*q_next;right = n_start + base) {
        base=q_next;
        std::uint64_t _q_next = base * ODD_PRIMES[prime_index];
        if (_q_next > UINT32_MAX) {
            q_next = UINT32_MAX;
            break;
        }
        q_next = _q_next;
        prime_index++;
    }
    left = n_start;
    if(! this->n_start) {
        this->n_start = left;
    }
    n_geom = sqrtl((long double)left * (long double)(right - 1));
    char buf[68];
    snprintf(buf,sizeof(buf),"%d",(int)left);
    label = std::string(buf);
}

