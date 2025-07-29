@echo off
REM V8CppShell Console Build Script for Windows 11
REM Builds only the console application, requires V8 to be built first

setlocal enabledelayedexpansion

REM Colors for output (Windows)
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "CYAN=[96m"
set "BOLD=[1m"
set "NC=[0m"

REM Print functions
:print_status
echo %BLUE%[INFO]%NC% %~1
goto :eof

:print_success
echo %GREEN%[SUCCESS]%NC% %~1
goto :eof

:print_warning
echo %YELLOW%[WARNING]%NC% %~1
goto :eof

:print_error
echo %RED%[ERROR]%NC% %~1
goto :eof

:print_header
echo %CYAN%╔══════════════════════════════════════════════════════════════════════════════╗%NC%
echo %CYAN%║%NC%                                                                              %CYAN%║%NC%
echo %CYAN%║%NC%  %GREEN%██╗   ██╗ █████╗ %NC%     %BLUE%██████╗ ██████╗ ███╗   ██╗███████╗ ██████╗ ██╗     ███████╗%CYAN%║%NC%
echo %CYAN%║%NC%  %GREEN%██║   ██║██╔══██╗%NC%    %BLUE%██╔════╝██╔═══██╗████╗  ██║██╔════╝██╔═══██╗██║     ██╔════╝%CYAN%║%NC%
echo %CYAN%║%NC%  %GREEN%██║   ██║╚█████╔╝%NC%    %BLUE%██║     ██║   ██║██╔██╗ ██║███████╗██║   ██║██║     █████╗  %CYAN%║%NC%
echo %CYAN%║%NC%  %GREEN%╚██╗ ██╔╝██╔══██╗%NC%    %BLUE%██║     ██║   ██║██║╚██╗██║╚════██║██║   ██║██║     ██╔══╝  %CYAN%║%NC%
echo %CYAN%║%NC%   %GREEN%╚████╔╝ ╚█████╔╝%NC%    %BLUE%╚██████╗╚██████╔╝██║ ╚████║███████║╚██████╔╝███████╗███████╗%CYAN%║%NC%
echo %CYAN%║%NC%    %GREEN%╚═══╝   ╚════╝%NC%      %BLUE%╚═════╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝ ╚═════╝ ╚══════╝╚══════╝%CYAN%║%NC%
echo %CYAN%║%NC%                                                                              %CYAN%║%NC%
echo %CYAN%║%NC%                        %BOLD%V8 Console Application Build%NC%                      %CYAN%║%NC%
echo %CYAN%║%NC%                                                                              %CYAN%║%NC%
echo %CYAN%╚══════════════════════════════════════════════════════════════════════════════╝%NC%
echo.
goto :eof

REM Get V8 library paths for Windows
:get_v8_paths
set "V8_LIB=v8\out\x64.release\obj\v8.lib"
set "V8_PLATFORM=v8\out\x64.release\obj\v8_libplatform.lib"
set "V8_BASE=v8\out\x64.release\obj\v8_libbase.lib"
goto :eof

REM Check if V8 is built
:check_v8_built
call :print_status "Checking if V8 is built..."

call :get_v8_paths

set "V8_READY=1"

if not exist "%V8_LIB%" (
    set "V8_READY=0"
)
if not exist "%V8_PLATFORM%" (
    set "V8_READY=0"
)
if not exist "%V8_BASE%" (
    set "V8_READY=0"
)

REM Check if files are not empty
if exist "%V8_LIB%" (
    for %%A in ("%V8_LIB%") do (
        if %%~zA equ 0 set "V8_READY=0"
    )
)

if "%V8_READY%"=="1" (
    call :print_success "V8 libraries found and ready"
    exit /b 0
) else (
    call :print_error "V8 libraries not found or incomplete"
    call :print_error "Expected libraries:"
    call :print_error "  - %V8_LIB%"
    call :print_error "  - %V8_PLATFORM%"
    call :print_error "  - %V8_BASE%"
    echo.
    call :print_error "Please run build-v8.bat first to build V8"
    exit /b 1
)

REM Build console application
:build_console
call :print_status "Building V8CppShell console application..."

mkdir build 2>nul

call :print_status "Configuring CMake build..."
cmake -B build -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DUSE_SYSTEM_V8=OFF ^
    -DENABLE_TESTING=OFF ^
    -DENABLE_EXAMPLES=OFF

REM Use fewer parallel jobs on Windows
set /a JOBS=%NUMBER_OF_PROCESSORS%
if %JOBS% gtr 8 set JOBS=8

