#include "settings_menu.h"
#include "config.h"
#include "types.h"
#include "variables.h"
#include "load_gfx.h"
#include "features.h"
#include "zelda_rtl.h"
#include "snes/ppu.h"
#include "updater.h"
#include <SDL.h>
#include <stdio.h>
#include <string.h>

bool g_settings_menu_active;

// --- Externs from main.c ---
extern uint8 g_current_window_scale;
extern SDL_Window *g_window;
extern uint32 g_win_flags;
extern int g_sdl_audio_mixer_volume;
extern uint8 g_paused;
extern void ChangeWindowScale(int scale_step);

// Access the emulated SNES RAM for cheats
extern uint8 g_ram[131072];

// --- 8x8 bitmap font for ASCII 32-126 (IBM VGA 8x8, public domain) ---
static const uint8 kFont8x8[760] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x00,0x20,0x00,
  0x50,0x50,0x50,0x00,0x00,0x00,0x00,0x00,0x50,0x50,0xf8,0x50,0xf8,0x50,0x50,0x00,
  0x20,0x78,0xa0,0x70,0x28,0xf0,0x20,0x00,0xc0,0xc8,0x10,0x20,0x40,0x98,0x18,0x00,
  0x40,0xa0,0x40,0x48,0x90,0x90,0x68,0x00,0x10,0x10,0x10,0x00,0x00,0x00,0x00,0x00,
  0x10,0x20,0x40,0x40,0x40,0x20,0x10,0x00,0x40,0x20,0x10,0x10,0x10,0x20,0x40,0x00,
  0x00,0x20,0xa8,0x50,0xa8,0x20,0x00,0x00,0x00,0x20,0x20,0xf8,0x20,0x20,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x40,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x08,0x10,0x20,0x40,0x80,0x00,0x00,
  0x20,0x50,0x88,0x88,0x88,0x50,0x20,0x00,0x20,0x60,0x20,0x20,0x20,0x20,0x70,0x00,
  0x70,0x88,0x08,0x10,0x60,0x80,0xf8,0x00,0x70,0x88,0x08,0x30,0x08,0x88,0x70,0x00,
  0x08,0x18,0x28,0x48,0xf8,0x08,0x08,0x00,0xf8,0x80,0x80,0xf0,0x08,0x08,0xf0,0x00,
  0x30,0x40,0x80,0xf0,0x88,0x88,0x70,0x00,0xf8,0x08,0x10,0x20,0x20,0x40,0x40,0x00,
  0x70,0x88,0x88,0x70,0x88,0x88,0x70,0x00,0x70,0x88,0x88,0x78,0x08,0x08,0x70,0x00,
  0x00,0x00,0x20,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x20,0x20,0x40,
  0x08,0x10,0x20,0x40,0x20,0x10,0x08,0x00,0x00,0x00,0xf8,0x00,0xf8,0x00,0x00,0x00,
  0x40,0x20,0x10,0x08,0x10,0x20,0x40,0x00,0x70,0x88,0x08,0x10,0x20,0x00,0x20,0x00,
  0x70,0x88,0x08,0x68,0xa8,0xa8,0x70,0x00,0x20,0x50,0x88,0x88,0xf8,0x88,0x88,0x00,
  0xf0,0x48,0x48,0x70,0x48,0x48,0xf0,0x00,0x30,0x48,0x80,0x80,0x80,0x48,0x30,0x00,
  0xe0,0x50,0x48,0x48,0x48,0x50,0xe0,0x00,0xf8,0x80,0x80,0xf0,0x80,0x80,0xf8,0x00,
  0xf8,0x80,0x80,0xf0,0x80,0x80,0x80,0x00,0x30,0x48,0x80,0x98,0x88,0x48,0x30,0x00,
  0x88,0x88,0x88,0xf8,0x88,0x88,0x88,0x00,0x70,0x20,0x20,0x20,0x20,0x20,0x70,0x00,
  0x08,0x08,0x08,0x08,0x08,0x48,0x30,0x00,0x88,0x90,0xa0,0xc0,0xa0,0x90,0x88,0x00,
  0x80,0x80,0x80,0x80,0x80,0x80,0xf8,0x00,0x88,0xd8,0xa8,0xa8,0x88,0x88,0x88,0x00,
  0x88,0xc8,0xa8,0x98,0x88,0x88,0x88,0x00,0x30,0x48,0x88,0x88,0x88,0x48,0x30,0x00,
  0xf0,0x88,0x88,0xf0,0x80,0x80,0x80,0x00,0x30,0x48,0x88,0x88,0xa8,0x48,0x38,0x00,
  0xf0,0x88,0x88,0xf0,0xa0,0x90,0x88,0x00,0x70,0x88,0x80,0x70,0x08,0x88,0x70,0x00,
  0xf8,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x88,0x88,0x88,0x88,0x88,0x88,0x70,0x00,
  0x88,0x88,0x88,0x88,0x50,0x50,0x20,0x00,0x88,0x88,0x88,0xa8,0xa8,0xd8,0x88,0x00,
  0x88,0x88,0x50,0x20,0x50,0x88,0x88,0x00,0x88,0x88,0x50,0x20,0x20,0x20,0x20,0x00,
  0xf8,0x08,0x10,0x20,0x40,0x80,0xf8,0x00,0x70,0x40,0x40,0x40,0x40,0x40,0x70,0x00,
  0x00,0x80,0x40,0x20,0x10,0x08,0x00,0x00,0x70,0x10,0x10,0x10,0x10,0x10,0x70,0x00,
  0x00,0x20,0x50,0x88,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,
  0x20,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x08,0x78,0x88,0x78,
  0x80,0x80,0xb0,0xc8,0x88,0x88,0xf0,0x00,0x00,0x00,0x00,0x70,0x88,0x80,0x88,0x70,
  0x08,0x08,0x68,0x98,0x88,0x88,0x78,0x00,0x00,0x00,0x00,0x70,0x88,0xf8,0x80,0x70,
  0x10,0x20,0x70,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x00,0x78,0x88,0x88,0x78,0x08,
  0x80,0x80,0xb0,0xc8,0x88,0x88,0x88,0x00,0x20,0x00,0x60,0x20,0x20,0x20,0x70,0x00,
  0x10,0x00,0x10,0x10,0x10,0x10,0x50,0x20,0x80,0x80,0x88,0x90,0xa0,0xd0,0x88,0x00,
  0x60,0x20,0x20,0x20,0x20,0x20,0x70,0x00,0x00,0x00,0x00,0xd0,0xa8,0xa8,0x88,0x00,
  0x00,0x00,0x00,0xb0,0xc8,0x88,0x88,0x00,0x00,0x00,0x00,0x70,0x88,0x88,0x88,0x70,
  0x00,0x00,0x00,0xf0,0x88,0x88,0xf0,0x80,0x00,0x00,0x00,0x78,0x88,0x88,0x78,0x08,
  0x00,0x00,0x00,0xb0,0xc8,0x80,0x80,0x00,0x00,0x00,0x00,0x78,0x80,0x70,0x08,0xf0,
  0x20,0x20,0x70,0x20,0x20,0x20,0x10,0x00,0x00,0x00,0x00,0x88,0x88,0x88,0x88,0x70,
  0x00,0x00,0x00,0x88,0x88,0x50,0x50,0x20,0x00,0x00,0x00,0x88,0x88,0xa8,0xa8,0x50,
  0x00,0x00,0x00,0x88,0x50,0x20,0x50,0x88,0x00,0x00,0x00,0x88,0x88,0x88,0x78,0x08,
  0x00,0x00,0x00,0xf8,0x10,0x20,0x40,0xf8,0x10,0x20,0x20,0x40,0x20,0x20,0x10,0x00,
  0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x40,0x20,0x20,0x10,0x20,0x20,0x40,0x00,
  0x00,0x00,0x40,0xa8,0x10,0x00,0x00,0x00,
};

