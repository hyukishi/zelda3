# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Development

### Build the game (macOS/Linux)
```sh
make -j$(sysctl -n hw.logicalcpu)   # macOS
make -j$(nproc)                      # Linux
make clean all                       # Clean rebuild
```

Requires SDL2 (`brew install sdl2`) and Python with `pillow` and `pyyaml` (`python3 -m pip install Pillow PyYAML`, or use a venv). Override the Python used for asset extraction: `make PYTHON="venv/bin/python3"`.

### Build for other targets
```sh
./build.sh macos     # macOS binary
./build.sh appimage  # Linux AppImage via Docker
./build.sh windows   # Windows via MinGW
./build.sh macapp    # macOS .app bundle
```

### No test suite
This project has no automated tests. The primary verification mechanism is the optional ROM comparison mode: run `./zelda3 zelda3.sfc` to run the C reimplementation and original ROM side-by-side, comparing RAM state each frame to verify correctness.

### Version
Current release: `v0.9.1`. The updater in `src/updater.c` checks `https://api.github.com/repos/hyukishi/zelda3/releases/latest` for newer versions.

## Architecture

### Overview
This is a reverse-engineered C port of Zelda 3: A Link to the Past (~80k LOC). It emulates the SNES PPU (graphics), DSP (audio), and CPU at a high level — rendering tiles from the original ROM data, not using conventional textures. All game logic runs via the emulated SNES CPU executing recompiled game code.

### Key source directories
- `src/` — Game logic, rendering pipeline, UI, config, audio player, updater
- `snes/` — SNES emulation core: PPU, CPU, APU, DMA, DSP, SPC700
- `assets/` — Python scripts (`restool.py`) that extract and compile resources from the ROM
- `AppDir/` — AppImage packaging skeleton (AppRun, .desktop, icon)
- `platform/` — Platform-specific code (Switch, Windows)
- `third_party/` — `gl_core_3.1` (OpenGL loader) and `opus` decoder
- `glsl-shaders/` — GLSL shader presets for post-processing upscaling (bundled in repo)

### Source files (key ones)
| File | Purpose |
|------|---------|
| `src/main.c` | Entry point, SDL window/event loop, input handling, audio, main game loop. Owns `ChangeWindowScale()`, `HandleInput()`, `HandleCommand()` |
| `src/config.c` / `config.h` | INI file parser, key binding system, config save/load. `Config g_config` holds all settings. `CheatConfig g_cheat_config` holds cheat toggle state. Define `kKeys_*` enum and `kGamepadBtn_*` enum |
| `src/settings_menu.c` / `settings_menu.h` | F12/Escape settings overlay. Draws into pixel buffer using an embedded 8x8 bitmap font (IBM VGA ROM). Handles navigation, value changes, cheat toggles, per-frame cheat application, gamepad support, updater status |
| `src/zelda_rtl.c` / `zelda_rtl.h` | Runtime layer — bridges the SNES emulator to the native platform. Defines `ZeldaEnv` struct (`g_zenv`) containing `ppu`, `ram`, `sram`, `vram`, `dma`, `player`. Functions: `ZeldaRunFrame()`, `ZeldaDrawPpuFrame()`, `PatchCommand()` |
| `src/zelda_cpu_infra.c` | Low-level CPU infrastructure, ROM patching, state snapshot save/load/replay |
| `src/features.h` | `kFeatures0_*` bitmask flags for optional game modifications (e.g., SwitchLR, TurnWhileDashing, MiscBugFixes). Stored at `g_ram[0x64c]`. Also defines special RAM locations like `hud_cur_item_x`, `hud_cur_item_l/r` |
| `src/variables.h` | **Critical file** (~1500 lines) — `#define` macros mapping SNES RAM addresses (`g_ram + offset`) to named game variables like `link_sword_type`, `link_health_current`, `link_item_bow`, etc. Every game system reads state through these macros |
| `src/load_gfx.h` / `load_gfx.c` | Graphics decompression and VRAM loading. Functions like `DecompressSwordGraphics()`, `DecompressShieldGraphics()`, `LoadGearPalettes()` |
| `src/opengl.c` | OpenGL render backend (`SDL_WINDOW_OPENGL`) — core profile 3.3+, GLSL 330. Owns shader-based post-processing |
| `src/glsl_shader.c` | GLSL shader file loader and compiler. Handles multi-pass shader chains from `.glslp` preset files. Auto-upgrades `#version 130` to `#version 330` for core-profile contexts |
| `src/updater.c` | GitHub release checker — fetches latest version tag from API, downloads new binary, stages for next launch |
| `src/messaging.c` / `messaging.h` | In-game text rendering system (used for dialogue, item names, etc.) |
| `snes/ppu.c` / `ppu.h` | SNES PPU emulation (from LakeSnes). Renders the game frame from tile data |

