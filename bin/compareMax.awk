#!/usr/bin/awk -f
# compareMax - joins summary and HL-A prediction for max values
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

# Usage: awk -f compareMax.awk file1.csv file2.csv > merged.csv
# v0.1.5: A=Dec, H=n_1, I=C_max, J=n_geom
# v0.2.0: A=FIRST, J=n_1, K=C_max(n_1), L=n_geom
# For decade 0, include minAt (col B) in the key.

BEGIN {
    FS=","; OFS=","
    
    # Global variables
    col_label = 0
    col_minat = 0
    col_n1 = 0
    col_cmax = 0
    col_ngeom = 0
    
    # Second file variables
    col_label2 = 0
    col_minat_2 = 0
    col_n1_2 = 0
    col_cmax_2 = 0
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

# Get column numbers based on format
function get_columns(format) {
    if (format == "v0.2.0") {
        col_label = 3
        col_n1 = 10
        col_cmax = 11
        col_ngeom = 12
    } else {
        col_label = 1
        col_n1 = 8
        col_cmax = 9
        col_ngeom = 10
    }
}

# ---------- Pass 1: read file1, stash C_max by key ----------
FNR==NR {
    sub(/\r$/, "")
    if (FNR==1) {
        format = detect_format($0)
        if (format == "v0.2.0") {
            col_label = 3; col_minat = 4; col_n1 = 10; col_cmax = 11; col_ngeom = 12
        } else {
            col_label = 1; col_minat = 2; col_n1 = 8; col_cmax = 9; col_ngeom = 10
        }
        next  # skip header
    }
    
    # Set column variables for first file if not already set
    if (col_label == 0) {
        if (index($0, "FIRST") > 0) {
            col_label = 3; col_minat = 4; col_n1 = 10; col_cmax = 11; col_ngeom = 12
        } else {
            col_label = 1; col_minat = 2; col_n1 = 8; col_cmax = 9; col_ngeom = 10
        }
    }
    
    # normalize fields we use
    label  = trim($col_label)
    n_1  = trim($col_n1)
    c    = trim($col_cmax) + 0
    ngeo = trim($col_ngeom)

    # Validate key fields - empty values indicate data corruption
    if (label == "" || ngeo == "") {
        printf("ERROR: file1 (summary) has row with empty key fields at line %d: START='%s', n_geom='%s'\n",
               FNR, label, ngeo) > "/dev/stderr"
        exit 1
    }

    # build key: (Dec,n_geom)
    key = label "\034" ngeo
    cmax[key] = c
    n1[key] = n_1
    count1[key]++
    next
}

# ---------- Pass 2: file2, emit merged ----------
FNR==1 {
    format = detect_format($0)
    if (format == "v0.2.0") {
        col_label2 = 3; col_minat_2 = 4; col_n1_2 = 10; col_cmax_2 = 11; col_ngeom_2 = 12
    } else {
        col_label2 = 1; col_minat_2 = 2; col_n1_2 = 8; col_cmax_2 = 9; col_ngeom_2 = 10
    }
    
    if(col_label2 == 1) {
        print "Dec","n_1","C_max","Npred_1","Cpred_max","Lambda_max"
    }
    else {
        print "START","n_1","C_max","Npred_1","Cpred_max","Lambda_max"
    }
    next
}

{
    sub(/\r$/, "")
    
    # Set column variables for second file if not already set
    if (col_label2 == 0) {
        if (index($0, "FIRST") > 0) {
            col_label2 = 3; col_minat_2 = 4; col_n1_2 = 10; col_cmax_2 = 11; col_ngeom_2 = 12
        } else {
            col_label2 = 1; col_minat_2 = 2; col_n1_2 = 8; col_cmax_2 = 9; col_ngeom_2 = 10
        }
    }
    
    label  = trim($col_label2)
    minA = trim($col_minat_2)
    np_1  = trim($col_n1_2)
    cp   = trim($col_cmax_2) + 0
    ngeo = trim($col_ngeom_2)

    key = label "\034" ngeo
    cmx = (key in cmax) ? cmax[key] : ""
    n = (key in n1) ? n1[key] : ""

    if (cmx=="") {
        # No match from file1 for this row
        printf("ERROR: unmatched key in file2: Dec=%s, n_geom=%s%s\n",
               label, ngeo, (label=="0"?sprintf(", minAt=%s",minA):"")) > "/dev/stderr"; exit 1
        next
    }

    # Lambda_max = log(C_max/Cpred_max) in scientific notation; blank if C_max==0
    # TODO: Consider adding ratio column (cmx/cp) alongside lambda for easier debugging
    if ((cmx+0) > 0 && (cp+0) > 0) {
        printf "%s,%d,%.6f,%d,%.6f,%.6e\n", label, n, cmx, np_1, cp, log((cmx+1e-12)/(cp+1e-12))
    }
    seen[key]++
}

END {
    # Warn if file1 had keys that didn’t appear in file2
    for (k in count1) {
        if (!(k in seen)) {
            # reconstruct a readable note
            split(k, p, SUBSEP)
            if (length(p)==3) {
                printf("ERROR: key present only in file1: Dec=%s, n_geom=%s, minAt=%s\n",
                       p[1], p[3], p[2]) > "/dev/stderr"; exit 1
            } else {
                printf("ERROR: key present only in file1: Dec=%s, n_geom=%s\n",
                       p[1], p[2]) > "/dev/stderr"; exit 1
            }
        }
    }
}

