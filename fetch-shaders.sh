#!/bin/bash
# Fetch glsl-shaders from libretro repository
# The shader files are needed for OpenGL-based upscaling shaders.
# They are not included in the source repo due to size.

set -euo pipefail
cd "$(dirname "$0")"

REPO="https://github.com/libretro/glsl-shaders.git"
DEST="glsl-shaders"

if [ -d "$DEST" ]; then
    echo "glsl-shaders directory already exists. To re-fetch, remove it first: rm -rf $DEST"
    exit 0
fi

echo "Fetching glsl-shaders from libretro repository..."
echo "This will download only the directories needed by zelda3."

# Shader directories used by presets and build scripts
DIRS=(
    "presets"
    "scalefx"
    "xbr"
    "xbrz"
    "sharpen"
    "sabr"
    "nnedi3"
    "scalehq"
    "shaders"
)

# Sparse checkout: only clone the directories we need
git clone --depth 1 --no-checkout "$REPO" "$DEST"
cd "$DEST"
git sparse-checkout init --cone
git sparse-checkout set "${DIRS[@]}"
git checkout
cd ..

echo ""
echo "Shaders fetched to $DEST/"
echo "You can now select shaders in Settings > Shader."
