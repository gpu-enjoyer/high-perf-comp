#!/bin/bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CODE_DIR="$ROOT_DIR/code"
BUILD_DIR="$ROOT_DIR/build"
CONFIG_DIR="$ROOT_DIR/config"
VENV_DIR="$ROOT_DIR/.venv"
VENV_PYTHON="$VENV_DIR/bin/python"

mkdir -p "$BUILD_DIR" "$ROOT_DIR/results/plots"

if [ ! -x "$VENV_PYTHON" ]; then
    python3 -m venv "$VENV_DIR"
fi

"$VENV_PYTHON" -m pip install --upgrade pip >/dev/null
"$VENV_PYTHON" -m pip install -r "$ROOT_DIR/script/requirements.txt"

mapfile -t sources < <(find "$CODE_DIR" -maxdepth 1 -type f -name 'OpenMP_*.cpp' | sort)

if [ "${#sources[@]}" -eq 0 ]; then
    echo "No OpenMP_*.cpp files found in $CODE_DIR" >&2
    exit 1
fi

for source in "${sources[@]}"; do
    task="$(basename "${source%.cpp}")"
    executable="$BUILD_DIR/$task"
    config_file="$CONFIG_DIR/$task.conf"

    echo "Compiling $task"
    g++ -O3 -fopenmp "$source" -o "$executable"

    if [ ! -f "$config_file" ]; then
        echo "Missing config file for $task: $config_file" >&2
        exit 1
    fi

    echo "Running $task"
    "$ROOT_DIR/script/run_task.sh" "$executable" "$config_file"
done

echo "Building plots"
"$VENV_PYTHON" "$ROOT_DIR/script/plot.py"

echo "Done"
