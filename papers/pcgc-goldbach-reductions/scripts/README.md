# Table Reproduction Scripts

This directory contains Python scripts to reproduce the four tables in the reductions-to-pcc paper.

## Scripts

1. **table1_efflocmod.py** - Table 1: Effective Local Moduli
   - ✅ **Complete** - Calculates EffLocMod_p^{(3)} and EffLocMod_p^{(5)}
   - Formula: Multiplicative product of (q-1) for primes from base to p

2. **table2_omegaprimenorm.py** - Table 2: Normalized Prime Curvature Constants
   - ⚠️ **Needs formula** - Currently uses expected values for verification
   - TODO: Implement OmegaPrimeNorm calculation formula from Paper A

3. **table3_c_values.py** - Table 3: Bounding Envelope Constants
   - ⚠️ **Needs formula** - Currently uses expected values for verification
   - TODO: Implement c(2n;L) calculation formula (see paper line 500-502)

4. **table4_c_ratios.py** - Table 4: Bounding Envelope Constant Ratios
   - ⚠️ **Needs formula** - Currently uses expected values for verification
   - TODO: Implement ratio = c(2n;L) / SsemHead^{EffLocModCap,bullet}(2n;L)

## Usage

Run each script to verify the table values:

```bash
python3 table1_efflocmod.py
python3 table2_omegaprimenorm.py
python3 table3_c_values.py
python3 table4_c_ratios.py
```

## Status

- **Table 1**: ✅ Fully implemented and verified
- **Tables 2-4**: ⚠️ Need formulas from Paper A to complete

## Next Steps

1. Find the formulas for OmegaPrimeNorm, c(2n;L), and SsemHead^{EffLocModCap,bullet} in Paper A
2. Implement the formulas in the respective scripts
3. Verify all calculated values match the paper
4. Document any discrepancies and determine if they represent corrections or errors

## Notes

- All scripts use high-precision Decimal arithmetic
- Expected values are included for verification
- Scripts will raise errors if formulas are missing
- Once formulas are implemented, scripts can be used to:
  - Verify paper values
  - Recalculate if OmegaPrime changes
  - Generate tables for different parameter ranges

