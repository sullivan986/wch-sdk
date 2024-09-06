#!/bin/sh -e
mkdir riscv-gcc
cd riscv-gcc/
wget https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases/download/v14.2.0-2/xpack-riscv-none-elf-gcc-14.2.0-2-linux-x64.tar.gz
tar -xvf xpack-riscv-none-elf-gcc-14.2.0-2-linux-x64.tar.gz
rm xpack-riscv-none-elf-gcc-14.2.0-2-linux-x64.tar.gz
mv xpack-riscv-none-elf-gcc-14.2.0-2/ xpack-riscv-none-elf-gcc
cd ..

# mkdir -p riscv-llvm
# cd riscv-llvm
# git clone --depth=1 https://github.com/llvm/llvm-project.git
# cd llvm-project
# mkdir -p build
# mkdir -p build-install
# cd build

# cmake -G "Unix Makefiles" \
# -DCMAKE_BUILD_TYPE=Release \
# -DLLVM_TARGETS_TO_BUILD="RISCV" \
# -DDEFAULT_SYSROOT=../../../riscv-gcc/xpack-riscv-none-elf-gcc/riscv-none-elf \
# -DCMAKE_INSTALL_PREFIX=../build-install \
# -DLLVM_DEFAULT_TARGET_TRIPLE="riscv32-unknown-elf" \
# -DLLVM_ENABLE_LLD=False \
# -DLLVM_ENABLE_PROJECTS="clang;lld" \
# ../llvm

# make -j6
# make install