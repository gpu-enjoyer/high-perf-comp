#!/bin/bash

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

task=${1:?Usage: build.sh <task>}

mkdir -p build
g++ -std=c++17 -fopenmp "src/$task.cpp" -o "build/$task"

echo "build/$task compiled"
