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
                :"FIRST,LAST,START,minAt*,Gpred(minAt*),maxAt*,Gpred(maxAt*),n_0*,Cpred_min(n_0*),n_1*,Cpred_max(n_1*),n_geom,<COUNT>*,Cpred_avg\n"),
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
            :"FIRST,LAST,START,n_0*,Cpred_min(n_0*),n_1*,Cpred_max(n_1*),n_geom,Cpred_avg\n"),
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
    return primAgg.left;
}

void GBRange::calcAverage(GBWindow &w,GBLongInterval &interval, GBAggregate &agg,  bool useLegacy) {
    GBLongIntervalSummary &summary = interval.summary;
    summary.pairCountAvg = summary.pairCountTotal / (agg.right - agg.left);
    summary.cAvg = summary.pairCountTotalNorm / (agg.right - agg.left);
    if(model == Model::HLA && ! summary.useHLCorrInst) {
        const std::uint64_t n_geom_odd  = (useLegacy ? ((1ULL | (std::uint64_t)floorl(agg.n_geom))) : minPrefOdd(agg.n_geom,agg.right - 1));
        const std::uint64_t delta_odd = w.computeDelta(n_geom_odd);
        const std::uint64_t n_geom_even  = (compat_ver == CompatVer::V015 ? (1ULL + n_geom_odd) : maxPrefEven(agg.n_geom,agg.left));
        const std::uint64_t delta_even = w.computeDelta(n_geom_even);
        summary.applyHLCorr(n_geom_even, delta_even, n_geom_odd, delta_odd,
            agg.evenCalc, agg.oddCalc, agg.minCalc, agg.maxCalc, agg.minNormCalc, agg.maxNormCalc );
    }
}

void GBRange::outputFull(GBAggregate &agg,GBLongInterval &interval,bool useLegacy) {
    if(! (interval.out || interval.trace)) {
        return;
    }
    GBLongIntervalSummary &summary = interval.summary;
    if(! useLegacy) {
        fprintf_both(interval.out,interval.trace,
            (model == Model::Empirical)
                ? "%" PRIu64 ",%" PRIu64 ",%s,%" PRIu64 ",%.0Lf,%" PRIu64 ",%.0Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.8Lf,%.0Lf,%.6Lf,%.9Lf\n"
                : "%" PRIu64 ",%" PRIu64 ",%s,%" PRIu64 ",%.3Lf,%" PRIu64 ",%.3Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.8Lf,%.0Lf,%.6Lf,%.9Lf\n",
            agg.left, agg.right -1,
            agg.label.c_str(),
            summary.minAtLast, summary.pairCountMinLast,
            summary.maxAtFirst, summary.pairCountMaxFirst,
            summary.n0Last, summary.cMinLast,
            summary.n1First, summary.cMaxFirst,
            agg.n_geom,
            summary.pairCountAvg,
            summary.cAvg
        );
        return;
    }
    if(model == Model::Empirical) {
        fprintf_both(interval.out,interval.trace,
            "%s,%" PRIu64 ",%.0Lf,%" PRIu64 ",%.0Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf,%.6Lf\n",
            agg.label.c_str(),
            summary.minAtFirst, summary.pairCountMinFirst,
            summary.maxAtFirst, summary.pairCountMaxFirst,
            summary.n0First, summary.cMinFirst,
            summary.n1First, summary.cMaxFirst,
            ((std::uint64_t)floorl(agg.n_geom)) | (agg.n_geom >= 10L ? 1ULL : 0ULL),
            summary.pairCountAvg,
            summary.cAvg
        );
        return;
    }
    fprintf_both(interval.out,interval.trace,
        "%s,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.8Lf,%" PRIu64 ",%.8Lf,%" PRIu64 ",%.8Lf,%.8Lf,%.8Lf\n",
        agg.label.c_str(),
        summary.minAtFirst, summary.pairCountMinFirst,
        summary.maxAtFirst, summary.pairCountMaxFirst,
        summary.n0First, summary.cMinFirst,
        summary.n1First, summary.cMaxFirst,
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
            summary.minAtLast, summary.pairCountMinLast,
            summary.maxAtFirst, summary.pairCountMaxFirst,
            agg.n_geom,
            summary.pairCountAvg
        );
    }
}

void GBRange::outputNorm(GBAggregate &agg,GBLongInterval &interval) {
    if(interval.norm) {
        GBLongIntervalSummary &summary = interval.summary;
        std::fprintf(interval.norm,
            (model == Model::Empirical)
                ? "%" PRIu64 ",%" PRIu64 ",%s,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.8Lf,%.0Lf,%.9Lf\n"
                : "%" PRIu64 ",%" PRIu64 ",%s,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.8Lf,%.0Lf,%.9Lf\n",
            agg.left, agg.right -1,
            agg.label.c_str(),
            summary.n0First, summary.cMinFirst,
            summary.n1Last, summary.cMaxLast,
            agg.n_geom,
            summary.cAvg
        );
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
        prim_summary.pairCount = dec_summary.pairCount = pairCount;
        prim_summary.c_of_n = dec_summary.c_of_n = c_of_n;
        w.checkCrossing(n,c_of_n <= cminus);
        w.checkCrossingAsymp(n,c_of_n <= cminusAsymp);
        w.updateN5percent(n,delta,logNlogN,c_of_n-cminus,c_of_n-cminusAsymp);
    }
    else { // HLA
        prim_summary.pairCount = dec_summary.pairCount = 0.0L;
        prim_summary.c_of_n = dec_summary.c_of_n = 0.0L;

        prim_summary.hlCorrAvg = dec_summary.hlCorrAvg = 1.0L;
        long double hlCorrAvg = 0.0L;
        if(w.is_prim_active()) {
            if(primAgg.minor < 5) {
                prim_summary.useHLCorrInst = 1;
                prim_summary.hlCorrAvg = hlCorrAvg = hlCorr(n,delta);
                prim_summary.c_of_n = twoSGB*hlCorrAvg;
            }
            else {
                prim_summary.c_of_n = twoSGB;
            }
            if (pc) {
                prim_summary.pairCount  = (norm > 0.5L) ? (prim_summary.c_of_n / deltaL) : 1.0L;
                prim_summary.c_of_n = prim_summary.pairCount * norm;
            } else if (norm > 0.0L) {
                prim_summary.pairCount = prim_summary.c_of_n / norm;
            }
        }
        if(w.is_dec_active()) {
            if (decAgg.base < 10) {
                dec_summary.useHLCorrInst  = 1;
                if(hlCorrAvg == 0.0L) {
                    hlCorrAvg = hlCorr(n,delta);
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
            } else if (norm > 0.0L) {
                dec_summary.pairCount = dec_summary.c_of_n / norm;
            }
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
    for (std::uint64_t n = n_start; n < n_end; ) {

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
            }
            if (w->is_prim_active() && n == primAgg.right) {
                calcAverage(*w,w->prim,primAgg,false);
                outputFull(primAgg,w->prim,false);
                outputRaw(primAgg,w->prim);
                outputNorm(primAgg,w->prim);
                w->prim.summary.outputCps(w->prim,w->alpha_n,-1,n_start,w->preMertens,w->preMertensAsymp);
                need_primReset = true;
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


