#!/usr/bin/env python3
from __future__ import annotations

import csv
from collections import defaultdict
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT_DIR = Path(__file__).resolve().parents[1]
RESULTS_DIR = ROOT_DIR / "results"
AGGREGATION = "min"


def aggregate(values: list[float]) -> float:
    if AGGREGATION == "min":
        return min(values)
    if AGGREGATION == "mean":
        return sum(values) / len(values)
    raise ValueError(f"Unsupported aggregation: {AGGREGATION}")


def sanitize_filename(s: str) -> str:
    replaced = s.replace(" ", "_")
    for ch in ',/\\:;()[]{}':
        replaced = replaced.replace(ch, "_")

    cleaned = "".join(ch if ch.isalnum() or ch in "-_" else "_" for ch in replaced)
    while "__" in cleaned:
        cleaned = cleaned.replace("__", "_")

    cleaned = cleaned.strip("_")
    return cleaned or "empty"


def discover_task_csvs() -> list[tuple[str, Path]]:
    csvs: list[tuple[str, Path]] = []
    for task_dir in sorted(RESULTS_DIR.glob("OpenMP_*")):
        if not task_dir.is_dir():
            continue
        csv_path = task_dir / "data.csv"
        if csv_path.exists():
            csvs.append((task_dir.name, csv_path))
    return csvs


def load_rows(csv_path: Path) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    with csv_path.open("r", encoding="utf-8", newline="") as fh:
        reader = csv.DictReader(fh)
        required = {
            "taskName",
            "argString",
            "numThreads",
            "timeSeconds",
            "result",
            "runIndex",
        }
        if not reader.fieldnames or not required.issubset(set(reader.fieldnames)):
            return rows

        rows.extend(reader)
    return rows


def plot_group(
    task_name: str,
    arg_string: str,
    rows: list[dict[str, str]],
    time_plots_dir: Path,
    speedup_plots_dir: Path,
) -> None:
    thread_to_times: dict[int, list[float]] = defaultdict(list)
    for row in rows:
        threads = int(row["numThreads"])
        elapsed = float(row["timeSeconds"])
        thread_to_times[threads].append(elapsed)

    sorted_threads = sorted(thread_to_times)
    if not sorted_threads:
        return

    aggregated_times = [aggregate(thread_to_times[t]) for t in sorted_threads]
    baseline = aggregated_times[0]
    speedups = [baseline / t if t > 0 else 0.0 for t in aggregated_times]

    safe_task = sanitize_filename(task_name)
    safe_arg_string = sanitize_filename(arg_string)
    filename = f"{safe_task}__{safe_arg_string}.png"

    title_suffix = f"args={arg_string} ({AGGREGATION})"

    plt.figure(figsize=(8, 5))
    plt.plot(sorted_threads, aggregated_times, marker="o")
    plt.xlabel("Threads")
    plt.ylabel("Time (seconds)")
    plt.title(f"{task_name}: time vs threads\\n{title_suffix}")
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(time_plots_dir / filename, dpi=140)
    plt.close()

    plt.figure(figsize=(8, 5))
    plt.plot(sorted_threads, speedups, marker="o")
    plt.xlabel("Threads")
    plt.ylabel("Speedup")
    plt.title(f"{task_name}: speedup vs threads\\n{title_suffix}")
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(speedup_plots_dir / filename, dpi=140)
    plt.close()


def main() -> int:
    for task_name, csv_path in discover_task_csvs():
        rows = load_rows(csv_path)
        if not rows:
            continue

        task_root = RESULTS_DIR / task_name
        time_plots_dir = task_root / "plots" / "time"
        speedup_plots_dir = task_root / "plots" / "speedup"
        time_plots_dir.mkdir(parents=True, exist_ok=True)
        speedup_plots_dir.mkdir(parents=True, exist_ok=True)

        grouped: dict[tuple[str, str], list[dict[str, str]]] = defaultdict(list)
        for row in rows:
            grouped[(row["taskName"], row["argString"])].append(row)

        for (group_task_name, arg_string), group_rows in grouped.items():
            plot_group(group_task_name, arg_string, group_rows, time_plots_dir, speedup_plots_dir)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
