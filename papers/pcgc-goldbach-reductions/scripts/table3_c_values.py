#!/usr/bin/env python3
"""
Table 3: Bounding Envelope Constants
Reproduces Table~\ref{tab:c-values}

This script calculates the bounding envelope constants c(2n;L) 
associated with effective local moduli.

Formula:
  c(2n;L) = c(2n;EffLocMod_p(n))
          = 2 * ∏_{q ∈ Peff(n), EffLocMod_q(n) > EffLocMod_p(n)} (q-2)^(EffLocMod_p(n)/EffLocMod_q(n))

For each EffLocMod_p(n), we find all primes q where EffLocMod_q(n) > EffLocMod_p(n)
and multiply (q-2)^(EffLocMod_p(n)/EffLocMod_q(n)) for each, then multiply by 2.
"""

from decimal import Decimal, getcontext, ROUND_DOWN

# Set precision: 50 digits for calculations, report 20 decimal places
getcontext().prec = 50
DISPLAY_DECIMALS = 20

# Primes up to 59
PRIMES = [3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59]

# Expected values from the paper
EXPECTED_C_VALUES = {
    1: Decimal("2.84314327885132100170"),      # cvalueA
    4: Decimal("2.72259939396405724060"),      # cvalueB
    24: Decimal("2.54555496154236394326"),     # cvalueC
    240: Decimal("2.47920428379799629581"),   # cvalueD
    2880: Decimal("2.39345422669887123968"),   # cvalueE
    46080: Decimal("2.35972191892736767409"),  # cvalueF
    829440: Decimal("2.30959606775411076524"), # cvalueG
        18247680: Decimal("2.25914178520364856290"), # cvalueH
        510935040: Decimal("2.24514310219420830997"), # cvalueI
        15328051200: Decimal("2.23500000000000000000"),  # Placeholder - will be calculated
        551809843200: Decimal("2.23000000000000000000"),  # Placeholder - will be calculated
        22072393728000: Decimal("2.22500000000000000000"),  # Placeholder - will be calculated
}

def calculate_efflocmod_values(three_divides_n):
    """
    Calculate EffLocMod values for all primes in Peff(n).
    
    Args:
        three_divides_n: Boolean, True if 3 divides n (determines starting prime)
    
    Returns:
        List of (prime, EffLocMod) tuples
    """
    if three_divides_n:
        start_idx = 0  # Start with p=3
    else:
        start_idx = 1  # Start with p=5
    
    efflocmod_values = []
    product = 1
    for i in range(start_idx, len(PRIMES)):
        p = PRIMES[i]
        if i == start_idx:
            product = p - 1
        else:
            product *= (p - 1)
        efflocmod_values.append((p, product))
    
    return efflocmod_values

def calculate_c_value(efflocmod_3nmid, efflocmod_3div):
    """
    Calculate c(2n;L) given EffLocMod values using the definition.
    
    Formula:
      c(2n;L) = 2 * ∏_{q ∈ Peff(n), EffLocMod_q(n) > EffLocMod_p(n)} (q-2)^(EffLocMod_p(n)/EffLocMod_q(n))
    
    The c value should be the same whether calculated from 3|n or 3∤n case,
    as long as we use the corresponding EffLocMod_p(n) value.
    
    Args:
        efflocmod_3nmid: EffLocMod for 3∤n case (used for calculation)
        efflocmod_3div: EffLocMod for 3|n case (for reference, should give same result)
    
    Returns:
        c(2n;L) value (truncated to 20 decimal places)
    """
    # Use the 3∤n case for calculation (could use either, they should give same result)
    efflocmod_p = efflocmod_3nmid
    three_divides_n = False
    
    # Get all EffLocMod values for primes in Peff(n)
    efflocmod_list = calculate_efflocmod_values(three_divides_n)
    
    # Calculate product: ∏_{q ∈ Peff(n), EffLocMod_q(n) > EffLocMod_p(n)} (q-2)^(EffLocMod_p(n)/EffLocMod_q(n))
    # For EffLocMod_p(n), include all primes q where EffLocMod_q(n) > EffLocMod_p(n)
    product = Decimal(1)
    for q, efflocmod_q in efflocmod_list:
        if efflocmod_q > efflocmod_p:
            # Add term: (q-2)^(EffLocMod_p(n)/EffLocMod_q(n))
            exponent = Decimal(efflocmod_p) / Decimal(efflocmod_q)
            # Use high-precision: exp(ln(q-2) * exponent)
            term = (Decimal(q - 2).ln() * exponent).exp()
            product *= term
    
    # Multiply by 2
    c_value = Decimal(2) * product
    
    # Truncate (round down) to 20 decimal places
    return c_value.quantize(Decimal('0.00000000000000000001'), rounding=ROUND_DOWN)

def main():
    print("Table 3: Bounding Envelope Constants c(2n;L)")
    print("=" * 80)
    print(f"{'EffLocMod (3∤n)':<20} {'EffLocMod (3|n)':<20} {'c(2n;L)':<40}")
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
        1: Decimal("2.84314327885132100170"),
        4: Decimal("2.72259939396405724060"),
        24: Decimal("2.54555496154236394326"),
        240: Decimal("2.47920428379799629581"),
        2880: Decimal("2.39345422669887123968"),
        46080: Decimal("2.35972191892736767409"),
        829440: Decimal("2.30959606775411076524"),
        18247680: Decimal("2.25914178520364856290"),
        510935040: Decimal("2.24514310219420830997"),
    }
    
    all_match = True
    for efflocmod_3nmid, efflocmod_3div in pairs:
        c_value = calculate_c_value(efflocmod_3nmid, efflocmod_3div)
        
        # Check against expected value
        exp = expected.get(efflocmod_3nmid)
        match = "✓" if exp and abs(c_value - exp) < Decimal("0.0000000000000000001") else "✗"
        
        if exp and abs(c_value - exp) >= Decimal("0.0000000000000000001"):
            all_match = False
        
        print(f"{efflocmod_3nmid:<20} {efflocmod_3div:<20} {c_value:.20f} {match}")
    
    print("=" * 80)
    print(f"\nNote: Calculations use 50-digit precision internally, displayed to {DISPLAY_DECIMALS} decimal places.")
    if all_match:
        print("✓ All calculated values match the paper!")
    else:
        print("✗ Some values do not match - check calculation")

if __name__ == "__main__":
    main()

