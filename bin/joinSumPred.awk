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

# Global variables
col_label = 0
col_n0 = 0
col_cmin = 0
col_n1 = 0
col_cmax = 0
col_ngeom = 0
col_cavg = 0
first_file_processed = 0

# Second file variables
col_label2 = 0
col_n0_2 = 0
col_cmin_2 = 0
col_n1_2 = 0
col_cmax_2 = 0
col_ngeom_2 = 0
col_cavg_2 = 0

function trim(s){ sub(/^[ \t\r]+/,"",s); sub(/[ \t\r]+$/,"",s); return s }

# Detect format version based on header
function detect_format(header) {
    if (index(header, "FIRST") > 0) {
        return "v0.2.0"
    } else {
        return "v0.1.5"
    }
}

# Set column numbers based on format (inline)

# ---- Pass 1: read pairrangesummary (file1) ----
FNR==NR {
    sub(/\r$/,"")
    if (FNR==1) {
        format = detect_format($0)
        if (format == "v0.2.0") {
            col_label = 3
            col_n0 = 8
            col_cmin = 9
            col_n1 = 10
            col_cmax = 11
            col_ngeom = 12
            col_cavg = 14
        } else {
            col_label = 1
            col_n0 = 6
            col_cmin = 7
            col_n1 = 8
            col_cmax = 9
            col_ngeom = 10
            col_cavg = 12
        }
        first_file_processed = 1
        next  # skip header
    }
    # Set column variables for first file if not already set
    if (col_label == 0) {
        # This is the first data row of the first file, set columns
        if (index($0, "FIRST") > 0) {
            col_label = 3
            col_n0 = 8
            col_cmin = 9
            col_n1 = 10
            col_cmax = 11
            col_ngeom = 12
            col_cavg = 14
        } else {
            col_label = 1
            col_n0 = 6
            col_cmin = 7
            col_n1 = 8
            col_cmax = 9
            col_ngeom = 10
            col_cavg = 12
        }
    }
    
    label   = trim($col_label)
    n0    = trim($col_n0)
    cmin  = trim($col_cmin)
    n1    = trim($col_n1)
    cmax  = trim($col_cmax)
    ngeom = trim($col_ngeom)
    cavg  = trim($col_cavg)
    

    key = label "\034" ngeom
    
    
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
    
    # Detect format from second file header and set columns
    format = detect_format($0)
    if (format == "v0.2.0") {
        col_label2 = 3
        col_n0_2 = 8
        col_cmin_2 = 9
        col_n1_2 = 10
        col_cmax_2 = 11
        col_ngeom_2 = 12
        col_cavg_2 = 14
    } else {
        col_label2 = 1
        col_n0_2 = 6
        col_cmin_2 = 7
        col_n1_2 = 8
        col_cmax_2 = 9
        col_ngeom_2 = 10
        col_cavg_2 = 12
    }
    if(col_label2 == 1) {
        print "DECADE","n_0","C_min","Npred_0","Cpred_min",
            "n_1","C_max","Npred_1","Cpred_max",
            "n_geom","C_avg","Cpred_avg"
    }
    else {
        print "START","n_0","C_min","Npred_0","Cpred_min",
            "n_1","C_max","Npred_1","Cpred_max",
            "n_geom","C_avg","Cpred_avg"

    }
    next
}

{
    sub(/\r$/,"")
    
    # Set column variables for second file if not already set
    if (col_label2 == 0) {
        # This is the first data row of the second file, set columns
        if (index($0, "FIRST") > 0) {
            col_label2 = 3
            col_n0_2 = 8
            col_cmin_2 = 9
            col_n1_2 = 10
            col_cmax_2 = 11
            col_ngeom_2 = 12
            col_cavg_2 = 14
        } else {
            col_label2 = 1
            col_n0_2 = 6
            col_cmin_2 = 7
            col_n1_2 = 8
            col_cmax_2 = 9
            col_ngeom_2 = 10
            col_cavg_2 = 12
        }
    }
    
    label  = trim($col_label2)
    n0p    = trim($col_n0_2)
    cpmin  = trim($col_cmin_2)
    n1p    = trim($col_n1_2)
    cpmax  = trim($col_cmax_2)
    ngeomp = trim($col_ngeom_2)     # N_geom in file2
    cpavg  = trim($col_cavg_2)
    

    key = label "\034" ngeomp
    

    if (!(key in sum_n0)) {
        if(col_label2 == 1) {
            printf("WARN: no match for DECADE=%s n_geom=%s in file1\n", label, ngeomp) > "/dev/stderr"
        }
        else {
            printf("WARN: no match for START=%s n_geom=%s in file1\n", label, ngeomp) > "/dev/stderr"
        }
        next
    }

    print label,
          sum_n0[key], sum_cmin[key], n0p, cpmin,
          sum_n1[key], sum_cmax[key], n1p, cpmax,
          sum_ng[key], sum_cavg[key], cpavg
}

