// gbrange - class for defining ranges
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
#include <cstdarg>   // for va_list, va_start, va_end, va_copy
#include <cstdio>    // for std::vfprintf, std::fflush
#include <cstring>   // for strstr
#include "gbrange.hpp"

// ----- Small helpers -----
static inline std::uint64_t maxPrefEven(long double value, std::uint64_t minValue) {
    std::uint64_t retval = (~1ULL) & (std::uint64_t)ceill(value);
    return (retval >= minValue) ? retval : minValue;
}

static inline std::uint64_t minPrefOdd(long double value, std::uint64_t maxValue) {
    std::uint64_t retval = 1ULL | (std::uint64_t)floorl(value);
    return (retval <= maxValue) ? retval : maxValue;
}

// Printf to up to two FILE*s.
static inline void vfprintf_both(FILE* a, FILE* b, const char* fmt, va_list ap) {
    if (a) {
        va_list ap2;
        va_copy(ap2, ap);
        std::vfprintf(a, fmt, ap2); va_end(ap2);
        std::fflush(a);
    }
    if (b) {
        std::vfprintf(b, fmt, ap);
        std::fflush(b);
    }
}

static inline long double expose_next_log_fast(
    long double w, long double p_next, long double q_next, bool next_aligns, bool require_span)
{
    if (!next_aligns || (require_span && w < p_next)) return 0.0L;
    // fractional part of w/q_next in [0,1)
    long double t = w / q_next;
    t -= floorl(t);
    return t * logl(p_next - 2.0L);   // add to sum of logs
}

long double effective_small_prime_deficit(uint64_t n, long double w)
{
    if (n < 3ULL) return 0.0L;

    // seed with mod-3 admissible fraction
    long double sumlog = logl((n % 3ULL == 0ULL) ? (2.0L/3.0L) : 1.0L);

    // --- up to p=5 ---
    static constexpr long double PR5 = 3.0L*5.0L;
    const bool a5 = (n % 5ULL) != 0ULL;
    if (w < PR5) {
        sumlog += expose_next_log_fast(w, 5.0L, PR5, a5, /*require_span=*/true);
        return expl(sumlog);
    }
    if (a5) sumlog += logl(5.0L - 2.0L); // *3

    // --- up to p=7/11/13/17/19 ---
    static constexpr long double PR7  = 7.0L*PR5;
    static constexpr long double PR11 = 11.0L*PR7;
    static constexpr long double PR13 = 13.0L*PR11;
    static constexpr long double PR17 = 17.0L*PR13;
    static constexpr long double PR19 = 19.0L*PR17;

    const bool a7  = (n %  7ULL) != 0ULL;
    const bool a11 = (n % 11ULL) != 0ULL;
    const bool a13 = (n % 13ULL) != 0ULL;
    const bool a17 = (n % 17ULL) != 0ULL;
    const bool a19 = (n % 19ULL) != 0ULL;

    if (w < PR7) {
        sumlog += expose_next_log_fast(w,  7.0L, PR7,  a7,  false);
        sumlog += expose_next_log_fast(w, 11.0L, PR11, a11, false);
        sumlog += expose_next_log_fast(w, 13.0L, PR13, a13, false);
        sumlog += expose_next_log_fast(w, 17.0L, PR17, a17, /*require_span=*/true);
        sumlog += expose_next_log_fast(w, 19.0L, PR19, a19, /*require_span=*/true);
        return expl(sumlog);
    }
    if (a7) sumlog += logl(7.0L - 2.0L);   // *5

    // --- up to p=23 ---
    static constexpr long double PR23 = 23.0L*PR19;
    const bool a23 = (n % 23ULL) != 0ULL;
    if (w < PR11) {
        sumlog += expose_next_log_fast(w, 11.0L, PR11, a11, false);
        sumlog += expose_next_log_fast(w, 13.0L, PR13, a13, false);
        sumlog += expose_next_log_fast(w, 17.0L, PR17, a17, false);
        sumlog += expose_next_log_fast(w, 19.0L, PR19, a19, false);
        sumlog += expose_next_log_fast(w, 23.0L, PR23, a23, false);
        return expl(sumlog);
    }
    if (a11) sumlog += logl(11.0L - 2.0L); // *9

    // --- up to p=29 ---
    static constexpr long double PR29 = 29.0L*PR23;
    const bool a29 = (n % 29ULL) != 0ULL;
    if (w < PR13) {
        sumlog += expose_next_log_fast(w, 13.0L, PR13, a13, false);
        sumlog += expose_next_log_fast(w, 17.0L, PR17, a17, false);
        sumlog += expose_next_log_fast(w, 19.0L, PR19, a19, false);
        sumlog += expose_next_log_fast(w, 23.0L, PR23, a23, false);
        sumlog += expose_next_log_fast(w, 29.0L, PR29, a29, false);
        return expl(sumlog);
    }
    if (a13) sumlog += logl(13.0L - 2.0L); // *11

    // --- up to p=31 ---
    static constexpr long double PR31 = 31.0L*PR29;
    const bool a31 = (n % 31ULL) != 0ULL;
    if (w < PR17) {
        sumlog += expose_next_log_fast(w, 17.0L, PR17, a17, false);
        sumlog += expose_next_log_fast(w, 19.0L, PR19, a19, false);
        sumlog += expose_next_log_fast(w, 23.0L, PR23, a23, false);
        sumlog += expose_next_log_fast(w, 29.0L, PR29, a29, false);
        sumlog += expose_next_log_fast(w, 31.0L, PR31, a31, false);
        return expl(sumlog);
    }
    if (a17) sumlog += logl(17.0L - 2.0L); // *15

    // --- up to p=37 ---
    static constexpr long double PR37 = 37.0L*PR31;
    const bool a37 = (n % 37ULL) != 0ULL;
    if (w < PR19) {
        sumlog += expose_next_log_fast(w, 19.0L, PR19, a19, false);
        sumlog += expose_next_log_fast(w, 23.0L, PR23, a23, false);
        sumlog += expose_next_log_fast(w, 29.0L, PR29, a29, false);
        sumlog += expose_next_log_fast(w, 31.0L, PR31, a31, false);
        sumlog += expose_next_log_fast(w, 37.0L, PR37, a37, false);
        return expl(sumlog);
    }
    if (a19) sumlog += logl(19.0L - 2.0L); // *17

    // --- up to p=41,43,47,53 (likely never reached, but cheap if we do) ---
    static constexpr long double PR41 = 41.0L*PR37;
    static constexpr long double PR43 = 43.0L*PR41;
    static constexpr long double PR47 = 47.0L*PR43;
    static constexpr long double PR53 = 53.0L*PR47;

    const bool a41 = (n % 41ULL) != 0ULL;
    const bool a43 = (n % 43ULL) != 0ULL;
    const bool a47 = (n % 47ULL) != 0ULL;
    const bool a53 = (n % 53ULL) != 0ULL;

    if (w < PR23) {
        sumlog += expose_next_log_fast(w, 23.0L, PR23, a23, false);
        sumlog += expose_next_log_fast(w, 29.0L, PR29, a29, false);
        sumlog += expose_next_log_fast(w, 31.0L, PR31, a31, false);
        sumlog += expose_next_log_fast(w, 37.0L, PR37, a37, false);
        sumlog += expose_next_log_fast(w, 41.0L, PR41, a41, false);
        return expl(sumlog);
    }
    if (a23) sumlog += logl(21.0L); // 23-2

    if (w < PR29) {
        sumlog += expose_next_log_fast(w, 29.0L, PR29, a29, false);
        sumlog += expose_next_log_fast(w, 31.0L, PR31, a31, false);
        sumlog += expose_next_log_fast(w, 37.0L, PR37, a37, false);
        sumlog += expose_next_log_fast(w, 41.0L, PR41, a41, false);
        sumlog += expose_next_log_fast(w, 43.0L, PR43, a43, false);
        return expl(sumlog);
    }
    if (a29) sumlog += logl(27.0L); // 29-2

    if (w < PR31) {
        sumlog += expose_next_log_fast(w, 31.0L, PR31, a31, false);
        sumlog += expose_next_log_fast(w, 37.0L, PR37, a37, false);
        sumlog += expose_next_log_fast(w, 41.0L, PR41, a41, false);
        sumlog += expose_next_log_fast(w, 43.0L, PR43, a43, false);
        sumlog += expose_next_log_fast(w, 47.0L, PR47, a47, false);
        return expl(sumlog);
    }

    // Tail (practically negligible, but harmless if reached)
    sumlog += expose_next_log_fast(w, 37.0L, PR37, a37, false);
    sumlog += expose_next_log_fast(w, 41.0L, PR41, a41, false);
    sumlog += expose_next_log_fast(w, 43.0L, PR43, a43, false);
    sumlog += expose_next_log_fast(w, 47.0L, PR47, a47, false);
    sumlog += expose_next_log_fast(w, 53.0L, PR53, a53, false);
    return expl(sumlog);
}