call :print_status "Building console application with %JOBS% parallel jobs..."
cmake --build build --target v8c --config Release --parallel %JOBS%

REM Check if build was successful
set "CONSOLE_BIN=build\Source\App\Console\Release\v8c.exe"
set "TARGET_BIN=Bin\v8c.exe"

if exist "%CONSOLE_BIN%" (
    mkdir Bin 2>nul
    copy "%CONSOLE_BIN%" "%TARGET_BIN%" >nul
    
    call :print_success "V8 Console (v8c) built successfully!"
    call :print_status "Console absolute path:"
    for %%F in ("%TARGET_BIN%") do echo   - %%~fF
    call :print_status "Executable info:"
    dir "%TARGET_BIN%"
    
    exit /b 0
) else (
    call :print_error "Console build failed - executable not found"
    call :print_error "Expected: %CONSOLE_BIN%"
    exit /b 1
)

REM Test the console application
:test_console
call :print_status "Testing console application..."

set "CONSOLE_BIN=Bin\v8c.exe"

if exist "%CONSOLE_BIN%" (
    call :print_status "Running version check..."
    "%CONSOLE_BIN%" --help | head -5
    call :print_success "Console application is working!"
    exit /b 0
) else (
    call :print_error "Console executable not found: %CONSOLE_BIN%"
    exit /b 1
)

REM Show completion message
:show_completion
call :print_success "V8 Console build completed successfully!"
echo.
echo %CYAN%╔══════════════════════════════════════════════════════════════════════════════╗%NC%
echo %CYAN%║%NC%                               %BOLD%BUILD COMPLETE%NC%                               %CYAN%║%NC%
echo %CYAN%║%NC%                                                                              %CYAN%║%NC%
echo %CYAN%║%NC%  %GREEN%✓%NC% V8 Console application built successfully                               %CYAN%║%NC%
echo %CYAN%║%NC%                                                                              %CYAN%║%NC%
echo %CYAN%║%NC%  %YELLOW%Quick Start:%NC%                                                             %CYAN%║%NC%
echo %CYAN%║%NC%    %BLUE%.\Bin\v8c.exe%NC%               - Run interactive V8 console                %CYAN%║%NC%
echo %CYAN%║%NC%    %BLUE%.\Bin\v8c.exe --help%NC%        - Show help and options                     %CYAN%║%NC%
echo %CYAN%║%NC%    %BLUE%.\Bin\v8c.exe script.js%NC%     - Run JavaScript file                       %CYAN%║%NC%
echo %CYAN%║%NC%                                                                              %CYAN%║%NC%
echo %CYAN%║%NC%  %YELLOW%Configuration:%NC%                                                           %CYAN%║%NC%
echo %CYAN%║%NC%    %BLUE%.\Bin\v8c.exe --config%NC%      - Set up configuration files                %CYAN%║%NC%
echo %CYAN%║%NC%    %BLUE%.\Bin\v8c.exe --configure%NC%   - Run interactive setup wizard              %CYAN%║%NC%
echo %CYAN%║%NC%                                                                              %CYAN%║%NC%
echo %CYAN%╚══════════════════════════════════════════════════════════════════════════════╝%NC%
echo.
goto :eof

REM Main execution
:main
call :print_header
call :print_status "Starting V8 Console build process..."
call :print_status "Working directory: %CD%"
call :print_status "Detected platform: Windows 11 (x64)"

call :check_v8_built
if %errorlevel% neq 0 exit /b %errorlevel%

call :build_console
if %errorlevel% neq 0 (
    call :print_error "Console build failed"
    exit /b 1
)

call :test_console
call :show_completion

call :print_status "Build completed successfully!"
goto :eof

REM Show usage if help requested
if "%1"=="--help" goto :show_help
if "%1"=="-h" goto :show_help
goto :run_main

:show_help
echo V8CppShell Console Build Script
echo.
echo Usage: %0 [options]
echo.
echo This script builds only the V8 console application.
echo V8 must be built first using build-v8.bat
echo.
echo Options:
echo   --help, -h    Show this help message
echo.
echo The script will:
echo   1. Check that V8 libraries are built and available
echo   2. Configure CMake build for console only
echo   3. Build the v8c console application
echo   4. Copy executable to Bin\ directory
echo   5. Test the console application
echo.
echo Output: Bin\v8c.exe
echo Build time: 1-5 minutes
exit /b 0

:run_main
REM Run main function
call :main %*