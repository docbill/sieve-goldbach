# Sieve-Goldbach

This repository contains the source code, data, scripts, and manuscript supporting&#x20;
**sieve-theoretic analyses of Goldbach’s Conjecture**.

The project combines:

* **C/C++ implementations** of prime generation, Goldbach pair counting,&#x20;
  and certified verification tools.
* **AWK scripts** for post-processing, joining tables, and computing λ statistics.
* **Data outputs** (CSV files with summaries, predictions, and statistics).
* **Certification artifacts** to ensure reproducibility (checksums, `.cert` and `.verify` files).
* **Manuscript** (`sieve_goldbach.tex` and compiled `sieve_goldbach.pdf`).

---

## Directory Layout

```
sieve-goldbach/
├── papers/        # Manuscript (LaTeX, PDF, bibliography) and example CSVs
├── bin/          # AWK utilities for analysis and comparison
├── data/         # Generated CSVs, certification files, and checksums
├── src/          # Source code (C / C++ implementations)
└── Makefile      # Top-level automation for build, generate, certify, verify
```

### `papers/`

* `sieve_goldbach.tex` – main LaTeX manuscript.
* `sieve_goldbach.pdf` – compiled version.
* `.bib` and `.bbl` – bibliography.
* Example CSVs (`pairrangejoin-100M.csv`, `cpslowerbound-100M.csv`, `lambda*.csv`) referenced in the paper.

### `bin/`

Portable AWK scripts for joining and comparing CSV outputs:

* `lambdaStats.awk` – compute λ statistics per decade.
* `compareAvg.awk`, `compareMin.awk`, `compareMax.awk` – compare measured vs predicted values.
* `joinSumPred.awk` – join summary tables with predictions.

### `data/`

* Verified **prime sequences** and **Goldbach pair lists**:

  * `primes-200M.bitmap.cert`, `primes-200M.raw.cert`
  * `gbpairs-10000.csv`, `.verify`, `.cert`
* **Summary tables**: `pairrangesummary-*.csv`, `pairrange2sgbll-*.csv`, `pairrangejoin-*.csv`
* **Lambda tables**: `lambdamin-*.csv`, `lambdaavg-*.csv`, `lambdamax-*.csv`
* **Lower bound data**: `cpslowerbound-*.csv`
* **Checksums and certifications**: `.sha256`, `.verify`, `.cert`

### `src/`

Source code in modular subdirectories:

* `primesieve_bitmap/` – generate odd-only prime bitmaps.
* `storeprimes/` – convert bitmaps to raw sequential prime files.
* `findgbpairs/` – generate Goldbach pairs.
* `pairrangesummary/` – compute fast Goldbach pair counts by range.
* `pairrange2sgbll/` – predicted Goldbach pair counts (HL-A).
* `cpslowerbound/` – certified product-series lower bounds.
* `certifyprimes/` – independent prime sequence verification.
* `certifygbpairs/` – verify Goldbach pair lists against certified primes.
* `validatepairrangesummary/` – validator for summary tables.
* `lib/` – common routines for sieving, counting pairs, and constants.

### `output/` and Alpha-Based Organization

All generated output files are organized by alpha value (the scaling parameter for the short interval analysis). The output directory structure is:

```
output/
├── alpha-0.0009765625/    # Results for α = 2^-10
├── alpha-0.00106494895768/ # Results for α = 2^-10 * 2^(1/8)
├── ...
├── alpha-0.25/            # Results for α = 0.25
├── alpha-0.5/             # Results for α = 0.5 (default)
├── ...
└── alpha-1/               # Results for α = 1.0
```

**Finding results for a specific alpha value:** To find results for a particular alpha value (e.g., α = 0.25), look in the corresponding `output/alpha-0.25/` directory. Each directory contains CSV files for that alpha value, including:

* **Summary files**: `gbpairsummary-*-empirical-full-0.25-v0.2.0.csv` (empirical measurements)
* **Prediction files**: `gbpairsummary-*-hl-a-full-0.25-v0.2.0.csv` (HLA predictions)
* **Bound ratio files**: `boundratiomin-*-0.25-v0.2.0.csv`, `boundratiomax-*-0.25-v0.2.0.csv` (when `POINTWISE=1`)
* **Lambda files**: `lambdaalignmin-*-0.25-v0.2.0.csv`, `lambdaboundmax-*-0.25-v0.2.0.csv`, etc.
* **Partial files**: `*.partial.csv` (intermediate files during generation)

**Alpha value generation:** Alpha values are generated in a geometric progression: starting from 2^-10 ≈ 0.0009765625, each subsequent value is multiplied by 2^(1/8) ≈ 1.0905077, up to α = 1.0. This provides approximately 80 different alpha values for analysis.

**Default alpha:** The default alpha value is 0.5, and results for this value are also copied to the root `output/` directory (without the alpha suffix in the filename) for convenience.

---

## System Requirements

* **Operating System**: Linux or macOS (tested on Apple M2 Pro with macOS 15; also works on Linux distributions with GCC/Clang).

* **Compiler**: GCC ≥ 10 or Clang ≥ 12 recommended. On macOS, install via:

  ```sh
  brew install gcc
  ```

  or use Apple’s Command Line Tools (provides `clang`).

* **Disk & Memory**:

  * Disk usage is always < 1 GB, regardless of validation size.
  * Tested with 8 GB RAM on Linux; more memory improves runtime.
  * Likely to run on devices with 4 GB RAM (e.g., Raspberry Pi), but this has not been tested.

* **Runtime**:

  * Small runs (`1M`) finish in minutes.
  * Medium runs (`10M`) take \~12 hours on an Apple M2 Pro.
  * Large runs (`100M`) may run for several weeks.

---

## Building

The project builds with a standard C compiler (tested on GCC and Clang):

```sh
make
```

This compiles all binaries into their subdirectories under `src/`. All data files are generated and validated in the `output/` folder, except for the largest summary files (`pairrangesummary-10M.csv` and `pairrangesummary-100M.csv`).

**Parallel builds:** You can speed up compilation and verification by using the `-j` option to run multiple jobs in parallel:

```sh
make -j 12
make -j 12 verify
```

The number after `-j` specifies how many parallel jobs to run (e.g., `-j 12` for 12 parallel jobs). This is especially useful for verification runs that process multiple files independently.

To generate and validate these larger files:

```sh
make validate-medium
make validate-large
```

It is recommended to run these long jobs inside a persistent session (e.g., with `screen` or `tmux`), since:

* **Medium validation** takes more than 12 hours on an Apple M2 Pro.
* **Large validation** may take several weeks.

---

## Quick Start: Make Targets

The top-level `Makefile` provides several convenient targets:

* `make` – build all binaries under `src/` and generate core outputs.
* `make generate` – generate small test datasets (fast to run).
* `make validate-medium` – build and validate medium datasets (≈12+ hours).
* `make validate-large` – build and validate large datasets (can run for weeks).
* `make verify` – compare generated outputs against golden reference `.verify` and `.sha256` files.
* `make clean` – remove temporary certification files (`*.verify`) from `output/`.
* `make clobber` – perform `clean` and also remove all generated data products (`output/*.csv`, `.raw`, `.bitmap`, etc.), leaving only source and reference data.
* `make backup` – create a compressed backup of all partial CSV files (`.partial.csv`) in the `backups/` directory. Useful for preserving progress on long-running jobs. Backups are named with a timestamp and compatibility version (e.g., `backup-20251111145800-v0.2.0.tar.bz2`).
* `make restore` – restore the most recent backup from the `backups/` directory. After restoration, all partial files are automatically touched to prevent unnecessary rebuilds.

Use `make help` to see a summary if available. Always run long jobs inside `screen` or `tmux` for persistence.