// --- Colors ---
enum {
  kCol_Tint      = 0x80000000,
  kCol_PanelBg   = 0xCC0A0A1E,
  kCol_Title     = 0xFFFFD700,
  kCol_Label     = 0xFFFFFFFF,
  kCol_Value     = 0xFF40B0FF,
  kCol_Highlight = 0x662A2A6E,
  kCol_HiAccent  = 0xFF5060FF,
  kCol_BarFill   = 0xFF30D080,
  kCol_BarBg     = 0xFF333355,
  kCol_Close     = 0xFFFF6060,
  kCol_On        = 0xFF40FF80,
  kCol_Off       = 0xFF888888,
  kCol_Action    = 0xFF90D0FF,
  kCol_Cheat     = 0xFFFFB040,
  kCol_Back      = 0xFF808080,
  kCol_Sep       = 0xFF444466,
};

enum { kFontW = 8, kFontH = 8, kPanelPad = 12, kLineH = 14 };

// --- Pages ---
enum { kPage_Main, kPage_Controls, kPage_Cheats };

// --- State ---
static int g_cursor;
static int g_page;
static int g_main_scroll;
static int g_cheat_scroll;


// --- Cheat state (persisted via g_cheat_config) ---
#define g_cheat_health   g_cheat_config.infinite_health
#define g_cheat_magic    g_cheat_config.infinite_magic
#define g_cheat_bombs    g_cheat_config.infinite_bombs
#define g_cheat_arrows   g_cheat_config.infinite_arrows
#define g_cheat_keys     g_cheat_config.infinite_keys
#define g_cheat_rupees   g_cheat_config.infinite_rupees
#define g_cheat_potcarry g_cheat_config.pot_carry
#define g_cheat_wall     g_cheat_config.walk_wall

// --- Drawing helpers with bounds safety ---

static void DrawPixel(uint8 *buf, int pitch, int x, int y, uint32 color) {
  if (!buf || x < 0 || y < 0) return;
  ((uint32 *)buf)[y * (pitch / 4) + x] = color;
}

static void DrawChar(uint8 *buf, int pitch, int x, int y, char c, uint32 color) {
  if (c < 32 || c > 126) c = ' ';
  const uint8 *glyph = kFont8x8 + (c - 32) * 8;
  for (int row = 0; row < 8; row++) {
    uint8 bits = glyph[row];
    int py = y + row;
    if (py < 0) continue;
    for (int col = 0; col < 8; col++) {
      if (bits & (1 << (7 - col)))
        DrawPixel(buf, pitch, x + col, py, color);
    }
  }
}

static void DrawString(uint8 *buf, int pitch, int x, int y, const char *s, uint32 color) {
  int ox = x;
  while (s && *s) {
    if (*s == '\n') { x = ox; y += kLineH; s++; continue; }
    DrawChar(buf, pitch, x, y, *s, color);
    x += kFontW + 1;
    s++;
  }
}