### Game logic source files (recompiled SNES code)
| File | Purpose |
|------|---------|
| `src/player.c` | Link movement, sword attacks, item usage, collision |
| `src/overworld.c` | Overworld map logic, transitions, events |
| `src/dungeon.c` | Dungeon room logic, puzzles, bosses |
| `src/sprite.c` / `src/sprite_main.c` | NPC and enemy sprites, behaviors |
| `src/ancilla.c` | Ancillary objects (sword beams, boomerangs, bombs, arrows, etc.) |
| `src/hud.c` | Heads-up display rendering |
| `src/attract.c` | Title screen and attract mode |
| `src/ending.c` | End credits sequence |
| `src/select_file.c` | File select screen |
| `src/overlord.c` | Scene/entity manager |
| `src/nmi.c` | Non-maskable interrupt handler (V-blank) |
| `src/tagalong.c` | Following companion logic |
| `src/poly.c` | Polygon rendering (used for crystals) |
| `src/tile_detect.c` | Tile property detection |
| `src/misc.c` | Miscellaneous game routines |
| `src/player_oam.c` | Player sprite OAM (Object Attribute Memory) |

### Rendering pipeline
1. `ZeldaRunFrame()` — runs one frame of SNES emulation (CPU + PPU)
2. `ZeldaDrawPpuFrame()` — PPU draws the frame into a pixel buffer (ARGB8888)
3. Optional: FPS counter (`RenderNumber`) and settings menu overlay (`SettingsMenu_Draw`) are drawn into the same pixel buffer
4. `g_renderer_funcs.EndDraw()` — presents the buffer (SDL texture copy or OpenGL swap)

### Render backends
Selected at startup by `g_config.output_method` (`kOutputMethod_SDL=0`, `kOutputMethod_SDLSoftware=1`, `kOutputMethod_OpenGL=2`, `kOutputMethod_OpenGL_ES=3`). Uses a function-pointer table (`RendererFuncs` struct) with `Initialize`, `BeginDraw`, `EndDraw`, and `Destroy`. SDL backend uses `kSdlRendererFuncs`; OpenGL backends go through `OpenGLRenderer_Create()`.

### Memory & state
- `g_ram[131072]` — emulated SNES WRAM. All game state (Link position, health, items, etc.) lives here. Accessed via `#define` macros in `variables.h`
- `g_zenv` — the emulator environment struct (`ZeldaEnv`): ppu, ram, sram, vram, dma, audio player, dialogue data
- `g_config` — `Config` struct with all parsed INI values
- `g_cheat_config` — `CheatConfig` struct, cheat toggle state (persisted to `[Cheats]` in INI)
- `g_wanted_zelda_features` — bitmask of `kFeatures0_*` flags for optional game modifications

### Input flow
`SDL_KEYDOWN` → `HandleInput()` → `FindCmdForSdlKey()` (looks up key binding hash) → `HandleCommand()` → sets SNES joypad bitset (`g_input1_state`) or triggers action. Gamepad input: `HandleGamepadInput()` → `FindCmdForGamepadButton()` → `HandleCommand()`. The settings menu intercepts inputs when `g_settings_menu_active` is true.

### Config system
`zelda3.ini` at project root. INI parsing in `config.c`: sections map to integers (0=General, 1=Graphics, 2=Sound, 3=Features, 4=KeyMap, 5=Cheats). `SaveConfigFile()` writes current settings on exit and when closing the settings menu. The game auto-creates this file on first run.

Key binding setup: `kDefaultKbdControls[]` (position-mapped to `kKeys_*` enum) → `KeyMapHash_Add()` during `RegisterDefaultKeys()` → `FindCmdForSdlKey()` for lookup.

### ROM requirement
Place US `zelda3.sfc` (SHA256: `66871d66be19ad2c34c927d6b14cd8eb6fc3181965b6e517cb361f7316009cfb`) in the project root or alongside the binary. Assets are auto-extracted on first launch via `assets/restool.py` into `zelda3_assets.dat`. The ROM is only needed once.

### Adding a new key bind
1. Add `kKeys_*` entry to the enum in `config.h` (before `kKeys_Total`)
2. Add the SDL keycode entry at the corresponding position in `kDefaultKbdControls[]` in `config.c`
3. Add `S(Name)` to `kKeyNameId[]` in `config.c`
4. Add handling in `HandleCommand_Locked()` switch in `main.c`

### Adding a new feature toggle
1. Add a `kFeatures0_*` bit in `src/features.h`
2. Add the setting to the `[Features]` section parsing in `config.c`
3. Add a menu entry in `src/settings_menu.c`
4. Read the bit where needed via `enhanced_features0` or via `g_config.features0`

### Noteworthy modifications
- **Sword beam cuts grass/bushes**: `Ancilla_SwordBeam()` in `ancilla.c` calls `Overworld_BombTile()` every 4th frame
- **GLSL version upgrade**: `GlslPass_Compile()` auto-upgrades `#version 130` to `#version 330` for core-profile OpenGL 3.3+
- **Shader update menu**: "Update Shaders" option in settings menu downloads latest GLSL shader pack from GitHub releases
- **Pause on menu**: Game pauses when settings menu is open
- **Gamepad in menus**: Settings menu fully navigable via controller (DPad + A to select, Select+Start to open)
- **Widescreen**: Aspect ratio changes update `g_config.extended_aspect_ratio` only (takes effect on PPU init at next launch)
- **Stretch to Fill**: `IgnoreAspectRatio` applies immediately via `SDL_RenderSetLogicalSize`

### Repository
This is a fork of `hyukishi/zelda3` (origin), itself derived from `snesrev/zelda3` (MIT license). Current branch: `master`.
