#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# build.sh – one-shot build script for the Address Book application.
#
# Usage:
#   ./build.sh             # build app only
#   ./build.sh --tests     # build app + tests
#   ./build.sh --run       # build app and run it
#   ./build.sh --clean     # remove build artefacts
# ---------------------------------------------------------------------------
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
TESTS_BUILD_DIR="$ROOT_DIR/tests/build"

RUN_APP=false
BUILD_TESTS=false
CLEAN=false

for arg in "$@"; do
    case "$arg" in
        --run)    RUN_APP=true ;;
        --tests)  BUILD_TESTS=true ;;
        --clean)  CLEAN=true ;;
        *)
            echo "Unknown option: $arg"
            echo "Usage: $0 [--run] [--tests] [--clean]"
            exit 1
            ;;
    esac
done

# ---- Detect qmake ----------------------------------------------------------
if command -v qmake6 &>/dev/null; then
    QMAKE=qmake6
elif command -v qmake &>/dev/null; then
    QMAKE=qmake
else
    echo "ERROR: qmake not found. Install Qt development packages:"
    echo "  sudo apt-get install qt6-base-dev qt6-base-dev-tools libqt6sql6-sqlite"
    exit 1
fi

echo "Using: $($QMAKE --version | head -1)"

# ---- Clean -----------------------------------------------------------------
if $CLEAN; then
    echo "Cleaning build directories…"
    rm -rf "$BUILD_DIR" "$TESTS_BUILD_DIR"
    echo "Done."
    exit 0
fi

# ---- Build application -----------------------------------------------------
echo ""
echo "==> Building Address Book application…"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
$QMAKE "$ROOT_DIR/addressbook.pro" CONFIG+=release
make -j"$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 2)"
echo "==> Application built: $BUILD_DIR/addressbook"

# ---- Build & run tests -----------------------------------------------------
if $BUILD_TESTS; then
    echo ""
    echo "==> Building unit tests…"
    mkdir -p "$TESTS_BUILD_DIR"
    cd "$TESTS_BUILD_DIR"
    $QMAKE "$ROOT_DIR/tests/tests.pro"
    make -j"$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 2)"
    echo ""
    echo "==> Running unit tests…"
    ./addressbook_tests -v2
fi

# ---- Launch ----------------------------------------------------------------
if $RUN_APP; then
    echo ""
    echo "==> Launching Address Book…"
    "$BUILD_DIR/addressbook"
fi

echo ""
echo "Done."
