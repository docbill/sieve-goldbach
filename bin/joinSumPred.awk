#!/usr/bin/awk -f
# jointSumPred - joins summary and HLA full output files for plotting
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
#   awk -f joinSumPred.awk pairrangesummary-100M.csv pairrange2sgbll-full-100M.csv > joined.csv
#
# File 1 (pairrangesummary) columns:
#   v0.1.5: 1: DECADE, 6: n_0, 7: C_min, 8: n_1, 9: C_max, 10: n_geom, 12: C_avg
#   v0.2.0: 1: FIRST, 8: n_0, 9: C_min(n_0), 10: n_1, 11: C_max(n_1), 12: n_geom, 14: C_avg
# File 2 (HLA full output) columns:
#   v0.1.5: 1: DECADE, 6: n_0(pred), 7: Cpred_min, 8: n_1(pred), 9: Cpred_max, 10: N_geom, 12: Cpred_avg
#   v0.2.0: 1: FIRST, 8: n_0(pred), 9: Cpred_min, 10: n_1(pred), 11: Cpred_max, 12: N_geom, 14: Cpred_avg, 16: n_align, 17: C_align
#
# Join key: (DECADE, n_geom/N_geom)
# Uses actual alignment values from HLA full output instead of computing approximate values

BEGIN {
    FS=","; OFS=","
    
    # Twin prime constant (C₂) times 4 for theoretical Hardy-Littlewood baseline
    TWIN_PRIME_CONST_4 = 2.6406472634
    
    # Global column variables
    col_label = 0
    col_n0 = 0
    col_cmin = 0
    col_n1 = 0
    col_cmax = 0
    col_ngeom = 0
    col_cavg = 0
    
    # Second file column variables (HLA full output)
    col_label2 = 0
    col_n0_2 = 0
    col_cmin_2 = 0
    col_n1_2 = 0
    col_cmax_2 = 0
    col_ngeom_2 = 0
    col_cavg_2 = 0
    col_nalign = 0
    col_calign = 0
    col_ncbound = 0
    col_ccbound = 0
}

function trim(s){ sub(/^[ \t\r]+/,"",s); sub(/[ \t\r]+$/,"",s); return s }

function R(n) {
    if(n >= 3234846615 ) {
        return 214708725
    }
    else if(n >= 111546435) {
        return 7952175
    }
    else if(n >= 4849845) {
        return 378675
    }
    else if(n >= 255255 ) {
        return 22275
    }
    else if(n >= 15015) {
        return 1485 
    }
    else if(n >= 1155 ) {
        return 135 
    }
    else if(n >= 105 ) {
        return 15
    }
    else if(n >= 15 ) {
        return 3
    }
    return 1;
}

# No longer needed - we use actual alignment values from HLA full output

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
    
    # Force string conversion to preserve scientific notation
    label   = "" trim($col_label)
    n0    = trim($col_n0)
    cmin  = trim($col_cmin)
    n1    = trim($col_n1)
    cmax  = trim($col_cmax)
    ngeom = trim($col_ngeom)
    cavg  = trim($col_cavg)

    # For v0.2.0 files, use n_geom as the key (it's unique)
    # For v0.1.5 files, use label + n_geom as the key
    if (format == "v0.2.0") {
        key = ngeom
    } else {
        key = label "\034" ngeom
    }
    
    sum_n0[key]   = n0
    sum_cmin[key] = cmin
    sum_n1[key]   = n1
    sum_cmax[key] = cmax
    sum_ng[key]   = ngeom
    sum_cavg[key] = cavg
    sum_label[key] = label  # Store label separately for output
    next
}

