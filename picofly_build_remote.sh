#!/bin/bash
set -e

# colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # no color

echo -e "${GREEN}=== Picofly build script for EndeavourOS ===${NC}\n"

# step 1: install dependencies
echo -e "${YELLOW}[1/8] Installing dependencies...${NC}"
sudo pacman -S --needed arm-none-eabi-gcc arm-none-eabi-newlib cmake make python git

# step 2: create workspace directory
WORKSPACE="$HOME/Picofly-build"
echo -e "${YELLOW}[2/8] Creating workspace at $WORKSPACE...${NC}"
mkdir -p "$WORKSPACE"
cd "$WORKSPACE"

# step 3: clone repositories
echo -e "${YELLOW}[3/8] Cloning repositories...${NC}"

if [ ! -d "usk" ]; then
    git clone https://github.com/DefenderOfHyrule/usk.git
else
    echo "usk already exists, skipping..."
fi

if [ ! -d "busk" ]; then
    git clone https://github.com/DefenderOfHyrule/busk.git
else
    echo "busk already exists, skipping..."
fi

if [ ! -d "pico-sdk" ]; then
    git clone --recursive https://github.com/raspberrypi/pico-sdk.git
else
    echo "pico-sdk already exists, skipping..."
fi

# step 4: set environment variable
echo -e "${YELLOW}[4/8] Setting PICO_SDK_PATH...${NC}"
export PICO_SDK_PATH="$WORKSPACE/pico-sdk"

# step 5: create symbolic links
echo -e "${YELLOW}[5/8] Creating symbolic links...${NC}"
ln -sf "$PICO_SDK_PATH/external/pico_sdk_import.cmake" "$WORKSPACE/busk/pico_sdk_import.cmake"
ln -sf "$PICO_SDK_PATH/external/pico_sdk_import.cmake" "$WORKSPACE/usk/pico_sdk_import.cmake"

# step 6: create generated directory
echo -e "${YELLOW}[6/8] Creating generated directory...${NC}"
mkdir -p "$WORKSPACE/usk/generated"

# step 7: build busk
echo -e "${YELLOW}[7/8] Building busk...${NC}"

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

# step 8: build usk
echo -e "${YELLOW}[8/8] Building usk...${NC}"
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