/*
static inline long double effective_small_prime_deficit(const std::uint64_t n, const long double w)
{
    // n should reflect small-prime divisibility of the center.
    if (n < 3ULL) {
        return 0.0L;
    }

    long double r = (n%3ULL == 0ULL) ? (1ULL/3ULL) : 1ULL;  // accumulate ∏(p-2) only when p ∤ center

    // ---- up to p=5 ----
    const bool aligns_next5 = (n % 5ULL) != 0ULL;
    const long double PR5 = 3.0L*5.0L;
    if (w < PR5) {
        return r * expose_next(w, aligns_next5 && (w >= 5.0L), 5.0L, PR5);
    }
    if (aligns_next5) {
        r *= 3.0L;
    }

    // ---- up to p=7 ----
    const bool aligns_next7 = (n % 7ULL) != 0ULL;
    const long double PR7 = 7.0L*PR5;
    // ---- up to p=11 ----
    const bool aligns_next11 = (n % 11ULL) != 0ULL;
    const long double PR11 = 11.0L*PR7;
    // ---- up to p=13 ----
    const bool aligns_next13 = (n % 13ULL) != 0ULL;
    const long double PR13 = 13.0L*PR11;

    // ---- up to p=17 ----
    const bool aligns_next17 = (n % 17ULL) != 0ULL;
    const long double PR17 = 17.0L*PR13;
    // ---- up to p=19 ----
    const bool aligns_next19 = (n % 19ULL) != 0ULL;
    const long double PR19 = 19.0L*PR17;
    if (w < PR7) {
        return r * expose_next(w, aligns_next7, 7.0L, PR7)
            * expose_next(w, aligns_next11, 11.0L, PR11)
            * expose_next(w, aligns_next13, 13.0L, PR13)
            * expose_next(w, aligns_next17&&(w >= 17.0L), 17.0L, PR17)
            * expose_next(w, aligns_next19&&(w >= 19.0L), 19.0L, PR19);
    }
    if (aligns_next7) {
        r *= 5.0L;
    }

    // ---- up to p=23 ----
    const bool aligns_next23 = (n % 23ULL) != 0ULL;
    const long double PR23 = 23.0L*PR19;
    if (w < PR11) {
        return r * expose_next(w, aligns_next11, 11.0L, PR11)
            * expose_next(w, aligns_next13, 13.0L, PR13)
            * expose_next(w, aligns_next17, 17.0L, PR17)
            * expose_next(w, aligns_next19, 19.0L, PR19)
            * expose_next(w, aligns_next23, 23.0L, PR23);
    }
    if (aligns_next11) {
        r *= 9.0L;
    }

    // ---- up to p=29 ----
    const bool aligns_next29 = (n % 29ULL) != 0ULL;
    const long double PR29 = 29.0L*PR23;
    if (w < PR13) {
        return r * expose_next(w, aligns_next13, 13.0L, PR13)
            * expose_next(w, aligns_next17, 17.0L, PR17)
            * expose_next(w, aligns_next19, 19.0L, PR19)
            * expose_next(w, aligns_next23, 23.0L, PR23)
            * expose_next(w, aligns_next29, 29.0L, PR29);
    }
    if (aligns_next13) {
        r *= 11.0L;
    }

    // ---- up to p=31 ----
    const bool aligns_next31 = (n % 31ULL) != 0ULL;
    const long double PR31 = 31.0L*PR29;
    if (w < PR17) {
        return r * expose_next(w, aligns_next17, 17.0L, PR17)
            * expose_next(w, aligns_next19, 19.0L, PR19)
            * expose_next(w, aligns_next23, 23.0L, PR23)
            * expose_next(w, aligns_next29, 29.0L, PR29)
            * expose_next(w, aligns_next31, 31.0L, PR31);
    }
    if (aligns_next17) {
        r *= 15.0L;
    }

    // ---- up to p=37 ----
    const bool aligns_next37 = (n % 37ULL) != 0ULL;
    const long double PR37 = 37.0L*PR31;
    if (w < PR19) {
        return r * expose_next(w, aligns_next19, 19.0L, PR19)
            * expose_next(w, aligns_next23, 23.0L, PR23)
            * expose_next(w, aligns_next29, 29.0L, PR29)
            * expose_next(w, aligns_next31, 31.0L, PR31)
            * expose_next(w, aligns_next37, 37.0L, PR37);
    }
    if (aligns_next19) {
        r *= 17.0L;
    }

    // ---- up to p=41 ----
    const bool aligns_next41 = (n % 41ULL) != 0ULL;
    const long double PR41 = 41.0L*PR37;
    if (w < PR23) {
        return r * expose_next(w, aligns_next23, 23.0L, PR23)
            * expose_next(w, aligns_next29, 29.0L, PR29)
            * expose_next(w, aligns_next31, 31.0L, PR31)
            * expose_next(w, aligns_next37, 37.0L, PR37)
            * expose_next(w, aligns_next41, 41.0L, PR41);
    }
    if (aligns_next23) {
        r *= 21.0L;
    }

    // ---- up to p=43 ----
    const bool aligns_next43 = (n % 43ULL) != 0ULL;
    const long double PR43 = 43.0L*PR41;
    if (w < PR29) {
        return r * expose_next(w, aligns_next29, 29.0L, PR29)
            * expose_next(w, aligns_next31, 31.0L, PR31)
            * expose_next(w, aligns_next37, 37.0L, PR37)
            * expose_next(w, aligns_next41, 41.0L, PR41)
            * expose_next(w, aligns_next43, 43.0L, PR43);
    }
    if (aligns_next29) {
        r *= 27.0L;
    }

    // ---- up to p=47 ----
    const bool aligns_next47 = (n % 47ULL) != 0ULL;
    const long double PR47 = 47.0L*PR43;
    if (w < PR31) {
        return r * expose_next(w, aligns_next31, 31.0L, PR31)
            * expose_next(w, aligns_next37, 37.0L, PR37)
            * expose_next(w, aligns_next41, 41.0L, PR41)
            * expose_next(w, aligns_next43, 43.0L, PR43)
            * expose_next(w, aligns_next47, 47.0L, PR47);
    }

    // beyond PR37, we'll have the wrong answer.   But that will well exceed the language's precision for a short interval anyway.
    const bool aligns_next53 = (n % 53ULL) != 0ULL;
    const long double PR53 = 53.0L*PR47;
    return r * expose_next(w, aligns_next37, 37.0L, PR37)
        * expose_next(w, aligns_next41, 41.0L, PR41)
        * expose_next(w, aligns_next43, 43.0L, PR43)
        * expose_next(w, aligns_next47, 47.0L, PR47)
        * expose_next(w, aligns_next53, 53.0L, PR53);
}
*/

