# TailorMe

Implementation of the paper Wenninger, Kemper, Schwanecke, Botsch, "TailorMe: Self-Supervised Learning of an Anatomically Constrained Volumetric Human Shape Model".

## Prerequisites

### Cloning this Repository
```bash
git clone https://github.com/mbotsch/TailorMe
```

### Install external dependencies

- gcc (>=11.0,<13.0)
- clang (>=15.0.0.0,<16.0.0.0)
- cuda (>=11.8,<=12.4)
- python3 (==3.11)
- *Optional* OpenMP

#### Debian/Ubuntu

```bash
apt install zlib1g-dev libzip-dev liblzma-dev libbz2-dev zipcmp zipmerge ziptool nlohmann-json3-dev libfmt-dev cmake ninja-build
```

#### MacOS homebrew
```bash
brew install wget libzip nlohmann-json fmt libomp ninja
```

### Python3 venv

```bash
python3.11 -m venv .venv
source .venv/bin/activate
```

#### Linux

```bash
pip3 install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu121
```

#### MacOS

```bash
pip3 install torch torchvision torchaudio
```

### Pytorch + Pytorch Geometric

We have prepared some scripts for downloading and compiling pytorch and pytorch geometric for C++.

#### Linux

```bash
cd external
source get-libtorch-x86.sh
cd ..
```

#### MacOS

```bash
cd external 
source get-libtorch-osx-arm.sh
cd ..
```

## Building the project

### MacOS
```bash
# If you want to use OpenMP, only works when libtorch compiled from source
# export OpenMP_ROOT=$(brew --prefix)/opt/libomp
cp $(brew --prefix)/opt/libomp/lib/libomp.dylib external/libtorch/lib/libomp.dylib
```

### All operating systems
```bash
cmake -B build -G Ninja -S .
cmake --build build
```

## Running

```bash
./build/tailorme_viewer
```

## Third Party
- PMP Library is included from [pmp-library](https://www.pmp-library.org)
- The tri-tri-intersection algorithm is borrowed from [libigl](https://libigl.github.io/).