static void DrawRectSafe(uint8 *buf, int pitch, int x, int y, int w, int h,
                         uint32 color, int fb_w, int fb_h) {
  if (!buf || h <= 0 || w <= 0 || pitch <= 0) return;
  // Clamp to framebuffer
  if (x >= fb_w || y >= fb_h) return;
  if (x + w > fb_w) w = fb_w - x;
  if (y + h > fb_h) h = fb_h - y;
  if (w <= 0 || h <= 0) return;

  int stride = pitch / 4;
  for (int row = 0; row < h; row++) {
    uint32 *line = (uint32 *)buf + (y + row) * stride;
    if ((color >> 24) == 0xFF) {
      for (int col = 0; col < w; col++) line[x + col] = color;
    } else {
      for (int col = 0; col < w; col++) {
        uint32 *p = line + x + col;
        uint32 bg = *p;
        uint8 a = color >> 24, inv = 255 - a;
        uint8 r = (((color >> 16) & 0xFF) * a + ((bg >> 16) & 0xFF) * inv) / 255;
        uint8 g = (((color >> 8) & 0xFF) * a + ((bg >> 8) & 0xFF) * inv) / 255;
        uint8 b = ((color & 0xFF) * a + (bg & 0xFF) * inv) / 255;
        *p = (0xFF << 24) | (r << 16) | (g << 8) | b;
      }
    }
  }
}

// ====================================================================
//  MAIN SETTINGS PAGE
// ====================================================================

enum {
  kOpt_WindowScale,
  kOpt_Fullscreen,
  kOpt_AspectRatio,
  kOpt_Volume,
  kOpt_LinearFilter,
  kOpt_StretchToFill,
  kOpt_Shader,
  kOpt_OutputMethod,
  kOpt_Controls,
  kOpt_Cheats,
  kOpt_Autosave,
  kOpt_Close,
  kOpt_MAIN_COUNT,
};

static const char *kOptLabels[] = {
  "Window Scale",
  "Fullscreen",
  "Aspect Ratio",
  "Volume",
  "Linear Filter",
  "Stretch to Fill",
  "Shader",
  "Output Method",
  "Show Controls",
  "Cheats",
  "Autosave",
  "Close",
};

static const int kOutputValues[] = { 0, 2, 1, 3 };
static const int kOutputCount = 4;

// Shader presets
static const char *kShaderNames[] = { "None", "scalefx-aa", "6xBRZ", "ScaleHQ", "6xBRZ+ScaleHQ" };
static const char *kShaderPaths[] = { NULL, "glsl-shaders/presets/scalefx-aa.glslp", "glsl-shaders/xbrz/6xbrz-linear.glslp", "glsl-shaders/scalehq/4xScaleHQ.glslp", "glsl-shaders/presets/6xbrz+scalehq.glslp" };
static const int kShaderCount = 5;

static int GetShaderIndex(void) {
  if (!g_config.shader) return 0;
  for (int i = 1; i < kShaderCount; i++)
    if (kShaderPaths[i] && strcmp(g_config.shader, kShaderPaths[i]) == 0)
      return i;
  return 0;
}

// --- Aspect ratio helpers ---
static const char *kAspectNames[] = { "4:3", "16:9", "16:10", "18:9" };
static const int kAspectValues[] = { 0, 48, 34, 64 };
static const int kAspectCount = 4;

static int GetAspectIndex(void) {
  for (int i = 0; i < kAspectCount; i++)
    if (g_config.extended_aspect_ratio == kAspectValues[i])
      return i;
  return 1;
}

static void DrawMainPage(uint8 *buf, int pitch, int px, int py, int pw, int fb_w, int fb_h,
                         int content_y, int content_h) {
  int lx = px + kPanelPad;
  int vx = px + pw - kPanelPad - 80;
  int rh = kLineH + 1;

  // Calculate which items fit
  int lines_fit = (content_h > 0) ? content_h / rh : 1;
  if (lines_fit < 1) lines_fit = 1;

  // Clamp scroll so cursor is visible
  if (g_cursor >= 0 && g_cursor < kOpt_MAIN_COUNT) {
    if (g_cursor < g_main_scroll)
      g_main_scroll = g_cursor;
    else if (g_cursor >= g_main_scroll + lines_fit)
      g_main_scroll = g_cursor - lines_fit + 1;
  }
  if (g_main_scroll > kOpt_MAIN_COUNT - lines_fit)
    g_main_scroll = (kOpt_MAIN_COUNT > lines_fit) ? kOpt_MAIN_COUNT - lines_fit : 0;
  if (g_main_scroll < 0) g_main_scroll = 0;

  int y = content_y;
  for (int i = g_main_scroll; i < kOpt_MAIN_COUNT && i < g_main_scroll + lines_fit; i++) {
    int sel = (i == g_cursor);

    if (sel)
      DrawRectSafe(buf, pitch, px + kPanelPad, y, pw - kPanelPad * 2, kLineH, kCol_Highlight, fb_w, fb_h);
    if (sel)
      DrawChar(buf, pitch, lx - kFontW - 2, y + 2, '>', kCol_HiAccent);

    uint32 lcol = (i == kOpt_Close) ? kCol_Close : (i == kOpt_Cheats) ? kCol_Cheat : (i == kOpt_Autosave) ? kCol_Action : kCol_Label;
    DrawString(buf, pitch, lx, y + 2, kOptLabels[i], lcol);

    char vb[32];
    switch (i) {
    case kOpt_WindowScale:
      snprintf(vb, sizeof(vb), "%dx", g_current_window_scale);
      DrawString(buf, pitch, vx, y, vb, kCol_Value);
      break;
    case kOpt_Fullscreen:
      DrawString(buf, pitch, vx, y,
        (g_win_flags & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN)) ? "ON" : "OFF",
        (g_win_flags & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN)) ? kCol_On : kCol_Value);
      break;
    case kOpt_AspectRatio:
      DrawString(buf, pitch, vx, y, kAspectNames[GetAspectIndex()], kCol_Value);
      break;
    case kOpt_Volume: {
      int vol = (g_sdl_audio_mixer_volume * 100) / SDL_MIX_MAXVOLUME;
      int bw = 40;
      int bx = vx;
      int fill = (vol * bw) / 100;
      DrawRectSafe(buf, pitch, bx, y + 2, bw, 5, kCol_BarBg, fb_w, fb_h);
      if (fill > 0)
        DrawRectSafe(buf, pitch, bx, y + 2, fill, 5, kCol_BarFill, fb_w, fb_h);
      snprintf(vb, sizeof(vb), "%d%%", vol);
      DrawString(buf, pitch, bx + bw + 4, y, vb, kCol_Value);
      break;
    }
    case kOpt_LinearFilter:
      DrawString(buf, pitch, vx, y, g_config.linear_filtering ? "ON" : "OFF",
                 g_config.linear_filtering ? kCol_On : kCol_Value);
      break;
    case kOpt_StretchToFill:
      DrawString(buf, pitch, vx, y, g_config.ignore_aspect_ratio ? "ON" : "OFF",
                 g_config.ignore_aspect_ratio ? kCol_On : kCol_Value);
      break;
    case kOpt_Shader:
      DrawString(buf, pitch, vx, y, kShaderNames[GetShaderIndex()], kCol_Value);
      break;
    case kOpt_OutputMethod: {
      int om = g_config.output_method;
      const char *nm = "SDL";
      if (om == 2) nm = "OpenGL";
      else if (om == 1) nm = "SDL-SW";
      else if (om == 3) nm = "GL-ES";
      DrawString(buf, pitch, vx, y, nm, kCol_Value);
      break;
    }
    case kOpt_Autosave:
      DrawString(buf, pitch, vx, y, g_config.autosave ? "ON" : "OFF",
                 g_config.autosave ? kCol_On : kCol_Value);
      break;
    default: break;
    }
    y += rh;
  }
  // Scroll indicators
  bool scroll_up = (g_main_scroll > 0);
  bool scroll_dn = (g_main_scroll + lines_fit < kOpt_MAIN_COUNT);
  if (scroll_up)
    DrawString(buf, pitch, lx, content_y - 8, "^", kCol_Back);
  if (scroll_dn)
    DrawString(buf, pitch, lx, content_y + content_h - 10, "v", kCol_Back);
}

