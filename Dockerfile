FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    libsdl2-dev \
    python3 \
    python3-pip \
    wget \
    file \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install --no-cache-dir pillow pyyaml

WORKDIR /build
COPY . .

# Remove broken symlinks in AppDir (upstream points to macOS paths)
RUN rm -f AppDir/glsl-shaders AppDir/sprites-gfx

RUN make -j"$(nproc)"

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
