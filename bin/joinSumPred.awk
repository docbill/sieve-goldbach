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

# Find column position by name
function find_column(header, name) {
    split(header, cols, ",")
    for (i = 1; i <= length(cols); i++) {
        if (cols[i] == name) {
            return i
        }
    }
    return -1
}

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
    
    # Version checking
    VERSION = ENVIRON["VERSION"]
    if (VERSION == "") {
        print "ERROR: VERSION environment variable not set. Use: VERSION=v0.1.x or VERSION=v0.2.x" > "/dev/stderr"
        exit 1
    }
    # Accept v0.1.x (v0.1.5, v0.1.6, etc.) or v0.2.x versions
    if (substr(VERSION, 1, 5) != "v0.1." && substr(VERSION, 1, 5) != "v0.2.") {
        print "ERROR: Invalid VERSION '" VERSION "'. Must be v0.1.x or v0.2.x" > "/dev/stderr"
        exit 1
    }
    
    # Global column variables
    col_label = 0
    col_n0 = 0
    file2_format = ""
    col_cmin = 0
    col_n1 = 0
    col_cmax = 0
    col_ngeom = 0
    col_cavg = 0
    
    # File tracking variables
    file1_name = ""
    file2_name = ""
    
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
        col_nalignmax = 0
        col_calignmax = 0
        col_ncbound = 0
        col_ccbound = 0
        col_ncboundmax = 0
        col_ccboundmax = 0
        
        # Validate required columns for v0.1.5 (using hardcoded positions)
        # For v0.1.5, use dynamic column detection
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

# Detect VERSION version based on header
function detect_VERSION(header) {
    if (index(header, "FIRST") > 0) {
        return "v0.2.0"
    } else {
        return "v0.1.5"
    }
}

# Set column numbers based on VERSION (inline)

