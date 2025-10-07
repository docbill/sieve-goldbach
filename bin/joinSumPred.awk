#!/usr/bin/awk -f
# jointSumPred - joins summary and HL-A prediction files for plotting
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

# Usage:
#   awk -f joinSumPred.awk pairrangesummary-100M.csv pairrange2sgbll-100M.csv > joined.csv
#
# File 1 (pairrangesummary) columns:
#   v0.1.5: 1: DECADE, 6: n_0, 7: C_min, 8: n_1, 9: C_max, 10: n_geom, 12: C_avg
#   v0.2.0: 1: FIRST, 8: n_0, 9: C_min(n_0), 10: n_1, 11: C_max(n_1), 12: n_geom, 14: C_avg
# File 2 (pairrange2sgbll) columns:
#   v0.1.5: 1: DECADE, 6: n_0(pred), 7: Cpred_min, 8: n_1(pred), 9: Cpred_max, 10: N_geom, 12: Cpred_avg
#   v0.2.0: 1: FIRST, 8: n_0(pred), 9: Cpred_min, 10: n_1(pred), 11: Cpred_max, 12: N_geom, 14: Cpred_avg
#
# Join key: (DECADE, n_geom/N_geom)

BEGIN { FS=","; OFS="," }

function trim(s){ sub(/^[ \t\r]+/,"",s); sub(/[ \t\r]+$/,"",s); return s }

# Detect format version based on header
function detect_format(header) {
    if (index(header, "FIRST") > 0) {
        return "v0.2.0"
    } else {
        return "v0.1.5"
    }
}

# Get column numbers based on format
function get_columns(format) {
    if (format == "v0.2.0") {
        col_dec = 1
        col_n0 = 8
        col_cmin = 9
        col_n1 = 10
        col_cmax = 11
        col_ngeom = 12
        col_cavg = 14
    } else {
        col_dec = 1
        col_n0 = 6
        col_cmin = 7
        col_n1 = 8
        col_cmax = 9
        col_ngeom = 10
        col_cavg = 12
    }
}

# ---- Pass 1: read pairrangesummary (file1) ----
FNR==NR {
    sub(/\r$/,"")
    if (FNR==1) {
        format = detect_format($0)
        get_columns(format)
        next  # skip header
    }
    dec   = trim($col_dec)
    n0    = trim($col_n0)
    cmin  = trim($col_cmin)
    n1    = trim($col_n1)
    cmax  = trim($col_cmax)
    ngeom = trim($col_ngeom)
    cavg  = trim($col_cavg)

    key = dec SUBSEP ngeom
    sum_n0[key]   = n0
    sum_cmin[key] = cmin
    sum_n1[key]   = n1
    sum_cmax[key] = cmax
    sum_ng[key]   = ngeom
    sum_cavg[key] = cavg
    next
}

# ---- Pass 2: read pairrange2sgbll (file2), emit join ----
FNR==1 {
    print "DECADE","n_0","C_min","Npred_0","Cpred_min",
          "n_1","C_max","Npred_1","Cpred_max",
          "n_geom","C_avg","Cpred_avg"
    next
}

{
    sub(/\r$/,"")
    dec    = trim($col_dec)
    n0p    = trim($col_n0)
    cpmin  = trim($col_cmin)
    n1p    = trim($col_n1)
    cpmax  = trim($col_cmax)
    ngeomp = trim($col_ngeom)     # N_geom in file2
    cpavg  = trim($col_cavg)

    key = dec SUBSEP ngeomp

    if (!(key in sum_n0)) {
        printf("WARN: no match for DECADE=%s n_geom=%s in file1\n", dec, ngeomp) > "/dev/stderr"
        next
    }

    print dec,
          sum_n0[key], sum_cmin[key], n0p, cpmin,
          sum_n1[key], sum_cmax[key], n1p, cpmax,
          sum_ng[key], sum_cavg[key], cpavg
}