static inline void fprintf_both(FILE* a, FILE* b, const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vfprintf_both(a, b, fmt, ap);
    va_end(ap);
}

static inline void fputs_both(const char* s, FILE* a, FILE* b) {
    if (a) {
        std::fputs(s, a);
    }
    if (b) {
        std::fputs(s, b);
    }
}
 
static void printHeaderFull(FILE *out1,FILE *out2,bool useLegacy,Model model) {
    fputs_both(
        useLegacy
            ?(model == Model::Empirical
                ?"DECADE,MIN AT,MIN,MAX AT,MAX,n_0,C_min,n_1,C_max,n_geom,<COUNT>,C_avg\n"
                :"DECADE,MIN AT,MIN,MAX AT,MAX,n_0,Cpred_min,n_1,Cpred_max,N_geom,<COUNT>,Cpred_avg,HLCorr\n")
            :(model == Model::Empirical
                ?"FIRST,LAST,START,minAt,G(minAt),maxAt,G(maxAt),n_0,C_min(n_0),n_1,C_max(n_1),n_geom,<COUNT>,C_avg\n"
                :"FIRST,LAST,START,minAt*,Gpred(minAt*),maxAt*,Gpred(maxAt*),n_0*,Cpred_min(n_0*),n_1*,Cpred_max(n_1*),n_geom,<COUNT>*,Cpred_avg,n_alignMax,c_alignMax,n_alignMin,c_alignMin,n_cBound,c_cBound\n"),
        out1, out2);
}

static void printHeaderRaw(FILE *out1,FILE *out2,Model model) {
    fputs_both(
        (model == Model::Empirical
            ?"FIRST,LAST,START,minAt,G(minAt),maxAt,G(maxAt),n_geom,<COUNT>\n"
            :"FIRST,LAST,START,minAt*,Gpred(minAt*),maxAt*,Gpred(maxAt*),n_geom,<COUNT>*\n"),
        out1, out2);
}

static void printHeaderNorm(FILE *out1,FILE *out2,Model model) {
    fputs_both(
        (model == Model::Empirical
            ?"FIRST,LAST,START,n_0,C_min(n_0),n_1,C_max(n_1),n_geom,<COUNT>,C_avg\n"
            :"FIRST,LAST,START,n_0*,Cpred_min(n_0*),n_1*,Cpred_max(n_1*),n_geom,Cpred_avg,n_align,C_align\n"),
        out1, out2);
}

static void printHeaderCps(FILE *out1,bool legacy) {
    if(out1) {
        std::fputs(
            (legacy
                ?"Dec.,n_0,Cmin,Cminus,Cmin-Cminus,CminusAsym,Cmin-CminusAsym\n"
                :"n,C(n),Cminus(n),C(n)-Cminus(n),CminusAsym(n),C(n)-CminusAsym(n),preMertens,preMertensAsymp,alpha(n)\n"),
        out1);
    }
}

static void printHeaderCpsSummary(FILE *out1,FILE *out2,Model model) {
    if(model == Model::Empirical) {
        fputs_both(
            "FIRST,LAST,Alpha,PreMertens,Mertens,DeltaMertens,n_5precent,NzeroStat,EtaStat,MertensAsymp,DeltaMertensAsymp,NzeroStatAsymp,EtaStatAsymp\n",
        out1,out2);
    }
}

