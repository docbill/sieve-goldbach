#!/usr/bin/awk -f
# lamdbaStats - summarises lambda values per decade
# Copyright (C) 2025 Bill C. Riemers
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# SPDX-License-Identifier: GPL-3.0-or-later

# Usage: awk -f lambdaStats.awk input.csv > lambda_decade_stats.csv
# Uses Dec in column 1 and Lambda in the LAST column (from header).
# Portable for BSD/POSIX awk (no asort, no C-style comments).

BEGIN {
    FS=","; OFS=","
    want_decades = 8
    header_out = "Dec.,max |lambda|,2nd max |lambda|,min |lambda|,2nd min |lambda|,median |lambda|,trimmed < |lambda| >,Spread_raw^IQR (of |lambda|),% positive"
    print header_out

    TMPDIR = (ENVIRON["TMPDIR"] != "" ? ENVIRON["TMPDIR"] : "/tmp")
    pid = PROCINFO["pid"]; if (pid == "") pid = int(1000000 * rand() + 1)
}

function trim(s){ sub(/^[ \t\r]+/,"",s); sub(/[ \t\r]+$/,"",s); return s }
function absd(x){ return (x<0 ? -x : x) }

# Type-7 quantile (R’s default): sorted[1..n], p in [0,1]
function quantile(sorted, n, p,   h,k,frac,val) {
    if (n <= 0) return ""
    if (p <= 0) return sorted[1]
    if (p >= 1) return sorted[n]
    h = 1 + (n - 1) * p
    k = int(h); frac = h - k
    if (k >= n) return sorted[n]
    val = sorted[k] + frac * (sorted[k+1] - sorted[k])
    return val
}

NR == 1 {
    header_line = $0
    next
}

{
    sub(/\r$/,"")

    dec = trim($1) + 0
    if (dec < 0 || dec >= want_decades) next

    lam = trim($NF) + 0.0   # last column = Lambda

    k = ++cnt[dec]
    lam_abs[dec,k] = absd(lam)
    sum_abs[dec]  += lam_abs[dec,k]
    if (lam > 0) poscnt[dec]++

    if (tfile[dec] == "") {
        tfile[dec] = TMPDIR "/lambda_abs_dec" dec "_" pid ".tmp"
        sfile[dec] = TMPDIR "/lambda_abs_dec" dec "_" pid ".srt"
    }
    printf "%0.8f\n", lam_abs[dec,k] >> tfile[dec]
}

END {
    for (d = 0; d < want_decades; d++) {
        n = cnt[d] + 0
        if (n == 0) { printf "%d,,,,,,,,\n", d; continue }

        # sort -> read back
        system("sort -n " q(tfile[d]) " > " q(sfile[d]))
        delete sorted
        i = 0
        cmd = "cat " q(sfile[d])
        while ((cmd | getline line) > 0) {
            line = trim(line); if (line=="") continue
            sorted[++i] = line + 0.0
        }
        close(cmd); m = i

        min1 = sorted[1];           min2 = (m>=2 ? sorted[2]     : "")
        max1 = sorted[m];           max2 = (m>=2 ? sorted[m-1]   : "")

        # ---- median(|Lambda|) (replaces mean) ----
        if (m % 2) med_abs = sorted[(m+1)/2]
        else       med_abs = (sorted[m/2] + sorted[m/2+1]) / 2

        # trimmed mean of |Lambda| (drop min & max if possible)
        if (m >= 3) {
            tsum = 0; for (i=2; i<=m-1; i++) tsum += sorted[i]
            tmean = tsum / (m - 2)
        } else tmean = ""

        q1 = quantile(sorted, m, 0.25)
        q3 = quantile(sorted, m, 0.75)
        iqr = ((q1=="" || q3=="") ? "" : (q3 - q1))

        pospct = 100.0 * (poscnt[d] + 0) / n

        printf "%d,%.3e,%s,%.3e,%s,%.3e,%s,%.3e,%.1f\n", \
            d, \
            max1, (max2=="" ? "" : sprintf("%.3e", max2)), \
            min1, (min2=="" ? "" : sprintf("%.3e", min2)), \
            med_abs, (tmean=="" ? "" : sprintf("%.3e", tmean)), \
            (iqr=="" ? 0.0 : iqr), \
            pospct

        if (tfile[d]!="") system("rm -f " q(tfile[d]))
        if (sfile[d]!="") system("rm -f " q(sfile[d]))
    }
}

# shell-quote
function q(s){ gsub(/'/, "'\\''", s); return "'" s "'" }

