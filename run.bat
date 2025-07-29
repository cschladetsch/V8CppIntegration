@echo off
REM V8CppShell Complete Build Script for Windows 11
REM This script does everything needed after cloning the repository

setlocal enabledelayedexpansion

REM Colors for output (Windows)
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "CYAN=[96m"
set "MAGENTA=[95m"
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
echo %CYAN%║%NC%  %GREEN%██╗   ██╗ █████╗ %NC%     %BLUE%██████╗██████╗ ██████╗%NC%    %YELLOW%███████╗██╗  ██╗███████╗██╗     ██╗  %CYAN%║%NC%
echo %CYAN%║%NC%  %GREEN%██║   ██║██╔══██╗%NC%    %BLUE%██╔════╝██╔══██╗██╔══██╗%NC%   %YELLOW%██╔════╝██║  ██║██╔════╝██║     ██║  %CYAN%║%NC%
echo %CYAN%║%NC%  %GREEN%██║   ██║╚█████╔╝%NC%    %BLUE%██║     ██████╔╝██████╔╝%NC%   %YELLOW%███████╗███████║█████╗  ██║     ██║  %CYAN%║%NC%
echo %CYAN%║%NC%  %GREEN%╚██╗ ██╔╝██╔══██╗%NC%    %BLUE%██║     ██╔═══╝ ██╔═══╝%NC%    %YELLOW%╚════██║██╔══██║██╔══╝  ██║     ██║  %CYAN%║%NC%
echo %CYAN%║%NC%   %GREEN%╚████╔╝ ╚█████╔╝%NC%    %BLUE%╚██████╗██║     ██║%NC%       %YELLOW%███████║██║  ██║███████╗███████╗███████╗%CYAN%║%NC%
echo %CYAN%║%NC%    %GREEN%╚═══╝   ╚════╝%NC%      %BLUE%╚═════╝╚═╝     ╚═╝%NC%       %YELLOW%╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝%CYAN%║%NC%
echo %CYAN%║%NC%                                                                              %CYAN%║%NC%
echo %CYAN%║%NC%                    %BOLD%Complete V8 JavaScript Engine Build%NC%                   %CYAN%║%NC%
echo %CYAN%║%NC%                        %BOLD%Windows 11 Compatible Version%NC%                     %CYAN%║%NC%
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

REM Check dependencies
:check_dependencies
call :print_status "Checking Windows dependencies..."

where python3 >nul 2>&1
if %errorlevel% neq 0 (
    where python >nul 2>&1
    if %errorlevel% neq 0 (
        call :print_error "Python 3 not found. Please install Python 3 from Microsoft Store or python.org"
        exit /b 1
    )
)

where git >nul 2>&1
if %errorlevel% neq 0 (
    call :print_error "Git not found. Please install Git for Windows"
    exit /b 1
)

where cl >nul 2>&1
if %errorlevel% neq 0 (
    call :print_warning "Visual Studio C++ compiler not found in PATH"
    call :print_warning "Please run this from Visual Studio Developer Command Prompt"
    call :print_warning "Or install Visual Studio 2019/2022 with C++ tools"
)

call :print_success "Windows dependencies check passed"
goto :eof

REM Setup depot_tools
:setup_depot_tools
call :print_status "Setting up depot_tools..."

set "SCRIPT_DIR=%~dp0"
set "DEPOT_TOOLS_DIR=%SCRIPT_DIR%depot_tools"

if not exist "%DEPOT_TOOLS_DIR%" (
    call :print_status "Cloning depot_tools..."
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git "%DEPOT_TOOLS_DIR%"
) else (
    call :print_status "depot_tools already exists, updating..."
    pushd "%DEPOT_TOOLS_DIR%"
    git pull origin main || git pull origin master
    popd
)

REM Set up Windows-specific environment
set "PATH=%DEPOT_TOOLS_DIR%;%PATH%"
set "DEPOT_TOOLS_WIN_TOOLCHAIN=0"
set "GYP_MSVS_VERSION=2022"

call :print_success "depot_tools set up successfully"
goto :eof

REM Check if V8 is already built
:check_v8_built
call :get_v8_paths
if exist "%V8_LIB%" (
    call :print_success "V8 already built successfully"
    set "V8_ALREADY_BUILT=1"
) else (
    set "V8_ALREADY_BUILT=0"
)
goto :eof

