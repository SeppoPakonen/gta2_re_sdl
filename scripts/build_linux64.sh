#!/usr/bin/env bash
# Build the 64-bit (modern/Linux) GTA2 port.
# Run from the repository root:  ./scripts/build_linux64.sh
set -euo pipefail

# Resolve repo root from this script's location so it works from anywhere.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$REPO_ROOT"

BUILD_DIR="${BUILD_DIR:-build_debug}"
BUILD_TYPE="${BUILD_TYPE:-Debug}"
CMAKE="${CMAKE:-cmake}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"

# English tool output (the host locale is Finnish) and keep all temp files inside
# the repo's gitignored ./tmp (the system /tmp is a small, often-full tmpfs).
export LC_MESSAGES=C
export LANG=C
TMP_DIR="$REPO_ROOT/tmp"
mkdir -p "$TMP_DIR"
export TMPDIR="$TMP_DIR"

# Configure if not already configured.
if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
    echo "[build_linux64] Configuring $BUILD_DIR ($BUILD_TYPE)..."
    "$CMAKE" -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
fi

echo "[build_linux64] Building $BUILD_DIR with $JOBS jobs..."
"$CMAKE" --build "$BUILD_DIR" -j "$JOBS"

echo "[build_linux64] Done: $BUILD_DIR/gta2"