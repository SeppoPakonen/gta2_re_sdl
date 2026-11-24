#!/usr/bin/env bash
set -euo pipefail

# Package native Linux build of GTA2 into a redistributable archive.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${1:-${ROOT_DIR}/build_linux}"
OUTPUT_DIR="${2:-${ROOT_DIR}/dist}"

echo "==> Packaging GTA2 Linux Desktop Release..."
echo "    Build directory:  ${BUILD_DIR}"
echo "    Output directory: ${OUTPUT_DIR}"

if [[ ! -f "${BUILD_DIR}/gta2" ]]; then
    echo "Error: '${BUILD_DIR}/gta2' not found. Please build the target first:"
    echo "  cmake -B ${BUILD_DIR} -DCMAKE_BUILD_TYPE=Release -DGTA2_BACKEND=SDL2"
    echo "  cmake --build ${BUILD_DIR} --target gta2 -j\$(nproc)"
    exit 1
fi

ARCH="$(uname -m)"
PACKAGE_NAME="gta2-linux-${ARCH}"
STAGING_DIR="${OUTPUT_DIR}/${PACKAGE_NAME}"

rm -rf "${STAGING_DIR}"
mkdir -p "${STAGING_DIR}/bin"
mkdir -p "${STAGING_DIR}/share/applications"
mkdir -p "${STAGING_DIR}/share/gta2"

# Copy binary
cp "${BUILD_DIR}/gta2" "${STAGING_DIR}/bin/gta2"
chmod +x "${STAGING_DIR}/bin/gta2"

# Copy desktop integration
if [[ -f "${ROOT_DIR}/packaging/linux/gta2.desktop" ]]; then
    cp "${ROOT_DIR}/packaging/linux/gta2.desktop" "${STAGING_DIR}/share/applications/gta2.desktop"
fi

# Create readme for assets
cat > "${STAGING_DIR}/share/gta2/README.txt" << 'EOF'
Grand Theft Auto 2 - Modern Port for Linux
==========================================

Place original GTA2 data files (data/, audio/, etc.) in the working directory
or alongside the executable to play the game.
EOF

# Create tarball
mkdir -p "${OUTPUT_DIR}"
TARBALL="${OUTPUT_DIR}/${PACKAGE_NAME}.tar.gz"
tar -czf "${TARBALL}" -C "${OUTPUT_DIR}" "${PACKAGE_NAME}"

echo "==> Successfully packaged: ${TARBALL}"
ls -lh "${TARBALL}"