# ---- Pass 1: read pairrangesummary (file1) ----
FNR==NR {
    sub(/\r$/,"")
    if (FNR==1) {
        file1_name = FILENAME
        # Detect format from file header
        if (index($0, "FIRST") > 0) {
            # v0.2.0 format
            col_label = find_column($0, "START")
            col_n0 = find_column($0, "n_0")
            col_cmin = find_column($0, "C_min(n_0)")
            col_n1 = find_column($0, "n_1")
            col_cmax = find_column($0, "C_max(n_1)")
            col_ngeom = find_column($0, "n_geom")
            col_cavg = find_column($0, "C_avg")
        } else {
            # v0.1.5 format
            col_label = find_column($0, "DECADE")
            col_n0 = find_column($0, "n_0")
            col_cmin = find_column($0, "C_min")
            col_n1 = find_column($0, "n_1")
            col_cmax = find_column($0, "C_max")
            col_ngeom = find_column($0, "n_geom")
            col_cavg = find_column($0, "C_avg")
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

    # For v0.2.x files, use n_geom as the key (it's unique)
    # For v0.1.x files, use label + n_geom as the key
    if (substr(VERSION, 1, 5) == "v0.2.") {
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
    file2_name = FILENAME
    
    # Detect format from file header
    if (index($0, "FIRST") > 0) {
        # Check if it has align/bound columns to distinguish v0.2.0 from v0.1.5 primorial
        if (index($0, "n_v") > 0 && index($0, "Calign_min") > 0) {
            file2_format = "v0.2.0"
        } else {
            file2_format = "v0.1.5"
        }
        if (file2_format == "v0.2.0") {
            # HLA v0.2.0 VERSION: FIRST,LAST,START,minAt*,Gpred(minAt*),maxAt*,Gpred(maxAt*),n_0*,Cpred_min(n_0*),n_1*,Cpred_max(n_1*),n_geom,<COUNT>*,Cpred_avg,n_v,Calign_min(n_v),n_u,Calign_max(n_u),n_a,CboundMin,jitter
            col_label2 = find_column($0, "START")
            col_n0_2 = find_column($0, "n_0*")
            col_cmin_2 = find_column($0, "Cpred_min(n_0*)")
            col_n1_2 = find_column($0, "n_1*")
            col_cmax_2 = find_column($0, "Cpred_max(n_1*)")
            col_ngeom_2 = find_column($0, "n_geom")
            col_cavg_2 = find_column($0, "Cpred_avg")
            col_nalign = find_column($0, "n_v")
            col_calign = find_column($0, "Calign_min(n_v)")
            col_nalignmax = find_column($0, "n_u")
            col_calignmax = find_column($0, "Calign_max(n_u)")
            col_ncbound = find_column($0, "n_a")
            col_ccbound = find_column($0, "CboundMin(n_a)")
            col_ncboundmax = find_column($0, "n_b")
            col_ccboundmax = find_column($0, "CboundMax(n_b)")
        } else {
            # v0.1.5 primorial format: FIRST,LAST,START,minAt*,Gpred(minAt*),maxAt*,Gpred(maxAt*),n_0*,Cpred_min(n_0*),n_1*,Cpred_max(n_1*),n_geom,<COUNT>*,Cpred_avg
            col_label2 = find_column($0, "START")
            col_n0_2 = find_column($0, "n_0*")
            col_cmin_2 = find_column($0, "Cpred_min(n_0*)")
            col_n1_2 = find_column($0, "n_1*")
            col_cmax_2 = find_column($0, "Cpred_max(n_1*)")
            col_ngeom_2 = find_column($0, "n_geom")
            col_cavg_2 = find_column($0, "Cpred_avg")
            col_nalign = 0
            col_calign = 0
            col_nalignmax = 0
            col_calignmax = 0
            col_ncbound = 0
            col_ccbound = 0
            col_ncboundmax = 0
            col_ccboundmax = 0
        }
        
        # Validate required columns based on format
        missing_columns = 0
        if (file2_format == "v0.2.0") {
            if (col_nalign == -1) { print "ERROR: n_v column not found for v0.2.0" > "/dev/stderr"; missing_columns++ }
            if (col_calign == -1) { print "ERROR: Calign_min(n_v) column not found for v0.2.0" > "/dev/stderr"; missing_columns++ }
            if (col_nalignmax == -1) { print "ERROR: n_u column not found for v0.2.0" > "/dev/stderr"; missing_columns++ }
            if (col_calignmax == -1) { print "ERROR: Calign_max(n_u) column not found for v0.2.0" > "/dev/stderr"; missing_columns++ }
            if (col_ncbound == -1) { print "ERROR: n_a column not found for v0.2.0" > "/dev/stderr"; missing_columns++ }
            if (col_ccbound == -1) { print "ERROR: CboundMin(n_a) column not found for v0.2.0" > "/dev/stderr"; missing_columns++ }
            if (col_ncboundmax == -1) { print "ERROR: n_b column not found for v0.2.0" > "/dev/stderr"; missing_columns++ }
            if (col_ccboundmax == -1) { print "ERROR: CboundMax(n_b) column not found for v0.2.0" > "/dev/stderr"; missing_columns++ }
        } else {
            if (col_label2 == -1) { print "ERROR: START column not found for v0.1.5" > "/dev/stderr"; missing_columns++ }
            if (col_n0_2 == -1) { print "ERROR: n_0* column not found for v0.1.5" > "/dev/stderr"; missing_columns++ }
            if (col_cmin_2 == -1) { print "ERROR: Cpred_min(n_0*) column not found for v0.1.5" > "/dev/stderr"; missing_columns++ }
            if (col_n1_2 == -1) { print "ERROR: n_1* column not found for v0.1.5" > "/dev/stderr"; missing_columns++ }
            if (col_cmax_2 == -1) { print "ERROR: Cpred_max(n_1*) column not found for v0.1.5" > "/dev/stderr"; missing_columns++ }
            if (col_ngeom_2 == -1) { print "ERROR: n_geom column not found for v0.1.5" > "/dev/stderr"; missing_columns++ }
            if (col_cavg_2 == -1) { print "ERROR: Cpred_avg column not found for v0.1.5" > "/dev/stderr"; missing_columns++ }
        }
        if (missing_columns > 0) {
            print "ERROR: " missing_columns " required columns missing for " file2_format ". Cannot proceed." > "/dev/stderr"
            exit 1
        }
    } else {
        file2_format = "v0.1.5"
        # v0.1.5 format: DECADE,MIN AT,MIN,MAX AT,MAX,n_0,Cpred_min,n_1,Cpred_max,N_geom,<COUNT>,Cpred_avg,HLCorr
        col_label2 = find_column($0, "DECADE")
        col_n0_2 = find_column($0, "n_0")
        col_cmin_2 = find_column($0, "Cpred_min")
        col_n1_2 = find_column($0, "n_1")
        col_cmax_2 = find_column($0, "Cpred_max")
        col_ngeom_2 = find_column($0, "N_geom")
        col_cavg_2 = find_column($0, "Cpred_avg")
        col_nalign = 0
        col_calign = 0
        col_ncbound = 0
        col_ccbound = 0
        
        # Validate required columns for v0.1.5
        missing_columns = 0
        if (col_label2 == -1) { print "ERROR: DECADE column not found for v0.1.5" > "/dev/stderr"; missing_columns++ }
        if (col_n0_2 == -1) { print "ERROR: n_0 column not found for v0.1.5" > "/dev/stderr"; missing_columns++ }
        if (col_cmin_2 == -1) { print "ERROR: Cpred_min column not found for v0.1.5" > "/dev/stderr"; missing_columns++ }
        if (col_n1_2 == -1) { print "ERROR: n_1 column not found for v0.1.5" > "/dev/stderr"; missing_columns++ }
        if (col_cmax_2 == -1) { print "ERROR: Cpred_max column not found for v0.1.5" > "/dev/stderr"; missing_columns++ }
        if (col_ngeom_2 == -1) { print "ERROR: N_geom column not found for v0.1.5" > "/dev/stderr"; missing_columns++ }
        if (col_cavg_2 == -1) { print "ERROR: Cpred_avg column not found for v0.1.5" > "/dev/stderr"; missing_columns++ }
        if (missing_columns > 0) {
            print "ERROR: " missing_columns " required columns missing for v0.1.5. Cannot proceed." > "/dev/stderr"
            exit 1
        }
    }
    # Use VERSION environment variable for output format
    if (substr(VERSION, 1, 5) == "v0.2.") {
        # v0.2.x format: include align/bound columns
        print "START","n_0","C_min","Npred_0","Cpred_min",
            "n_1","C_max","Npred_1","Cpred_max",
            "n_geom","C_avg","Cpred_avg","n_v","Calign_min","n_u","Calign_max","n_a","Cbound_min","n_b","Cbound_max"
    }
    else {
        # v0.1.x format: original 12 columns only
        print "DECADE","n_0","C_min","Npred_0","Cpred_min",
            "n_1","C_max","Npred_1","Cpred_max",
            "n_geom","C_avg","Cpred_avg"
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
    n_alignmax = (col_nalignmax > 0) ? trim($col_nalignmax) + 0 : 0
    c_alignmax = (col_calignmax > 0) ? trim($col_calignmax) + 0 : 0
    n_cbound = (col_ncbound > 0) ? trim($col_ncbound) + 0 : 0
    c_cbound = (col_ccbound > 0) ? trim($col_ccbound) + 0 : 0
    n_cboundmax = (col_ncboundmax > 0) ? trim($col_ncboundmax) + 0 : 0
    c_cboundmax = (col_ccboundmax > 0) ? trim($col_ccboundmax) + 0 : 0
    
    # For v0.2.x files, use n_geom as the key (it's unique)
    # For v0.1.x files, use label + n_geom as the key
    if (substr(VERSION, 1, 5) == "v0.2.") {  # v0.2.x format
        key = ngeomp
    } else {  # v0.1.x format
        key = label "\034" ngeomp
    }
    
    # Store align and bound values in sum arrays for later use
    sum_n_align[key] = n_align
    sum_c_align[key] = c_align
    sum_n_alignmax[key] = n_alignmax
    sum_c_alignmax[key] = c_alignmax
    sum_n_cbound[key] = n_cbound
    sum_c_cbound[key] = c_cbound
    sum_n_cboundmax[key] = n_cboundmax
    sum_c_cboundmax[key] = c_cboundmax

    if (!(key in sum_n0)) {
        if (substr(VERSION, 1, 5) == "v0.1.") {
            printf("ERROR: no match for DECADE=%s n_geom=%s in file1\n", label, ngeomp) > "/dev/stderr"
            printf("ERROR: Processing files: file1='%s' file2='%s'\n", file1_name, file2_name) > "/dev/stderr"; exit 1
        }
        else {
            printf("ERROR: no match for START=%s n_geom=%s in file1\n", label, ngeomp) > "/dev/stderr"
            printf("ERROR: Processing files: file1='%s' file2='%s'\n", file1_name, file2_name) > "/dev/stderr"; exit 1
        }
        next
    }
    
    # Use stored label for output (preserves scientific notation)
    output_label = sum_label[key]
    
    # # Use actual alignment values from HLA full output
    # if (col_calign > 0) {
    #     # Use actual alignment values from HLA output (even if zero)
    #     align = c_align  # Use C_align as the alignment value
    #     cpred_align = c_align  # Use C_align as the corrected prediction
    # } else {
    #     # No alignment column available - this shouldn't happen with v0.1.5+ output
    #     n0_val = sum_n0[key]
    #     cpmin_val = cpmin
    #
    #     align = 2* R(sqrt(2*n0p)) 
    #     log_n0p = (n0p >= 2.72 ? log(n0p) : 1.0)
    #     cpred_align = 2.6406472634;
    #     # Guard against n0p too small for log(log(n0p))
    #     if (log_n0p > 1.0) {
    #         loglog_n0p = log(log_n0p)
    #         align = 2.0 * sqrt(n0p) / (loglog_n0p * loglog_n0p)
    #         cpred_align = cpmin_val;
    #     }
    #     cpred_align -= align * (log_n0p * log_n0p) / (n0p)  # Simplified without alpha
    #     if (cpred_align < 0.0) cpred_align = 0.0
    # }
    
    # Use actual conservative bound values from HLA full output
    if (col_ccbound > 0) {
        # Use actual C_cBound value from HLA output (even if zero)
        cpred_bound = c_cbound
    } else {
        # No cBound column available - this shouldn't happen with v0.1.5+ output
        cpred_bound = cpmin
    }
    
    # Use VERSION environment variable for data output
    if (substr(VERSION, 1, 5) == "v0.2.") {
        # v0.2.x format: include align/bound columns
        printf "%s,%d,%.6f,%d,%.6f,%d,%.8f,%d,%.8f,%.0f,%.9f,%.9f,%d,%.6f,%d,%.8f,%d,%.6f,%d,%.8f\n",
            output_label, sum_n0[key], sum_cmin[key], n0p, cpmin,
            sum_n1[key], sum_cmax[key], n1p, cpmax,
            sum_ng[key], sum_cavg[key], cpavg, 
            sum_n_align[key], sum_c_align[key], sum_n_alignmax[key], sum_c_alignmax[key], sum_n_cbound[key], sum_c_cbound[key], sum_n_cboundmax[key], sum_c_cboundmax[key]
    } else {
        # v0.1.x format: original 12 columns only
        printf "%s,%d,%.6f,%d,%.8f,%d,%.6f,%d,%.8f,%.0f,%.6f,%.8f\n",
            output_label, sum_n0[key], sum_cmin[key], n0p, cpmin,
            sum_n1[key], sum_cmax[key], n1p, cpmax,
            sum_ng[key], sum_cavg[key], cpavg
    }
}

