#!/bin/bash
set -euo pipefail

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <executable> <config-file>" >&2
    exit 1
fi

EXECUTABLE="$1"
CONFIG_FILE="$2"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TASK_NAME="$(basename "$EXECUTABLE")"
TASK_RESULTS_DIR="$ROOT_DIR/results/${TASK_NAME}"
OUTPUT_FILE="$TASK_RESULTS_DIR/data.csv"

if [ ! -x "$EXECUTABLE" ]; then
    echo "Executable not found or not executable: $EXECUTABLE" >&2
    exit 1
fi

if [ ! -f "$CONFIG_FILE" ]; then
    echo "Config file not found: $CONFIG_FILE" >&2
    exit 1
fi

# shellcheck source=/dev/null
source "$CONFIG_FILE"

if ! declare -p ARGS_LIST >/dev/null 2>&1 || ! declare -p THREADS_LIST >/dev/null 2>&1 || [ -z "${RUNS:-}" ]; then
    echo "Config must define ARGS_LIST, THREADS_LIST and RUNS" >&2
    exit 1
fi

if [ "${#ARGS_LIST[@]}" -eq 0 ] || [ "${#THREADS_LIST[@]}" -eq 0 ] || [ "$RUNS" -le 0 ]; then
    echo "Config values must be non-empty and RUNS > 0" >&2
    exit 1
fi

mkdir -p "$TASK_RESULTS_DIR"
echo "taskName,argString,numThreads,timeSeconds,result,runIndex" > "$OUTPUT_FILE"

for arg_string in "${ARGS_LIST[@]}"; do
    for thread_count in "${THREADS_LIST[@]}"; do
        export OMP_NUM_THREADS="$thread_count"

        for ((run_index = 1; run_index <= RUNS; run_index++)); do
            read -r -a args_array <<< "$arg_string"
            program_output=$("$EXECUTABLE" "${args_array[@]}")

            if [ "$(printf '%s\n' "$program_output" | wc -l)" -ne 1 ]; then
                echo "Program must output exactly one line for $TASK_NAME" >&2
                exit 1
            fi

            IFS=',' read -r f1 f2 f3 f4 f5 extra <<< "$program_output"
            if [ -z "$f1" ] || [ -z "$f2" ] || [ -z "$f3" ] || [ -z "$f4" ] || [ -z "$f5" ] || [ -n "${extra:-}" ]; then
                echo "Program output must have exactly 5 CSV fields: $program_output" >&2
                exit 1
            fi

            echo "$program_output,$run_index" >> "$OUTPUT_FILE"
        done
    done
done
