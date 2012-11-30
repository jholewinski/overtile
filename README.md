

OverTile - GPU Stencil Compiler for Overlapped Tiling


Building
========

Dependencies

  * CMake 2.8+
  * Bison


By default, OverTile is configured to build against an internal version of
LLVM maintained as a git submodule that tracks known-good, up-to-date versions
of LLVM. Configuring and building OverTile is as easy as:

    $ git submodule init
    $ git submodule update
    $ mkdir build && cd build
    $ cmake -DUSE_INTERNAL_LLVM=YES -DCMAKE_BUILD_TYPE=Debug -DLLVM_TARGETS_TO_BUILD=X86 ..
    $ make

The built compiler library will be in bin/otsc of your build directory.

If you would like to use your own installation of LLVM, please configure with
-DUSE_INTERNAL_LLVM=NO and set OT_LLVM_BINARY_DIR to an installation of LLVM.
Note that this installation must have been built with CMake.



Running
=======

For a C/C++ file with embedded SSP, the compiler can be invoked with:

    $ bin/otsc -c my-file.cpp -o my-file.out.cu

and then compiled with nvcc:

    $ nvcc -O3 my-file.out.cu -o my-file

