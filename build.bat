@echo off
setlocal EnableDelayedExpansion

:: ============================================================================
:: build.bat  –  Address Book build script for Windows
::
:: Usage:
::   build.bat              Build application only
::   build.bat --run        Build and launch the application
::   build.bat --tests      Build application + run unit tests
::   build.bat --clean      Remove build artefacts
::
:: Requirements:
::   - Qt 6.x installed (Qt Online Installer: https://www.qt.io/download)
::   - Qt bin directory on PATH  OR  set QT_DIR below
::   - MSVC 2019/2022  OR  MinGW-w64 (comes bundled with Qt)
:: ============================================================================

:: --------------------------------------------------------------------------
:: Optional: hard-code your Qt path here if qmake is not on PATH.
:: Examples:
::   set QT_DIR=C:\Qt\6.7.0\msvc2022_64\bin
::   set QT_DIR=C:\Qt\6.7.0\mingw_64\bin
:: --------------------------------------------------------------------------
set QT_DIR=

set ROOT_DIR=%~dp0
set BUILD_DIR=%ROOT_DIR%build
set TESTS_BUILD_DIR=%ROOT_DIR%tests\build

set RUN_APP=0
set BUILD_TESTS=0
set CLEAN=0

:: Parse arguments
:parse_args
if "%~1"=="" goto end_parse
if /I "%~1"=="--run"    set RUN_APP=1
if /I "%~1"=="--tests"  set BUILD_TESTS=1
if /I "%~1"=="--clean"  set CLEAN=1
shift
goto parse_args
:end_parse

:: Add Qt to PATH if QT_DIR is set
if not "%QT_DIR%"=="" set PATH=%QT_DIR%;%PATH%

:: ---- Locate qmake ----------------------------------------------------------
where qmake >nul 2>&1
if errorlevel 1 (
    echo.
    echo ERROR: qmake not found on PATH.
    echo.
    echo Please do ONE of the following:
    echo   1. Edit build.bat and set QT_DIR to your Qt bin folder, e.g.:
    echo        set QT_DIR=C:\Qt\6.7.0\msvc2022_64\bin
    echo   2. Add Qt bin to your system PATH manually.
    echo   3. Open the Qt 6 command prompt and run build.bat from there.
    echo.
    echo Qt can be downloaded free from: https://www.qt.io/download-qt-installer
    exit /b 1
)

for /f "tokens=*" %%v in ('qmake --version 2^>^&1 ^| findstr /i "QMake"') do (
    echo Using: %%v
)

:: ---- Clean -----------------------------------------------------------------
if %CLEAN%==1 (
    echo Cleaning build directories...
    if exist "%BUILD_DIR%"       rmdir /s /q "%BUILD_DIR%"
    if exist "%TESTS_BUILD_DIR%" rmdir /s /q "%TESTS_BUILD_DIR%"
    echo Done.
    exit /b 0
)

:: ---- Detect make tool (nmake for MSVC, mingw32-make for MinGW) -------------
set MAKE_CMD=
where nmake >nul 2>&1 && set MAKE_CMD=nmake
if "%MAKE_CMD%"=="" (
    where mingw32-make >nul 2>&1 && set MAKE_CMD=mingw32-make
)
if "%MAKE_CMD%"=="" (
    where make >nul 2>&1 && set MAKE_CMD=make
)
if "%MAKE_CMD%"=="" (
    echo ERROR: No make tool found. Install MSVC Build Tools or MinGW.
    exit /b 1
)
echo Make tool: %MAKE_CMD%

:: ---- Build application -----------------------------------------------------
echo.
echo ==^> Building Address Book application...
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
pushd "%BUILD_DIR%"
qmake "%ROOT_DIR%addressbook.pro" CONFIG+=release
if errorlevel 1 ( popd & echo ERROR: qmake failed. & exit /b 1 )
%MAKE_CMD%
if errorlevel 1 ( popd & echo ERROR: build failed. & exit /b 1 )
popd

echo ==^> Application built: %BUILD_DIR%\release\addressbook.exe

:: ---- Build and run tests ---------------------------------------------------
if %BUILD_TESTS%==1 (
    echo.
    echo ==^> Building unit tests...
    if not exist "%TESTS_BUILD_DIR%" mkdir "%TESTS_BUILD_DIR%"
    pushd "%TESTS_BUILD_DIR%"
    qmake "%ROOT_DIR%tests\tests.pro"
    if errorlevel 1 ( popd & echo ERROR: qmake for tests failed. & exit /b 1 )
    %MAKE_CMD%
    if errorlevel 1 ( popd & echo ERROR: test build failed. & exit /b 1 )
    echo.
    echo ==^> Running unit tests...
    release\addressbook_tests.exe -v2
    popd
)

:: ---- Launch ----------------------------------------------------------------
if %RUN_APP%==1 (
    echo.
    echo ==^> Launching Address Book...
    start "" "%BUILD_DIR%\release\addressbook.exe"
)

echo.
echo Done.
endlocal
