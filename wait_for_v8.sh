#!/bin/bash
cd /home/christian/local/repos/V8CppIntegration/v8

echo "Waiting for V8 monolith build to complete..."
ninja -C out/x64.release v8_monolith

if [ $? -eq 0 ]; then
    echo "V8 monolith build completed successfully!"
    ls -la out/x64.release/obj/libv8_monolith.a
else
    echo "V8 monolith build failed!"
    exit 1
fi