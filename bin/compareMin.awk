#!/usr/bin/awk -f
# compareMin - generates combines summary and SGB file for a min summary
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

# Usage: awk -f compareMin.awk file1.csv file2.csv > merged.csv
# v0.1.5: A=Dec, F=n_0, G=C_min, J=n_geom
# v0.2.0: A=FIRST, H=n_0, I=C_min(n_0), L=n_geom
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
        col_minat = 2
        col_n0 = 8
        col_cmin = 9
        col_ngeom = 12
    } else {
        col_dec = 1
        col_minat = 2
        col_n0 = 6
        col_cmin = 7
        col_ngeom = 10
    }
}

# ---------- Pass 1: read file1, stash C_min by key ----------
FNR==NR {
    sub(/\r$/, "")
    if (FNR==1) {
        format = detect_format($0)
        get_columns(format)
        next  # skip header
    }
    # normalize fields we use
    dec  = trim($col_dec)
    minA = trim($col_minat)
    n_0  = trim($col_n0)
    c    = trim($col_cmin) + 0
    ngeo = trim($col_ngeom)

    # build key: (Dec,n_geom) normally; (Dec,minAt,n_geom) for decade 0
    key = dec SUBSEP ngeo
    cmin[key] = c
    n0[key] = n_0
    count1[key]++
    next
}

# ---------- Pass 2: file2, emit merged ----------
FNR==1 {
    print "Dec","n_0","C_min","Npred_0","Cpred_min","Lambda_min"
    next
}

{
    sub(/\r$/, "")
    dec  = trim($col_dec)
    minA = trim($col_minat)
    np_0  = trim($col_n0)
    cp   = trim($col_cmin) + 0
    ngeo = trim($col_ngeom)

    key = dec SUBSEP ngeo
    cmn = (key in cmin) ? cmin[key] : ""
    n = (key in n0) ? n0[key] : ""

    if (cmn=="") {
        # No match from file1 for this row
        printf("WARN: unmatched key in file2: Dec=%s, n_geom=%s%s\n",
               dec, ngeo, (dec=="0"?sprintf(", minAt=%s",minA):"")) > "/dev/stderr"
        next
    }

    # Lambda_min = log(C_min/Cpred_min) in scientific notation; blank if C_min==0
    if ((cmn+0) > 0 && (cp+0) > 0) {
        printf "%d,%d,%.6f,%d,%.6f,%.6e\n", dec, n, cmn, np_0, cp, log((cmn+0)/(cp+0))
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

