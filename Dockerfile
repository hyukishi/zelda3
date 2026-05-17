FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    libsdl2-dev \
    python3 \
    python3-pip \
    wget \
    file \
    patchelf \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install --no-cache-dir pillow pyyaml

WORKDIR /build
COPY . .

# Replace macOS absolute-path symlinks with the real directories
RUN rm -f AppDir/glsl-shaders AppDir/sprites-gfx \
    && cp -r glsl-shaders AppDir/ \
    && cp -r sprites-gfx AppDir/

RUN make clean_obj && make -j"$(nproc)"

# Bundle binary, config, and extraction scripts into AppDir
RUN cp zelda3 AppDir/ \
    && cp assets/restool.py AppDir/ \
    && cp -r assets AppDir/ \
    && cp -r other AppDir/ \
    && chmod +x AppDir/zelda3 AppDir/AppRun

# Bundle SDL2 shared library into AppImage so users don't need it installed
RUN mkdir -p AppDir/usr/lib \
    && cp -L /usr/lib/x86_64-linux-gnu/libSDL2-2.0.so.0 AppDir/usr/lib/ \
    && patchelf --set-rpath '$ORIGIN/../lib' AppDir/zelda3

# Download appimagetool and create the AppImage
RUN wget -qO /tmp/appimagetool \
    https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage \
    && chmod +x /tmp/appimagetool \
    && ARCH=x86_64 /tmp/appimagetool --appimage-extract-and-run AppDir zelda3-linux-x86_64.AppImage \
    && chmod +x zelda3-linux-x86_64.AppImage \
    && rm /tmp/appimagetool

ENTRYPOINT ["cp", "zelda3-linux-x86_64.AppImage", "/out/"]
