#!/bin/bash
# Simple script to compile a V8 C++ application after build_from_source.sh

echo "Compiling standalone V8 example..."

# Check if V8 is built
if [ ! -f "v8/out/x64.release/obj/libv8_monolith.a" ]; then
    echo "Error: V8 not built yet. Please run ./from_source.sh first"
    exit 1
fi

# Since V8 is built with Chromium's libc++, we need to use it too
# This ensures ABI compatibility
clang++ -std=c++20 \
    -stdlib=libc++ \
    -I./v8/include \
    standalone_example.cpp \
    -o standalone_example \
    -fuse-ld=lld \
    ./v8/out/x64.release/obj/libv8_monolith.a \
    ./v8/out/x64.release/obj/libv8_libbase.a \
    ./v8/out/x64.release/obj/libv8_libplatform.a \
    ./v8/out/x64.release/obj/buildtools/third_party/libc++/libc++.a \
    ./v8/out/x64.release/obj/buildtools/third_party/libc++abi/libc++abi.a \
    -pthread -ldl -lm

if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    echo "Run with: ./standalone_example"
    echo ""
    echo "Note: This uses V8's bundled libc++ for ABI compatibility"
else
    echo "Compilation failed!"
    echo ""
    echo "Make sure you have:"
    echo "1. clang++ installed"
    echo "2. lld installed (LLVM linker)"
    echo "3. V8 built with ./from_source.sh"
fi