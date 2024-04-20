#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$SCRIPT_DIR" || exit > /dev/null
printf "Base dir: %s\n" "$SCRIPT_DIR"

# --- download torch dependencies (set to false)
SKIP_DOWNLOAD=false

# TODO: Add prebuilt library
# https://github.com/mlverse/libtorch-mac-m1

if [[ $SKIP_DOWNLOAD = false ]] ; then
  # torch
  if [ ! -d libtorch ] ; then
    mkdir -p torch
    TORCH_VERSION="2.2.2"
    # cpu or cu118
    TORCH_ARCH="cu118"
    #wget https://download.pytorch.org/libtorch/nightly/cpu/libtorch-shared-with-deps-latest.zip -O torch.zip
    #wget "https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-${TORCH_VERSION}%2Bcpu.zip" -O torch.zip
    case "$(uname -s)" in
	    #Linux*)  torchUrl="https://download.pytorch.org/libtorch/nightly/cpu/libtorch-shared-with-deps-latest.zip";;
	    #Linux*)  torchUrl="https://download.pytorch.org/libtorch/cpu/libtorch-shared-with-deps-${TORCH_VERSION}%2Bcpu.zip";;
	    Linux*)  torchUrl="https://download.pytorch.org/libtorch/${TORCH_ARCH}/libtorch-cxx11-abi-shared-with-deps-${TORCH_VERSION}%2B${TORCH_ARCH}.zip";;
	    Darwin*) torchUrl="https://download.pytorch.org/libtorch/cpu/libtorch-macos-arm64-${TORCH_VERSION}.zip";;
	    CYGWIN*) torchUrl="https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-${TORCH_VERSION}%2Bcpu.zip";;
	    MINGW*)  torchUrl="https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-${TORCH_VERSION}%2Bcpu.zip";;
	    *)       torchUrl="https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-${TORCH_VERSION}%2Bcpu.zip";;
    esac
    wget $torchUrl -O torch.zip

    unzip torch.zip -d torch
    mv torch/libtorch libtorch
    rm -f torch.zip
    rmdir torch
  fi
fi

# build variables
PATH=/opt/cuda/bin:$PATH
#PATH=PATH $(which bash) build-pyg.sh

# no cuda on macos
TORCH_BUILD_OPTS="-DWITH_CUDA:BOOL=OFF"

source build-pyg.sh