**Tip:** Use `make -j N` (where `N` is the number of parallel jobs) to speed up builds and verification. For example, `make -j 12 verify` will run up to 12 verification tasks in parallel.

### Makefile Options

The Makefile supports several optional flags to control output generation and verification:

* **`TAINTED` (environment variable)** – When set to `1` or `true`, verification failures print warnings instead of exiting with an error. Useful when intentionally generating different outputs (e.g., during development or when testing new algorithms). This is implemented as an environment variable (rather than a make variable) for convenience, allowing AWK scripts to access it without requiring additional parameter passing. Set as an environment variable:
  ```sh
  export TAINTED=1
  make verify
  ```
  Or inline:
  ```sh
  TAINTED=1 make verify
  ```

* **`COMPAT=v0.1.5` (make variable)** – When set, uses the legacy v0.1.5 compatibility mode. The default is `v0.1.6`, which supports all current features.  Example:
  ```sh
  make COMPAT=v0.1.6 generate
  ```

These options can be combined:
```sh
TAINTED=1 make COMPAT=v0.1.5 verify
```

---

## Reproducibility Workflow

1. **Generate primes** (bitmap and raw file):

   ```sh
   ./src/primesieve_bitmap/primesieve_bitmap 200000000 > output/primes-200M.bitmap
   ./src/storeprimes/storeprimes output/primes-200M.bitmap output/primes-200M.raw
   ```

2. **Generate Goldbach pairs** (example: first 10,000):

   ```sh
   ./src/findgbpairs/findgbpairs output/primes-200M.raw 10000 > output/gbpairs-10000.csv
   ```

3. **Certify outputs**:

   ```sh
   ./src/certifyprimes/certifyprimes --bitmap --file output/primes-200M.bitmap
   ./src/certifyprimes/certifyprimes --binary --file output/primes-200M.raw
   ./src/certifygbpairs/certifygbpairs --bitmap output/primes-200M.bitmap --file output/gbpairs-10000.csv
   ```

4. **Generate summaries and predictions**:

   ```sh
   ./src/pairrangesummary/pairrangesummary output/primes-200M.raw 1000000 > output/pairrangesummary-1M.csv
   ./src/pairrange2sgbll/pairrange2sgbll output/primes-200M.raw 1000000 > output/pairrange2sgbll-1M.csv
   ```

5. **Post-process with AWK scripts**:

   ```sh
   ./bin/joinSumPred.awk output/pairrangesummary-1M.csv output/pairrange2sgbll-1M.csv > output/pairrangejoin-1M.csv
   ./bin/lambdaStats.awk output/lambdamin-1M.csv > output/lambdastatsmin-1M.csv
   ```

6. **Validate against golden references**:

   ```sh
   make verify
   ```

---

## Certification & Checksums

* `.verify` and `.cert` files capture certification outputs (including FNV-1a digests).
* `.sha256` files provide cryptographic checksums for archival integrity.
* These ensure any reproduced results are **bitwise identical** to the deposited data.

### Provenance of Certification Artifacts

The repository includes two kinds of certification artifacts:

* **Programmatic certification** (`.verify`) — produced automatically by the validator programs, summarizing properties of the generated files (counts, last values, FNV-1a hash).
* **Manual certification** (`.cert`) — attested files prepared after human review, confirming that the `.verify` output matches expectations and is correct for inclusion in the archive.

This separation ensures that verification can be re-run independently, while manual certifications provide a stable reference for long-term archival.

---

## License

This project uses dual licensing:

- **Source code (C, C++, AWK scripts):**  
  Licensed under the GNU General Public License, version 3 or later (GPL-3.0-or-later).  
  See [`LICENSES/GPL-3.0-or-later.txt`](LICENSES/GPL-3.0-or-later.txt).

- **Documentation and manuscript (LaTeX `.tex`, `.bib`, `.pdf`):**  
  Licensed under Creative Commons Attribution 4.0 International (CC-BY-4.0).  
  See [`LICENSES/CC-BY-4.0.txt`](LICENSES/CC-BY-4.0.txt).