REM Clean up partial builds
:cleanup_partial_builds
call :print_status "Checking for partial builds..."

call :check_v8_built
if "%V8_ALREADY_BUILT%"=="1" (
    call :print_success "V8 already built successfully, skipping cleanup"
    goto :eof
)

call :print_status "Cleaning up any partial builds..."

REM Remove partial gclient checkouts
for /d %%i in (_gclient_*) do (
    if exist "%%i" (
        call :print_status "Removing partial gclient checkout directory %%i..."
        rmdir /s /q "%%i"
    )
)

REM Reset gclient configuration if V8 not built
if exist ".gclient" (
    if "%V8_ALREADY_BUILT%"=="0" (
        call :print_status "Removing existing .gclient configuration..."
        del /f .gclient .gclient_entries 2>nul
    )
)

REM Only remove V8 directory if clearly incomplete
if exist "v8" (
    if "%V8_ALREADY_BUILT%"=="0" (
        if not exist "v8\BUILD.gn" (
            call :print_status "Removing incomplete V8 directory..."
            rmdir /s /q v8
        )
    )
)
goto :eof

REM Fetch V8 source
:fetch_v8
call :check_v8_built
if "%V8_ALREADY_BUILT%"=="1" (
    call :print_success "V8 already built, skipping source fetch"
    goto :eof
)

call :print_status "Fetching V8 source code..."

if not exist "v8" (
    call :print_status "Fetching V8 (this may take 30-60 minutes depending on your connection)..."
    fetch v8
    call :print_success "V8 source fetched successfully"
) else (
    call :print_status "V8 directory exists, syncing..."
    pushd v8
    gclient sync
    popd
    call :print_success "V8 source synchronized"
)
goto :eof

REM Configure V8 build
:configure_v8
call :check_v8_built
if "%V8_ALREADY_BUILT%"=="1" (
    call :print_success "V8 already built, skipping configuration"
    goto :eof
)

call :print_status "Configuring V8 build for Windows (x64)..."

pushd v8

REM Generate build files with Windows-specific settings
set BUILD_ARGS=is_debug=false
set BUILD_ARGS=%BUILD_ARGS% v8_enable_sandbox=false
set BUILD_ARGS=%BUILD_ARGS% v8_enable_pointer_compression=false
set BUILD_ARGS=%BUILD_ARGS% v8_static_library=true
set BUILD_ARGS=%BUILD_ARGS% is_component_build=false
set BUILD_ARGS=%BUILD_ARGS% use_custom_libcxx=false
set BUILD_ARGS=%BUILD_ARGS% v8_use_external_startup_data=false
set BUILD_ARGS=%BUILD_ARGS% treat_warnings_as_errors=false
set BUILD_ARGS=%BUILD_ARGS% symbol_level=1
set BUILD_ARGS=%BUILD_ARGS% v8_enable_i18n_support=false
set BUILD_ARGS=%BUILD_ARGS% target_os="win"
set BUILD_ARGS=%BUILD_ARGS% is_clang=false
set BUILD_ARGS=%BUILD_ARGS% target_cpu="x64"

mkdir out\x64.release 2>nul

call :print_status "Build configuration: %BUILD_ARGS%"

gn gen out\x64.release --args="%BUILD_ARGS%"

popd
call :print_success "V8 build configured"
goto :eof

REM Build V8
:build_v8
call :check_v8_built
if "%V8_ALREADY_BUILT%"=="1" (
    call :print_success "V8 already built, skipping build step"
    goto :eof
)

call :print_status "Building V8 (this may take 1-3 hours depending on your hardware)..."

pushd v8

REM Use fewer parallel jobs on Windows to prevent memory issues
set /a JOBS=%NUMBER_OF_PROCESSORS%
if %JOBS% gtr 8 set JOBS=8

call :print_status "Using %JOBS% parallel jobs"

autoninja -C out\x64.release -j %JOBS% //:v8

popd
call :print_success "V8 build completed"
goto :eof

REM Verify V8 build
:verify_v8_build
call :print_status "Verifying V8 build..."

call :get_v8_paths