// ====================================================================
//  CONTROLS PAGE
// ====================================================================

static void DrawControlsPage(uint8 *buf, int pitch, int px, int py, int pw, int fb_w, int fb_h) {
  (void)pw;
  static const struct { const char *act; int kid; } keys[] = {
    {"Up",kKeys_Controls},{"Down",kKeys_Controls+1},{"Left",kKeys_Controls+2},{"Right",kKeys_Controls+3},
    {"Select",kKeys_Controls+4},{"Start",kKeys_Controls+5},{"A",kKeys_Controls+6},{"B",kKeys_Controls+7},
    {"X",kKeys_Controls+8},{"Y",kKeys_Controls+9},{"L",kKeys_Controls+10},{"R",kKeys_Controls+11},
    {"Fullscreen",kKeys_Fullscreen},{"Pause",kKeys_Pause},{"Turbo",kKeys_Turbo},
    {"Settings",kKeys_Settings},
  };
  const uint16 *def = GetDefaultKbdControls();
  int y = py + 6 * kFontH + 4;
  int lx = px + kPanelPad;
  int vx = px + kPanelPad + 80;

  for (size_t i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
    uint16 key = def ? def[keys[i].kid] : 0;
    int sc = key & 0x1FF;
    SDL_Keycode kc = sc;
    char kn[32] = {0};
    if (key & 0x1000) strcat(kn, "Ctrl+");
    if (key & 0x800)  strcat(kn, "Shft+");
    if (key & 0x400)  strcat(kn, "Alt+");
    const char *n = SDL_GetKeyName(kc);
    strncat(kn, n, sizeof(kn)-strlen(kn)-1);

    DrawString(buf, pitch, lx, y, keys[i].act, kCol_Label);
    DrawString(buf, pitch, vx, y, kn, kCol_Value);
    y += kLineH + 2;
  }
  y += 4;
  DrawString(buf, pitch, lx, y, "F12/Esc = back", kCol_Back);
}

// ====================================================================
//  CHEATS PAGE
// ====================================================================

enum {
  kCheat_FillHealth,
  kCheat_FillItems,
  kCheat_FillKeys,
  kCheat_Sep1,
  kCheat_InfHealth,
  kCheat_InfMagic,
  kCheat_InfBombs,
  kCheat_InfArrows,
  kCheat_InfKeys,
  kCheat_InfRupees,
  kCheat_Sep2,
  kCheat_UnlockAll,
  kCheat_PotCarry,
  kCheat_Wall,
  kCheat_Sep3,
  kCheat_Back,
  kCheat_COUNT,
};

