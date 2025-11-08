#!/usr/bin/awk -f
# compareAlign - combines summary and HLA full output for alignment-corrected min comparison
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

# Usage: awk -f compareAlign.awk pairrangesummary.csv pairrange2sgbll-full.csv > merged.csv
# v0.1.5: A=Dec, F=n_0, G=C_min, J=n_geom
# v0.2.0: A=FIRST, H=n_0, I=C_min(n_0), L=n_geom, Q=n_align, R=C_align
# Uses actual alignment values from HLA full output instead of computing approximate values

BEGIN {
    FS=","; OFS=","
    
    # Version checking
    VERSION = ENVIRON["VERSION"]
    if (VERSION == "") {
        print "ERROR: VERSION environment variable not set. Use: VERSION=v0.1.5 or VERSION=v0.2.0" > "/dev/stderr"
        exit 1
    }
    if (VERSION != "v0.1.5" && VERSION != "v0.2.0") {
        print "ERROR: Invalid VERSION '" VERSION "'. Must be v0.1.5 or v0.2.0" > "/dev/stderr"
        exit 1
    }
    
    # Global variables
    col_label = 0
    col_minat = 0
    col_n0 = 0
    col_cmin = 0
    col_ngeom = 0
    
    # Second file variables (HLA full output)
    col_label2 = 0
    col_minat_2 = 0
    col_n0_2 = 0
    col_cmin_2 = 0
    col_ngeom_2 = 0
    col_nalign = 0
    col_calign = 0
    col_jitter = 0
}

function trim(s){ sub(/^[ \t\r]+/, "", s); sub(/[ \t\r]+$/, "", s); return s }
function absd(x){ return (x<0 ? -x : x) }

# Detect format version based on header
function detect_format(header) {
    if (index(header, "FIRST") > 0) {
        return "v0.2.0"
    } else {
        return "v0.1.5"
    }
}

# Find column position by name
function find_column(header, name) {
    split(header, cols, ",")
    for (i = 1; i <= length(cols); i++) {
        gsub(/^[ \t\r]+|[ \t\r]+$/, "", cols[i])  # trim whitespace
        if (cols[i] == name) {
            return i
        }
    }
    return 0
}

# Compute CpredAlign from predicted values
# CpredAlign = Cpred_min - Align * ln^2(n0p) / (alpha * n0p)
# Align = 2.0 * sqrt(n0p) / (log log n0p)^2
function compute_cpred_align(n0p, cpred_min) {
    if (n0p < 3.0) {
        return cpred_min
    }
    log_n0p = log(n0p)
    loglog_n0p = log(log_n0p)
    align = 2.0 * sqrt(n0p) / (loglog_n0p * loglog_n0p)
    cpred_align = cpred_min - align * (log_n0p * log_n0p) / (alpha * n0p)
    if (cpred_align < 0.0) cpred_align = 0.0
    return cpred_align
}

# ---------- Pass 1: read file1 (summary), stash C_min by key ----------
FNR==NR {
    sub(/\r$/, "")
    if (FNR==1) {
        format = detect_format($0)
        if (format == "v0.2.0") {
            col_label = 3; col_minat = 4; col_n0 = 8; col_cmin = 9; col_ngeom = 12
        } else {
            col_label = 1; col_minat = 2; col_n0 = 6; col_cmin = 7; col_ngeom = 10
        }
        next  # skip header
    }
    
    # normalize fields we use
    label  = trim($col_label)
    minA = trim($col_minat)
    n_0  = trim($col_n0)
    c    = trim($col_cmin) + 0
    ngeo = trim($col_ngeom)

    # Validate key fields - empty values indicate data corruption
    if (label == "" || ngeo == "") {
        printf("ERROR: file1 (summary) has row with empty key fields at line %d: START='%s', n_geom='%s'\n",
               FNR, label, ngeo) > "/dev/stderr"
        exit 1
    }

    # build key: (START,n_geom) - unique identifier
    key = label "\034" ngeo
    cmin[key] = c
    n0[key] = n_0
    count1[key]++
    next
}

