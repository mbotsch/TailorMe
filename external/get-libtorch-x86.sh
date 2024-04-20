#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$SCRIPT_DIR" || exit > /dev/null
printf "Base dir: %s\n" "$SCRIPT_DIR"

# --- download torch dependencies (set to false)
SKIP_DOWNLOAD=false

# only compatible with gcc-11
export CC=/usr/bin/gcc-11
export CXX=/usr/bin/g++-11

if [[ $SKIP_DOWNLOAD = false ]] ; then
  # torch
  if [ ! -d libtorch ] ; then
    mkdir -p torch
    TORCH_VERSION="2.2.2"
    # cpu or cu118
    TORCH_ARCH="cu121"
    #wget https://download.pytorch.org/libtorch/nightly/cpu/libtorch-shared-with-deps-latest.zip -O torch.zip
    #wget "https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-${TORCH_VERSION}%2Bcpu.zip" -O torch.zip
    case "$(uname -s)" in
	    #Linux*)  torchUrl="https://download.pytorch.org/libtorch/nightly/cpu/libtorch-shared-with-deps-latest.zip";;
	    #Linux*)  torchUrl="https://download.pytorch.org/libtorch/cpu/libtorch-shared-with-deps-${TORCH_VERSION}%2Bcpu.zip";;
	    Linux*)  torchUrl="https://download.pytorch.org/libtorch/${TORCH_ARCH}/libtorch-cxx11-abi-shared-with-deps-${TORCH_VERSION}%2B${TORCH_ARCH}.zip";;
	    Darwin*) torchUrl="https://download.pytorch.org/libtorch/cpu/libtorch-macos-${TORCH_VERSION}.zip";;
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

# apply patches
patch -p0 -i patches/libtorch-cuda-12.4-fix.patch


# build variables
PATH=/opt/cuda/bin:$PATH
#PATH=PATH $(which bash) build-pyg.sh
source build-pyg.sh


