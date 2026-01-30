#!/bin/bash
set -e

# colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Picofly build script (multi-distro) ===${NC}\n"

# check if we're in the usk directory
if [ ! -f "config.h" ] || [ ! -d ".git" ]; then
    echo -e "${RED}Error: This script must be run from the usk repository root${NC}"
    echo "Please cd into the usk directory first"
    exit 1
fi

# detect distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    DISTRO=$ID
else
    echo -e "${RED}Error: Cannot detect distribution${NC}"
    exit 1
fi

# step 1: install dependencies
echo -e "${YELLOW}[1/7] Installing dependencies for $DISTRO...${NC}"

case $DISTRO in
    arch|endeavouros|manjaro)
        sudo pacman -S --needed arm-none-eabi-gcc arm-none-eabi-newlib cmake make python git
        ;;
    fedora)
        sudo dnf install -y arm-none-eabi-gcc-cs arm-none-eabi-gcc-cs-c++ arm-none-eabi-newlib cmake make python3 git gcc-c++
        ;;
    ubuntu|debian|pop|linuxmint)
        sudo apt update
        sudo apt install -y gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential cmake make python3 git
        ;;
    *)
        echo -e "${RED}Error: Unsupported distribution: $DISTRO${NC}"
        echo "Please install the following packages manually:"
        echo "  - ARM embedded GCC toolchain (arm-none-eabi-gcc)"
        echo "  - newlib for ARM (arm-none-eabi-newlib)"
        echo "  - cmake, make, python3, git"
        exit 1
        ;;
esac

# set workspace as build directory inside usk
WORKSPACE="$(pwd)/Picofly-build-local"
echo -e "${YELLOW}[2/7] Creating workspace at $WORKSPACE...${NC}"
mkdir -p "$WORKSPACE"

# step 2: clone sibling repositories
echo -e "${YELLOW}[3/7] Cloning sibling repositories...${NC}"
cd "$WORKSPACE"

if [ ! -d "busk" ]; then
    git clone https://github.com/DefenderOfHyrule/busk.git
else
    echo "busk already exists, using local version..."
fi

if [ ! -d "pico-sdk" ]; then
    git clone --recursive https://github.com/raspberrypi/pico-sdk.git
else
    echo "pico-sdk already exists, using local version..."
fi

# step 3: set environment variable
echo -e "${YELLOW}[4/7] Setting PICO_SDK_PATH...${NC}"
export PICO_SDK_PATH="$WORKSPACE/pico-sdk"

# step 4: create symbolic links
echo -e "${YELLOW}[5/7] Creating symbolic links...${NC}"
USK_DIR="$(dirname "$WORKSPACE")"
ln -sf "$PICO_SDK_PATH/external/pico_sdk_import.cmake" "$WORKSPACE/busk/pico_sdk_import.cmake"
ln -sf "$PICO_SDK_PATH/external/pico_sdk_import.cmake" "$USK_DIR/pico_sdk_import.cmake"

# step 5: create generated directory
mkdir -p "$USK_DIR/generated"

# step 6: build busk
echo -e "${YELLOW}[6/7] Building busk...${NC}"

# backup and modify memmap_default.ld for busk build
MEMMAP_PATH="$WORKSPACE/pico-sdk/src/rp2_common/pico_crt0/rp2040/memmap_default.ld"
cp "$MEMMAP_PATH" "$MEMMAP_PATH.bak"
sed -i 's/RAM(rwx) : ORIGIN =  0x20000000, LENGTH = 256k/RAM(rwx) : ORIGIN = 0x20038000, LENGTH = 32k/g' "$MEMMAP_PATH"

# build busk
mkdir -p "$WORKSPACE/build/busk"
cd "$WORKSPACE/build/busk"
cmake "$WORKSPACE/busk"
make

# restore original memmap_default.ld
rm -f "$MEMMAP_PATH"
mv "$MEMMAP_PATH.bak" "$MEMMAP_PATH"
cd "$WORKSPACE"

# step 7: build usk
echo -e "${YELLOW}[7/7] Building usk...${NC}"
mkdir -p "$WORKSPACE/build/usk"
cd "$WORKSPACE/build/usk"
cmake "$USK_DIR"
make

# prepare.py looks for ../busk/busk.bin relative to build/usk
python3 "$USK_DIR/prepare.py"

# clean up both builds
cd "$WORKSPACE/build/busk"
make clean
cd "$WORKSPACE/build/usk"
make clean

# success!
echo -e "\n${GREEN}=== Build Complete! ===${NC}"
echo -e "${GREEN}Output files:${NC}"
echo -e "  - firmware.uf2: ${WORKSPACE}/build/usk/firmware.uf2"
echo -e "  - update.bin:   ${WORKSPACE}/build/usk/update.bin"

# get version info
USK_VERSION_LO=$(sed -n 's/#define VER_LO \([0-9]*\)/\1/p' "$USK_DIR/config.h")
USK_VERSION_HI=$(sed -n 's/#define VER_HI \([0-9]*\)/\1/p' "$USK_DIR/config.h")
USK_VERSION="${USK_VERSION_HI}.${USK_VERSION_LO}"
echo -e "${GREEN}Version: Picofly ${USK_VERSION}${NC}\n"