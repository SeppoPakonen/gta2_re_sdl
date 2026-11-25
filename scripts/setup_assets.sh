#!/usr/bin/env bash
set -euo pipefail

GTA2_DATA_DIR="${GTA2_DATA_DIR:-$HOME/.wine/drive_c/Program Files (x86)/Rockstar Games/GTA2}"
GAME_DIR="${GTA2_DATA_DIR%/}"

mkdir -p assets
ln -sfn "$GAME_DIR/data" assets/data
ln -sfn "$GAME_DIR/player" assets/player

echo "Linked GTA2 assets from: $GAME_DIR"
