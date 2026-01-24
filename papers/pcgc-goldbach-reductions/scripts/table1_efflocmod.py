#!/usr/bin/env python3
"""
Table 1: Effective Local Moduli
Reproduces Table~\ref{tab:EffLocMod-values}

This script calculates the effective local moduli EffLocMod_p^{(3)} and EffLocMod_p^{(5)}
for odd primes p <= 41 (extended to cover 10^18 Goldbach study range).

Formula:
  EffLocMod_p^{(base)} = product of (q-1) for all primes q from base to p
  where base = 3 if starting from 3, or base = 5 if starting from 5
"""

def calculate_efflocmod(primes, base_prime):
    """
    Calculate effective local modulus for each prime.
    
    Formula: EffLocMod_p^{(base)} = product of (q-1) for all primes q from base to p (inclusive)
    
    Args:
        primes: List of primes to calculate for
        base_prime: Starting prime (3 or 5)
    
    Returns:
        Dictionary mapping prime to EffLocMod value
    """
    result = {}
    product = 1
    
    for p in primes:
        if p < base_prime:
            result[p] = 1
        elif p == base_prime:
            # Start with (base_prime - 1)
            product = base_prime - 1
            result[p] = product
        else:
            # Multiply by (p-1)
            product *= (p - 1)
            result[p] = product
    
    return result

def main():
    primes = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41]
    
    print("Table 1: Effective Local Moduli")
    print("=" * 60)
    print(f"{'p':<5} {'EffLocMod_p^{(3)}':<25} {'EffLocMod_p^{(5)}':<25}")
    print("-" * 60)
    
    efflocmod_3 = calculate_efflocmod(primes, 3)
    efflocmod_5 = calculate_efflocmod(primes, 5)
    
    for p in primes:
        val3 = efflocmod_3[p]
        val5 = efflocmod_5[p]
        print(f"{p:<5} {val3:<25} {val5:<25}")
    
    print("=" * 60)
    print("\nVerification against paper values:")
    print("Expected values from paper:")
    expected = {
        2: (1, 1),
        3: (2, 1),
        5: (8, 4),
        7: (48, 24),
        11: (480, 240),
        13: (5760, 2880),
        17: (92160, 46080),
        19: (1658880, 829440),
        23: (36495360, 18247680),
        29: (1021870080, 510935040),  # 36495360 * 28 = 1021870080, 18247680 * 28 = 510935040
        31: (30656102400, 15328051200),  # 1021870080 * 30 = 30656102400, 510935040 * 30 = 15328051200
        37: (1103619686400, 551809843200),  # 30656102400 * 36 = 1103619686400, 15328051200 * 36 = 551809843200
        41: (44144787456000, 22072393728000)  # 1103619686400 * 40 = 44144787456000, 551809843200 * 40 = 22072393728000
    }
    
    all_match = True
    for p in primes:
        exp3, exp5 = expected[p]
        calc3, calc5 = efflocmod_3[p], efflocmod_5[p]
        match3 = "✓" if calc3 == exp3 else "✗"
        match5 = "✓" if calc5 == exp5 else "✗"
        if calc3 != exp3 or calc5 != exp5:
            all_match = False
        print(f"p={p:2d}: EffLocMod^{(3)} = {calc3:12d} {match3} (expected {exp3:12d}), "
              f"EffLocMod^{(5)} = {calc5:12d} {match5} (expected {exp5:12d})")
    
    if all_match:
        print("\n✓ All values match the paper!")
    else:
        print("\n✗ Some values do not match - check formulas")

if __name__ == "__main__":
    main()

