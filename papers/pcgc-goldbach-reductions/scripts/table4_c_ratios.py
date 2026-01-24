#!/usr/bin/env python3
"""
Table 4: Bounding Envelope Constant Ratios
Reproduces Table~\ref{tab:c-ratios}

This script calculates the ratio c(2n;L) / SsemHead^{EffLocModCap,bullet}(2n;L)
associated with effective local moduli.

Formula:
  Ssem^bullet(2n;1) = 1 (by definition)
  Ssem^bullet(2n;EffLocMod_p) = Ssem^bullet(2n;EffLocMod_{p-1}) * [(p-1)/(p-2)]
  
  Or equivalently:
  Ssem^bullet(2n;EffLocMod_p) = ∏_{q from start to p} (q-1)/(q-2)
  
  Then: ratio = c(2n;L) / Ssem^bullet(2n;L)
"""

from decimal import Decimal, getcontext, ROUND_DOWN
import sys
sys.path.insert(0, '.')
from table3_c_values import calculate_c_value

# Set precision: 50 digits for calculations, report 20 decimal places
getcontext().prec = 50
DISPLAY_DECIMALS = 20

# Primes up to 59
PRIMES = [3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59]

# Expected values from the paper
EXPECTED_RATIOS = {
    # (EffLocMod, 3|n case): ratio value
    (1, False): Decimal("2.84314327885132100170"),      # xCratioNDivA
    (2, True): Decimal("1.42157163942566050085"),       # xCratioDivA (same as OmegaPrimeNormA)
    (4, False): Decimal("2.04194954547304293045"),      # xCratioNDivB
    (8, True): Decimal("1.02097477273652146522"),       # xCratioDivB
    (24, False): Decimal("1.59097185096397746453"),     # xCratioNDivC
    (48, True): Decimal("0.79548592548198873226"),      # xCratioDivC
    (240, False): Decimal("1.39455240963637291639"),    # xCratioNDivD
    (480, True): Decimal("0.69727620481818645819"),     # xCratioDivD
    (2880, False): Decimal("1.23412483564160548296"),   # xCratioNDivE
    (5760, True): Decimal("0.61706241782080274148"),    # xCratioDivE
    (46080, False): Decimal("1.14068588854399120964"),  # xCratioNDivF
    (92160, True): Decimal("0.57034294427199560482"),    # xCratioDivF
    (829440, False): Decimal("1.05442984538578787378"), # xCratioNDivG
    (1658880, True): Decimal("0.52721492269289393689"),  # xCratioDivG
    (18247680, False): Decimal("0.98451369301477360468"), # xCratioNDivH
    (36495360, True): Decimal("0.49225684650738680234"),  # xCratioDivH
        (510935040, False): Decimal("0.94346986441693873377"), # xCratioNDivI
        (1021870080, True): Decimal("0.47173493220846936688"), # xCratioDivI
        (15328051200, False): Decimal("0.93000000000000000000"),  # Placeholder - will be calculated
        (30656102400, True): Decimal("0.46500000000000000000"),  # Placeholder - will be calculated
        (551809843200, False): Decimal("0.92000000000000000000"),  # Placeholder - will be calculated
        (1103619686400, True): Decimal("0.46000000000000000000"),  # Placeholder - will be calculated
        (22072393728000, False): Decimal("0.91000000000000000000"),  # Placeholder - will be calculated
        (44144787456000, True): Decimal("0.45500000000000000000"),  # Placeholder - will be calculated
}

def calculate_ssem_bullet(efflocmod, three_divides_n):
    """
    Calculate Ssem^bullet(2n;L) using the recursive formula.
    
    Formula:
      Ssem^bullet(2n;1) = 1 (by definition)
      Ssem^bullet(2n;EffLocMod_p) = Ssem^bullet(2n;EffLocMod_{p-1}) * [(p-1)/(p-2)]
    
    Or equivalently:
      Ssem^bullet(2n;EffLocMod_p) = ∏_{q from start to p} (q-1)/(q-2)
    
    Args:
        efflocmod: Effective local modulus value
        three_divides_n: Boolean, True if 3 divides n (determines starting prime)
    
    Returns:
        Ssem^bullet(2n;L) value
    """
    if efflocmod == 1:
        return Decimal(1)
    
    # Determine starting prime and calculate EffLocMod values
    if three_divides_n:
        start_idx = 0  # Start with p=3
    else:
        start_idx = 1  # Start with p=5
    
    # Calculate EffLocMod values and build product
    ssem = Decimal(1)
    product = 1
    
    for i in range(start_idx, len(PRIMES)):
        p = PRIMES[i]
        if i == start_idx:
            product = p - 1
        else:
            product *= (p - 1)
        
        # Multiply by (p-1)/(p-2)
        ssem *= Decimal(p - 1) / Decimal(p - 2)
        
        # Stop when we've reached the target EffLocMod
        if product >= efflocmod:
            break
    
    return ssem

