#!/usr/bin/env bash
torch_scatter_version="2.1.2"
torch_sparse_version="0.6.18"
parallel_hashmap_version="1.3.11"
BUILD_FROM_GIT=true
CLEAN_BUILD=false
# set to true
DOWNLOAD=true

# for cuda 12
GIT_SCATTER_COMMIT="f4696b75534cac73c559d43e79dc25d71be32c25"
GIT_SPARSE_COMMIT="85ace2586952fa366b6bcc3b7dc8e698f021f12c"


SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$SCRIPT_DIR" || exit > /dev/null
printf "Base dir: %s\n" "$SCRIPT_DIR"

CMAKE_TORCH_DIR=$(realpath "$SCRIPT_DIR/libtorch/share/cmake/Torch")


printf "Obtaining source code ... \n"

if [[ $BUILD_FROM_GIT = true ]] ; then
  ### building latest git versions
  printf "Building from git. \n"

  # pytorch scatter (from git)
  if [[ $CLEAN_BUILD = true && -d pytorch_scatter ]] ; then
    rm -rf pytorch_scatter
  fi
  if [ ! -d pytorch_scatter ]; then
    git clone --recursive https://github.com/rusty1s/pytorch_scatter.git pytorch_scatter
    pushd pytorch_scatter || exit
    git checkout $GIT_SCATTER_COMMIT
    git submodule update --init --recursive
    popd || exit
  fi

  # pytorch sparse (from git)
  if [[ $CLEAN_BUILD = true && -d pytorch_sparse ]] ; then
    rm -rf pytorch_sparse
  fi
  if [ ! -d pytorch_sparse ]; then
    git clone --recursive https://github.com/rusty1s/pytorch_sparse.git pytorch_sparse
    pushd pytorch_sparse || exit
    git checkout $GIT_SPARSE_COMMIT
    git submodule update --init --recursive
    popd || exit
  fi

else
  ### building release versions
  printf "Building latest release version. \n"

  rm -rf torch_scatter torch_sparse parallel_hashmap
  if [[ $CLEAN_BUILD = true ]] ; then
    rm -rf pytorch_scatter pytorch_sparse
  fi

  if [[ $DOWNLOAD != false ]] ; then
  
    if [ ! -d "torch_scatter" ]; then
      # pytorch scatter
      wget "https://github.com/rusty1s/pytorch_scatter/archive/refs/tags/$torch_scatter_version.zip" -O torch_scatter.zip
      unzip -qq torch_scatter.zip -d torch_scatter
      mv "torch_scatter/pytorch_scatter-$torch_scatter_version" pytorch_scatter
      rm -f torch_scatter.zip
      rmdir torch_scatter
    fi

    if [ ! -d "torch_sparse" ]; then
      # pytorch sparse (from release)
      wget "https://github.com/rusty1s/pytorch_sparse/archive/refs/tags/$torch_sparse_version.zip" -O torch_sparse.zip
      unzip -qq torch_sparse.zip -d torch_sparse
      mv "torch_sparse/pytorch_sparse-$torch_sparse_version" pytorch_sparse
      rm -f torch_sparse.zip
      rmdir torch_sparse
    fi

    # parallel hashmap
    if [ -z "$(ls -A pytorch_sparse/third_party/parallel-hashmap)" ]; then
      wget "https://github.com/greg7mdp/parallel-hashmap/archive/refs/tags/v${parallel_hashmap_version}.zip" -O parallel_hashmap.zip
      unzip -qq parallel_hashmap.zip -d parallel_hashmap
      rm -rf pytorch_sparse/third_party/parallel-hashmap
      mv "parallel_hashmap/parallel-hashmap-${parallel_hashmap_version}/" pytorch_sparse/third_party/parallel-hashmap
      rm -f parallel_hashmap.zip
      rmdir parallel_hashmap
    fi
  fi
fi


# change c++ version to 17
if true; then
  sed -i "s/set(CMAKE_CXX_STANDARD 14)/set(CMAKE_CXX_STANDARD 17)/g" pytorch_scatter/CMakeLists.txt
  sed -i "s/set(CMAKE_CXX_STANDARD 14)/set(CMAKE_CXX_STANDARD 17)/g" pytorch_sparse/CMakeLists.txt
fi

# build options
if [ -z "$TORCH_BUILD_OPTS" ]; then
  TORCH_BUILD_OPTS="-DWITH_CUDA:BOOL=ON"
fi

printf "Start building ... \n"

# build torch_scatter
pushd pytorch_scatter || exit

# has no effect?
# -D CMAKE_C_COMPILER=$CC -D CMAKE_CXX_COMPILER=$CXX \

cmake -S . -B build -G Ninja -Wno-dev \
  -DTorch_DIR="${CMAKE_TORCH_DIR}" "${TORCH_BUILD_OPTS}"
cmake --build build
cmake --install build --prefix "$SCRIPT_DIR/pyg"
popd || exit


# build torch_scatter
pushd pytorch_sparse || exit
cmake -S . -B build -G Ninja -Wno-dev \
  -DTorch_DIR="${CMAKE_TORCH_DIR}" "${TORCH_BUILD_OPTS}"

cmake --build build
cmake --install build --prefix "$SCRIPT_DIR/pyg"
popd || exit


#conda deactivate
printf "Finished building. \n"
