TARGET_EXEC:=zelda3
SRCS:=$(wildcard src/*.c snes/*.c) third_party/gl_core/gl_core_3_1.c third_party/opus-1.3.1-stripped/opus_decoder_amalgam.c
OBJS:=$(SRCS:%.c=%.o)
PYTHON:=/usr/bin/env python3

# Default CFLAGS — override via CFLAGS=... on command line
CFLAGS:=$(if $(CFLAGS),$(CFLAGS),-O2 -Wall -Wno-unknown-warning-option -Wno-deprecated-non-prototype -Wno-bitwise-op-parentheses -Wno-logical-op-parentheses -Wno-parentheses -Wno-unused-variable -Wno-unused-const-variable -Wno-unused-function -Wno-unused-but-set-variable -Wno-maybe-uninitialized -Wno-strict-aliasing) -I .
CFLAGS:=${CFLAGS} $(shell sdl2-config --cflags) -DSYSTEM_VOLUME_MIXER_AVAILABLE=0

# Version — read VERSION file, fall back to git describe
VERSION := $(shell cat VERSION 2>/dev/null || git describe --tags --abbrev=0 2>/dev/null || echo "v0.0.0")
CFLAGS := $(CFLAGS) -DCURRENT_VERSION='"$(VERSION)"'

ifeq (${OS},Windows_NT)
    WINDRES:=windres
    RES:=zelda3.res
    SDLFLAGS:=-Wl,-Bstatic $(shell sdl2-config --static-libs)
else
    SDLFLAGS := $(shell sdl2-config --libs) -lm
endif

.PHONY: all clean clean_obj clean_gen

all: $(TARGET_EXEC)
$(TARGET_EXEC): $(OBJS) $(RES)
	$(CC) $^ -o $@ $(LDFLAGS) $(SDLFLAGS)
%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(RES): src/platform/win32/zelda3.rc
	@echo "Generating Windows resources"
	@$(WINDRES) $< -O coff -o $@

clean: clean_obj clean_gen
clean_obj:
	@$(RM) $(OBJS) $(TARGET_EXEC)
clean_gen:
	@$(RM) $(RES) tables/zelda3_assets.dat tables/*.txt tables/*.png tables/sprites/*.png tables/*.yaml
	@rm -rf tables/__pycache__ tables/dungeon tables/img tables/overworld tables/sound