void GBRange::printHeaders() {
    for(auto &w : windows) {
        printHeaderFull(w->dec.out,w->dec.trace,(compat_ver == CompatVer::V015),model);
        printHeaderFull(w->prim.out,w->prim.trace,false,model);
        printHeaderRaw(w->dec.raw,w->prim.raw,model);
        printHeaderNorm(w->dec.norm,w->prim.norm,model);
        printHeaderCps(w->dec.cps,(compat_ver == CompatVer::V015));
        printHeaderCps(w->prim.cps,false);
    }
}

void GBRange::printCpsSummaryHeaders() {
    printHeaderCpsSummary(decAgg.cps_summary,primAgg.cps_summary,model);
}

std::uint64_t GBRange::decReset(std::uint64_t n_start) {
    int need_reset = 0;
    for(auto &w : windows) {
        if(w->is_dec_active()) {
            need_reset = 1;
            w->dec.summary.reset();
        }
    }
    if(! need_reset) {
        return n_start;
    }
    decAgg.reset(n_start, (compat_ver == CompatVer::V015));
    if(decAgg.left >= decAgg.n_end) {
        dec_close();
    }
    // Init interpolators for new aggregate range
    if (model == Model::HLA && compat_ver != CompatVer::V015) {
        for(auto &w : windows) {
            if(w->is_dec_active()) {
                w->dec.summary.hlCorrEstimate.init(decAgg.left, decAgg.right, &decState);
            }
        }
    }
    return decAgg.left;
}

std::uint64_t GBRange::primReset(std::uint64_t n_start) {
    int need_reset = 0;
    for(auto &w : windows) {
        if(w->is_prim_active()) {
            need_reset = 1;
            w->prim.summary.reset();
        }
    }
    if(! need_reset) {
        return n_start;
    }
    primAgg.reset(n_start,false);
    if(primAgg.left >= primAgg.n_end) {
        prim_close();
    }
    // Init interpolators for new aggregate range
    if (model == Model::HLA && compat_ver != CompatVer::V015) {
        for(auto &w : windows) {
            if(w->is_prim_active()) {
                w->prim.summary.hlCorrEstimate.init(primAgg.left, primAgg.right, &primState);
            }
        }
    }
    return primAgg.left;
}

void GBRange::calcAverage(GBWindow &w,GBLongInterval &interval, GBAggregate &agg,  bool useLegacy) {
    GBLongIntervalSummary &summary = interval.summary;
    summary.pairCountAvg = summary.pairCountTotal / (agg.right - agg.left);
    summary.cAvg = summary.pairCountTotalNorm / (agg.right - agg.left);
    if(model != Model::HLA) {
        return;
    }
    if(compat_ver != CompatVer::V015  && summary.useHLCorrInst) {
        summary.applyHLCorr(agg.minCalc, agg.maxCalc, agg.minNormCalc, agg.maxNormCalc, agg.alignNormCalc );
    }
    else if(! summary.useHLCorrInst) {
        const std::uint64_t n_geom_odd  = (useLegacy ? ((1ULL | (std::uint64_t)floorl(agg.n_geom))) : minPrefOdd(agg.n_geom,agg.right - 1));
        const std::uint64_t delta_odd = w.computeDelta(n_geom_odd);
        const std::uint64_t n_geom_even  = (compat_ver == CompatVer::V015 ? (1ULL + n_geom_odd) : maxPrefEven(agg.n_geom,agg.left));
        const std::uint64_t delta_even = w.computeDelta(n_geom_even);
        summary.applyHLCorr(n_geom_even, delta_even, n_geom_odd, delta_odd,
            agg.evenCalc, agg.oddCalc, agg.minCalc, agg.maxCalc, agg.minNormCalc, agg.maxNormCalc, agg.alignNormCalc );
    }
}

void GBRange::outputFull(GBAggregate &agg,GBLongInterval &interval,bool useLegacy) {
    if(! (interval.out || interval.trace)) {
        return;
    }
    GBLongIntervalSummary &summary = interval.summary;
    if(! useLegacy) {
        if (model == Model::Empirical) {
            fprintf_both(interval.out,interval.trace,
                "%" PRIu64 ",%" PRIu64 ",%s,%" PRIu64 ",%.0Lf,%" PRIu64 ",%.0Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.8Lf,%.0Lf,%.6Lf,%.9Lf\n",
                agg.left, agg.right -1,
                agg.label.c_str(),
                summary.pairCountMinima.n_last, summary.pairCountMinima.c_last,
                summary.pairCountMaxima.n_first, summary.pairCountMaxima.c_first,
                summary.cMinima.n_last, summary.cMinima.c_last,
                summary.cMaxima.n_first, summary.cMaxima.c_first,
                agg.n_geom,
                summary.pairCountAvg,
                summary.cAvg
            );
            return;
        }
        fprintf_both(interval.out,interval.trace,
            "%" PRIu64 ",%" PRIu64 ",%s,%" PRIu64 ",%.3Lf,%" PRIu64 ",%.3Lf,%" PRIu64
                ",%.6Lf,%" PRIu64 ",%.8Lf,%.0Lf,%.6Lf,%.9Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf\n",
            agg.left, agg.right -1,
            agg.label.c_str(),
            summary.pairCountMinima.n_last, summary.pairCountMinima.c_last,
            summary.pairCountMaxima.n_first, summary.pairCountMaxima.c_first,
            summary.cMinima.n_last, summary.cMinima.c_last,
            summary.cMaxima.n_first, summary.cMaxima.c_first,
            agg.n_geom,
            summary.pairCountAvg,
            summary.cAvg,
            summary.pairCountAlignMaxima.n_last,
            summary.pairCountAlignMaxima.c_last,
            summary.alignMinima.n_last,
            summary.alignMinima.c_last,
            summary.alignNoHLCorrMinima.n_last,
            summary.alignNoHLCorrMinima.c_last
        );
        return;
    }
    if(model == Model::Empirical) {
        fprintf_both(interval.out,interval.trace,
            "%s,%" PRIu64 ",%.0Lf,%" PRIu64 ",%.0Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf,%.6Lf\n",
            agg.label.c_str(),
            summary.pairCountMinima.n_first, summary.pairCountMinima.c_first,
            summary.pairCountMaxima.n_first, summary.pairCountMaxima.c_first,
            summary.cMinima.n_first, summary.cMinima.c_first,
            summary.cMaxima.n_first, summary.cMaxima.c_first,
            ((std::uint64_t)floorl(agg.n_geom)) | (agg.n_geom >= 10L ? 1ULL : 0ULL),
            summary.pairCountAvg,
            summary.cAvg
        );
        return;
    }
    fprintf_both(interval.out,interval.trace,
        "%s,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.8Lf,%" PRIu64 ",%.8Lf,%" PRIu64 ",%.8Lf,%.8Lf,%.8Lf\n",
        agg.label.c_str(),
        summary.pairCountMinima.n_first, summary.pairCountMinima.c_first,
        summary.pairCountMaxima.n_first, summary.pairCountMaxima.c_first,
        summary.cMinima.n_first, summary.cMinima.c_first,
        summary.cMaxima.n_first, summary.cMaxima.c_first,
        ((std::uint64_t)floorl(agg.n_geom)) | (agg.n_geom >= 10L ? 1ULL : 0ULL),
        summary.pairCountAvg,
        summary.cAvg,
        summary.hlCorrAvg
    );
}

