#!/usr/bin/awk -f
# mergeCPSLowerBound.awk - Merge CPS lower bound files with inheritance for blank values
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
#
# SPDX-License-Identifier: GPL-3.0-or-later

BEGIN {
    # Initialize inherited values
    inherited_preMertens = ""
    inherited_preMertensAsymp = ""
    header_printed = 0
    is_old_format = 0
}

# Process header lines (first line or lines that look like headers)
NR == 1 || ($0 ~ /^Dec/ || $0 ~ /^n,C\(n\)/) {
    # Check if this is old format (starts with "Dec")
    if ($0 ~ /^Dec/) {
        is_old_format = 1
    }
    
    if (header_printed == 0) {
        print $0
        header_printed = 1
    }
    # Skip subsequent headers (don't print them)
    next
}

# Process data lines
{
    if (is_old_format) {
        # Old format - just pass through (no inheritance needed)
        print $0
    } else {
        # New format - apply inheritance rules to preMertens and preMertensAsymp
        nf = split($0, fields, ",")
        
        # Extract preMertens (7th column) and preMertensAsymp (8th column)
        preMertens = fields[7]
        preMertensAsymp = fields[8]
        
        # If preMertens is blank, inherit from previous line
        if (preMertens == "" && inherited_preMertens != "") {
            fields[7] = inherited_preMertens
        } else if (preMertens != "") {
            # Update inherited value
            inherited_preMertens = preMertens
        }
        
        # If preMertensAsymp is blank, inherit from previous line
        if (preMertensAsymp == "" && inherited_preMertensAsymp != "") {
            fields[8] = inherited_preMertensAsymp
        } else if (preMertensAsymp != "") {
            # Update inherited value
            inherited_preMertensAsymp = preMertensAsymp
        }
        
        # Reconstruct the line
        output = fields[1]
        for (i = 2; i <= nf; i++) {
            output = output "," fields[i]
        }
        print output
    }
}