def calculate_c_ratio(efflocmod, three_divides_n):
    """
    Calculate c(2n;L) / SsemHead^{EffLocModCap,bullet}(2n;L).
    
    Formula:
      ratio = c(2n;L) / Ssem^bullet(2n;L)
    
    Note: The c value is the same for both 3|n and 3∤n cases when using
    the corresponding EffLocMod values. The table shows:
    - EffLocMod=1 (3∤n) and EffLocMod=2 (3|n) both use cValueA
    - EffLocMod=4 (3∤n) and EffLocMod=8 (3|n) both use cValueB
    - etc.
    
    So we calculate c using the 3∤n EffLocMod value.
    
    Args:
        efflocmod: Effective local modulus value (for the current case)
        three_divides_n: Boolean, True if 3 divides n
    
    Returns:
        Ratio value (truncated to 20 decimal places)
    """
    # Calculate c(2n;L) - use the 3∤n EffLocMod value
    if three_divides_n:
        # For 3|n, EffLocMod values are 2x those of 3∤n
        efflocmod_3nmid = efflocmod // 2
        efflocmod_3div = efflocmod
    else:
        # For 3∤n, use the same value
        efflocmod_3nmid = efflocmod
        efflocmod_3div = efflocmod * 2
    
    c_val = calculate_c_value(efflocmod_3nmid, efflocmod_3div)
    
    # Calculate Ssem^bullet(2n;L)
    ssem_bullet = calculate_ssem_bullet(efflocmod, three_divides_n)
    
    # Calculate ratio
    ratio = c_val / ssem_bullet
    
    # Truncate (round down) to 20 decimal places
    return ratio.quantize(Decimal('0.00000000000000000001'), rounding=ROUND_DOWN)

def main():
    print("Table 4: Bounding Envelope Constant Ratios")
    print("=" * 80)
    print(f"{'EffLocMod (3∤n)':<20} {'Ratio (3∤n)':<30} {'EffLocMod (3|n)':<20} {'Ratio (3|n)':<30}")
    print("-" * 80)
    
    # Pairs: (EffLocMod for 3∤n, EffLocMod for 3|n)
    # Extended to cover 10^18 Goldbach study range
    pairs = [
        (1, 2), (4, 8), (24, 48), (240, 480), (2880, 5760),
        (46080, 92160), (829440, 1658880), (18247680, 36495360),
        (510935040, 1021870080), (15328051200, 30656102400),
        (551809843200, 1103619686400), (22072393728000, 44144787456000)
    ]
    
    # Expected values for verification
    expected = {
        (1, False): Decimal("2.84314327885132100170"),
        (2, True): Decimal("1.42157163942566050085"),
        (4, False): Decimal("2.04194954547304293045"),
        (8, True): Decimal("1.02097477273652146522"),
        (24, False): Decimal("1.59097185096397746453"),
        (48, True): Decimal("0.79548592548198873226"),
        (240, False): Decimal("1.39455240963637291639"),
        (480, True): Decimal("0.69727620481818645819"),
        (2880, False): Decimal("1.23412483564160548296"),
        (5760, True): Decimal("0.61706241782080274148"),
        (46080, False): Decimal("1.14068588854399120964"),
        (92160, True): Decimal("0.57034294427199560482"),
        (829440, False): Decimal("1.05442984538578787378"),
        (1658880, True): Decimal("0.52721492269289393689"),
        (18247680, False): Decimal("0.98451369301477360468"),
        (36495360, True): Decimal("0.49225684650738680234"),
        (510935040, False): Decimal("0.94346986441693873377"),
        (1021870080, True): Decimal("0.47173493220846936688"),
    }
    
    all_match = True
    for efflocmod_3nmid, efflocmod_3div in pairs:
        ratio_3nmid = calculate_c_ratio(efflocmod_3nmid, False)
        ratio_3div = calculate_c_ratio(efflocmod_3div, True)
        
        # Check against expected values
        exp_3nmid = expected.get((efflocmod_3nmid, False))
        exp_3div = expected.get((efflocmod_3div, True))
        match_3nmid = "✓" if exp_3nmid and abs(ratio_3nmid - exp_3nmid) < Decimal("0.0000000000000000001") else "✗"
        match_3div = "✓" if exp_3div and abs(ratio_3div - exp_3div) < Decimal("0.0000000000000000001") else "✗"
        
        if exp_3nmid and abs(ratio_3nmid - exp_3nmid) >= Decimal("0.0000000000000000001"):
            all_match = False
        if exp_3div and abs(ratio_3div - exp_3div) >= Decimal("0.0000000000000000001"):
            all_match = False
        
        print(f"{efflocmod_3nmid:<20} {ratio_3nmid:.20f} {match_3nmid} {efflocmod_3div:<20} {ratio_3div:.20f} {match_3div}")
    
    print("=" * 80)
    print(f"\nNote: Calculations use 50-digit precision internally, displayed to {DISPLAY_DECIMALS} decimal places.")
    if all_match:
        print("✓ All calculated values match the paper!")
    else:
        print("✗ Some values do not match - check calculation")

if __name__ == "__main__":
    main()

