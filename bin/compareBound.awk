#!/usr/bin/awk -f
# compareBound - combines summary and HLA full output for conservative bound comparison
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

# Usage: awk -f compareBound.awk pairrangesummary.csv pairrange2sgbll-full.csv > merged.csv
# v0.1.5: A=Dec, F=n_0, G=C_min, J=n_geom
# v0.2.0: A=FIRST, H=n_0, I=C_min(n_0), L=n_geom, Q=n_align, R=C_align, S=n_cBound, T=c_cBound
# Uses actual conservative bound values from HLA full output instead of computing approximate values

BEGIN {
    FS=","; OFS=","
    
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
    col_nbound = 0
    col_cbound = 0
    col_jitter = 0
}

function trim(s){ sub(/^[ \t\r]+/, "", s); sub(/[ \t\r]+$/, "", s); return s }

# Detect format version based on header
function detect_format(header) {
    if (index(header, "FIRST") > 0) {
        return "v0.2.0"
    } else {
        return "v0.1.5"
    }
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

    # build key: (Dec,n_geom)
    key = label "\034" ngeo
    cmin[key] = c
    n0[key] = n_0
    count1[key]++
    next
}

# ---------- Pass 2: file2 (HLA full output), use actual conservative bound values ----------
FNR==1 {
    format = detect_format($0)
    if (format == "v0.2.0") {
        # HLA v0.2.0 format: FIRST,LAST,START,minAt*,Gpred(minAt*),maxAt*,Gpred(maxAt*),n_0*,Cpred_min(n_0*),n_1*,Cpred_max(n_1*),n_geom,<COUNT>*,Cpred_avg,n_alignMax,c_alignMax,n_alignMin,c_alignMin,n_cBound,c_cBound,jitter
        col_label2 = 3; col_minat_2 = 4; col_n0_2 = 8; col_cmin_2 = 9; col_ngeom_2 = 12; col_nbound = 19; col_cbound = 20; col_jitter = 21
    } else {
        # v0.1.5 format: DECADE,MIN AT,MIN,MAX AT,MAX,n_0,Cpred_min,n_1,Cpred_max,N_geom,<COUNT>,Cpred_avg,HLCorr,n_alignMax,c_alignMax,n_alignMin,c_alignMin,n_cBound,c_cBound,jitter
        col_label2 = 1; col_minat_2 = 2; col_n0_2 = 6; col_cmin_2 = 7; col_ngeom_2 = 10; col_nbound = 0; col_cbound = 0; col_jitter = 19
    }
    
    if(col_label2 == 1) {
       print "Dec","n_0","C_min","Npred_0","CpredBound","Lambda_min","Jitter","JitterRatio"
    }
    else {
       print "START","n_0","C_min","Npred_0","CpredBound","Lambda_min","Jitter","JitterRatio"
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
    n_bound = (col_nbound > 0) ? trim($col_nbound) + 0 : 0
    c_bound = (col_cbound > 0) ? trim($col_cbound) + 0 : 0
    jitter = (col_jitter > 0) ? trim($col_jitter) + 0 : 0

    key = label "\034" ngeo
    cmn = (key in cmin) ? cmin[key] : ""
    n = (key in n0) ? n0[key] : ""

    if (cmn=="") {
        # No match from file1 for this row
        printf("ERROR: unmatched key in file2: Dec=%s, n_geom=%s%s\n",
               label, ngeo, (label=="0"?sprintf(", minAt=%s",minA):"")) > "/dev/stderr"
        exit 1
    }

    # Use actual conservative bound values from HLA full output
    if (col_cbound > 0) {
        # Use actual c_cBound value from HLA output (even if zero)
        cpred_bound = c_bound
    } else {
        # No bound column available - this shouldn't happen with v0.1.5+ output
        cpred_bound = cpred_min
    }

    # Calculate jitter ratio: (Cmeas_min_raw-CpredBound_raw)/[jitter]
    # where C_raw = C * αn/log²n
    jitter_ratio = 0.0
    if (jitter > 0 && np_0 > 0) {
        log_n = log(np_0)
        log_squared = log_n * log_n
        alpha_n = alpha * np_0
        conversion_factor = alpha_n / log_squared
        
        c_min_raw = (cmn+0) * conversion_factor
        c_bound_raw = cpred_bound * conversion_factor
        c_diff_raw = c_min_raw - c_bound_raw
        jitter_ratio = c_diff_raw / jitter
    }

    # Lambda_min = log(C_min/CpredBound) in scientific notation; blank if C_min==0
    if ((cmn+0) > 0 && cpred_bound > 0) {
        printf "%s,%d,%.6f,%d,%.6f,%.6e,%.6f,%.6e\n", label, n, cmn, np_0, cpred_bound, log((cmn+0)/cpred_bound), jitter, jitter_ratio
    }
    seen[key]++
}

END {
    # Error if file1 had keys that didn't appear in file2
    for (k in count1) {
        if (!(k in seen)) {
            # reconstruct a readable note
            split(k, p, SUBSEP)
            if (length(p)==3) {
                printf("ERROR: key present only in file1: Dec=%s, n_geom=%s, minAt=%s\n",
                       p[1], p[3], p[2]) > "/dev/stderr"
            } else {
                printf("ERROR: key present only in file1: Dec=%s, n_geom=%s\n",
                       p[1], p[2]) > "/dev/stderr"
            }
            exit 1
        }
    }
}
