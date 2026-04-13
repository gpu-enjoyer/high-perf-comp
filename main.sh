#!/bin/bash

set -euo pipefail

if [[ $# -eq 0 ]]; then
    TASKS=(omp1 omp2 omp3 omp4 omp5 omp6 omp7 omp8 omp9)
else
    TASKS=("$@")
fi

PY=".venv/bin/python3"

cd "$(dirname "${BASH_SOURCE[0]}")"
mkdir -p result result/archive

echo "=== Running pipeline ... ==="

./script/install.sh
"$PY" script/prepare.py

for task in "${TASKS[@]}"; do
    ./script/build.sh "$task"
done

for task in "${TASKS[@]}"; do
    echo ""
    echo "=== Processing: \"$task\" ==="

    backup_dir="result/.backup_${task}"
    rm -rf "$backup_dir"

    if [[ -d "result/$task" ]]; then
        mv "result/$task" "$backup_dir"
    fi

    if ./script/run.sh "$task" && "$PY" script/draw.py "$task"; then
        if [[ -d "$backup_dir" ]]; then
            archive_dir="result/archive/$task"
            mkdir -p "$archive_dir"
            mv "$backup_dir" "$archive_dir/$(date +%Y.%m.%d_%H:%M:%S)"
        fi
        echo "=== \"$task\" completed ==="
    else
        rm -rf "result/$task"
        if [[ -d "$backup_dir" ]]; then
            mv "$backup_dir" "result/$task"
        fi
        echo "=== \"$task\" NOT completed ==="
        exit 1
    fi
done

echo ""
echo "=== All tasks complete ==="
echo "Saved to \"result/\""
