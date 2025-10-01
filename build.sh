#!/bin/bash

# InfiniTime Build Script
# Run this from the InfiniTime root directory

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}InfiniTime Build Script${NC}"
echo "=========================="

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: Please run this script from the InfiniTime root directory${NC}"
    exit 1
fi

# Set paths
ARM_TOOLCHAIN_PATH="/home/eric/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi"
NRF5_SDK_PATH="/home/eric/nRF5_SDK_15.3.0_59ac345"

# Check if paths exist
if [ ! -d "$ARM_TOOLCHAIN_PATH" ]; then
    echo -e "${RED}Error: ARM toolchain not found at $ARM_TOOLCHAIN_PATH${NC}"
    echo "Please update the ARM_TOOLCHAIN_PATH variable in this script"
    exit 1
fi

if [ ! -d "$NRF5_SDK_PATH" ]; then
    echo -e "${RED}Error: NRF5 SDK not found at $NRF5_SDK_PATH${NC}"
    echo "Please update the NRF5_SDK_PATH variable in this script"
    exit 1
fi

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo -e "${YELLOW}Creating build directory...${NC}"
    mkdir build
fi

# Change to build directory
cd build

echo -e "${YELLOW}Configuring with CMake...${NC}"
echo "ARM Toolchain: $ARM_TOOLCHAIN_PATH"
echo "NRF5 SDK: $NRF5_SDK_PATH"

# Run CMake configuration
cmake \
    -DARM_NONE_EABI_TOOLCHAIN_PATH="$ARM_TOOLCHAIN_PATH" \
    -DNRF5_SDK_PATH="$NRF5_SDK_PATH" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_DFU=0 \
    -DBUILD_RESOURCES=0 \
    -DTARGET_DEVICE=PINETIME \
    -S ..

if [ $? -eq 0 ]; then
    echo -e "${GREEN}CMake configuration successful!${NC}"
    
    echo -e "${YELLOW}Building pinetime-app...${NC}"
    make -j$(nproc) pinetime-app
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Build successful!${NC}"
        echo "Binary files are in: $(pwd)/src/"
        echo "Main binary: $(pwd)/src/pinetime-app.bin"
    else
        echo -e "${RED}Build failed!${NC}"
        exit 1
    fi
else
    echo -e "${RED}CMake configuration failed!${NC}"
    exit 1
fi