#!/usr/bin/env awk -f
# Convert HLA (Hardy-Littlewood A) full files to raw files
# Usage: awk -f full2raw_hla.awk input.full.csv > output.raw.csv
# Preserves exact formatting by copying column values as strings
# Only supports v0.2.0 format (FIRST,LAST,START) - legacy format (DECADE) will error

BEGIN {
    FS = ","
    OFS = ","
}

# Process header
NR == 1 {
    # Check if this is v0.2.0 format (has FIRST,LAST,START) or legacy (DECADE)
    if (index($0, "FIRST") > 0) {
        # v0.2.0 format: FIRST,LAST,START,minAt*,Gpred(minAt*),maxAt*,Gpred(maxAt*),n_0*,Cpred_min(n_0*),n_1*,Cpred_max(n_1*),n_geom,<COUNT>*,Cpred_avg,n_v,Calign_min(n_v),n_u,Calign_max(n_u),n_a,CboundMin(n_a),n_b,CboundMax(n_b),jitter
        # Raw format: FIRST,LAST,START,minAt*,Gpred(minAt*),maxAt*,Gpred(maxAt*),n_geom,<COUNT>*
        print "FIRST,LAST,START,minAt*,Gpred(minAt*),maxAt*,Gpred(maxAt*),n_geom,<COUNT>*"
    } else {
        # Legacy format not supported - only v0.2.0 format is supported
        print "ERROR: Legacy format (DECADE) not supported. Only v0.2.0 format (FIRST,LAST,START) is supported." > "/dev/stderr"
        exit 1
    }
    next
}

# Process data rows - copy column values as strings to preserve formatting
# Only v0.2.0 format reaches here (legacy format exits in header processing)
{
    sub(/\r$/, "")  # Remove Windows line endings
    
    # Extract: FIRST(1), LAST(2), START(3), minAt*(4), Gpred(minAt*)(5), maxAt*(6), Gpred(maxAt*)(7), n_geom(12), <COUNT>*(13)
    # Copy as strings to preserve exact formatting
    if (NF >= 13) {
        printf "%s,%s,%s,%s,%s,%s,%s,%s,%s\n", $1, $2, $3, $4, $5, $6, $7, $12, $13
    }
}
