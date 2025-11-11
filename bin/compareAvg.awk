#!/usr/bin/awk -f
# compareAvg - generates combines summary and SGB file for a avg summary
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

# Usage: awk -f compareAvg.awk file1.csv file2.csv > merged.csv
# v0.1.5: A=Dec, J=n_geom, L=C_avg
# v0.2.0: A=FIRST, L=n_geom, N=C_avg
# For decade 0, include minAt (col B) in the key.

BEGIN {
    FS=","; OFS=","
    
    # Global variables
    col_label = 0
    col_minat = 0
    col_ngeom = 0
    col_cavg = 0
    
    # Second file variables
    col_label2 = 0
    col_minat_2 = 0
    col_ngeom_2 = 0
    col_cavg_2 = 0
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
        col_minat = 4
        col_ngeom = 12
        col_cavg = 14
    } else {
        col_label = 1
        col_minat = 2
        col_ngeom = 10
        col_cavg = 12
    }
}

# ---------- Pass 1: read file1, stash C_avg by key ----------
FNR==NR {
    sub(/\r$/, "")
    if (FNR==1) {
        format = detect_format($0)
        if (format == "v0.2.0") {
            col_label = 3
            col_minat = 4
            col_ngeom = 12
            col_cavg = 14
        } else {
            col_label = 1
            col_minat = 2
            col_ngeom = 10
            col_cavg = 12
        }
        next  # skip header
    }
    
    # Columns are already set in header processing
    
    # normalize fields we use
    label  = trim($col_label)
    minA = trim($col_minat)
    ngeo = trim($col_ngeom)
    c    = trim($col_cavg) + 0

    # build key: (Dec,n_geom) normally; (Dec,minAt,n_geom) for decade 0
    key = (label=="0") ? (label "\034" minA "\034" ngeo) : (label "\034" ngeo)
    
    cavg[key] = c
    count1[key]++
    next
}

# ---------- Pass 2: file2, emit merged ----------
FNR==1 {
    # Detect format from second file header and set columns
    format = detect_format($0)
    if (format == "v0.2.0") {
        col_label2 = 3
        col_minat_2 = 4
        col_ngeom_2 = 12
        col_cavg_2 = 14
    } else {
        col_label2 = 1
        col_minat_2 = 2
        col_ngeom_2 = 10
        col_cavg_2 = 12
    }
    
    if(col_label2 == 1) {
        print "Dec","n_geom","C_avg","Cpred_avg","Lambda_avg"
    }
    else {
        print "START","n_geom","C_avg","Cpred_avg","Lambda_avg"
    }
    next
}

{
    sub(/\r$/, "")
    
    # Set column variables for second file if not already set
    if (col_label2 == 0) {
        # This is the first data row of the second file, set columns
        if (index($0, "FIRST") > 0) {
            col_label2 = 3
            col_minat_2 = 4
            col_ngeom_2 = 12
            col_cavg_2 = 14
        } else {
            col_label2 = 1
            col_minat_2 = 2
            col_ngeom_2 = 10
            col_cavg_2 = 12
        }
    }
    
    label  = trim($col_label2)
    minA = trim($col_minat_2)
    ngeo = trim($col_ngeom_2)
    cp   = trim($col_cavg_2) + 0

    key = (label=="0") ? (label "\034" minA "\034" ngeo) : (label "\034" ngeo)
    cav = (key in cavg) ? cavg[key] : ""

    if (cav=="") {
        # No match from file1 for this row
        if(col_label2 == 1) {
            printf("ERROR: unmatched key in file2: Dec=%s, n_geom=%s%s\n",
               label, ngeo, (label=="0"?sprintf(", minAt=%s",minA):"")) > "/dev/stderr"; exit 1
        }
        else {
            printf("ERROR: unmatched key in file2: START=%s, n_geom=%s%s\n",
               label, ngeo, (label=="0"?sprintf(", minAt=%s",minA):"")) > "/dev/stderr"; exit 1
        }
        next
    }

    # Lambda_avg = log(C_avg/Cpred_avg) in scientific notation; blank if C_avg==0
    # TODO: Consider adding ratio column (cav/cp) alongside lambda for easier debugging
    if ((cav+0) > 0 && (cp+0) > 0) {
        printf "%s,%d,%.6f,%.6f,%.6e\n", label, ngeo, cav, cp, log((cav+1e-12)/(cp+1e-12))
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

