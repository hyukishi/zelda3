#!/bin/bash
# Build Zelda3 Linux AppImage with HD upscaling shaders and ZSPR sprite packs
#
# Prerequisites:
#   - Docker (for building the Linux binary) OR a pre-built Linux binary at build-linux/zelda3
#   - The ROM file named zelda3.sfc for asset extraction
#
# Usage: ./build-appimage.sh

set -euo pipefail
cd "$(dirname "$0")"

APP_DIR="AppDir"
OUTPUT_NAME="zelda3-linux-x86_64.AppImage"

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; NC='\033[0m'
info()  { echo -e "${GREEN}[INFO]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Check if the Linux binary exists, build if needed
if [ ! -f "build-linux/zelda3" ]; then
    info "No cached Linux binary found."
    if command -v docker &>/dev/null; then
        info "Building via Docker..."
        make clean_obj 2>/dev/null || true
        docker build -f Dockerfile.build-linux -t zelda3-builder . 2>&1
        CID=$(docker create zelda3-builder)
        mkdir -p build-linux
        docker cp "$CID:/src/zelda3" ./build-linux/
        docker rm "$CID" >/dev/null
    else
        error "Docker not available. Pre-build a Linux binary and place it at build-linux/zelda3"
        exit 1
    fi
fi

# Also ensure assets are extracted
if [ ! -f "zelda3_assets.dat" ] && [ ! -f "tables/zelda3_assets.dat" ]; then
    if [ -f "zelda3.sfc" ]; then
        info "Extracting assets..."
        source venv/bin/activate 2>/dev/null || true
        python3 assets/restool.py --extract-from-rom
    else
        error "zelda3.sfc not found - can't extract assets"
        exit 1
    fi
fi

# 1. Set up AppDir
info "Setting up AppDir..."
rm -rf "${APP_DIR}/zelda3" "${APP_DIR}/zelda3_assets.dat" "${APP_DIR}/glsl-shaders" "${APP_DIR}/sprites-gfx" 2>/dev/null || true

# Binary
cp build-linux/zelda3 "${APP_DIR}/zelda3"
chmod 755 "${APP_DIR}/zelda3"

# Assets
cp zelda3_assets.dat "${APP_DIR}/"
cp AppDir/zelda3.ini "${APP_DIR}/zelda3.ini"

# 2. Bundle upscaling shaders (only the best ones to keep size down)
info "Bundling HD upscaling shaders..."
mkdir -p "${APP_DIR}/glsl-shaders"

# Copy the preset we use
mkdir -p "${APP_DIR}/glsl-shaders/presets"
cp glsl-shaders/presets/xsoft+scalefx-level2aa+sharpsmoother.glslp "${APP_DIR}/glsl-shaders/presets/" 2>/dev/null || true
# Copy another great presets
for p in scalefx-aa scalefx-aa-fast xsoft+scalefx-level2aa xsoft+scalefx-hybrid+level2aa; do
    cp "glsl-shaders/presets/${p}.glslp" "${APP_DIR}/glsl-shaders/presets/" 2>/dev/null || true
done

# Copy just the best upscaling shader families
for dir in scalefx xbr sharpen sabr nnedi3; do
    [ -d "glsl-shaders/$dir" ] && cp -r "glsl-shaders/$dir" "${APP_DIR}/glsl-shaders/$dir"
done

# Shader common support files
cp -r glsl-shaders/shaders "${APP_DIR}/glsl-shaders/shaders" 2>/dev/null || true

# 3. Bundle ZSPR sprite packs
info "Bundling 343 ZSPR sprite packs..."
mkdir -p "${APP_DIR}/sprites-gfx/snes/zelda3/link/sheets"
cp sprites-gfx/snes/zelda3/link/sheets/*.zspr "${APP_DIR}/sprites-gfx/snes/zelda3/link/sheets/" 2>/dev/null
SPRITE_COUNT=$(ls "${APP_DIR}/sprites-gfx/snes/zelda3/link/sheets/"*.zspr 2>/dev/null | wc -l | tr -d ' ')

# 4. Download appimagetool if not present
info "Setting up appimagetool..."
APPIMAGETOOL_BIN="${APP_DIR}/appimagetool"
if [ ! -f "$APPIMAGETOOL_BIN" ]; then
    # Use appimagetool from GitHub releases
    if command -v appimagetool &>/dev/null; then
        APPIMAGETOOL_BIN=$(command -v appimagetool)
    else
        URL="https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
        if command -v wget &>/dev/null; then
            wget -q "$URL" -O "$APPIMAGETOOL_BIN" 2>/dev/null
        elif command -v curl &>/dev/null; then
            curl -sL "$URL" -o "$APPIMAGETOOL_BIN"
        else
            error "Need wget or curl to download appimagetool"
            exit 1
        fi
        chmod +x "$APPIMAGETOOL_BIN"
    fi
fi

# 5. Create .desktop file and icon
chmod +x "${APP_DIR}/AppRun"

# 6. Create the AppImage
info "Creating AppImage: ${OUTPUT_NAME}..."
ARCH=x86_64 "$APPIMAGETOOL_BIN" "${APP_DIR}" "${OUTPUT_NAME}"

# 7. Cleanup and report
rm -f "$APPIMAGETOOL_BIN" 2>/dev/null || true
info "AppImage created: ${OUTPUT_NAME}"
echo ""
echo "  Run: ./${OUTPUT_NAME}"
echo ""
echo "Bundle contents:"
echo "  - Game binary (Linux x86_64)"
echo "  - Game assets (zelda3_assets.dat)"
echo "  - HD upscaling shaders (scalefx, xBR, sharpen, sabr, nnedi3)"
echo "  - ${SPRITE_COUNT} ZSPR character sprite packs"
echo "  - Pre-configured: 16:9 widescreen, auto-save, OpenGL rendering"
echo ""
echo "First launch copies config to ~/.config/zelda3/"
echo "Saves go to ~/.local/share/zelda3/"
