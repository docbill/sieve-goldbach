#!/usr/bin/env python3
"""
Summarize lambdaboundmin files for plotting.

Reads from output/ and writes summary to current directory.

For each file:
- Aggregate over 12 lines (working backwards from the end)
- Extract: alpha value, n for minimum lambda, minimum lambda, n for maximum lambda, maximum lambda
- Filter to only alpha values that are powers of 2: 1, 1/2, 1/4, ..., 1/1024

Usage:
    summarizelambdabound.py <file_pattern> <output_file>
    
Example:
    summarizelambdabound.py lambdaboundmin-23PR.5--=ALPHA=--v0.2.0.csv lambdaboundmin-summary-23PR.5-v0.2.0.csv
"""

import argparse
import csv
import os
import sys
import glob
from pathlib import Path

# Alpha values that are powers of 2: 1, 1/2, 1/4, ..., 1/1024
POWER_OF_2_ALPHAS = [1.0 / (2**i) for i in range(11)]  # 1, 0.5, 0.25, ..., 1/1024
POWER_OF_2_ALPHAS.reverse()  # Start from smallest: 1/1024, ..., 1

# Format alpha values to match directory names (with appropriate precision)
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
        # Fallback to string representation
        return str(alpha)

def parse_lambda(value_str):
    """Parse lambda value, handling empty strings and zero values."""
    if not value_str or value_str.strip() == '' or value_str == '0.000000':
        return None
    try:
        return float(value_str)
    except ValueError:
        return None

def process_file(filepath, alpha_value, lambda_column):
    """Process a single lambdabound file and return summary data."""
    results = []
    
    try:
        with open(filepath, 'r') as f:
            reader = csv.DictReader(f)
            rows = list(reader)
        
        # Work backwards from the end
        # Group into batches of 12 lines
        batch_size = 12
        num_rows = len(rows)
        
        if num_rows == 0:
            return results
        
        # Start from the end, working backwards
        batch_start = max(0, num_rows - batch_size)
        
        # Process batches from the end
        while batch_start >= 0:
            batch_end = min(batch_start + batch_size, num_rows)
            batch = rows[batch_start:batch_end]
            
            # Find min and max lambda in this batch
            min_lambda = None
            min_lambda_n = None
            max_lambda = None
            max_lambda_n = None
            
            for row in batch:
                lambda_val = parse_lambda(row.get(lambda_column, ''))
                if lambda_val is not None:
                    # Try n_0 first (for lambdaboundmin), then n_1 (for lambdaboundmax)
                    n_val_str = row.get('n_0', '') or row.get('n_1', '')
                    if not n_val_str:
                        continue
                    n_val = int(n_val_str)
                    
                    if min_lambda is None or lambda_val < min_lambda:
                        min_lambda = lambda_val
                        min_lambda_n = n_val
                    
                    if max_lambda is None or lambda_val > max_lambda:
                        max_lambda = lambda_val
                        max_lambda_n = n_val
            
            # Only add result if we found at least one valid lambda
            if min_lambda is not None or max_lambda is not None:
                results.append({
                    'alpha': alpha_value,
                    'n_min_lambda': min_lambda_n,
                    'min_lambda': min_lambda,
                    'n_max_lambda': max_lambda_n,
                    'max_lambda': max_lambda
                })
            
            # Move to next batch (backwards)
            if batch_start == 0:
                break
            batch_start = max(0, batch_start - batch_size)
    
    except Exception as e:
        print(f"Error processing {filepath}: {e}", file=sys.stderr)
        return []
    
    return results

def main():
    parser = argparse.ArgumentParser(
        description='Summarize lambdabound files for plotting',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s lambdaboundmin-23PR.5--=ALPHA=--v0.2.0.csv lambdaboundmin-summary-23PR.5-v0.2.0.csv
  %(prog)s lambdaboundmax-19PR--=ALPHA=--v0.2.0.csv lambdaboundmax-summary-19PR-v0.2.0.csv
        """
    )
    parser.add_argument('file_pattern', 
                       help='File pattern with --=ALPHA=-- placeholder (e.g., lambdaboundmin-23PR.5--=ALPHA=--v0.2.0.csv)')
    parser.add_argument('output_file',
                       help='Output CSV filename (e.g., lambdaboundmin-summary-23PR.5-v0.2.0.csv)')
    
    args = parser.parse_args()
    
    # Get script directory and project root
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    
    # Read from output/
    output_dir = project_root / "output"
    
    if not output_dir.exists():
        print(f"Error: Output directory {output_dir} does not exist", file=sys.stderr)
        sys.exit(1)
    
    # Check that file_pattern contains --=ALPHA=--
    if '--=ALPHA=--' not in args.file_pattern:
        print(f"Error: File pattern must contain '--=ALPHA=--' placeholder", file=sys.stderr)
        print(f"Example: lambdaboundmin-23PR.5--=ALPHA=--v0.2.0.csv", file=sys.stderr)
        sys.exit(1)
    
    # Detect lambda column name from filename pattern
    if 'lambdaboundmin' in args.file_pattern:
        lambda_column = 'Lambda_min'
    elif 'lambdaboundmax' in args.file_pattern:
        lambda_column = 'Lambda_max'
    else:
        print(f"Error: Cannot determine lambda column from filename pattern", file=sys.stderr)
        print(f"Pattern must contain 'lambdaboundmin' or 'lambdaboundmax'", file=sys.stderr)
        sys.exit(1)
    
    all_results = []
    
    # Find lambdabound files for power-of-2 alpha values
    for alpha in POWER_OF_2_ALPHAS:
        alpha_str = format_alpha(alpha)
        alpha_dir = output_dir / f"alpha-{alpha_str}"
        
        if not alpha_dir.exists():
            print(f"Warning: Directory {alpha_dir} does not exist", file=sys.stderr)
            continue
        
        # Replace --=ALPHA=-- with -{alpha}- (with dashes around alpha value)
        file_pattern = args.file_pattern.replace('--=ALPHA=--', f'-{alpha_str}-')
        file_path = alpha_dir / file_pattern
        
        if not file_path.exists():
            print(f"Warning: File {file_path} does not exist", file=sys.stderr)
            continue
        
        # Process the file
        results = process_file(file_path, alpha, lambda_column)
        all_results.extend(results)
    
    # Output results as CSV
    output_path = Path(args.output_file)
    if all_results:
        with open(output_path, 'w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=['alpha', 'n_min_lambda', 'min_lambda', 'n_max_lambda', 'max_lambda'])
            writer.writeheader()
            writer.writerows(all_results)
        print(f"Summary written to {output_path} ({len(all_results)} rows)", file=sys.stderr)
    else:
        print("No results found", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()
