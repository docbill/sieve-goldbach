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

BEGIN { FS=","; OFS="," }

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
        col_dec = 1
        col_n1 = 10
        col_cmax = 11
        col_ngeom = 12
    } else {
        col_dec = 1
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
        get_columns(format)
        next  # skip header
    }
    # normalize fields we use
    dec  = trim($col_dec)
    n_1  = trim($col_n1)
    c    = trim($col_cmax) + 0
    ngeo = trim($col_ngeom)

    # build key: (Dec,n_geom)
    key = dec SUBSEP ngeo
    cmax[key] = c
    n1[key] = n_1
    count1[key]++
    next
}

# ---------- Pass 2: file2, emit merged ----------
FNR==1 {
    print "Dec","n_1","C_max","Npred_1","Cpred_max","Lambda_max"
    next
}

{
    sub(/\r$/, "")
    dec  = trim($col_dec)
    minA = trim($2)
    np_1  = trim($col_n1)
    cp   = trim($col_cmax) + 0
    ngeo = trim($col_ngeom)

    key = dec SUBSEP ngeo
    cmx = (key in cmax) ? cmax[key] : ""
    n = (key in n1) ? n1[key] : ""

    if (cmx=="") {
        # No match from file1 for this row
        printf("WARN: unmatched key in file2: Dec=%s, n_geom=%s%s\n",
               dec, ngeo, (dec=="0"?sprintf(", minAt=%s",minA):"")) > "/dev/stderr"
        next
    }

    # Lambda_max = log(C_max/Cpred_max) in scientific notation; blank if C_max==0
    if ((cmx+0) > 0 && (cp+0) > 0) {
        printf "%d,%d,%.6f,%d,%.6f,%.6e\n", dec, n, cmx, np_1, cp, log((cmx+0)/(cp+0))
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
                printf("WARN: key present only in file1: Dec=%s, n_geom=%s, minAt=%s\n",
                       p[1], p[3], p[2]) > "/dev/stderr"
            } else {
                printf("WARN: key present only in file1: Dec=%s, n_geom=%s\n",
                       p[1], p[2]) > "/dev/stderr"
            }
        }
    }
}