Contributions via pull request are welcome; all contributions must be licensed under GPL-3.0-or-later for code and CC-BY-4.0 for documentation.

---

## Citation

If you use this code or data, please cite the accompanying manuscript:

> B.C. Riemers, *Sieve-Theoretic Reformulation of Goldbach’s Conjecture*, 2025.&#x20;
> (Preprint included as `papers/goldbach-reformulation/sieve_goldbach.pdf`)

---

## Archival Identifiers

This project is preserved in Software Heritage.

- **Tree (v0.1.0)**  
  [`swh:1:dir:74b69f1e7...`](https://archive.softwareheritage.org/swh:1:dir:74b69f1e739bbef1dd234621d565ea67e680bd6b/)  

- **Revision (v0.1.0 commit)**  
  [`swh:1:rev:114bbaa7...`](https://archive.softwareheritage.org/swh:1:rev:114bbaa7380b436b20661119798139a2fa6db0ab/)  

- **Tree (v0.1.1)**  
  [ `swh:1:dir:0ab6250e1...` ](https://archive.softwareheritage.org/swh:1:dir:0ab6250e1b3593f5bbc5ad745d16a36841f90d93/)

- **Revision (v0.1.1 commit)**  
  [ `swh:1:rev:22b8e5ff8...`](https://archive.softwareheritage.org/swh:1:rev:22b8e5ff8858370462631bf4ddd08037de702f3a/)

- **Tree (v0.1.2)**  
  [ `swh:1:dir:382857a49...` ](https://archive.softwareheritage.org/swh:1:dir:382857a4908d9a8fa22733e77c069d05185d8c05/)

- **Revision (v0.1.2 commit)**  
  [ `swh:1:rev:5b97ddeba...` ](https://archive.softwareheritage.org/swh:1:rev:5b97ddeba4a8dc2c45f1c2357f570f997a603a9f)

- **Tree (v0.1.3)**  
  [ `swh:1:dir:1d62ae88f...` ](https://archive.softwareheritage.org/swh:1:dir:1d62ae88f1c53ab62c641f60c88f4d6343232496/)

- **Revision (v0.1.3 commit)**  
  [ `swh:1:rev:485382a63...` ](https://archive.softwareheritage.org/swh:1:rev:485382a63b4efdc1bae0d078b9b9abee87241b59)

- **Tree (v0.1.4)**  
  [ `swh:1:dir:cb261af57...` ](https://archive.softwareheritage.org/swh:1:dir:cb261af57c123344624bc465da55aad06075bf27)

- **Revision (v0.1.4 commit)**  
  [ `swh:1:rev:e6bdcb08ae...` ](https://archive.softwareheritage.org/swh:1:rev:e6bdcb08ae64fe5c1e488951d68575100c9f6e5d)

- **Tree (v0.1.5)**  
  [ `swh:1:dir:1be084fd56...` ](https://archive.softwareheritage.org/swh:1:dir:1be084fd561c2b621a7c1ccef5b34a5786e49afa)

- **Revision (v0.1.5 commit)**  
  [ `swh:1:rev:f707b77f37...` ](https://archive.softwareheritage.org/swh:1:rev:f707b77f37ae6c331f11bbefb74a5fc405964da2)

- **Tree (v0.1.6)**  
  [ `swh:1:dir:da5e41c4b6...` ](https://archive.softwareheritage.org/swh:1:dir:da5e41c4b6ba0bf16d71e62ac362e5c89ee06e43)

- **Revision (v0.1.6 commit)**  
  [ `swh:1:rev:994cc351983...` ](https://archive.softwareheritage.org/swh:1:rev:994cc35198344c6a04eab34c71217a3e37814ea8)

[![SWH](https://archive.softwareheritage.org/badge/origin/https://github.com/docbill/sieve-goldbach/)](https://archive.softwareheritage.org/browse/origin/https://github.com/docbill/sieve-goldbach/)


