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
#   1: DECADE, 6: n_0, 7: C_min, 8: n_1, 9: C_max, 10: n_geom, 12: C_avg
# File 2 (pairrange2sgbll) columns:
#   1: DECADE, 6: n_0(pred), 7: Cpred_min, 8: n_1(pred), 9: Cpred_max, 10: N_geom, 12: Cpred_avg
#
# Join key: (DECADE, n_geom/N_geom)

BEGIN { FS=","; OFS="," }

function trim(s){ sub(/^[ \t\r]+/,"",s); sub(/[ \t\r]+$/,"",s); return s }

# ---- Pass 1: read pairrangesummary (file1) ----
FNR==NR {
    sub(/\r$/,"")
    if (FNR==1) next  # skip header
    dec   = trim($1)
    n0    = trim($6)
    cmin  = trim($7)
    n1    = trim($8)
    cmax  = trim($9)
    ngeom = trim($10)
    cavg  = trim($12)

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
    dec    = trim($1)
    n0p    = trim($6)
    cpmin  = trim($7)
    n1p    = trim($8)
    cpmax  = trim($9)
    ngeomp = trim($10)     # N_geom in file2
    cpavg  = trim($12)

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

