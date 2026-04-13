#!/usr/bin/python3

###  Люблю элегантные решения (по возможности)

###  Тут могут быть ошибки ...

import csv
import os
import sys
from collections import defaultdict

root = os.environ.get("ROOT_DIR", ".")
os.chdir(root)


try:
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
except ImportError:
    print("Error: matplotlib not installed. Run: pip install matplotlib")
    sys.exit(1)

def read_csv(filepath):
    """Read CSV and return dict: mode -> {threads: time}"""
    data = defaultdict(dict)
    with open(filepath, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            mode = row['mode'].strip()
            threads = int(row['threads'].strip())
            time = float(row['time'].strip())
            # Keep only first entry per (mode, threads)
            if threads not in data[mode]:
                data[mode][threads] = time
    return data

def plot_results(data, output_path):
    modes = sorted(data.keys())

    # Get all thread counts
    all_threads = set()
    for mode_data in data.values():
        all_threads.update(mode_data.keys())
    all_threads = sorted(all_threads)

    # Get base times (threads=1)
    base_times = {m: data[m].get(1, None) for m in modes}

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    colors = plt.cm.Set2(range(len(modes)))

    for i, mode in enumerate(modes):
        threads = sorted(data[mode].keys())
        times = [data[mode].get(t, None) for t in all_threads if t in data[mode]]
        valid_threads = [t for t in all_threads if t in data[mode]]

        if not times:
            continue

        base = base_times.get(mode)
        if base and base > 0:
            speedups = [base / t for t in times]
        else:
            speedups = [1.0] * len(times)

        ax1.plot(valid_threads, times, 'o-', label=mode, color=colors[i], linewidth=2, markersize=8)
        ax2.plot(valid_threads, speedups, 's--', label=mode, color=colors[i], linewidth=2, markersize=8)

    # Ideal speedup line
    max_threads = max(all_threads) if all_threads else 1
    ax2.plot([1, max_threads], [1, max_threads], 'k:', alpha=0.5, label='ideal')

    ax1.set_xlabel('Number of Threads')
    ax1.set_ylabel('Time (seconds)')
    ax1.set_title('Execution Time')
    ax1.legend()
    ax1.grid(True, alpha=0.3)

    ax2.set_xlabel('Number of Threads')
    ax2.set_ylabel('Speedup')
    ax2.set_title('Speedup vs Sequential')
    ax2.legend()
    ax2.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig(output_path, dpi=150)
    print(f"Plot saved to {output_path}")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python draw.py <task_name>")
        sys.exit(1)

    task = sys.argv[1]
    csv_path = f'result/{task}/data.csv'
    output_path = f'result/{task}/plot.png'

    if not os.path.exists(csv_path):
        print(f"Error: {csv_path} not found")
        sys.exit(1)

    data = read_csv(csv_path)
    if not data:
        print("No data found in CSV")
        sys.exit(1)

    plot_results(data, output_path)