static void DrawCheatsPage(uint8 *buf, int pitch, int px, int py, int pw, int fb_w, int fb_h,
                           int content_y, int content_h) {
  // Build label list
  struct { const char *lbl; uint32 col; bool show_val; const char *val; } items[kCheat_COUNT] = {{0}};
  int yi = 0;
  items[yi].lbl = "FILL:  Health"; items[yi].col = kCol_Action; items[yi].show_val = false; yi++;
  items[yi].lbl = "FILL:  Items";  items[yi].col = kCol_Action; items[yi].show_val = false; yi++;
  items[yi].lbl = "FILL:  Keys";   items[yi].col = kCol_Action; items[yi].show_val = false; yi++;
  items[yi].lbl = NULL; items[yi].col = 0; yi++; // separator
  items[yi].lbl = "INF:   Health"; items[yi].col = kCol_Label; items[yi].show_val = true; items[yi].val = g_cheat_health ? "ON" : "OFF"; yi++;
  items[yi].lbl = "INF:   Magic";  items[yi].col = kCol_Label; items[yi].show_val = true; items[yi].val = g_cheat_magic ? "ON" : "OFF"; yi++;
  items[yi].lbl = "INF:   Bombs";  items[yi].col = kCol_Label; items[yi].show_val = true; items[yi].val = g_cheat_bombs ? "ON" : "OFF"; yi++;
  items[yi].lbl = "INF:   Arrows"; items[yi].col = kCol_Label; items[yi].show_val = true; items[yi].val = g_cheat_arrows ? "ON" : "OFF"; yi++;
  items[yi].lbl = "INF:   Keys";   items[yi].col = kCol_Label; items[yi].show_val = true; items[yi].val = g_cheat_keys ? "ON" : "OFF"; yi++;
  items[yi].lbl = "INF:   Rupees"; items[yi].col = kCol_Label; items[yi].show_val = true; items[yi].val = g_cheat_rupees ? "ON" : "OFF"; yi++;
  items[yi].lbl = NULL; items[yi].col = 0; yi++; // separator
  items[yi].lbl = "Unlock All Items"; items[yi].col = kCol_Action; items[yi].show_val = false; yi++;
  items[yi].lbl = "Carry Pots";       items[yi].col = kCol_Label; items[yi].show_val = true; items[yi].val = g_cheat_potcarry ? "ON" : "OFF"; yi++;
  items[yi].lbl = "Walk Walls";       items[yi].col = kCol_Label; items[yi].show_val = true; items[yi].val = g_cheat_wall ? "ON" : "OFF"; yi++;
  items[yi].lbl = NULL; items[yi].col = 0; yi++; // separator
  items[yi].lbl = "Back"; items[yi].col = kCol_Close; items[yi].show_val = false; yi++;

  // Build visible-line index (skip separators)
  int vis_idx[kCheat_COUNT], vis_count = 0;
  for (int i = 0; i < kCheat_COUNT; i++)
    if (items[i].lbl != NULL)
      vis_idx[vis_count++] = i;

  int lx = px + kPanelPad;
  int vx = px + pw - kPanelPad - 50;
  int rh = kLineH + 2;

  // Clamp scroll so cursor is always visible
  int lines_fit = (content_h > 0) ? content_h / rh : 1;
  if (lines_fit < 1) lines_fit = 1;
  if (g_cursor >= 0 && g_cursor < kCheat_COUNT) {
    // Find which visible index the cursor is at
    int cursor_vis = -1;
    for (int i = 0; i < vis_count; i++)
      if (vis_idx[i] == g_cursor) { cursor_vis = i; break; }
    if (cursor_vis >= 0) {
      if (cursor_vis < g_cheat_scroll)
        g_cheat_scroll = cursor_vis;
      else if (cursor_vis >= g_cheat_scroll + lines_fit)
        g_cheat_scroll = cursor_vis - lines_fit + 1;
    }
  }
  // Clamp scroll to valid range
  if (g_cheat_scroll > vis_count - lines_fit)
    g_cheat_scroll = (vis_count > lines_fit) ? vis_count - lines_fit : 0;
  if (g_cheat_scroll < 0) g_cheat_scroll = 0;

  // Draw visible items
  int y = content_y;
  int end_y = content_y + content_h;
  int scroll_up = (g_cheat_scroll > 0);
  int scroll_dn = (g_cheat_scroll + lines_fit < vis_count);

  for (int vi = g_cheat_scroll; vi < vis_count && vi < g_cheat_scroll + lines_fit; vi++) {
    int i = vis_idx[vi];
    if (y + rh > end_y) break;
    int sel = (i == g_cursor);
    if (sel)
      DrawRectSafe(buf, pitch, px + kPanelPad, y, pw - kPanelPad * 2, kLineH, kCol_Highlight, fb_w, fb_h);
    if (sel)
      DrawChar(buf, pitch, lx - kFontW - 2, y + 3, '>', kCol_HiAccent);
    DrawString(buf, pitch, lx, y + 3, items[i].lbl, items[i].col);
    if (items[i].show_val) {
      bool on = (items[i].val && strcmp(items[i].val, "ON") == 0);
      DrawString(buf, pitch, vx, y + 3, items[i].val, on ? kCol_On : kCol_Off);
    }
    y += rh;
  }

  // Scroll indicators
  if (scroll_up)
    DrawString(buf, pitch, lx, content_y - 10, "^", kCol_Back);
  if (scroll_dn)
    DrawString(buf, pitch, lx, content_y + content_h - 10, "v", kCol_Back);
}

// ====================================================================
//  DRAWING
// ====================================================================

