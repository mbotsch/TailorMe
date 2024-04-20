#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$SCRIPT_DIR" || exit > /dev/null
printf "Base dir: %s\n" "$SCRIPT_DIR"

# leftover from downloads
rm -rf parallel_hashmap torch_sparse torch_scatter

# library files
rm -rf libtorch

# building directories
rm -rf pytorch_scatter pytorch_sparse pyg

