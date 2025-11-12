#!/bin/bash

# Create dummy files for HUGE partial files to test the Makefile logic
# This allows testing merge and verification steps without generating actual data

set -euo pipefail

# Create output directories for all alphas
for alpha in 0.0009765625 0.001953125 0.00390625 0.0078125 0.015625 0.03125 0.0625 0.125 0.25 0.5; do
    mkdir -p "output/alpha-$alpha"
done

# Create dummy content (minimal CSV with header)
DUMMY_CSV="n,count,sum
1000000,0,0
"

# Create dummy files for HUGE parts (A-W)
for part in A B C D E F G H I J K L M N O P Q R S T U V W; do
    for alpha in 0.0009765625 0.001953125 0.00390625 0.0078125 0.015625 0.03125 0.0625 0.125 0.25 0.5; do
        # HUGE decade partial files (dummy)
        echo "$DUMMY_CSV" > "output/alpha-$alpha/gbpairsummary-dummy${part}-empirical-full-$alpha-v0.1.5.partial.csv"
        echo "$DUMMY_CSV" > "output/alpha-$alpha/gbpairsummary-dummy${part}-empirical-cps-$alpha-v0.1.5.partial.csv"
        
        # XPRIM primorial partial files (dummy)
        echo "$DUMMY_CSV" > "output/alpha-$alpha/gbpairsummary-23PR${part}-empirical-full-$alpha-v0.1.5.partial.csv"
        echo "$DUMMY_CSV" > "output/alpha-$alpha/gbpairsummary-23PR${part}-empirical-cps-$alpha-v0.1.5.partial.csv"
    done
done

echo "Created dummy HUGE partial files for testing"
