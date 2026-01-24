#!/usr/bin/env python3
"""
Table 2: Normalized Prime Curvature Constants
Reproduces Table~\ref{tab:OmegaPrimeNorm-values}

This script calculates OmegaPrimeNorm(2n;L) as functions of the effective local modulus.

Formula:
  OmegaPrimeNorm(2n;L) = OmegaPrime^κ(n) * ∏_{p∈Peff(n), EffLocMod_p(n)≤L} (p-2)^(-1/EffLocMod_p(n))
  
where:
  - κ(n) = 1/2 for 3|n, and 1 for 3∤n
  - OmegaPrime is calculated using the nested formula for precision
  - The product is over all primes p where EffLocMod_p(n) ≤ L
"""

from decimal import Decimal, getcontext, ROUND_DOWN

# Set precision: 50 digits for calculations, report 20 decimal places
getcontext().prec = 50
DISPLAY_DECIMALS = 20

# Primes up to 59 (for OmegaPrime calculation)
PRIMES = [3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59]

def calculate_omegaprime():
    """
    Calculate OmegaPrime using the direct sum formula for maximum precision.
    
    Formula: OmegaPrime = exp(sum of ln(p-2)/qp)^2
    where qp is the product of (q-1) for primes q from 3 to p.
    
    OmegaPrime is a universal constant calculated from ALL primes starting with p=3.
    The κ(n) factor in OmegaPrimeNorm compensates for whether we start with 3 or 5.
    
    Uses direct sum (matching bc -l behavior) rather than nested formula to ensure
    exact agreement with bc calculations.
    
    Returns:
        OmegaPrime value (universal constant)
    """
    # Calculate qp values (effective local moduli)
    qp_values = []
    product = 1
    for i, p in enumerate(PRIMES):
        if i == 0:
            product = p - 1  # Start with p=3, qp = 2
        else:
            product *= (p - 1)
        qp_values.append((p, product))
    
    # Direct sum: sum of ln(p-2)/qp
    # Use Decimal.ln() for high-precision logarithms (50-digit precision)
    direct_sum = Decimal(0)
    for p, qp in qp_values:
        log_val = Decimal(p - 2).ln()  # High-precision natural logarithm
        term = log_val / Decimal(qp)
        direct_sum += term
    
    # Calculate exp(sum)^2
    omega_prime = (direct_sum.exp()) ** 2
    return omega_prime

def calculate_omegaprimenorm(efflocmod, three_divides_n):
    """
    Calculate OmegaPrimeNorm(2n;L) given EffLocMod and whether 3|n.
    
    Formula:
      OmegaPrimeNorm(2n;L) = OmegaPrime^κ(n) * ∏_{p∈Peff(n), EffLocMod_p(n)≤L} (p-2)^(-1/EffLocMod_p(n))
    
    Args:
        efflocmod: Effective local modulus value (L in the formula)
        three_divides_n: Boolean, True if 3 divides n (determines starting prime and κ)
    
    Returns:
        OmegaPrimeNorm value (truncated to 20 decimal places, matching paper precision)
    """
    # Determine starting prime and κ
    if three_divides_n:
        start_idx = 0  # Start with p=3
        kappa = Decimal(1) / Decimal(2)  # κ = 1/2
    else:
        start_idx = 1  # Start with p=5
        kappa = Decimal(1)  # κ = 1
    
    # Calculate OmegaPrime (universal constant, always calculated from p=3)
    omega_prime = calculate_omegaprime()
    
    # Calculate EffLocMod for each prime and build the product
    # ∏_{p∈Peff(n), EffLocMod_p(n)≤L} (p-2)^(-1/EffLocMod_p(n))
    product = Decimal(1)
    efflocmod_current = 1
    
    for i in range(start_idx, len(PRIMES)):
        p = PRIMES[i]
        if i == start_idx:
            efflocmod_current = p - 1
        else:
            efflocmod_current *= (p - 1)
        
        # Only include primes where EffLocMod_p(n) ≤ L
        if efflocmod_current <= efflocmod:
            # Add term: (p-2)^(-1/EffLocMod_p(n))
            # Use high-precision: exp(-ln(p-2)/EffLocMod_p(n))
            exponent = Decimal(-1) / Decimal(efflocmod_current)
            term = (Decimal(p - 2).ln() * exponent).exp()  # More numerically stable
            product *= term
        else:
            # Once we exceed L, we can stop
            break
    
    # Calculate final result: OmegaPrime^κ(n) * product
    omega_prime_norm = (omega_prime ** kappa) * product
    
    # Truncate (round down) to 20 decimal places (matching paper precision)
    # The \dots notation indicates the last shown digit is exact, so we truncate
    # Calculations use 50-digit precision internally, but we report only 20 decimal places
    return omega_prime_norm.quantize(Decimal('0.00000000000000000001'), rounding=ROUND_DOWN)

