@echo off
REM Windows build script for V8CppIntegration
REM Requires Visual Studio 2022 and vcpkg to be installed

echo ========================================
echo V8 C++ Integration - Windows Build
echo ========================================
echo.

REM Check if vcpkg is available
where vcpkg >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: vcpkg not found in PATH
    echo Please install vcpkg and add it to your PATH
    echo See: https://github.com/Microsoft/vcpkg
    exit /b 1
)

REM Set vcpkg toolchain file (adjust path as needed)
set VCPKG_ROOT=C:\vcpkg
if not exist "%VCPKG_ROOT%" (
    echo Error: vcpkg not found at %VCPKG_ROOT%
    echo Please adjust VCPKG_ROOT variable in this script
    exit /b 1
)

echo Installing dependencies with vcpkg...
vcpkg install boost-program-options:x64-windows
vcpkg install gtest:x64-windows

echo.
echo Configuring CMake...
cmake -B build ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
    -DVCPKG_TARGET_TRIPLET=x64-windows ^
    -DUSE_SYSTEM_V8=OFF ^
    -DENABLE_TESTING=ON ^
    -G "Visual Studio 17 2022" ^
    -A x64

if %ERRORLEVEL% NEQ 0 (
    echo Error: CMake configuration failed
    exit /b 1
)

echo.
echo Building project...
cmake --build build --config Release --parallel

if %ERRORLEVEL% NEQ 0 (
    echo Error: Build failed
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo To run examples:
echo   build\Release\SystemV8Example.exe
echo   build\Release\BidirectionalExample.exe
echo   build\Release\AdvancedExample.exe
echo.
echo To run V8 console:
echo   Bin\v8c.exe
echo.
echo To run tests:
echo   cd build
echo   ctest --output-on-failure --parallel -C Release
echo.