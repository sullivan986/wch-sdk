#!/bin/sh -e

mkdir riscv-gcc
cd riscv-gcc/
wget http://file.mounriver.com/tools/MRS_Toolchain_Linux_x64_V1.91.tar.xz
tar -xvf  MRS_Toolchain_Linux_x64_V1.91.tar.xz
rm -f MRS_Toolchain_Linux_x64_V1.91.tar.xz
cd ..