# ---- Pass 2: read HLA full output (file2), emit join ----
FNR==1 {
    
    # Detect format from second file header and set columns
    format = detect_format($0)
    if (format == "v0.2.0") {
        # HLA v0.2.0 format: FIRST,LAST,START,minAt*,Gpred(minAt*),maxAt*,Gpred(maxAt*),n_0*,Cpred_min(n_0*),n_1*,Cpred_max(n_1*),n_geom,<COUNT>*,Cpred_avg,n_alignMax,c_alignMax,n_alignMin,c_alignMin
        col_label2 = 3
        col_n0_2 = 8
        col_cmin_2 = 9
        col_n1_2 = 10
        col_cmax_2 = 11
        col_ngeom_2 = 12
        col_cavg_2 = 14
        col_nalign = 17
        col_calign = 18
        col_ncbound = 19
        col_ccbound = 20
    } else {
        # v0.1.5 format: DECADE,MIN AT,MIN,MAX AT,MAX,n_0,Cpred_min,n_1,Cpred_max,N_geom,<COUNT>,Cpred_avg,HLCorr
        col_label2 = 1
        col_n0_2 = 6
        col_cmin_2 = 7
        col_n1_2 = 8
        col_cmax_2 = 9
        col_ngeom_2 = 10
        col_cavg_2 = 12
        col_nalign = 0
        col_calign = 0
        col_ncbound = 0
        col_ccbound = 0
    }
    if(col_label2 == 3) {
        print "START","n_0","C_min","Npred_0","Cpred_min",
            "n_1","C_max","Npred_1","Cpred_max",
            "n_geom","C_avg","Cpred_avg","Align","CpredAlign","CpredBound"
    }
    else {
        print "DECADE","n_0","C_min","Npred_0","Cpred_min",
            "n_1","C_max","Npred_1","Cpred_max",
            "n_geom","C_avg","Cpred_avg","Align","CpredAlign","CpredBound"

    }
    next
}

{
    sub(/\r$/,"")
    
    # Force string conversion to preserve scientific notation
    label  = "" trim($col_label2)
    n0p    = trim($col_n0_2)
    cpmin  = trim($col_cmin_2)
    n1p    = trim($col_n1_2)
    cpmax  = trim($col_cmax_2)
    ngeomp = trim($col_ngeom_2)     # N_geom in file2
    cpavg  = trim($col_cavg_2)
    n_align = (col_nalign > 0) ? trim($col_nalign) + 0 : 0
    c_align = (col_calign > 0) ? trim($col_calign) + 0 : 0
    n_cbound = (col_ncbound > 0) ? trim($col_ncbound) + 0 : 0
    c_cbound = (col_ccbound > 0) ? trim($col_ccbound) + 0 : 0
    
    # For v0.2.0 files, use n_geom as the key (it's unique)
    # For v0.1.5 files, use label + n_geom as the key
    if (col_label2 == 3) {  # v0.2.0 format
        key = ngeomp
    } else {  # v0.1.5 format
        key = label "\034" ngeomp
    }

    if (!(key in sum_n0)) {
        if(col_label2 == 1) {
            printf("WARN: no match for DECADE=%s n_geom=%s in file1\n", label, ngeomp) > "/dev/stderr"
        }
        else {
            printf("WARN: no match for START=%s n_geom=%s in file1\n", label, ngeomp) > "/dev/stderr"
        }
        next
    }
    
    # Use stored label for output (preserves scientific notation)
    output_label = sum_label[key]
    
    # Use actual alignment values from HLA full output
    if (col_calign > 0) {
        # Use actual alignment values from HLA output (even if zero)
        align = c_align  # Use C_align as the alignment value
        cpred_align = c_align  # Use C_align as the corrected prediction
    } else {
        # No alignment column available - this shouldn't happen with v0.1.5+ output
        n0_val = sum_n0[key]
        cpmin_val = cpmin

        align = 2* R(sqrt(2*n0p)) 
        log_n0p = (n0p >= 2.72 ? log(n0p) : 1.0)
        cpred_align = 2.6406472634;
        # Guard against n0p too small for log(log(n0p))
        if (log_n0p > 1.0) {
            loglog_n0p = log(log_n0p)
            align = 2.0 * sqrt(n0p) / (loglog_n0p * loglog_n0p)
            cpred_align = cpmin_val;
        }
        cpred_align -= align * (log_n0p * log_n0p) / (n0p)  # Simplified without alpha
        if (cpred_align < 0.0) cpred_align = 0.0
    }
    
    # Use actual conservative bound values from HLA full output
    if (col_ccbound > 0) {
        # Use actual C_cBound value from HLA output (even if zero)
        cpred_bound = c_cbound
    } else {
        # No cBound column available - this shouldn't happen with v0.1.5+ output
        cpred_bound = cpmin
    }
    
    printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%.6f,%.6f,%.6f\n",
        output_label, sum_n0[key], sum_cmin[key], n0p, cpmin,
        sum_n1[key], sum_cmax[key], n1p, cpmax,
        sum_ng[key], sum_cavg[key], cpavg, align, cpred_align, cpred_bound
}