static void SettingsMenu_DrawFrame(uint8 *buf, int pitch, int fb_w, int fb_h) {
  if (!buf) return;

  // Full-frame dim
  DrawRectSafe(buf, pitch, 0, 0, fb_w, fb_h, kCol_Tint, fb_w, fb_h);

  // Panel — size it to fit the framebuffer so we never write out of bounds
  int pw = 260;
  int max_ph = fb_h - 20;          // leave 10px top/bottom margin
  if (pw > fb_w - 10) pw = fb_w - 10;
  if (pw < 160) pw = 160;

  int title_h = 7 * kFontH + 8;    // space for title + underline
  int nav_h = kFontH + 8;          // footer hint
  int opt_h = 0;
  int item_count = 0, sep_count = 0;
  if (g_page == kPage_Main) {
    item_count = kOpt_MAIN_COUNT;
  } else if (g_page == kPage_Cheats) {
    item_count = kCheat_COUNT;
    // count separators
    for (int i = 0; i < kCheat_COUNT; i++) {
      if (i == kCheat_Sep1 || i == kCheat_Sep2 || i == kCheat_Sep3) sep_count++;
    }
  } else {
    item_count = 17; // controls
  }
  opt_h = item_count * (kLineH + 2) + sep_count * (kLineH/2);
  int ph = title_h + opt_h + nav_h + kPanelPad * 2;
  if (ph > max_ph) ph = max_ph;

  int px = (fb_w - pw) / 2;
  int py = (fb_h - ph) / 2;
  if (px < 0) px = 0;
  if (py < 0) py = 0;

  // Panel background
  DrawRectSafe(buf, pitch, px, py, pw, ph, kCol_PanelBg, fb_w, fb_h);

  // Title
  int tx = px + (pw - 8 * 7) / 2; // "SETTINGS" = 7 chars * 8px + spacing
  DrawString(buf, pitch, tx, py + kPanelPad, "SETTINGS", kCol_Title);
  DrawRectSafe(buf, pitch, px + kPanelPad, py + kPanelPad + kFontH + 2,
               pw - kPanelPad * 2, 1, 0xFF555577, fb_w, fb_h);

  // Page content
  if (g_page == kPage_Main) {
    int content_y = py + 7 * kFontH + 8;
    int content_h = py + ph - kPanelPad - kFontH - 4 - content_y;
    DrawMainPage(buf, pitch, px, py, pw, fb_w, fb_h, content_y, content_h);
  } else if (g_page == kPage_Controls)
    DrawControlsPage(buf, pitch, px, py, pw, fb_w, fb_h);
  else if (g_page == kPage_Cheats) {
    int content_y = py + 7 * kFontH + 8;
    int content_h = py + ph - kPanelPad - kFontH - 4 - content_y;
    DrawCheatsPage(buf, pitch, px, py, pw, fb_w, fb_h, content_y, content_h);
  }

  // Footer
  if (g_update_available && Updater_IsReady()) {
    char ubuf[64];
    snprintf(ubuf, sizeof(ubuf), "Update %s! Press Enter", g_update_version);
    DrawString(buf, pitch, px + (pw - 24 * 4) / 2, py + ph - kPanelPad - kFontH,
               ubuf, kCol_Cheat);
  } else {
    DrawString(buf, pitch, px + (pw - 13 * 8) / 2, py + ph - kPanelPad - kFontH,
               "F12/ Esc: Close", kCol_Back);
  }
}

// ====================================================================
//  PUBLIC DRAW
// ====================================================================

void SettingsMenu_Draw(uint8 *pixel_buffer, int pitch, int fb_w, int fb_h) {
  if (!g_settings_menu_active) return;
  SettingsMenu_DrawFrame(pixel_buffer, pitch, fb_w, fb_h);
}

// ====================================================================
//  INPUT HANDLING
// ====================================================================

// --- One-shot cheat helpers ---

static void CheatFillHealth(void) {
  g_ram[0xF372] = 0xA0; // health filler (full)
  g_ram[0xF373] = 0x80; // magic filler
  g_ram[0xF36D] = g_ram[0xF36C]; // health current = capacity
}

static void CheatFillItems(void) {
  g_ram[0xF375] = g_ram[0xF370] > 0 ? g_ram[0xF370] : 10; // bombs
  g_ram[0xF376] = g_ram[0xF371] > 0 ? g_ram[0xF371] : 10; // arrows
  uint16 r = *(uint16*)(g_ram + 0xF360);
  r += 100;
  g_ram[0xF360] = r & 0xFF;
  g_ram[0xF361] = (r >> 8) & 0xFF;
}

static void CheatFillKeys(void) {
  g_ram[0xF36F] = 1; // small keys
}

static void CheatUnlockAll(void) {
  // Items (value 2 = obtained/usable)
  g_ram[0xF340] = 2;  // bow
  g_ram[0xF341] = 2;  // boomerang
  g_ram[0xF342] = 2;  // hookshot
  g_ram[0xF343] = 2;  // bombs
  g_ram[0xF344] = 2;  // mushroom
  g_ram[0xF345] = 2;  // fire rod
  g_ram[0xF346] = 2;  // ice rod
  g_ram[0xF347] = 2;  // bombos
  g_ram[0xF348] = 2;  // ether
  g_ram[0xF349] = 2;  // quake
  g_ram[0xF34A] = 2;  // lantern
  g_ram[0xF34B] = 2;  // hammer
  g_ram[0xF34C] = 2;  // flute
  g_ram[0xF34D] = 2;  // bug net
  g_ram[0xF34E] = 2;  // book of mudora
  g_ram[0xF34F] = 2;  // bottle index
  g_ram[0xF350] = 2;  // cane of somaria
  g_ram[0xF351] = 2;  // cane of byrna
  g_ram[0xF352] = 2;  // cape
  g_ram[0xF353] = 2;  // mirror
  g_ram[0xF354] = 2;  // gloves (power glove)
  g_ram[0xF355] = 1;  // boots
  g_ram[0xF356] = 1;  // flippers
  g_ram[0xF357] = 1;  // moon pearl

  // Upgrades
  g_ram[0xF359] = 3;  // sword: tempered (3)
  g_ram[0xF35A] = 3;  // shield: mirror (3)
  g_ram[0xF35B] = 2;  // armor/ mail: red (2)

  // Capacity upgrades
  if (g_ram[0xF370] < 3) g_ram[0xF370] = 3; // bomb capacity
  if (g_ram[0xF371] < 3) g_ram[0xF371] = 3; // arrow capacity
  g_ram[0xF36E] = 2;  // magic upgrade: full (2)

  // Reload gear graphics so sword/shield/armor sprites appear immediately
  DecompressSwordGraphics();
  DecompressShieldGraphics();
  LoadGearPalettes(g_ram[0xF359], g_ram[0xF35A], g_ram[0xF35B]);

  // Fill them too
  CheatFillHealth();
  CheatFillItems();
  CheatFillKeys();
}

