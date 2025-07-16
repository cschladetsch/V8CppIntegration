# V8 Build Notes

## Important: Clang and libc++ Required

When building V8 from source, you MUST use Clang with libc++ due to V8's build system requirements. The build scripts now handle this automatically, but you need these packages installed:

```bash
sudo apt-get install clang libc++-dev libc++abi-dev
```

## Building V8 from Source

When building V8 from source, there's an ABI compatibility issue:
- V8 builds with Clang and Chromium's custom libc++ by default
- Your project might use GCC with the system's libstdc++
- This causes undefined reference errors when linking

## Solutions

### Option 1: Use Clang for Your Project (Recommended)
When building with locally-built V8:
```bash
# Configure with clang
CC=clang CXX=clang++ cmake .. -DUSE_SYSTEM_V8=OFF

# Build
make
```

You'll also need libc++ development packages:
```bash
sudo apt-get install libc++-dev libc++abi-dev
```

### Option 2: Use System V8
If available, use your system's V8 package:
```bash
cmake .. -DUSE_SYSTEM_V8=ON
make
```

### Option 3: Build V8 with GCC (Complex)
Modify the V8 build to use GCC - this requires additional dependencies and is not recommended.

## Quick Start

1. Build V8:
```bash
./build.sh --setup-v8
./build.sh --build-v8
```

2. Build examples with Clang:
```bash
cd build
CC=clang CXX=clang++ cmake .. -DUSE_SYSTEM_V8=OFF
make
```

## Troubleshooting

If you see "undefined reference to v8::platform::NewDefaultPlatform", it's an ABI mismatch. Use clang as shown above.