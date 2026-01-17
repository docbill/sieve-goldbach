#!/usr/bin/env python3
"""
Summarize boundratio files into a bounds-cert table.

For each alpha:
  L11_lo: lambda from boundratiomin nearest to n = 480^2/(2*alpha)
  L11_hi: lambda from boundratiomax nearest to n = 480^2/(2*alpha)
  L13_lo: lambda from boundratiomin nearest to n = 5760^2/(2*alpha)
  L13_hi: lambda from boundratiomax nearest to n = 5760^2/(2*alpha)

Nearest selection requires at least one n below and one n above the target.
If not, the value is left blank.

Final columns use the last 12 lambda values in each file:
  Lfinal_lo, Lfinal_lo_std from boundratiomin
  Lfinal_hi, Lfinal_hi_std from boundratiomax

Usage:
    summarizeboundscert.py <min_pattern> <max_pattern> <output_file>

Example:
    summarizeboundscert.py \
      boundratiomin-23PR.5--=ALPHA=--v0.2.0.csv \
      boundratiomax-23PR.5--=ALPHA=--v0.2.0.csv \
      output/bounds-cert-23PR.5-v0.2.0.csv
"""

import argparse
import csv
import math
import sys
from pathlib import Path


def parse_float(value_str):
    if value_str is None:
        return None
    value_str = value_str.strip()
    if value_str == "":
        return None
    try:
        return float(value_str)
    except ValueError:
        return None


def load_lambda_pairs(filepath):
    """Return list of (n, lambda) pairs with lambda defined."""
    pairs = []
    with open(filepath, "r") as f:
        reader = csv.DictReader(f)
        for row in reader:
            n_val = row.get("n", "").strip()
            lambda_val = parse_float(row.get("lambda", ""))
            if not n_val or lambda_val is None:
                continue
            try:
                n_int = int(n_val)
            except ValueError:
                continue
            pairs.append((n_int, lambda_val))
    return pairs


def fmt_value(value):
    """Format numeric output to 5 decimal places, or blank if None."""
    if value is None:
        return ""
    return f"{value:.5f}"


def nearest_lambda_with_bracket(pairs, target_n):
    """Return lambda nearest to target_n if both sides exist, else None."""
    below = None  # (n, lambda) with n <= target
    above = None  # (n, lambda) with n >= target
    for n_val, lambda_val in pairs:
        if n_val <= target_n:
            if below is None or n_val > below[0]:
                below = (n_val, lambda_val)
        if n_val >= target_n:
            if above is None or n_val < above[0]:
                above = (n_val, lambda_val)
    if below is None or above is None:
        return None
    # Choose nearer of the two; prefer below on ties
    if abs(target_n - below[0]) <= abs(above[0] - target_n):
        return below[1]
    return above[1]


def tail_stats(pairs, count):
    """Return (mean, stddev) of last `count` lambda values, or (None, None)."""
    if len(pairs) < count:
        return None, None
    tail = [val for _, val in pairs[-count:]]
    mean = sum(tail) / count
    variance = sum((x - mean) ** 2 for x in tail) / count
    return mean, math.sqrt(variance)


def main():
    parser = argparse.ArgumentParser(
        description="Summarize boundratio files into bounds-cert table"
    )
    parser.add_argument("min_pattern", help="Min file pattern with --=ALPHA=-- placeholder")
    parser.add_argument("max_pattern", help="Max file pattern with --=ALPHA=-- placeholder")
    parser.add_argument("output_file", help="Output CSV filename")
    parser.add_argument("--tail-count", type=int, default=12, help="Tail size (default: 12)")

    args = parser.parse_args()

    if "--=ALPHA=--" not in args.min_pattern or "--=ALPHA=--" not in args.max_pattern:
        print("Error: file patterns must contain '--=ALPHA=--' placeholder", file=sys.stderr)
        sys.exit(1)

    output_dir = Path(__file__).parent.parent / "output"
    alpha_dirs = sorted(
        [p for p in output_dir.iterdir() if p.is_dir() and p.name.startswith("alpha-")],
        key=lambda p: float(p.name.split("alpha-")[1]),
    )

    rows = []
    for alpha_dir in alpha_dirs:
        alpha_str = alpha_dir.name.split("alpha-")[1]
        try:
            alpha_val = float(alpha_str)
        except ValueError:
            continue

        min_file = alpha_dir / args.min_pattern.replace("--=ALPHA=--", f"-{alpha_str}-")
        max_file = alpha_dir / args.max_pattern.replace("--=ALPHA=--", f"-{alpha_str}-")
        if not min_file.exists() or not max_file.exists():
            continue

        min_pairs = load_lambda_pairs(min_file)
        max_pairs = load_lambda_pairs(max_file)

        if not min_pairs or not max_pairs:
            rows.append({
                "alpha": alpha_val,
                "L11_lo": "",
                "L11_hi": "",
                "L13_lo": "",
                "L13_hi": "",
                "Lfinal_lo": "",
                "Lfinal_lo_std": "",
                "Lfinal_hi": "",
                "Lfinal_hi_std": "",
            })
            continue

        target_l11 = (480.0 ** 2) / (2.0 * alpha_val)
        target_l13 = (5760.0 ** 2) / (2.0 * alpha_val)

        l11_lo = nearest_lambda_with_bracket(min_pairs, target_l11)
        l11_hi = nearest_lambda_with_bracket(max_pairs, target_l11)
        l13_lo = nearest_lambda_with_bracket(min_pairs, target_l13)
        l13_hi = nearest_lambda_with_bracket(max_pairs, target_l13)

        lfinal_lo, lfinal_lo_std = tail_stats(min_pairs, args.tail_count)
        lfinal_hi, lfinal_hi_std = tail_stats(max_pairs, args.tail_count)

        rows.append({
            "alpha": alpha_val,
            "L11_lo": fmt_value(l11_lo),
            "L11_hi": fmt_value(l11_hi),
            "L13_lo": fmt_value(l13_lo),
            "L13_hi": fmt_value(l13_hi),
            "Lfinal_lo": fmt_value(lfinal_lo),
            "Lfinal_lo_std": fmt_value(lfinal_lo_std),
            "Lfinal_hi": fmt_value(lfinal_hi),
            "Lfinal_hi_std": fmt_value(lfinal_hi_std),
        })

    if not rows:
        print("No results found", file=sys.stderr)
        sys.exit(1)

    output_path = Path(args.output_file)
    with open(output_path, "w", newline="") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=[
                "alpha",
                "L11_lo",
                "L11_hi",
                "L13_lo",
                "L13_hi",
                "Lfinal_lo",
                "Lfinal_lo_std",
                "Lfinal_hi",
                "Lfinal_hi_std",
            ],
        )
        writer.writeheader()
        writer.writerows(rows)

    print(f"Summary written to {output_path} ({len(rows)} rows)", file=sys.stderr)


if __name__ == "__main__":
    main()