static void CheatToggleWall(void) {
  g_ram[0x37F] ^= 1; // walk through walls
}

// --- Main page input ---

static void ChangeValue(int opt, int delta) {
  switch (opt) {
  case kOpt_WindowScale: {
    int ns = g_current_window_scale + delta;
    if (ns >= 1 && ns <= 10)
      for (int i = 0; i < (delta > 0 ? delta : -delta); i++)
        ChangeWindowScale(delta > 0 ? 1 : -1);
    break;
  }
  case kOpt_Fullscreen:
    if (delta > 0) {
      if (g_win_flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
        g_win_flags &= ~SDL_WINDOW_FULLSCREEN_DESKTOP;
      else
        g_win_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
      SDL_SetWindowFullscreen(g_window, g_win_flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    break;
  case kOpt_AspectRatio: {
    int idx = GetAspectIndex();
    idx = (idx + delta + kAspectCount) % kAspectCount;
    g_config.extended_aspect_ratio = kAspectValues[idx];
    break;
  }
  case kOpt_Volume: {
    int vol = (g_sdl_audio_mixer_volume * 100) / SDL_MIX_MAXVOLUME;
    vol += delta * 5;
    if (vol < 0) vol = 0;
    if (vol > 100) vol = 100;
    g_sdl_audio_mixer_volume = (vol * SDL_MIX_MAXVOLUME) / 100;
    break;
  }
  case kOpt_LinearFilter: {
    g_config.linear_filtering = !g_config.linear_filtering;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,
                g_config.linear_filtering ? "best" : "nearest");
    break;
  }
  case kOpt_StretchToFill: {
    g_config.ignore_aspect_ratio = !g_config.ignore_aspect_ratio;
    // Apply immediately for SDL mode
    extern SDL_Renderer *g_renderer;
    extern int g_snes_width, g_snes_height;
    if (g_renderer) {
      if (g_config.ignore_aspect_ratio)
        SDL_RenderSetLogicalSize(g_renderer, 0, 0);
      else
        SDL_RenderSetLogicalSize(g_renderer, g_snes_width, g_snes_height);
    }
    break;
  }
  case kOpt_Shader: {
    int idx = GetShaderIndex();
    idx = (idx + delta + kShaderCount) % kShaderCount;
    g_config.shader = kShaderPaths[idx];
    break;
  }
  case kOpt_Autosave:
    g_config.autosave = !g_config.autosave;
    break;
  case kOpt_OutputMethod: {
    int idx = 0;
    for (int i = 0; i < kOutputCount; i++)
      if (g_config.output_method == kOutputValues[i]) { idx = (i + delta + kOutputCount) % kOutputCount; break; }
    g_config.output_method = kOutputValues[idx];
    break;
  }
  }
}

static void HandleMainInput(int key_code, int key_mod, bool pressed) {
  (void)key_mod;
  if (!pressed) return;
  switch (key_code) {
  case SDLK_UP:     g_cursor = (g_cursor - 1 + kOpt_MAIN_COUNT) % kOpt_MAIN_COUNT; break;
  case SDLK_DOWN:   g_cursor = (g_cursor + 1) % kOpt_MAIN_COUNT; break;
  case SDLK_LEFT:   ChangeValue(g_cursor, -1); break;
  case SDLK_RIGHT:  ChangeValue(g_cursor, 1); break;
  case SDLK_RETURN:
  case SDLK_KP_ENTER:
    if (g_cursor == kOpt_Controls) { g_page = kPage_Controls; g_cursor = 0; }
    else if (g_cursor == kOpt_Cheats) { g_page = kPage_Cheats; g_cursor = 0; g_cheat_scroll = 0; }
    else if (g_cursor == kOpt_Close) SettingsMenu_Toggle();
    else ChangeValue(g_cursor, 1);
    break;
  }
}

// --- Controls page input ---

static void HandleControlsInput(int key_code, int key_mod, bool pressed) {
  (void)key_mod;
  if (!pressed) return;
  if (key_code == SDLK_ESCAPE || key_code == SDLK_F12 ||
      key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER) {
    g_page = kPage_Main;
    g_cursor = kOpt_Controls;
  }
}

// --- Cheats page input ---

static void HandleCheatsInput(int key_code, int key_mod, bool pressed) {
  (void)key_mod;
  if (!pressed) return;
  switch (key_code) {
  case SDLK_UP:
    do { g_cursor = (g_cursor - 1 + kCheat_COUNT) % kCheat_COUNT; }
    while (g_cursor == kCheat_Sep1 || g_cursor == kCheat_Sep2 || g_cursor == kCheat_Sep3);
    break;
  case SDLK_DOWN:
    do { g_cursor = (g_cursor + 1) % kCheat_COUNT; }
    while (g_cursor == kCheat_Sep1 || g_cursor == kCheat_Sep2 || g_cursor == kCheat_Sep3);
    break;
  case SDLK_RETURN:
  case SDLK_KP_ENTER:
    switch (g_cursor) {
    case kCheat_FillHealth: CheatFillHealth(); break;
    case kCheat_FillItems:  CheatFillItems();  break;
    case kCheat_FillKeys:   CheatFillKeys();   break;
    case kCheat_InfHealth:  g_cheat_health = !g_cheat_health; break;
    case kCheat_InfMagic:   g_cheat_magic  = !g_cheat_magic;  break;
    case kCheat_InfBombs:   g_cheat_bombs  = !g_cheat_bombs;  break;
    case kCheat_InfArrows:  g_cheat_arrows = !g_cheat_arrows; break;
    case kCheat_InfKeys:    g_cheat_keys   = !g_cheat_keys;   break;
    case kCheat_InfRupees:  g_cheat_rupees = !g_cheat_rupees; break;
    case kCheat_UnlockAll:  CheatUnlockAll(); break;
    case kCheat_PotCarry:   g_cheat_potcarry = !g_cheat_potcarry; break;
    case kCheat_Wall:       g_cheat_wall = !g_cheat_wall; CheatToggleWall(); break;
    case kCheat_Back:       g_page = kPage_Main; g_cursor = kOpt_Cheats; g_cheat_scroll = 0; break;
    }
    break;
  case SDLK_LEFT:
    // Toggle off for infinite cheats
    if (g_cursor == kCheat_InfHealth)  { g_cheat_health = false; }
    else if (g_cursor == kCheat_InfMagic)  { g_cheat_magic  = false; }
    else if (g_cursor == kCheat_InfBombs)  { g_cheat_bombs  = false; }
    else if (g_cursor == kCheat_InfArrows) { g_cheat_arrows = false; }
    else if (g_cursor == kCheat_InfKeys)   { g_cheat_keys   = false; }
    else if (g_cursor == kCheat_InfRupees) { g_cheat_rupees = false; }
    else if (g_cursor == kCheat_PotCarry)  { g_cheat_potcarry = false; }
    else if (g_cursor == kCheat_Wall)      { g_cheat_wall   = false; CheatToggleWall(); }
    break;
  case SDLK_RIGHT:
    // Toggle on for infinite cheats
    if (g_cursor == kCheat_InfHealth)  { g_cheat_health = true; }
    else if (g_cursor == kCheat_InfMagic)  { g_cheat_magic  = true; }
    else if (g_cursor == kCheat_InfBombs)  { g_cheat_bombs  = true; }
    else if (g_cursor == kCheat_InfArrows) { g_cheat_arrows = true; }
    else if (g_cursor == kCheat_InfKeys)   { g_cheat_keys   = true; }
    else if (g_cursor == kCheat_InfRupees) { g_cheat_rupees = true; }
    else if (g_cursor == kCheat_PotCarry)  { g_cheat_potcarry = true; }
    else if (g_cursor == kCheat_Wall)      { g_cheat_wall   = true; CheatToggleWall(); }
    break;
  }
}

// ====================================================================
//  PUBLIC INTERFACE
// ====================================================================

void SettingsMenu_Toggle(void) {
  if (g_settings_menu_active) {
    g_settings_menu_active = false;
    g_paused = false;
    extern void SaveConfigFile(const char *);
    SaveConfigFile(NULL);
  } else {
    g_settings_menu_active = true;
    g_paused = true;
    g_cursor = 0;
    g_page = kPage_Main;
    g_main_scroll = 0;
  }
}

void SettingsMenu_Input(int key_code, int key_mod, bool pressed) {
  if (!g_settings_menu_active) return;
  // Update check: Enter triggers update if available
  if (pressed && g_update_available && Updater_IsReady() &&
      (key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER)) {
    Updater_Apply();
    return;
  }
  // Global close
  if (pressed && (key_code == SDLK_F12 || key_code == SDLK_ESCAPE)) {
    SettingsMenu_Toggle();
    return;
  }
  switch (g_page) {
  case kPage_Main:    HandleMainInput(key_code, key_mod, pressed); break;
  case kPage_Controls: HandleControlsInput(key_code, key_mod, pressed); break;
  case kPage_Cheats:  HandleCheatsInput(key_code, key_mod, pressed); break;
  }
}

// --- Per-frame cheat application ---

void SettingsMenu_ApplyCheats(void) {
  if (g_cheat_health) {
    g_ram[0xF36D] = g_ram[0xF36C]; // health = capacity
  }
  if (g_cheat_magic) {
    g_ram[0xF373] = 0x80; // magic full
  }
  if (g_cheat_bombs && g_ram[0xF370] > 0) {
    g_ram[0xF375] = g_ram[0xF370]; // bombs = capacity
  }
  if (g_cheat_arrows && g_ram[0xF371] > 0) {
    g_ram[0xF376] = g_ram[0xF371]; // arrows = capacity
  }
  if (g_cheat_keys) {
    g_ram[0xF36F] = 1; // small keys
  }
  if (g_cheat_rupees) {
    uint16 r = *(uint16*)(g_ram + 0xF362);
    if (r < 500) {
      g_ram[0xF360] = 500 & 0xFF;
      g_ram[0xF361] = (500 >> 8) & 0xFF;
      g_ram[0xF362] = 500 & 0xFF;
      g_ram[0xF363] = (500 >> 8) & 0xFF;
    }
  }
  // Pot carry: preserve carry bit through room transitions
  if (g_cheat_potcarry) {
    static uint8 prev_hand = 0;
    uint8 cur_hand = g_ram[0x301];
    // If player had a pot lifted (bit 1 = 2) and it was just cleared by the game,
    // restore it immediately. This lets pots cross room boundaries.
    if ((prev_hand & 2) && !(cur_hand & 2))
      g_ram[0x301] = cur_hand | 2;
    prev_hand = g_ram[0x301];
  }
}
