FROM ubuntu:26.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    libsdl2-dev \
    python3 \
    python3-pip \
    python3-venv \
    wget \
    file \
    && rm -rf /var/lib/apt/lists/*

# Create a virtual environment for Python packages to avoid
# "installing outside a virtual environment" warnings
RUN python3 -m venv /venv \
    && /venv/bin/pip install --no-cache-dir pillow pyyaml

# Prepend the venv to PATH so 'python3' and scripts resolve to the venv
ENV PATH="/venv/bin:${PATH}"

WORKDIR /build
COPY . .

# Remove broken symlinks in AppDir (upstream points to macOS paths)
RUN rm -f AppDir/glsl-shaders AppDir/sprites-gfx

RUN make clean && make -j"$(nproc)"

# Pre-extract assets from the ROM so the game runs immediately at startup
RUN /venv/bin/python3 assets/restool.py --extract-from-rom

# Bundle binary, config, and extraction scripts into AppDir
RUN cp zelda3 AppDir/ \
    && cp zelda3.ini AppDir/ \
    && cp assets/restool.py AppDir/ \
    && cp -r assets AppDir/ \
    && cp -r other AppDir/ \
    && chmod +x AppDir/zelda3 AppDir/AppRun

# Download appimagetool and create the AppImage
RUN wget -qO /tmp/appimagetool \
    https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage \
    && chmod +x /tmp/appimagetool \
    && ARCH=x86_64 /tmp/appimagetool --appimage-extract-and-run AppDir zelda3-linux-x86_64.AppImage \
    && chmod +x zelda3-linux-x86_64.AppImage \
    && rm /tmp/appimagetool

ENTRYPOINT ["cp", "zelda3-linux-x86_64.AppImage", "/out/"]
