#!/usr/bin/env bash
set -e

BUILD_DIR="${1:-build_wasm}"
echo "Building GTA2 WebAssembly target in ${BUILD_DIR}..."

if ! command -v emcmake &> /dev/null; then
    echo "Error: emcmake (Emscripten) not found in PATH."
    echo "Please install or activate emsdk: source /path/to/emsdk/emsdk_env.sh"
    exit 1
fi

emcmake cmake -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DGTA2_BACKEND=SDL2

cmake --build "${BUILD_DIR}" --target gta2 -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "Build complete. Output files:"
ls -lh "${BUILD_DIR}/gta2.html" "${BUILD_DIR}/gta2.js" "${BUILD_DIR}/gta2.wasm" 2>/dev/null || true
echo "To run locally:"
echo "  python3 -m http.server 8080 --directory ${BUILD_DIR}"