void GBRange::outputRaw(GBAggregate &agg,GBLongInterval &interval) {
    if(interval.raw) {
        GBLongIntervalSummary &summary = interval.summary;
        std::fprintf(interval.raw,
            (model == Model::Empirical)
                ? "%" PRIu64 ",%" PRIu64 ",%s,%" PRIu64 ",%.0Lf,%" PRIu64 ",%.0Lf,%.0Lf,%.6Lf\n"
                : "%" PRIu64 ",%" PRIu64 ",%s,%" PRIu64 ",%.3Lf,%" PRIu64 ",%.3Lf,%.0Lf,%.6Lf\n",
            agg.left, agg.right -1,
            agg.label.c_str(),
            summary.pairCountMinima.n_last, summary.pairCountMinima.c_last,
            summary.pairCountMaxima.n_first, summary.pairCountMaxima.c_first,
            agg.n_geom,
            summary.pairCountAvg
        );
    }
}

void GBRange::outputNorm(GBAggregate &agg,GBLongInterval &interval) {
    if(interval.norm) {
        GBLongIntervalSummary &summary = interval.summary;
        if (model == Model::Empirical) {
            std::fprintf(interval.norm,
                "%" PRIu64 ",%" PRIu64 ",%s,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.8Lf,%.0Lf,%.9Lf\n",
                agg.left, agg.right -1,
                agg.label.c_str(),
                summary.cMinima.n_first, summary.cMinima.c_first,
                summary.cMaxima.n_last, summary.cMaxima.c_last,
                agg.n_geom,
                summary.cAvg );
        }
        else {
            std::fprintf(interval.norm,
                "%" PRIu64 ",%" PRIu64 ",%s,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.8Lf,%.0Lf,%.9Lf,%" PRIu64 ",%.3Lf,%" PRIu64 ",%.3Lf\n",
                agg.left, agg.right -1,
                agg.label.c_str(),
                summary.cMinima.n_first, summary.cMinima.c_first,
                summary.cMaxima.n_last, summary.cMaxima.c_last,
                agg.n_geom,
                summary.cAvg,
                summary.alignMinima.n_last,
                summary.alignMinima.c_last,
                summary.alignNoHLCorrMinima.n_last,
                summary.alignNoHLCorrMinima.c_last );
        }
    }
}

void GBRange::decOutputCpsSummary(GBWindow &w) {
    if(! decAgg.cps_summary) {
        return;
    }
    //if(w->preMertens > 1 && w->dec.nstar > 1 && w->dec.deltaMertens > 0.0L) {
        std::fprintf(decAgg.cps_summary,
            "%" PRIu64 ",%" PRIu64 ",%.12Lg,%" PRIu64 ",%" PRIu64 ",%.6Lf,%" PRIu64 ",%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf\n",
            decAgg.n_start, decAgg.n_end,
            w.alpha,
            w.preMertens,w.dec.nstar,w.dec.deltaMertens,
            w.n_5percent,w.nzeroStat,w.etaStat,
            w.dec.nstarAsymp,w.dec.deltaMertensAsymp,w.nzeroStatAsymp,w.etaStatAsymp );
    //}
}

