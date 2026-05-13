#!/bin/bash
# Build script for Zelda3 native cross-platform binary
# Usage: ./build.sh [macos|linux|windows|appimage|all]

set -euo pipefail
cd "$(dirname "$0")"

PYTHON="${PYTHON:-python3}"
BUILD_TARGET="${1:-macos}"

# Colors
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; NC='\033[0m'
info()  { echo -e "${GREEN}[INFO]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Ensure virtual environment exists
setup_venv() {
    if [ ! -d "venv" ]; then
        info "Creating Python virtual environment..."
        python3 -m venv venv
        source venv/bin/activate
        pip install -r requirements.txt
    else
        source venv/bin/activate
    fi
}

extract_assets() {
    if [ ! -f "zelda3_assets.dat" ] && [ ! -f "tables/zelda3_assets.dat" ]; then
        if [ -f "zelda3.sfc" ]; then
            info "Extracting assets from ROM..."
            python3 assets/restool.py --extract-from-rom
        else
            error "zelda3.sfc not found! Place the US ROM in this directory."
            echo "Expected SHA256: 66871d66be19ad2c34c927d6b14cd8eb6fc3181965b6e517cb361f7316009cfb"
            exit 1
        fi
    else
        info "Assets already extracted, skipping."
    fi
}

build_macos() {
    info "Building for macOS x86_64..."
    setup_venv
    extract_assets

    if command -v brew &>/dev/null; then
        brew list sdl2 &>/dev/null || brew install sdl2
    fi

    make -j$(sysctl -n hw.logicalcpu)
    info "macOS build complete: ./zelda3"
}

build_linux_native() {
    info "Building for Linux natively..."
    setup_venv
    extract_assets
    make -j$(nproc)
    info "Linux build complete: ./zelda3"
}

build_linux_docker() {
    info "Building Linux binary via Docker..."
    command -v docker &>/dev/null || { error "Docker required"; exit 1; }

    make clean_obj 2>/dev/null || true
    docker build -f Dockerfile.build-linux -t zelda3-builder .

    info "Extracting binary from Docker..."
    CID=$(docker create zelda3-builder)
    mkdir -p build-linux
    docker cp "$CID:/src/zelda3" ./build-linux/
    docker rm "$CID" >/dev/null

    # Copy platform-independent assets
    cp zelda3_assets.dat build-linux/ 2>/dev/null || warn "No pre-extracted assets, run make locally first"
    cp zelda3.ini build-linux/ 2>/dev/null || true

    info "Linux build complete: build-linux/zelda3"
    file build-linux/zelda3
}

build_appimage() {
    # Build Linux binary first, then package as AppImage
    if [ ! -f "build-linux/zelda3" ]; then
        if command -v docker &>/dev/null; then
            build_linux_docker
        else
            error "Need Docker or a Linux environment for AppImage builds"
            exit 1
        fi
    fi
    exec ./build-appimage.sh
}

build_windows() {
    info "Building for Windows x86_64..."
    setup_venv
    extract_assets

    if command -v x86_64-w64-mingw32-gcc &>/dev/null; then
        info "Cross-compiling with MinGW-w64..."
        make CC=x86_64-w64-mingw32-gcc \
             SDLFLAGS="-lSDL2 -lm" \
             CFLAGS="-O2 -I ./third_party/SDL2-2.26.3/include -I . -DSYSTEM_VOLUME_MIXER_AVAILABLE=0 -Wno-deprecated-non-prototype"
        mv zelda3 zelda3.exe 2>/dev/null || true
        info "Windows build complete: zelda3.exe"
    elif command -v tcc &>/dev/null; then
        info "Using TCC..."
        ./run_with_tcc.bat
    else
        error "No Windows cross-compiler found."
        echo "Options:"
        echo "  - Install MinGW-w64: brew install mingw-w64"
        echo "  - Use TCC (included in third_party/)"
        echo "  - Build natively on Windows with MSVC or TCC"
        exit 1
    fi
}

case "$BUILD_TARGET" in
    macos|mac)
        build_macos
        ;;
    macapp)
        build_macos
        echo "=== Packaging .app bundle ==="
        mkdir -p zelda3.app/Contents/{MacOS,Resources}
        cp zelda3 zelda3.app/Contents/MacOS/
        cp zelda3_assets.dat zelda3.app/Contents/Resources/
        cp zelda3.ini zelda3.app/Contents/Resources/
        # Generate icon if needed
        if [ ! -f zelda3.app/Contents/Resources/zelda3.icns ]; then
            mkdir -p /tmp/z3-iconset
            python3 -c "
