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

#include "gbprimorial.hpp"

// odd primes for primorial steps
static const std::uint32_t ODD_PRIMES[] = {
    3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97
};
static const std::size_t ODD_PRIMES_N = sizeof(ODD_PRIMES)/sizeof(ODD_PRIMES[0]);

static inline void odd_prim_base_and_next(std::uint64_t n, std::uint64_t* P, std::uint64_t* P_next) {
    std::uint64_t p = 1, next = 0;
    for (std::size_t i = 0; i < ODD_PRIMES_N; ++i) {
        __uint128_t cand = (__uint128_t)p * (__uint128_t)ODD_PRIMES[i];
        if (cand > (__uint128_t)n) {
            if (cand <= (__uint128_t)UINT64_MAX) {
                next = (std::uint64_t)cand;
            }
            break;
        }
        p = (std::uint64_t)cand;
        if (i + 1 < ODD_PRIMES_N) {
            __uint128_t cand2 = (__uint128_t)p * (__uint128_t)ODD_PRIMES[i + 1];
            next = (cand2 <= (__uint128_t)UINT64_MAX) ? (std::uint64_t)cand2 : 0;
        }
        else {
            next = 0;
        }
    }
    if (p < 3 && n >= 3) {
        p = 3;
    }
    *P = p;
    *P_next = next;
}

static inline std::uint64_t next_multiple_ceiling(std::uint64_t n, std::uint64_t B) {
    if (B == 0) return n;
    std::uint64_t k = (n + B - 1) / B;
    return k * B;
}

GBPrimorial::GBPrimorial() {}

void GBPrimorial::reset(std::uint64_t &n_start,bool) {
    if (n_start < left) {
        n_start = left;
    }
    if(n_start < 7*5*3) {
        if(n_start < 5*3) {
            base = 1;
            major = 3;
            minor = 2;
            thresholdMinor = 3;
            thresholdMajor = 5*3;
            right = n_start+1;
        }
        else {
            base = 5*3;
            major = 7;
            minor = 5;
            thresholdMinor = base;
            thresholdMajor = 7*base;
            right = next_multiple_ceiling(n_start, base);
        }
    }
    else {
        odd_prim_base_and_next(n_start, &thresholdMinor, &thresholdMajor);
        if (thresholdMinor <= base) {
            thresholdMinor = base;
            thresholdMajor = thresholdMinor * major;
        } else {
            std::uint64_t dummy;
            odd_prim_base_and_next(thresholdMinor - 1, &base, &dummy);
            major = (int)(thresholdMinor / base);
            std::uint64_t prev;
            odd_prim_base_and_next(base - 1, &prev, &dummy);
            minor = (int)(base / prev);
        }
        right = next_multiple_ceiling(n_start, base);
    }
    while (right <= n_start) {
        right += base;
    }
    left = right - base;
    if(! this->n_start) {
        this->n_start = left;
    }
    n_geom = sqrtl((long double)left * (long double)(right - 1));
    char buf[68];
    if(base == 1) {
        snprintf(buf,sizeof(buf),"%d",(int)left);
    }
    else {
        bool isMajor = (left % major == 0);
        int primorial = isMajor ? major : minor;
        int multiple = (int)((right - 1) / (isMajor ? thresholdMinor : base));
        if( multiple == 1) {
            snprintf(buf,sizeof(buf),"(%d#)/2",primorial);
        }
        else if( multiple == 2) {
            snprintf(buf,sizeof(buf),"(%d#)",primorial);
        }
        else if(! (multiple&1)) {
            snprintf(buf,sizeof(buf),"(%d#)%d",primorial,multiple/2);
        }
        else {
            snprintf(buf,sizeof(buf),"(%d#)%d/2",primorial,multiple);
        }
    }
    label = std::string(buf);
}