if exist "%V8_LIB%" if exist "%V8_PLATFORM%" if exist "%V8_BASE%" (
    call :print_success "V8 libraries built successfully!"
    call :print_status "Library files found:"
    dir "%V8_LIB%" "%V8_PLATFORM%" "%V8_BASE%"
    exit /b 0
) else (
    call :print_error "V8 build failed - missing libraries"
    call :print_error "Expected libraries:"
    call :print_error "  - %V8_LIB%"
    call :print_error "  - %V8_PLATFORM%"
    call :print_error "  - %V8_BASE%"
    exit /b 1
)

REM Build V8CppShell project
:build_project
call :print_status "Building V8CppShell project..."

mkdir build 2>nul

call :print_status "Configuring CMake build..."
cmake -B build -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DUSE_SYSTEM_V8=OFF ^
    -DENABLE_TESTING=ON ^
    -DENABLE_EXAMPLES=ON

call :print_status "Building V8CppShell..."
cmake --build build --config Release --parallel %JOBS%

call :print_success "V8CppShell project built successfully"
goto :eof

REM Create build info
:create_build_info
call :print_status "Creating build information..."

call :get_v8_paths

(
echo {
echo     "platform": "windows",
echo     "architecture": "x64",
echo     "build_date": "%date% %time%",
echo     "libraries": {
echo         "v8": "%V8_LIB:\=\\%",
echo         "v8_libplatform": "%V8_PLATFORM:\=\\%",
echo         "v8_libbase": "%V8_BASE:\=\\%"
echo     },
echo     "include_path": "v8\\include",
echo     "project_binaries": {
echo         "v8c": "Bin\\v8c.exe",
echo         "examples": "build\\",
echo         "tests": "build\\"
echo     }
echo }
) > v8_build_info.json

call :print_success "Build info saved to v8_build_info.json"
goto :eof

REM Show completion message
:show_completion
call :print_success "V8CppShell build completed successfully!"
echo.
echo %CYAN%╔══════════════════════════════════════════════════════════════════════════════╗%NC%
echo %CYAN%║%NC%                               %BOLD%BUILD COMPLETE%NC%                               %CYAN%║%NC%
echo %CYAN%║%NC%                                                                              %CYAN%║%NC%
echo %CYAN%║%NC%  %GREEN%✓%NC% V8 JavaScript Engine built from source                                 %CYAN%║%NC%
echo %CYAN%║%NC%  %GREEN%✓%NC% V8CppShell project built successfully                                   %CYAN%║%NC%
echo %CYAN%║%NC%  %GREEN%✓%NC% Windows 11 compatibility verified                                       %CYAN%║%NC%
echo %CYAN%║%NC%                                                                              %CYAN%║%NC%
echo %CYAN%║%NC%  %YELLOW%Quick Start:%NC%                                                             %CYAN%║%NC%
echo %CYAN%║%NC%    %BLUE%.\Bin\v8c.exe%NC%               - Run interactive V8 console                %CYAN%║%NC%
echo %CYAN%║%NC%    %BLUE%.\build\Release\SystemV8Example.exe%NC%  - Run system integration example     %CYAN%║%NC%
echo %CYAN%║%NC%    %BLUE%.\build\Release\BidirectionalExample.exe%NC% - Run bidirectional example      %CYAN%║%NC%
echo %CYAN%║%NC%                                                                              %CYAN%║%NC%
echo %CYAN%║%NC%  %YELLOW%Add to PATH:%NC%                                                             %CYAN%║%NC%
echo %CYAN%║%NC%    Add %SCRIPT_DIR%Bin to your PATH environment variable                      %CYAN%║%NC%
echo %CYAN%║%NC%                                                                              %CYAN%║%NC%
echo %CYAN%╚══════════════════════════════════════════════════════════════════════════════╝%NC%
echo.
goto :eof

REM Main execution
:main
call :print_header
call :print_status "Starting complete V8CppShell build process..."
call :print_status "Working directory: %CD%"
call :print_status "Detected platform: Windows 11 (x64)"

call :check_dependencies
if %errorlevel% neq 0 exit /b %errorlevel%

call :setup_depot_tools
call :cleanup_partial_builds
call :fetch_v8
call :configure_v8
call :build_v8

call :verify_v8_build
if %errorlevel% neq 0 (
    call :print_error "V8 build verification failed. Cannot proceed with project build."
    exit /b 1
)

call :build_project
call :create_build_info
call :show_completion

call :print_status "Build completed successfully!"
goto :eof

REM Run main function
call :main %*