#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")/.."

echo "Want to delete all generated files in \"$PROJ_DIR\"? (y/n)"
read -r answer

if [[ "$answer" == [yY] ]]; then
    for dir in .venv build result; do
        if [ -d "$dir" ]; then
            rm -r "$dir" && echo "Removed: \"$dir\""
        fi
    done
else
    echo "Canceled"
fi