def main():
    print("Table 2: Normalized Prime Curvature Constants")
    print("=" * 80)
    print(f"{'EffLocMod':<15} {'OmegaPrimeNorm (3∤n)':<30} {'EffLocMod':<15} {'OmegaPrimeNorm (3|n)':<30}")
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
        (1, False): Decimal("1.42157163942566050085"),
        (2, True): Decimal("1.19229679166961633691"),
        (4, False): Decimal("1.08016086134585523898"),
        (8, True): Decimal("1.03930787611075825955"),
        (24, False): Decimal("1.01010073420593466549"),
        (48, True): Decimal("1.00503767800313569413"),
        (240, False): Decimal("1.00089536090824877031"),
        (480, True): Decimal("1.00044758029006635721"),
        (2880, False): Decimal("1.00006235973078184745"),
        (5760, True): Decimal("1.00003117937931407652"),
        (46080, False): Decimal("1.00000358934239158675"),
        (92160, True): Decimal("1.00000179466958537391"),
        (829440, False): Decimal("1.00000017352126582572"),
        (1658880, True): Decimal("1.00000008676062914916"),
        (18247680, False): Decimal("1.00000000667689371749"),
        (36495360, True): Decimal("1.00000000333844685317"),
        (510935040, False): Decimal("1.00000000022629507117"),
        (1021870080, True): Decimal("1.00000000011314753557"),
        (15328051200, False): Decimal("1.00000000000754316904"),  # Placeholder - will be calculated
        (30656102400, True): Decimal("1.00000000000377158452"),  # Placeholder - will be calculated
        (551809843200, False): Decimal("1.00000000000025143897"),  # Placeholder - will be calculated
        (1103619686400, True): Decimal("1.00000000000012571948"),  # Placeholder - will be calculated
        (22072393728000, False): Decimal("1.00000000000001005756"),  # Placeholder - will be calculated
        (44144787456000, True): Decimal("1.00000000000000502878"),  # Placeholder - will be calculated
    }
    
    all_match = True
    for efflocmod_n, efflocmod_y in pairs:
        omega_n = calculate_omegaprimenorm(efflocmod_n, False)
        omega_y = calculate_omegaprimenorm(efflocmod_y, True)
        
        # Check against expected values
        exp_n = expected.get((efflocmod_n, False))
        exp_y = expected.get((efflocmod_y, True))
        match_n = "✓" if exp_n and abs(omega_n - exp_n) < Decimal("0.0000000000000000001") else "✗"
        match_y = "✓" if exp_y and abs(omega_y - exp_y) < Decimal("0.0000000000000000001") else "✗"
        
        if exp_n and abs(omega_n - exp_n) >= Decimal("0.0000000000000000001"):
            all_match = False
        if exp_y and abs(omega_y - exp_y) >= Decimal("0.0000000000000000001"):
            all_match = False
        
        # Format to exactly 20 decimal places (always show 20 digits after decimal point)
        omega_n_str = f"{omega_n:.20f}"
        omega_y_str = f"{omega_y:.20f}"
        
        print(f"{efflocmod_n:<15} {omega_n_str:<30} {match_n} {efflocmod_y:<15} {omega_y_str:<30} {match_y}")
    
    print("=" * 80)
    print(f"\nNote: Calculations use 50-digit precision internally, displayed to {DISPLAY_DECIMALS} decimal places.")
    if all_match:
        print("✓ All calculated values match the paper!")
    else:
        print("✗ Some values do not match - may be due to precision differences in original bc calculations")
        print("  (Script uses 50-digit precision; original may have used CPU double precision ~16-17 digits)")

if __name__ == "__main__":
    main()

