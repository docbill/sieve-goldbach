#!/usr/bin/env python3
"""
Create a lambdabound-audit CSV by filtering rows from a lambdabound-cert file.

Keeps only specific alpha values:
  1, 1/2, 1/4, ..., 1/1024, plus 0.00106494895768

Usage:
  summarizelambdaboundaudit.py <cert_file> <output_file>
"""

import argparse
import csv
import sys


TARGET_ALPHAS = [
    1.0,
    1.0 / 2,
    1.0 / 4,
    1.0 / 8,
    1.0 / 16,
    1.0 / 32,
    1.0 / 64,
    1.0 / 128,
    1.0 / 256,
    1.0 / 512,
    0.00106494895768,
    1.0 / 1024,
]


def main():
    parser = argparse.ArgumentParser(description="Filter lambdabound-cert into lambdabound-audit")
    parser.add_argument("cert_file", help="Input lambdabound-cert CSV")
    parser.add_argument("output_file", help="Output lambdabound-audit CSV")
    parser.add_argument("--tolerance", type=float, default=1e-12, help="Alpha match tolerance")
    args = parser.parse_args()

    try:
        with open(args.cert_file, "r", newline="") as f:
            reader = csv.DictReader(f)
            if not reader.fieldnames:
                print("Error: empty or invalid cert file", file=sys.stderr)
                sys.exit(1)
            rows = list(reader)
    except FileNotFoundError:
        print(f"Error: file not found: {args.cert_file}", file=sys.stderr)
        sys.exit(1)

    kept = []
    for row in rows:
        alpha_str = row.get("alpha", "")
        try:
            alpha_val = float(alpha_str)
        except ValueError:
            continue
        if any(abs(alpha_val - target) <= args.tolerance for target in TARGET_ALPHAS):
            kept.append(row)

    if not kept:
        print("No matching rows found", file=sys.stderr)
        sys.exit(1)

    with open(args.output_file, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=reader.fieldnames)
        writer.writeheader()
        writer.writerows(kept)

    print(f"Summary written to {args.output_file} ({len(kept)} rows)", file=sys.stderr)


if __name__ == "__main__":
    main()
