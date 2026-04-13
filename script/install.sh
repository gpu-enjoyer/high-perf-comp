#!/bin/bash

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

if ! sudo apt install -y libomp-dev python3-venv; then
    echo "Warning: apt install failed, continuing with existing toolchain" >&2
fi

python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip -q
pip install matplotlib -q
deactivate