int GBRange::decInputCpsSummary(const char* filename) {
    FILE* file = std::fopen(filename, "r");
    if (!file) {
        std::fprintf(stderr, "Error: Cannot open file %s for reading\n", filename);
        return -1;
    }
    
    char line[1024];
    int lineNum = 0;
    int updatedCount = 0;
    
    // Skip header line if it exists
    if (std::fgets(line, sizeof(line), file)) {
        lineNum++;
        // Check if this looks like a header (contains "FIRST" or "Alpha")
        if (strstr(line, "FIRST") || strstr(line, "Alpha")) {
            // This is a header, continue to next line
        } else {
            // This is data, rewind to beginning
            std::rewind(file);
            lineNum = 0;
        }
    }
    
    while (std::fgets(line, sizeof(line), file)) {
        lineNum++;
        
        // Skip empty lines
        if (line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        
        // Parse the line: n_start, n_end, alpha, preMertens, nstar, deltaMertens, n_5percent, nzeroStat, etaStat, nstarAsymp, deltaMertensAsymp, nzeroStatAsymp, etaStatAsymp
        std::uint64_t n_start, n_end, preMertens, nstar, n_5percent, nzeroStat, nstarAsymp, nzeroStatAsymp;
        long double alpha, deltaMertens, etaStat, deltaMertensAsymp, etaStatAsymp;
        
        int parsed = std::sscanf(line, "%" SCNu64 ",%" SCNu64 ",%Lf,%" SCNu64 ",%" SCNu64 ",%Lf,%" SCNu64 ",%" SCNu64 ",%Lf,%" SCNu64 ",%Lf,%" SCNu64 ",%Lf",
                                &n_start, &n_end, &alpha, &preMertens, &nstar, &deltaMertens, &n_5percent, &nzeroStat, &etaStat, &nstarAsymp, &deltaMertensAsymp, &nzeroStatAsymp, &etaStatAsymp);
        
        if (parsed != 13) {
            std::fprintf(stderr, "Warning: Skipping malformed line %d in %s\n", lineNum, filename);
            continue;
        }
        
        // Find the window with matching alpha value
        bool found = false;
        for (auto &w : windows) {
            // Use a small epsilon for floating point comparison
            if (std::abs(w->alpha - alpha) < 1e-12L) {
                // Update the window values
                w->preMertens = preMertens;
                w->dec.nstar = nstar;
                w->dec.deltaMertens = deltaMertens;
                w->dec.nstarAsymp = nstarAsymp;
                w->dec.deltaMertensAsymp = deltaMertensAsymp;
                w->n_5percent = n_5percent;
                w->nzeroStat = nzeroStat;
                w->nzeroStatAsymp = nzeroStatAsymp;
                w->etaStat = etaStat;
                w->etaStatAsymp = etaStatAsymp;
                
                updatedCount++;
                found = true;
                break;
            }
        }
        decAgg.n_start = n_start;
        if (!found) {
            std::fprintf(stderr, "Warning: No window found with alpha=%.12Lg on line %d\n", alpha, lineNum);
        }
    }
    
    std::fclose(file);
    
    if (updatedCount == 0) {
        std::fprintf(stderr, "Warning: No windows were updated from file %s\n", filename);
        return 1;
    }
    
    // std::fprintf(stderr, "Successfully updated %d windows from file %s\n", updatedCount, filename);
    return 0;
}

void GBRange::primOutputCpsSummary(GBWindow &w) {
    if(! primAgg.cps_summary) {
        return;
    }
    std::fprintf(primAgg.cps_summary,
        "%" PRIu64 ",%" PRIu64 ",%.12Lg,%" PRIu64 ",%" PRIu64 ",%.6Lf,%" PRIu64 ",%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf\n",
        primAgg.n_start, primAgg.n_end,
        w.alpha,
        w.preMertens,w.prim.nstar,w.prim.deltaMertens,
        w.n_5percent,w.nzeroStat,w.etaStat,
        w.prim.nstarAsymp,w.prim.deltaMertensAsymp,w.nzeroStatAsymp,w.etaStatAsymp );
}

int GBRange::primInputCpsSummary(const char* filename) {
    FILE* file = std::fopen(filename, "r");
    if (!file) {
        std::fprintf(stderr, "Error: Cannot open file %s for reading\n", filename);
        return -1;
    }
    
    char line[1024];
    int lineNum = 0;
    int updatedCount = 0;
    
    // Skip header line if it exists
    if (std::fgets(line, sizeof(line), file)) {
        lineNum++;
        // Check if this looks like a header (contains "FIRST" or "Alpha")
        if (strstr(line, "FIRST") || strstr(line, "Alpha")) {
            // This is a header, continue to next line
        } else {
            // This is data, rewind to beginning
            std::rewind(file);
            lineNum = 0;
        }
    }
    
    while (std::fgets(line, sizeof(line), file)) {
        lineNum++;
        
        // Skip empty lines
        if (line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        
        // Parse the line: n_start, n_end, alpha, preMertens, nstar, deltaMertens, n_5percent, nzeroStat, etaStat, nstarAsymp, deltaMertensAsymp, nzeroStatAsymp, etaStatAsymp
        std::uint64_t n_start, n_end, preMertens, nstar, n_5percent, nzeroStat, nstarAsymp, nzeroStatAsymp;
        long double alpha, deltaMertens, etaStat, deltaMertensAsymp, etaStatAsymp;
        
        int parsed = std::sscanf(line, "%" SCNu64 ",%" SCNu64 ",%Lf,%" SCNu64 ",%" SCNu64 ",%Lf,%" SCNu64 ",%" SCNu64 ",%Lf,%" SCNu64 ",%Lf,%" SCNu64 ",%Lf",
            &n_start, &n_end, &alpha, &preMertens,
            &nstar, &deltaMertens, &n_5percent, &nzeroStat, &etaStat,
            &nstarAsymp, &deltaMertensAsymp, &nzeroStatAsymp, &etaStatAsymp
        );
        
        if (parsed != 13) {
            std::fprintf(stderr, "Warning: Skipping malformed line %d in %s\n", lineNum, filename);
            continue;
        }
        
        // Find the window with matching alpha value
        bool found = false;
        for (auto &w : windows) {
            // Use a small epsilon for floating point comparison
            if (std::abs(w->alpha - alpha) < 1e-12L) {
                // Implement preMertens inheritance logic for out-of-order processing
                // If we have a resume file, use the preMertens value from it (what actually happened)
                // Otherwise, initialize to n_start - 1 (assumes never negative in range)
                // The existing logic will handle updating it based on actual count behavior:
                // - 0 means most recent value is negative
                // - n_start - 1 means it was never negative in the range  
                // - >= n_start means it was negative but turned positive
                w->preMertens = preMertens;
                
                w->prim.nstar = nstar;
                w->prim.deltaMertens = deltaMertens;
                w->prim.nstarAsymp = nstarAsymp;
                w->prim.deltaMertensAsymp = deltaMertensAsymp;
                w->n_5percent = n_5percent;
                w->nzeroStat = nzeroStat;
                w->nzeroStatAsymp = nzeroStatAsymp;
                w->etaStat = etaStat;
                w->etaStatAsymp = etaStatAsymp;
                
                updatedCount++;
                found = true;
                break;
            }
        }
        primAgg.n_start = n_start;
        if (!found) {
            std::fprintf(stderr, "Warning: No window found with alpha=%.12Lg on line %d\n", alpha, lineNum);
        }
    }
    
    std::fclose(file);
    
    if (updatedCount == 0) {
        std::fprintf(stderr, "Warning: No windows were updated from file %s\n", filename);
        return 1;
    }
    
    // std::fprintf(stderr, "Successfully updated %d windows from file %s\n", updatedCount, filename);
    return 0;
}

int GBRange::addRow(
    GBWindow &w,
    std::uint64_t n,
    std::uint64_t delta,
    const long double logN,
    const long double logNlogN,
    std::uint64_t pc,
    long double twoSGB
) {
    GBLongIntervalSummary &prim_summary = w.prim.summary;
    GBLongIntervalSummary &dec_summary = w.dec.summary;

    const long double deltaL = (long double)delta;
    const long double denom = (includeTrivial ? 0.5L : 0.0L) + deltaL;
    const long double norm  = (denom > 0.0L) ? (logNlogN / denom) : 0.0L;

    if (norm < 0.0L) {
        std::fprintf(stderr, "HL-A: non-positive norm at %" PRIu64 "\n", n);
        return -1;
    }

    prim_summary.useHLCorrInst = dec_summary.useHLCorrInst = 0;

    if (model == Model::Empirical) {
        long double cminus = w.calcCminus(n,delta,logNlogN);
        long double cminusAsymp = w.calcCminusAsymp(logN);
        long double pairCount = (long double)pc;
        long double c_of_n = pairCount * norm;
        prim_summary.pairCountMinima.current = prim_summary.pairCount = dec_summary.pairCountMinima.current = dec_summary.pairCount = pairCount;
        prim_summary.cMinima.current = prim_summary.c_of_n = dec_summary.cMinima.current = dec_summary.c_of_n = c_of_n;
        w.checkCrossing(n,c_of_n <= cminus);
        w.checkCrossingAsymp(n,c_of_n <= cminusAsymp);
        w.updateN5percent(n,delta,logNlogN,c_of_n-cminus,c_of_n-cminusAsymp);
    }
    else { // HLA
        prim_summary.pairCount = dec_summary.pairCount = 0.0L;
        prim_summary.c_of_n = dec_summary.c_of_n = 0.0L;

        prim_summary.hlCorrAvg = dec_summary.hlCorrAvg = 1.0L;
        long double hlCorrAvg = 0.0L;
        long double pairCountAlign = 2.0L*effective_small_prime_deficit(n, sqrtl(2.0L*(long double)n));
        if(w.is_prim_active()) {
            if(compat_ver != CompatVer::V015) {
                prim_summary.useHLCorrInst = 1;
                // Use interpolated HLCorr for better accuracy
                prim_summary.hlCorrAvg = hlCorrAvg = prim_summary.hlCorrEstimate(n,delta);
                prim_summary.c_of_n = twoSGB*hlCorrAvg;
            }
            else if(primAgg.minor < 5) {
                prim_summary.useHLCorrInst = 1;
                prim_summary.hlCorrAvg = hlCorrAvg = hlcorr(n,delta);
                prim_summary.c_of_n = twoSGB*hlCorrAvg;
            }
            else {
                prim_summary.c_of_n = twoSGB;
            }
            if (pc) {
                prim_summary.pairCount  = (norm > 0.5L) ? (prim_summary.c_of_n / deltaL) : 1.0L;
                prim_summary.c_of_n = prim_summary.pairCount * norm;
                prim_summary.pairCountMinima.current = (norm > 0.5L)? twoSGB / deltaL : 1.0L;
            } else if (norm > 0.0L) {
                prim_summary.pairCount = prim_summary.c_of_n / norm;
                prim_summary.pairCountMinima.current = twoSGB / norm;
            }
            prim_summary.pairCountAlignMaxima.current = pairCountAlign;
            prim_summary.alignMinima.current = (norm > 0.0L && prim_summary.c_of_n > pairCountAlign * norm)
                ? (prim_summary.c_of_n - pairCountAlign * norm)
                : 0.0L;
            // Conservative bound: use raw values without HLCorr
            prim_summary.alignNoHLCorrMinima.current = (norm > 0.0L && twoSGB > pairCountAlign * norm)
                ? (twoSGB - pairCountAlign * norm)
                : 0.0L;
        }
        if(w.is_dec_active()) {
            if (compat_ver != CompatVer::V015) {
                dec_summary.useHLCorrInst  = 1;
                // Use interpolated HLCorr for better accuracy
                dec_summary.hlCorrAvg = hlCorrAvg = dec_summary.hlCorrEstimate(n,delta);
                dec_summary.c_of_n = twoSGB*hlCorrAvg;
            }
            else if (decAgg.base < 10) {
                dec_summary.useHLCorrInst  = 1;
                if(hlCorrAvg == 0.0L) {
                    hlCorrAvg = hlcorr(n,delta);
                } 
                dec_summary.hlCorrAvg = hlCorrAvg;
                dec_summary.c_of_n = twoSGB*hlCorrAvg;
            }
            else {
                dec_summary.c_of_n = twoSGB;
            }
            if (pc) {
                dec_summary.pairCount  = (norm > 0.5L) ? (dec_summary.c_of_n / deltaL) : 1.0L;
                dec_summary.c_of_n = dec_summary.pairCount * norm;
                dec_summary.pairCountMinima.current = (norm > 0.5L)? twoSGB / deltaL : 1.0L;
                if (hlCorrAvg != 0.0L && dec_summary.useHLCorrInst && compat_ver == CompatVer::V015) {
                    dec_summary.pairCountMinima.current *= hlCorrAvg;
                }
            } else if (norm > 0.0L) {
                dec_summary.pairCount = dec_summary.c_of_n / norm;
                dec_summary.pairCountMinima.current = twoSGB / norm;
                if (hlCorrAvg != 0.0L && dec_summary.useHLCorrInst && compat_ver == CompatVer::V015) {
                    dec_summary.pairCountMinima.current *= hlCorrAvg;
                }
            }
            dec_summary.pairCountAlignMaxima.current = pairCountAlign;
            dec_summary.alignMinima.current = (norm > 0.0L && dec_summary.c_of_n > pairCountAlign * norm)
                ? (dec_summary.c_of_n - pairCountAlign * norm)
                : 0.0L;
            // Conservative bound: use raw values without HLCorr
            dec_summary.alignNoHLCorrMinima.current = (norm > 0.0L && twoSGB > pairCountAlign * norm)
                ? (twoSGB - pairCountAlign * norm)
                : 0.0L;
        }
    }
    aggregate(w, n, delta, w.calcCminus(n,delta,logNlogN), w.calcCminusAsymp(logN));
    return 0;
}

static inline void close(FILE *&out) {
    if(out && out != stdout) {
        std::fclose(out);
        out = nullptr;
    }
}

void GBRange::prim_close() {
    primAgg.right = 0;
    for(auto &w : windows) {
        w->closeInterval(w->prim);
        primOutputCpsSummary(*w);
    }
    close(primAgg.cps_summary);
}

void GBRange::dec_close() {
    decAgg.right = 0;
    for(auto &w : windows) {
        w->closeInterval(w->dec);
        decOutputCpsSummary(*w);
    }
    close(decAgg.cps_summary);
}

int GBRange::processRows() {
    const std::uint64_t* current = primeArray;
    bool prim_is_active = false;
    bool dec_is_active = false;
    
    // Also check for CPS summary outputs at the aggregate level
    for(auto &w : windows) {
        if(w->is_dec_active()) {
            dec_is_active = true;
        }
        else if(decAgg.cps_summary) {
            w->dec.out = std::fopen("/dev/null", "w");
            w->dec.active = true;
            dec_is_active = true;
        }
        if(w->is_prim_active()) {
            prim_is_active = true;
        }
        else if(primAgg.cps_summary) {
            w->prim.out = std::fopen("/dev/null", "w");
            w->prim.active = true;
            prim_is_active = true;
        }
    }

    std::uint64_t n_start, n_end;
    if (prim_is_active && dec_is_active) {
        // Both active - use the wider range
        n_start = (decAgg.left < primAgg.left) ? decAgg.left : primAgg.left;
        n_end = (decAgg.n_end > primAgg.n_end) ? decAgg.n_end : primAgg.n_end;
    } else if (prim_is_active) {
        // Only primorial active
        n_start = primAgg.left;
        n_end = primAgg.n_end;
    } else if (dec_is_active) {
        // Only decade active
        n_start = decAgg.left;
        n_end = decAgg.n_end;
    } else {
        // Neither active - this is an error condition
        std::fprintf(stderr, "Error: No output streams configured. At least one of decade or primorial output must be specified.\n");
        return -1;
    }
    for(auto &w : windows) {
        w->preMertens = w->preMertensAsymp = n_start - 1;
    }
    // Prescan now happens in decReset() and primReset() for each aggregate block
    std::vector<GBWindow*> dec_windows_to_prescan; 
    std::vector<GBWindow*> prim_windows_to_prescan;

    if(model == Model::HLA && compat_ver != CompatVer::V015) {
        for(auto & w : windows) {
            if(w->is_dec_active()) {
                w->dec.summary.hlCorrEstimate.init(decAgg.left, decAgg.right, &decState);
                dec_windows_to_prescan.push_back(w.get());
            }
            if(w->is_prim_active()) {
                w->prim.summary.hlCorrEstimate.init(primAgg.left, primAgg.right, &primState);
                if(model == Model::HLA && compat_ver != CompatVer::V015) {
                    prim_windows_to_prescan.push_back(w.get());
                }
            }
            prim_windows_to_prescan.push_back(w.get());
        }
    }
    for (std::uint64_t n = n_start; n < n_end; ) {
        if(model == Model::HLA && compat_ver != CompatVer::V015) {
            if(! dec_windows_to_prescan.empty()) {
                for(std::uint64_t i = n,next_n; i < n_end; i = next_n) {
                    next_n = n_end;
                    for(auto &w : dec_windows_to_prescan) {
                        w->dec.summary.hlCorrEstimate.prescan(i, next_n,[&w](long double n) { return w->computeDelta(n); });
                    }
                }
                dec_windows_to_prescan.clear();
            }
            if(! prim_windows_to_prescan.empty()) {
                for(std::uint64_t i = n,next_n; i < n_end; i = next_n) {
                    next_n = n_end;
                    for(auto &w : prim_windows_to_prescan) {
                        w->prim.summary.hlCorrEstimate.prescan(i, next_n,[&w](long double n) { return w->computeDelta(n); });
                    }
                }
                prim_windows_to_prescan.clear();
            }
        }
        const long double twoSGB_n = (model == Model::Empirical ? 0.0L : (long double)twoSGB(n, primeArray, primeArrayEndlen));
        if (twoSGB_n < 0.0L) {
            std::fprintf(stderr, "Failed HL-A prediction at %" PRIu64 "\n", n);
            return -1;
        }
        int need_trivial = includeTrivial;
        std::uint64_t pc = 0;
        // we use pointers here, so we know where we left off.
        const std::uint64_t *lo = nullptr;
        const std::uint64_t *hi = nullptr;
        // Here is where we add a loop if we needed to support multiple windows,
        // as twoSGB_n is alpha independant and does not need to be recomputed.
        bool need_decReset = false;
        bool need_primReset = false;
        long double logN = 0.0L;
        long double logNlogN = 0.0L;
        long double eulerCapAlpha = 0.0L;
        for(auto & w : windows) {
            std::uint64_t delta = w->computeDelta(n,eulerCapAlpha);
            if (delta == ~0ULL) {
                return 2;
            }
            if (model == Model::Empirical) {
                std::uint64_t _pc = countRangedPairsIter(n, n - delta - 1, &current, primeArray, primeArrayEndend, &lo, &hi);
                if (_pc == ~0ULL) {
                    std::fprintf(stderr, "Failed to count pairs at %" PRIu64 "\n", n);
                    return -1;
                }
                if (need_trivial && current > primeArray && current < primeArrayEndend && current[-1] == n) {
                    pc += 1ULL+_pc;
                    need_trivial = 0;
                }
                else {
                    pc += _pc;
                }
            }
            else if (need_trivial) {
                need_trivial = 0;
                // simply called to position the current pointer
                countRangedPairs(n, n, &current, primeArray, primeArrayEndend);
                if (current > primeArray && current < primeArrayEndend && current[-1] == n) {
                    pc = 1;
                }
            }
            if(logN == 0.0L) {
                logN = logl((long double)n); 
                logNlogN = logN*logN;
            }
            int retval = addRow(*w, n, delta, logN, logNlogN, pc, twoSGB_n);
            if(retval != 0) {
                return retval;
            }
        }
        n++;
        for(auto & w : windows) {
            if (w->is_dec_active() && n == decAgg.right) {
                calcAverage(*w,w->dec,decAgg,(compat_ver == CompatVer::V015));
                outputFull(decAgg,w->dec,(compat_ver == CompatVer::V015));
                outputRaw(decAgg,w->dec);
                outputNorm(decAgg,w->dec);
                w->dec.summary.outputCps(w->dec,w->alpha_n,(compat_ver == CompatVer::V015)?decAgg.decade:-1,n_start,w->preMertens,w->preMertensAsymp);
                need_decReset = true;
                if(model == Model::HLA && compat_ver != CompatVer::V015) {
                    dec_windows_to_prescan.push_back(w.get());
                }
            }
            if (w->is_prim_active() && n == primAgg.right) {
                calcAverage(*w,w->prim,primAgg,false);
                outputFull(primAgg,w->prim,false);
                outputRaw(primAgg,w->prim);
                outputNorm(primAgg,w->prim);
                w->prim.summary.outputCps(w->prim,w->alpha_n,-1,n_start,w->preMertens,w->preMertensAsymp);
                need_primReset = true;
                if(model == Model::HLA && compat_ver != CompatVer::V015) {
                    prim_windows_to_prescan.push_back(w.get());
                }
            }
        }
        if(need_decReset) {
            decReset(decAgg.right);
        }
        if(need_primReset) {
            primReset(primAgg.right);
        }
    }
    return 0;
}


