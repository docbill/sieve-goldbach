# Primorial Bin Sizing Documentation

## Overview

The primorial bin sizing system in `gbprimorial.cpp` uses full primorial boundaries to determine bin sizes, ensuring consistent statistical properties and mathematical significance.

## How Bin Sizing Works

The bin sizing logic operates as follows:

1. **At `19#/2`**: 
   - `odd_prim_base_and_next(19#/2, &thresholdMinor, &thresholdMajor)` returns:
     - `thresholdMinor = 17#` (largest primorial ≤ `19#/2`)
     - `thresholdMajor = 19#` (next primorial after `17#`)
   - Bin size remains `17#` until reaching `19#`

2. **At `19#`**:
   - `odd_prim_base_and_next(19#, &thresholdMinor, &thresholdMajor)` returns:
     - `thresholdMinor = 19#` (largest primorial ≤ `19#`)
     - `thresholdMajor = 23#` (next primorial after `19#`)
   - Bin size changes to `19#`

## Design Principles

**Full Primorial Boundaries:**
- Bin size changes when you reach the next primorial (e.g., `19#`)
- Not when you reach half of it (e.g., `19#/2`)

**Mathematical Benefits:**
- **Complete parity coverage** - Each bin contains both odd and even values
- **Statistically meaningful** - Each bin contains a full primorial's worth of data
- **Consistent properties** - Bin sizes remain constant for long periods
- **Mathematical significance** - Primorial boundaries are natural mathematical landmarks

## Statistical Advantages

**Constant Bin Sizes:**
- **No artificial trends** - Changes in statistics reflect real mathematical behavior, not bin size effects
- **Clean convergence analysis** - You can distinguish real convergence from sampling artifacts
- **Consistent aggregation** - Statistical measures remain comparable across bins
- **Reliable n_* heuristic** - Your predictions aren't contaminated by bin size effects

**Symmetric Analysis:**
- **Both sides of primorial** - You get bins of the same size on either side of primorial boundaries
- **Controlled comparison** - You can isolate the primorial effect from bin size effects
- **Clean experimental design** - Like having a control group in scientific experiments

## Visualization Strategy

**Key Observation Points:**
- **Half primorials** (dashed lines) - Transition zones where interesting behavior occurs
- **Full primorials** (dotted lines) - Primary boundaries where biggest changes happen
- **All primorial multiples** (tick marks) - Complete mathematical structure without plot clutter

**Expected Patterns:**
- **Bin size transitions** - Average deviation should drop when switching between primorial sizes
- **Natural stability** - % error should decrease as n increases due to more possible pairs
- **Primorial effects** - Clear statistical changes at primorial boundaries

## Research Benefits

This approach provides:
- **Mathematical validation** - Confirms expected mathematical properties
- **Statistical robustness** - Each bin has similar information content
- **Trend detection** - Real patterns emerge clearly without artificial stability
- **Convergence analysis** - Clean detection of mathematical convergence patterns
