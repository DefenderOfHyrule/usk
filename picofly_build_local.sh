#!/bin/bash
set -e

# colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Picofly build script for EndeavourOS ===${NC}\n"

# check if we're in the usk directory
if [ ! -f "config.h" ] || [ ! -d ".git" ]; then
    echo -e "${RED}Error: This script must be run from the usk repository root${NC}"
    echo "Please cd into the usk directory first"
    exit 1
fi

# step 1: install dependencies
echo -e "${YELLOW}[1/7] Installing dependencies...${NC}"
sudo pacman -S --needed arm-none-eabi-gcc arm-none-eabi-newlib cmake make python git

# set workspace as parent directory of usk
WORKSPACE="$(cd .. && pwd)"
USK_DIR="$WORKSPACE/usk"

echo -e "${YELLOW}[2/7] Using workspace: $WORKSPACE${NC}"

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
ln -sf "$PICO_SDK_PATH/external/pico_sdk_import.cmake" "$WORKSPACE/busk/pico_sdk_import.cmake"
ln -sf "$PICO_SDK_PATH/external/pico_sdk_import.cmake" "$WORKSPACE/usk/pico_sdk_import.cmake"

# step 5: create generated directory
mkdir -p "$WORKSPACE/usk/generated"

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

# prepare.py will look for ../busk/busk.bin from build/usk, which is build/busk/busk.bin

# restore original memmap_default.ld
rm -f "$MEMMAP_PATH"
mv "$MEMMAP_PATH.bak" "$MEMMAP_PATH"

cd "$WORKSPACE"

# step 7: build usk
echo -e "${YELLOW}[7/7] Building usk...${NC}"
mkdir -p "$WORKSPACE/build/usk"
cd "$WORKSPACE/build/usk"
cmake "$WORKSPACE/usk"
make

# prepare.py looks for ../busk/busk.bin relative to build/usk
python3 "$WORKSPACE/usk/prepare.py"

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
USK_VERSION_LO=$(sed -n 's/#define VER_LO \([0-9]*\)/\1/p' "$WORKSPACE/usk/config.h")
USK_VERSION_HI=$(sed -n 's/#define VER_HI \([0-9]*\)/\1/p' "$WORKSPACE/usk/config.h")
USK_VERSION="${USK_VERSION_HI}.${USK_VERSION_LO}"

echo -e "${GREEN}Version: Picofly ${USK_VERSION}${NC}\n"
