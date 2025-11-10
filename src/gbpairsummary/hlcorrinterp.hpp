// hlcorrinterp - Hardy-Littlewood correction with pre-scan and interpolation
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

#ifndef HLCORRINTERP_HPP
#define HLCORRINTERP_HPP

#include <cstdint>
#include <vector>
#include <cmath>
#include "hlcorrstate.hpp"

// Inline min/max for uint64_t to avoid template deduction issues
static inline std::uint64_t min_u64(std::uint64_t a, std::uint64_t b) {
    return (a < b) ? a : b;
}

static inline std::uint64_t max_u64(std::uint64_t a, std::uint64_t b) {
    return (a > b) ? a : b;
}

// Interpolation-based HLCorr: pre-scan to sample exact values, then interpolate
class HLCorrInterpolator {
private:
    struct Sample {
        std::uint64_t n;
        long double hlCorr;
    };
    
    std::vector<Sample> samples;
    std::uint64_t sample_interval = 0;
    std::uint64_t sample_size = 0;
    std::uint64_t n_start = 0;
    std::uint64_t n_end = 0;
    std::uint64_t range_size = 0;
    HLCorrState *state = nullptr;

public:
    HLCorrInterpolator() : sample_interval(0) {}
    
    // Assignment operator overload - do nothing to preserve samples
    HLCorrInterpolator& operator=(const HLCorrInterpolator& /* other */) {
        // Do nothing - preserve existing samples and state
        return *this;
    }

    void init(std::uint64_t n_start, std::uint64_t n_end, HLCorrState *state=nullptr) {
        if(n_start == this->n_start && n_end == this->n_end && state == this->state) {
            return;
        }
        if(state != nullptr) {
            this->state = state;
        }
        samples.clear();
        this->n_start = n_start;
        this->n_end = n_end;
        this->range_size = n_end - n_start;
        // For very small intervals: use sample_interval = 1 to get first and last points for linear interpolation
        // For range_size <= 1: only need one sample point
        // For larger ranges: use sqrt(sqrt(range_size)) formula, capped at 31
        if(range_size <= 1) {
            this->sample_interval = 1ULL;
        } else {
            // Clamp sample_interval to [1, 31]: minimum of 1 for small intervals, maximum of 31
            this->sample_interval = max_u64(1ULL, min_u64(31ULL, (std::uint64_t)std::ceil(1.0L + std::sqrt(std::sqrt((long double)range_size)))));
        }
        this->sample_size = (range_size + sample_interval - 1) / sample_interval;
        //fprintf(stderr, "DEBUG: hlcorrinterp: init(n_start=%llu, n_end=%llu, range_size=%llu, sample_interval=%llu, sample_size=%llu)\n", (unsigned long long)n_start, (unsigned long long)n_end, (unsigned long long)range_size, (unsigned long long)sample_interval, (unsigned long long)sample_size);
    }
    
    // Pre-scan: sample exact HLCorr at regular intervals
    // Uses the existing hlcorr() function with delta computed by computeDelta callback
    template<typename ComputeDeltaFunc>
    void prescan(std::uint64_t n, std::uint64_t &next_n, ComputeDeltaFunc computeDelta) {
        if(n >= n_end) {
            //fprintf(stderr, "DEBUG: (1) hlcorrinterp: prescan(n=%llu, next_n=%llu, n_end=%llu)\n", (unsigned long long)n, (unsigned long long)next_n, (unsigned long long)n_end);
            return;
        }
        if(n < n_start) {
            if(next_n > n_start) {
                next_n = n_start;
            }
            //fprintf(stderr, "DEBUG: (2) hlcorrinterp: prescan(n=%llu, next_n=%llu, n_start=%llu, n_end=%llu)\n", (unsigned long long)n, (unsigned long long)next_n, (unsigned long long)n_start, (unsigned long long)n_end);
            return;
        }
        std::uint64_t _next_n = ((n-n_start)/sample_size)*sample_size + n_start;
        if(n+1 == n_end || n == _next_n) {
            std::uint64_t delta = computeDelta((long double)n);
            long double hlCorr = state ? state->operator()(n, delta) : hlcorr(n, delta);
            samples.push_back({n, hlCorr});
            _next_n += sample_size;
            //fprintf(stderr, "DEBUG: (3) hlcorrinterp: prescan(n=%llu, _next_n=%llu, delta=%llu, hlCorr=%0.6LF)\n", (unsigned long long)n, (unsigned long long)_next_n, (unsigned long long)delta, hlCorr);
        }
        if(_next_n >= n_end) {
            _next_n = n_end-1;
        }
        if(next_n > _next_n && _next_n > n) {
            next_n = _next_n;
        }
    }

    
    // Interpolate HLCorr for a given n (linear interpolation)
    long double operator()(std::uint64_t n, std::uint64_t delta) {
        if (samples.empty()) {
            //fprintf(stderr, "DEBUG: hlcorrinterp: no samples, using hlcorr(n=%llu, delta=%llu,n_start=%llu,n_end=%llu)\n", (unsigned long long)n, (unsigned long long)delta, (unsigned long long)n_start, (unsigned long long)n_end);
            return hlcorr(n, delta);
        }
        
        // Find bracketing samples
        if (n <= samples.front().n) {
            return samples.front().hlCorr;
        }
        if (n >= samples.back().n) {
            return samples.back().hlCorr;
        }
        
        // Binary search for the right interval
        size_t left = 0;
        size_t right = samples.size() - 1;
        while (right - left > 1) {
            size_t mid = (left + right) / 2;
            if (samples[mid].n <= n) {
                left = mid;
            } else {
                right = mid;
            }
        }
        
        // Linear interpolation between samples[left] and samples[right]
        const Sample &s0 = samples[left];
        const Sample &s1 = samples[right];
        
        long double t = (long double)(n - s0.n) / (long double)(s1.n - s0.n);
        long double hlCorr = s0.hlCorr + t * (s1.hlCorr - s0.hlCorr);
        
        return hlCorr;
    }
};

#endif // HLCORRINTERP_HPP
