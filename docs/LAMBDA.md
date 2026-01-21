# Lambda Statistics Documentation

## Overview

This document explains the behavior of lambda statistics generation and why certain decades may be missing from the output files.

## Lambda Definition

Lambda is defined as:
```
λ = log(C_observed / C_predicted)
```

This means lambda is **undefined** when either:
- `C_observed = 0` (no observed pairs in that range)
- `C_predicted = 0` (HL-A model predicts no pairs)

## File Types and Data Filtering

### Lambda Average Files (`lambdaavg-*.csv`)
- **Source**: Empirical data vs HL-A predictions
- **Filtering**: Rows with `C_avg = 0` are included (lambda = undefined)
- **Decade calculation**: Uses `n_geom` values
- **Result**: Typically has data for all decades

### Lambda Minimum Files (`lambdamin-*.csv`)
- **Source**: Empirical data vs HL-A predictions  
- **Filtering**: Rows with `G(minAt) = 0` are **excluded** (lambda = undefined)
- **Decade calculation**: Uses `n_0` values
- **Result**: May have missing decades due to insufficient valid data points

### Lambda Maximum Files (`lambdamax-*.csv`)
- **Source**: Empirical data vs HL-A predictions
- **Filtering**: Rows with `G(maxAt) = 0` are **excluded** (lambda = undefined)  
- **Decade calculation**: Uses `n_1` values
- **Result**: Typically has data for all decades

## Missing Decades in Statistics

### Why Decades May Be Missing

The `lambdaStats.awk` script requires **at least 5 valid lambda values** per decade to generate statistics. Decades with fewer than 5 values are excluded from the output.

### Common Scenarios

1. **`lambdastatsmin` missing decade 1**: 
   - The `lambdamin` files filter out rows where `G(minAt) = 0`
   - This removes most data points that would fall in decade 1
   - If fewer than 5 valid minimum lambda values remain in decade 1, it's excluded

2. **`lambdastatsavg` and `lambdastatsmax` include decade 1**:
   - These files typically have more data points per decade
   - The filtering is less aggressive or the data distribution is different

### Mathematical Correctness

This behavior is **mathematically correct**:
- Lambda is undefined for zero values
- Statistics with fewer than 5 data points are not meaningful
- The missing decades indicate insufficient valid data, not a bug

## Decade Calculation

### v0.1.5 Files
- Uses decade values directly from column 1
- Format: `Dec,n_geom,C_avg,Cpred_avg,Lambda_avg`

### v0.2.0 Files  
- Calculates decade from `n_geom` values using `calculate_decade()` function
- Format: `START,n_geom,C_avg,Cpred_avg,Lambda_avg`
- Decade ranges:
  - 0: n < 10
  - 1: 10 ≤ n < 100  
  - 2: 100 ≤ n < 1000
  - 3: 1000 ≤ n < 10000
  - etc.

## Interpretation

When plotting lambda statistics:

1. **Missing decades are expected** when there's insufficient valid data
2. **Don't try to "fix" missing decades** - they indicate the mathematical reality
3. **Focus on decades with sufficient data** for meaningful analysis
4. **Consider the filtering behavior** when comparing different lambda file types

## Key Takeaway

The HL-A model never predicts 0 pairs, so comparing HL-A predictions against empirical data that has 0 observed pairs is not meaningful. The missing decades in `lambdastatsmin` reflect this mathematical reality, not a processing error.
