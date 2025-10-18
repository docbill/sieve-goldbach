#!/usr/bin/awk -f
# compareAlign - combines summary and SGB file for alignment-corrected min comparison
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

# Usage: awk -f compareAlign.awk -v alpha=0.5 pairrangesummary.csv pairrange2sgbll.csv > merged.csv
# v0.1.5: A=Dec, F=n_0, G=C_min, J=n_geom
# v0.2.0: A=FIRST, H=n_0, I=C_min(n_0), L=n_geom
# Computes CpredAlign = Cpred_min - Align * ln^2(n0p) / (alpha * n0p)

BEGIN {
    FS=","; OFS=","
    
    # Alpha parameter (should be passed via -v alpha=VALUE)
    if (alpha == "") {
        alpha = 0.5
        print "WARNING: alpha not provided, defaulting to 0.5" > "/dev/stderr"
    }
    
    # Global variables
    col_label = 0
    col_minat = 0
    col_n0 = 0
    col_cmin = 0
    col_ngeom = 0
    
    # Second file variables
    col_label2 = 0
    col_minat_2 = 0
    col_n0_2 = 0
    col_cmin_2 = 0
    col_ngeom_2 = 0
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

    # build key: (Dec,n_geom)
    key = label "\034" ngeo
    cmin[key] = c
    n0[key] = n_0
    count1[key]++
    next
}

# ---------- Pass 2: file2 (SGB), compute CpredAlign and emit merged ----------
FNR==1 {
    format = detect_format($0)
    if (format == "v0.2.0") {
        col_label2 = 3; col_minat_2 = 4; col_n0_2 = 8; col_cmin_2 = 9; col_ngeom_2 = 12
    } else {
        col_label2 = 1; col_minat_2 = 2; col_n0_2 = 6; col_cmin_2 = 7; col_ngeom_2 = 10
    }
    
    if(col_label2 == 1) {
       print "Dec","n_0","C_min","Npred_0","CpredAlign","Lambda_min"
    }
    else {
       print "START","n_0","C_min","Npred_0","CpredAlign","Lambda_min"
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

    key = label "\034" ngeo
    cmn = (key in cmin) ? cmin[key] : ""
    n = (key in n0) ? n0[key] : ""

    if (cmn=="") {
        # No match from file1 for this row
        printf("WARN: unmatched key in file2: Dec=%s, n_geom=%s%s\n",
               label, ngeo, (label=="0"?sprintf(", minAt=%s",minA):"")) > "/dev/stderr"
        next
    }

    # Compute CpredAlign using the alignment correction
    cpred_align = compute_cpred_align(np_0, cpred_min)

    # Lambda_min = log(C_min/CpredAlign) in scientific notation; blank if C_min==0
    if ((cmn+0) > 0 && cpred_align > 0) {
        printf "%s,%d,%.6f,%d,%.6f,%.6e\n", label, n, cmn, np_0, cpred_align, log((cmn+0)/cpred_align)
    }
    seen[key]++
}

END {
    # Warn if file1 had keys that didn't appear in file2
    for (k in count1) {
        if (!(k in seen)) {
            # reconstruct a readable note
            split(k, p, SUBSEP)
            if (length(p)==3) {
                printf("WARN: key present only in file1: Dec=%s, n_geom=%s, minAt=%s\n",
                       p[1], p[3], p[2]) > "/dev/stderr"
            } else {
                printf("WARN: key present only in file1: Dec=%s, n_geom=%s\n",
                       p[1], p[2]) > "/dev/stderr"
            }
        }
    }
}

