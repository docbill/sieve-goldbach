#!/bin/bash

# Create fake HUGE empirical partial files from hl-a partial files
# Uses colrm to remove columns to convert hl-a format to empirical format

# This script is to allow debugging of the makefiles merging code, etc while data analysis is pending.
# The quality of the fake data is very low, so use the environmental variable TAINTED=1 when using.


set -eo pipefail

# Get header from an empirical full file
EMP_FILE="output/alpha-0.5/gbpairsummary-23PR.5-empirical-full-0.5-v0.2.0.csv"
if [ ! -f "$EMP_FILE" ]; then
    EMP_FILE=$(find output/ -name "*23PR.5*empirical-full*.csv" ! -name "*.partial.csv" -type f | head -1)
fi

if [ ! -f "$EMP_FILE" ]; then
    echo "Error: Could not find empirical full file for headers" >&2
    exit 1
fi

HEADER=$(head -1 "$EMP_FILE")
echo "Using header from: $EMP_FILE"

# Get all alpha values (same calculation as Makefile)
ALPHAS=$(awk 'BEGIN{r=exp(log(2)/8);a=exp(log(2)*-10);eps=1e-12;while(a<1-eps){printf "%.12g ",a;a*=r}print "1"}')

echo "Processing alpha values..."

# XPRIM suffix mapping: A->19PR23D2, B->19PR24D2, ..., W->19PR45D2
xprim_suffixes=(19PR23D2 19PR24D2 19PR25D2 19PR26D2 19PR27D2 19PR28D2 19PR29D2 19PR30D2 \
                19PR31D2 19PR32D2 19PR33D2 19PR34D2 19PR35D2 19PR36D2 19PR37D2 19PR38D2 \
                19PR39D2 19PR40D2 19PR41D2 19PR42D2 19PR43D2 19PR44D2 19PR45D2)

# Process each HUGE/XPRIM part pair (A-W)
for part in A B C D E F G H I J K L M N O P Q R S T U V W; do
    for alpha in $ALPHAS; do
        # Try both v0.1.5 and v0.2.0 versions
        for version in v0.1.5 v0.2.0; do
            # Process dummy HUGE files - just touch them since they're empty
            emp_file="output/alpha-$alpha/gbpairsummary-dummy${part}-empirical-full-$alpha-$version.partial.csv"
            
            # Touch the dummy empirical file to ensure it exists (it will be empty)
            touch "$emp_file"
            
            # Process real XPRIM files - convert hl-a to empirical
            # Calculate index: A=0, B=1, ..., W=22
            part_index=$(($(printf '%d' "'$part") - 65))
            xprim_suffix="${xprim_suffixes[$part_index]}"
            xprim_hl_a="output/alpha-$alpha/gbpairsummary-${xprim_suffix}-hl-a-full-$alpha-$version.partial.csv"
            xprim_emp="output/alpha-$alpha/gbpairsummary-${xprim_suffix}-empirical-full-$alpha-$version.partial.csv"
            
            # If hl-a file exists and has data, ensure empirical file is populated
            if [ -f "$xprim_hl_a" ] && [ -s "$xprim_hl_a" ] && [ $(wc -l < "$xprim_hl_a") -gt 1 ]; then
                # Check if empirical file exists but only has header (needs conversion)
                if [ ! -f "$xprim_emp" ] || [ $(wc -l < "$xprim_emp") -le 1 ]; then
                    # Create/overwrite empirical file with header
                    echo "$HEADER" > "$xprim_emp"
                    
                    # Convert hl-a format to empirical format
                    tail -n +2 "$xprim_hl_a" | while IFS= read -r line; do
                        if [ -n "$line" ]; then
                            echo "$line" | colrm 10- >> "$xprim_emp" 2>/dev/null || echo "$line" >> "$xprim_emp"
                        fi
                    done
                    
                    echo "Created/updated: $xprim_emp (from $xprim_hl_a)"
                fi
            fi
        done
    done
done

echo "Done creating fake HUGE empirical partial files"
