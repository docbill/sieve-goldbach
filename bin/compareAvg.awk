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
# file1: A=Dec, J=n_geom, L=C_avg
# file2: A=Dec, J=n_geom, L=Cpred_avg
# For decade 0, include minAt (col B) in the key.

BEGIN { FS=","; OFS="," }

function trim(s){ sub(/^[ \t\r]+/, "", s); sub(/[ \t\r]+$/, "", s); return s }

# ---------- Pass 1: read file1, stash C_avg by key ----------
FNR==NR {
    sub(/\r$/, "")
    if (FNR==1) next  # skip header
    # normalize fields we use
    dec  = trim($1)
    minA = trim($2)
    ngeo = trim($10)
    c    = trim($12) + 0

    # build key: (Dec,n_geom) normally; (Dec,minAt,n_geom) for decade 0
    key = (dec=="0") ? (dec SUBSEP minA SUBSEP ngeo) : (dec SUBSEP ngeo)
    cavg[key] = c
    count1[key]++
    next
}

# ---------- Pass 2: file2, emit merged ----------
FNR==1 {
    print "Dec","n_geom","C_avg","Cpred_avg","Lambda_avg"
    next
}

{
    sub(/\r$/, "")
    dec  = trim($1)
    minA = trim($2)
    ngeo = trim($10)
    cp   = trim($12) + 0

    key = (dec=="0") ? (dec SUBSEP minA SUBSEP ngeo) : (dec SUBSEP ngeo)
    cav = (key in cavg) ? cavg[key] : ""

    if (cav=="") {
        # No match from file1 for this row
        printf("WARN: unmatched key in file2: Dec=%s, n_geom=%s%s\n",
               dec, ngeo, (dec=="0"?sprintf(", minAt=%s",minA):"")) > "/dev/stderr"
        next
    }

    # Lambda_avg = log(C_avg/Cpred_avg) in scientific notation; blank if C_avg==0
    if ((cav+0) > 0 && (cp+0) > 0) {
        printf "%d,%d,%.6f,%.6f,%.6e\n", dec, ngeo, cav, cp, log((cav+0)/(cp+0))
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