from PIL import Image
img = Image.open('AppDir/zelda3.png')
for s in 16,32,64,128,256,512:
    img.resize((s,s), Image.NEAREST).save(f'/tmp/z3-iconset/icon_{s}x{s}.png')
    if s*2 <= 1024:
        img.resize((s*2,s*2), Image.NEAREST).save(f'/tmp/z3-iconset/icon_{s}x{s}@2x.png')
" 2>/dev/null
            iconutil -c icns /tmp/z3-iconset -o zelda3.app/Contents/Resources/zelda3.icns 2>/dev/null || true
            rm -rf /tmp/z3-iconset
        fi
        # Info.plist
        cat > zelda3.app/Contents/Info.plist << 'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>zelda3</string>
    <key>CFBundleIdentifier</key>
    <string>com.snesrev.zelda3</string>
    <key>CFBundleName</key>
    <string>Zelda 3 - A Link to the Past</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleIconFile</key>
    <string>zelda3</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>LSMinimumSystemVersion</key>
    <string>10.13</string>
</dict>
</plist>
PLIST
        # Create a double-clickable .command launcher at project root
        cat > "Launch Zelda3.command" << 'CMDEOF'
#!/bin/bash
cd "$(dirname "$0")"
nohup ./zelda3 >/dev/null 2>&1 &
osascript -e 'tell application "Terminal" to close first window' &>/dev/null
CMDEOF
        chmod +x "Launch Zelda3.command"
        echo ""
        echo "=== .app bundle created: zelda3.app ==="
        echo ""
        echo "The .app bundle requires an Apple Developer certificate to pass"
        echo "Gatekeeper on macOS 14+. Without one, use the launcher instead:"
        echo "  Double-click 'Launch Zelda3.command' in the project folder."
        echo ""
        echo "Or run directly from terminal:"
        echo "  ./zelda3"
        echo ""
        ;;
    linux)
        build_linux_docker
        ;;
    appimage)
        build_appimage
        ;;
    windows|win)
        build_windows
        ;;
    all)
        build_macos
        info "--- macOS done ---"
        build_windows
        info "--- Windows done ---"
        [ -d build-linux ] || build_linux_docker
        info "--- Linux done ---"
        ;;
    *)
        echo "Usage: $0 [macos|macapp|linux|windows|appimage|all]"
        echo ""
        echo "Prerequisites:"
        echo "  - Place zelda3.sfc (US ROM) in this directory"
        echo "  - macOS: brew install sdl2  then  ./build.sh macos"
        echo "  - Linux: sudo apt install libsdl2-dev python3-pip  then  ./build.sh linux"
        echo "  - Windows via MinGW: brew install mingw-w64  then  ./build.sh windows"
        echo "  - AppImage: ./build.sh appimage  (needs Docker running)"
        echo "  - All: ./build.sh all"
        echo ""
        echo "HD features bundled in AppDir/"
        echo "  - OpenGL output with scalefx/xBR upscaling shader"
        echo "  - $(ls sprites-gfx/snes/zelda3/link/sheets/*.zspr 2>/dev/null | wc -l) ZSPR character sprite packs"
        echo "  - 16:9 widescreen, auto-save, enhanced mode7, QoL features"
        exit 0
        ;;
esac
