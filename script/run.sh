#!/bin/bash

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

task=${1:?Usage: run.sh <task>}

source "conf/common.conf"
source "conf/${task}.conf"

# Pin OpenMP threads to CPU cores for more stable benchmarking.
export OMP_DYNAMIC=false
export OMP_PROC_BIND=close
export OMP_PLACES=cores

mkdir -p "result/$task"
echo "program,mode,threads,time,result" > "result/$task/data.csv"

for threads in "${thread_list[@]}"; do
    for arg_line in "${arg_list[@]}"; do
        read -r size mode <<< "$arg_line"
        result_line=$(./build/"${task}" "$size" "$mode" --threads="$threads" --runs="$runs" 2>/dev/null || true)

        if [[ -z "$result_line" ]]; then
            echo "Failed to run ./build/${task} with args: $arg_line --threads=$threads" >&2
            continue
        fi

        IFS=',' read -r program csv_mode csv_threads csv_time csv_result <<< "$result_line"
        csv_mode="${csv_mode//[[:space:]]/}"
        csv_threads="${csv_threads//[[:space:]]/}"
        csv_time="${csv_time//[[:space:]]/}"
        csv_result="${csv_result## }"

        echo "${task},${csv_mode},${csv_threads},${csv_time},${csv_result}" >> "result/$task/data.csv"
    done
done

echo "Results saved to result/$task/data.csv"
