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
├── paper/        # Manuscript (LaTeX, PDF, bibliography) and example CSVs
├── bin/          # AWK utilities for analysis and comparison
├── data/         # Generated CSVs, certification files, and checksums
├── src/          # Source code (C / C++ implementations)
└── Makefile      # Top-level automation for build, generate, certify, verify
```

### `paper/`

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
* `make verify` – compare generated outputs against golden reference `.verify` and `.sha256` files.
* `make clean` – remove temporary certification files (`*.verify`) from `output/`.
* `make clobber` – perform `clean` and also remove all generated data products (`output/*.csv`, `.raw`, `.bitmap`, etc.), leaving only source and reference data.

Use `make help` to see a summary if available. Always run long jobs inside `screen` or `tmux` for persistence.

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
> (Preprint included as `paper/sieve_goldbach.pdf`)

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

[![SWH](https://archive.softwareheritage.org/badge/origin/https://github.com/docbill/sieve-goldbach/)](https://archive.softwareheritage.org/browse/origin/https://github.com/docbill/sieve-goldbach/)


