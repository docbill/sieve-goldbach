#!/usr/bin/env python3
"""
Summarize boundratio files for plotting.

Reads from output/ and writes summary to current directory.

For each file:
- Aggregate over 12 lines (working backwards from the end)
- Extract: alpha value, n for minimum ratio, minimum ratio, n for maximum ratio,
  maximum ratio, n for minimum lambda, minimum lambda, n for maximum lambda,
  maximum lambda
- Filter to only alpha values that are powers of 2: 1, 1/2, 1/4, ..., 1/1024

Usage:
    summarizeboundratio.py <file_pattern> <output_file>

Example:
    summarizeboundratio.py boundratiomin-23PR.5--=ALPHA=--v0.2.0.csv boundratiomin-summary-23PR.5-v0.2.0.csv
"""

import argparse
import csv
import sys
from pathlib import Path

# Alpha values that are powers of 2: 1, 1/2, 1/4, ..., 1/1024
POWER_OF_2_ALPHAS = [1.0 / (2**i) for i in range(11)]  # 1, 0.5, 0.25, ..., 1/1024
POWER_OF_2_ALPHAS.reverse()  # Start from smallest: 1/1024, ..., 1


def format_alpha(alpha):
    """Format alpha to match directory naming convention."""
    if alpha == 1.0:
        return "1"
    elif alpha == 0.5:
        return "0.5"
    elif alpha == 0.25:
        return "0.25"
    elif alpha == 0.125:
        return "0.125"
    elif alpha == 0.0625:
        return "0.0625"
    elif alpha == 0.03125:
        return "0.03125"
    elif alpha == 0.015625:
        return "0.015625"
    elif alpha == 0.0078125:
        return "0.0078125"
    elif alpha == 0.00390625:
        return "0.00390625"
    elif alpha == 0.001953125:
        return "0.001953125"
    elif alpha == 0.0009765625:
        return "0.0009765625"
    else:
        return str(alpha)


def parse_float(value_str):
    """Parse float value, handling empty strings."""
    if not value_str or value_str.strip() == '':
        return None
    try:
        return float(value_str)
    except ValueError:
        return None


def process_file(filepath, alpha_value, batch_size):
    """Process a boundratio file and return summary data."""
    results = []

    try:
        with open(filepath, 'r') as f:
            reader = csv.DictReader(f)
            rows = list(reader)

        num_rows = len(rows)
        if num_rows == 0:
            return results

        batch_start = max(0, num_rows - batch_size)

        while batch_start >= 0:
            batch_end = min(batch_start + batch_size, num_rows)
            batch = rows[batch_start:batch_end]

            min_ratio = None
            min_ratio_n = None
            max_ratio = None
            max_ratio_n = None

            min_lambda = None
            min_lambda_n = None
            max_lambda = None
            max_lambda_n = None

            for row in batch:
                n_str = row.get('n', '')
                if not n_str:
                    continue
                try:
                    n_val = int(n_str)
                except ValueError:
                    continue

                ratio_val = parse_float(row.get('ratio', ''))
                if ratio_val is not None:
                    if min_ratio is None or ratio_val < min_ratio:
                        min_ratio = ratio_val
                        min_ratio_n = n_val
                    if max_ratio is None or ratio_val > max_ratio:
                        max_ratio = ratio_val
                        max_ratio_n = n_val

                lambda_val = parse_float(row.get('lambda', ''))
                if lambda_val is not None:
                    if min_lambda is None or lambda_val < min_lambda:
                        min_lambda = lambda_val
                        min_lambda_n = n_val
                    if max_lambda is None or lambda_val > max_lambda:
                        max_lambda = lambda_val
                        max_lambda_n = n_val

            # Suppress rows when lambda is undefined (e.g., predicted or measured count is 0)
            if min_lambda is not None or max_lambda is not None:
                results.append({
                    'alpha': alpha_value,
                    'n_min_lambda': min_lambda_n,
                    'min_lambda': min_lambda,
                    'n_max_lambda': max_lambda_n,
                    'max_lambda': max_lambda,
                })

            if batch_start == 0:
                break
            batch_start = max(0, batch_start - batch_size)

    except Exception as e:
        print(f"Error processing {filepath}: {e}", file=sys.stderr)
        return []

    return results


def main():
    parser = argparse.ArgumentParser(
        description='Summarize boundratio files for plotting',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s boundratiomin-23PR.5--=ALPHA=--v0.2.0.csv boundratiomin-summary-23PR.5-v0.2.0.csv
  %(prog)s boundratiomax-23PR.5--=ALPHA=--v0.2.0.csv boundratiomax-summary-23PR.5-v0.2.0.csv
        """
    )
    parser.add_argument('file_pattern', help='File pattern with --=ALPHA=-- placeholder')
    parser.add_argument('output_file', help='Output CSV filename')
    parser.add_argument('--batch-size', type=int, default=12, help='Batch size for aggregation (default: 12)')

    args = parser.parse_args()

    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    output_dir = project_root / "output"

    if '--=ALPHA=--' not in args.file_pattern:
        print("Error: File pattern must contain '--=ALPHA=--' placeholder", file=sys.stderr)
        sys.exit(1)

    all_results = []
    for alpha in POWER_OF_2_ALPHAS:
        alpha_str = format_alpha(alpha)
        alpha_dir = output_dir / f"alpha-{alpha_str}"
        if not alpha_dir.exists():
            print(f"Warning: Directory {alpha_dir} does not exist", file=sys.stderr)
            continue

        file_pattern = args.file_pattern.replace('--=ALPHA=--', f'-{alpha_str}-')
        file_path = alpha_dir / file_pattern
        if not file_path.exists():
            print(f"Warning: File {file_path} does not exist", file=sys.stderr)
            continue

        results = process_file(file_path, alpha, args.batch_size)
        all_results.extend(results)

    if not all_results:
        print("No results found", file=sys.stderr)
        sys.exit(1)

    output_path = Path(args.output_file)
    with open(output_path, 'w', newline='') as f:
        writer = csv.DictWriter(
            f,
            fieldnames=[
                'alpha',
                'n_min_lambda',
                'min_lambda',
                'n_max_lambda',
                'max_lambda',
            ],
        )
        writer.writeheader()
        writer.writerows(all_results)

    alpha_count = len({row['alpha'] for row in all_results})
    print(
        f"Summary written to {output_path} ({len(all_results)} rows, {alpha_count} alphas)",
        file=sys.stderr,
    )


if __name__ == '__main__':
    main()