# ---------- Pass 2: file2 (HLA full output), use actual alignment values ----------
FNR==1 {
    format = detect_format($0)
    if (format == "v0.2.0") {
        # Use dynamic column detection for v0.2.0 format
        col_label2 = find_column($0, "START")
        col_minat_2 = find_column($0, "minAt*")
        col_n0_2 = find_column($0, "n_0*")
        col_cmin_2 = find_column($0, "Cpred_min(n_0*)")
        col_ngeom_2 = find_column($0, "n_geom")
        col_nalign = find_column($0, "n_v")
        col_calign = find_column($0, "Calign_min(n_v)")
        col_jitter = find_column($0, "jitter")
        
        # Validate required columns for v0.2.0
        missing_columns = 0
        if (col_label2 == 0) { print "ERROR: START column not found for " VERSION > "/dev/stderr"; missing_columns++ }
        if (col_n0_2 == 0) { print "ERROR: n_0* column not found for " VERSION > "/dev/stderr"; missing_columns++ }
        if (col_cmin_2 == 0) { print "ERROR: Cpred_min(n_0*) column not found for " VERSION > "/dev/stderr"; missing_columns++ }
        if (col_ngeom_2 == 0) { print "ERROR: n_geom column not found for " VERSION > "/dev/stderr"; missing_columns++ }
        if (col_calign == 0) { print "ERROR: Calign_min(n_v) column not found for " VERSION > "/dev/stderr"; missing_columns++ }
        if (missing_columns > 0) {
            print "ERROR: " missing_columns " required columns missing for " VERSION ". Cannot proceed." > "/dev/stderr"
            exit 1
        }
    } else {
        # v0.1.5 format: DECADE,MIN AT,MIN,MAX AT,MAX,n_0,Cpred_min,n_1,Cpred_max,N_geom,<COUNT>,Cpred_avg,HLCorr,n_u,Calign_max(n_u),n_v,Calign_min(n_v),n_a,CboundMin,jitter
        col_label2 = 1; col_minat_2 = 2; col_n0_2 = 6; col_cmin_2 = 7; col_ngeom_2 = 10; col_nalign = 0; col_calign = 0; col_jitter = 19
    }
    
    if(col_label2 == 1) {
       print "Dec","n_0","C_min","Npred_0","CpredAlign","Lambda_min","Jitter","JitterRatio"
    }
    else {
       print "START","n_0","C_min","Npred_0","CpredAlign","Lambda_min","Jitter","JitterRatio","Status"
    }
    next
}

{
    sub(/\r$/, "")
    
    label  = trim($col_label2)
    minA = trim($col_minat_2)
    np_0  = trim($col_n0_2) + 0
    cpred_min = trim($col_cmin_2) + 0
    ngeo = trim($col_ngeom_2)
    n_align = (col_nalign > 0) ? trim($col_nalign) + 0 : 0
    c_align = (col_calign > 0) ? trim($col_calign) + 0 : 0
    jitter = (col_jitter > 0) ? trim($col_jitter) + 0 : 0

    key = label "\034" ngeo  # Use START and n_geom for matching
    cmn = (key in cmin) ? cmin[key] : ""
    n = (key in n0) ? n0[key] : ""

    if (cmn=="") {
        # No match from file1 for this row
        printf("ERROR: unmatched key in file2: START=%s, n_geom=%s%s\n",
               label, ngeo, (label=="0"?sprintf(", minAt=%s",minA):"")) > "/dev/stderr"
        exit 1
    }

    # Use actual alignment values from HLA full output
    if (col_calign > 0) {
        # Use actual C_align value from HLA output (even if zero)
        cpred_align = c_align
    } else {
        # No alignment column available - this shouldn't happen with v0.1.5+ output
        cpred_align = cpred_min
    }

    # Calculate jitter ratio: (Cmeas - Calign)/jitter
    # Jitter is now normalized during generation
    jitter_ratio = 0.0
    if (jitter > 0) {
        c_diff = (cmn+0) - cpred_align
        jitter_ratio = c_diff / jitter
    }

    # Lambda_min = log(C_min/CpredAlign) in scientific notation; blank if C_min==0
    # Handle zero difference case: check raw count and use appropriate precision
    c_diff = (cmn+0) - cpred_align
    
    # Determine status based on difference
    if (absd(c_diff) < 1e-10) {
        status = "EXACT"
    } else if (c_diff > 0) {
        status = "EXPECTED"  # Observed > predicted (above minimum)
    } else {
        status = "OUTLIER"   # Observed < predicted (below minimum)
    }
    
    if ((cmn+0) > 0 && cpred_align > 0) {
        lambda_val = log((cmn+1e-12)/(cpred_align+1e-12))
        
        # If difference is zero and raw count is 0, omit the data point
        if (absd(c_diff) < 1e-10 && (cmn+0) == 0) {
            # Skip this data point - nothing to report
        } else {
            # Report with appropriate precision based on whether this is an average or not
            # For non-average cases, use 1e-6 precision; for average use 1e-8
            printf "%s,%d,%.6f,%d,%.6f,%.6e,%.6f,%.6e,%s\n", label, n, cmn, np_0, cpred_align, lambda_val, jitter, jitter_ratio, status
        }
    }
    seen[key]++
}

END {
    # Error if file1 had keys that didn't appear in file2
    for (k in count1) {
        if (!(k in seen)) {
            # reconstruct a readable note
            split(k, p, "\034")
            if (length(p)==2) {
                printf("ERROR: key present only in file1: START=%s, n_geom=%s\n",
                       p[1], p[2]) > "/dev/stderr"
            } else {
                printf("ERROR: key present only in file1: START=%s\n",
                       k) > "/dev/stderr"
            }
            exit 1
        }
    }
}

