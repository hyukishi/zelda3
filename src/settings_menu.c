#include <SDL.h>
#include "settings_menu.h"
#include "config.h"
#include "types.h"
#include "variables.h"
#include "load_gfx.h"
#include "features.h"
#include "zelda_rtl.h"
#include "snes/ppu.h"
#include "updater.h"
#include <stdio.h>
#include <stdlib.h>
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
enum { kPage_Main, kPage_Video, kPage_Audio, kPage_Game, kPage_Features, kPage_Controls, kPage_Cheats };

// --- State ---
static int g_cursor;
static int g_page;
static int g_main_scroll;
static int g_sub_scroll;
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

void DrawString(uint8 *buf, int pitch, int x, int y, const char *s, uint32 color) {
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
  kOpt_Video,
  kOpt_Audio,
  kOpt_Game,
  kOpt_Features,
  kOpt_Controls,
  kOpt_Cheats,
  kOpt_Update,
  kOpt_Close,
  kOpt_MAIN_COUNT,
};

static const char *kOptLabels[] = {
  "Video",
  "Audio",
  "Game",
  "Features",
  "Show Controls",
  "Cheats",
  "Update",
  "Close",
};

// Draw a string right-aligned so its last character lands at pixel x.
static void DrawStringRight(uint8 *buf, int pitch, int x, int y, const char *s, uint32 color) {
  DrawString(buf, pitch, x - (int)strlen(s) * kFontW, y, s, color);
}

static void DrawMainPage(uint8 *buf, int pitch, int px, int py, int pw, int fb_w, int fb_h,
                         int content_y, int content_h) {
  int lx = px + kPanelPad;
  int rx = px + pw - kPanelPad;
  int rh = kLineH + 1;

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

    // Categories (with sub-menus) get a different color and ">" indicator
    bool is_category = (i == kOpt_Video || i == kOpt_Audio || i == kOpt_Game ||
                        i == kOpt_Features || i == kOpt_Controls || i == kOpt_Cheats);
    uint32 lcol = (i == kOpt_Close) ? kCol_Close : (i == kOpt_Cheats) ? kCol_Cheat :
                  is_category ? kCol_Value : kCol_Label;
    DrawString(buf, pitch, lx, y + 2, kOptLabels[i], lcol);
    if (is_category)
      DrawStringRight(buf, pitch, rx, y, ">", kCol_Value);

    // Show Update status
    if (i == kOpt_Update) {
      char vb[48];
      if (g_update_available && Updater_IsReady())
        snprintf(vb, sizeof(vb), "v%s Ready", g_update_version ? g_update_version : "");
      else if (g_update_available)
        snprintf(vb, sizeof(vb), "Downloading...");
      else
        snprintf(vb, sizeof(vb), "Check...");
      DrawStringRight(buf, pitch, rx, y, vb, g_update_available ? kCol_Cheat : kCol_Value);
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
//  SUB-PAGE HELPERS
// ====================================================================

static const int kOutputValues[] = { 0, 2, 1, 3 };
static const int kOutputCount = 4;

static const char *kShaderNames[] = { "None", "scalefx-aa", "scalefx+AA fast", "6xBRZ", "ScaleHQ", "6xBRZ+ScaleHQ" };
static const char *kShaderPaths[] = { NULL, "glsl-shaders/presets/scalefx-aa.glslp", "glsl-shaders/presets/scalefx-aa-fast.glslp", "glsl-shaders/xbrz/6xbrz-linear.glslp", "glsl-shaders/scalehq/4xScaleHQ.glslp", "glsl-shaders/presets/6xbrz+scalehq.glslp" };
static const int kShaderCount = 6;
static const char *kFpsCornerNames[] = { "Off", "Top Left", "Top Right", "Bottom Left", "Bottom Right" };
static const char *kAspectNames[] = { "4:3", "16:9", "16:10", "18:9" };
static const int kAspectValues[] = { 0, 48, 34, 64 };
static const int kAspectCount = 4;

static int GetShaderIndex(void) {
  if (!g_config.shader) return 0;
  for (int i = 1; i < kShaderCount; i++)
    if (kShaderPaths[i] && strcmp(g_config.shader, kShaderPaths[i]) == 0) return i;
  return 0;
}
static int GetAspectIndex(void) {
  for (int i = 0; i < kAspectCount; i++)
    if (g_config.extended_aspect_ratio == kAspectValues[i]) return i;
  return 1;
}

// Generic sub-page: an array of { label, value_string } with a Back item at the end.
typedef struct {
  const char *label;
  const char *value;
} SubPageItem;

static void DrawSubPage(uint8 *buf, int pitch, int px, int py, int pw, int fb_w, int fb_h,
                        int content_y, int content_h, const char *title,
                        const SubPageItem *items, int item_count) {
  int lx = px + kPanelPad;
  int rx = px + pw - kPanelPad;  // right edge for value anchoring
  int rh = kLineH + 1;

  int lines_fit = (content_h > 0) ? content_h / rh : 1;
  if (lines_fit < 1) lines_fit = 1;

  // Clamp scroll
  if (g_cursor >= 0 && g_cursor < item_count) {
    if (g_cursor < g_sub_scroll)
      g_sub_scroll = g_cursor;
    else if (g_cursor >= g_sub_scroll + lines_fit)
      g_sub_scroll = g_cursor - lines_fit + 1;
  }
  if (g_sub_scroll > item_count - lines_fit)
    g_sub_scroll = (item_count > lines_fit) ? item_count - lines_fit : 0;
  if (g_sub_scroll < 0) g_sub_scroll = 0;

  // Title
  DrawString(buf, pitch, lx, content_y - kFontH - 4, title, kCol_Title);
  DrawRectSafe(buf, pitch, px + kPanelPad, content_y - 2, pw - kPanelPad * 2, 1,
               kCol_Sep, fb_w, fb_h);

  int y = content_y;
  for (int i = g_sub_scroll; i < item_count && i < g_sub_scroll + lines_fit; i++) {
    int sel = (i == g_cursor);
    if (sel)
      DrawRectSafe(buf, pitch, px + kPanelPad, y, pw - kPanelPad * 2, kLineH, kCol_Highlight, fb_w, fb_h);
    if (sel)
      DrawChar(buf, pitch, lx - kFontW - 2, y + 2, '>', kCol_HiAccent);

    bool is_back = (i == item_count - 1);
    uint32 lcol = is_back ? kCol_Close : kCol_Label;
    DrawString(buf, pitch, lx, y + 2, items[i].label, lcol);
    if (items[i].value) {
      bool on = (items[i].value[0] == 'O' && items[i].value[1] == 'N');
      DrawStringRight(buf, pitch, rx, y, items[i].value, on ? kCol_On : kCol_Value);
    }
    y += rh;
  }

  bool scroll_up = (g_sub_scroll > 0);
  bool scroll_dn = (g_sub_scroll + lines_fit < item_count);
  if (scroll_up)
    DrawString(buf, pitch, lx, content_y - kFontH - 14, "^", kCol_Back);
  if (scroll_dn)
    DrawString(buf, pitch, lx, content_y + content_h - 10, "v", kCol_Back);
}

// --- Video sub-page ---

enum {
  kOptV_WindowScale, kOptV_Fullscreen, kOptV_AspectRatio, kOptV_StretchToFill,
  kOptV_LinearFilter, kOptV_FpsCounter, kOptV_Vsync, kOptV_OutputMethod,
  kOptV_Shader, kOptV_UpdateShaders, kOptV_Back, kOptV_COUNT
};

static void DrawVideoPage(uint8 *buf, int pitch, int px, int py, int pw, int fb_w, int fb_h,
                          int content_y, int content_h) {
  char vb[64];
  char vs[12], vf[12], va[40], vsf[12], vlf[12], vfp[12], vvs[12], vom[12], vsh[40];
  snprintf(vs, sizeof(vs), "%dx", g_current_window_scale);
  snprintf(vf, sizeof(vf), "%s", (g_win_flags & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN)) ? "ON" : "OFF");
  snprintf(va, sizeof(va), "%s", kAspectNames[GetAspectIndex()]);
  snprintf(vsf, sizeof(vsf), "%s", g_config.ignore_aspect_ratio ? "ON" : "OFF");
  snprintf(vlf, sizeof(vlf), "%s", g_config.linear_filtering ? "ON" : "OFF");
  snprintf(vfp, sizeof(vfp), "%s", kFpsCornerNames[g_config.fps_counter]);
  snprintf(vvs, sizeof(vvs), "%s", g_config.vsync ? "ON" : "OFF");
  int om = g_config.output_method;
  snprintf(vom, sizeof(vom), "%s", om == 2 ? "OpenGL" : om == 1 ? "SDL-SW" : om == 3 ? "GL-ES" : "SDL");
  snprintf(vsh, sizeof(vsh), "%s", kShaderNames[GetShaderIndex()]);

  SubPageItem items[] = {
    {"Window Scale", vs}, {"Fullscreen", vf}, {"Aspect Ratio", va},
    {"Stretch to Fill", vsf}, {"Linear Filter", vlf}, {"FPS Counter", vfp},
    {"Vsync", vvs}, {"Output Method", vom}, {"Shader", vsh},
    {"Update Shaders", "Run..."}, {"Back", NULL},
  };
  DrawSubPage(buf, pitch, px, py, pw, fb_w, fb_h, content_y, content_h,
              "Video", items, kOptV_COUNT);
}

// --- Audio sub-page ---

enum { kOptA_Volume, kOptA_EnableAudio, kOptA_Back, kOptA_COUNT };

static void DrawAudioPage(uint8 *buf, int pitch, int px, int py, int pw, int fb_w, int fb_h,
                          int content_y, int content_h) {
  int vol = (g_sdl_audio_mixer_volume * 100) / SDL_MIX_MAXVOLUME;
  char vv[12], vea[12];
  snprintf(vv, sizeof(vv), "%d%%", vol);
  snprintf(vea, sizeof(vea), "%s", g_config.enable_audio ? "ON" : "OFF");

  SubPageItem items[] = {
    {"Volume", vv}, {"Enable Audio", vea}, {"Back", NULL},
  };
  DrawSubPage(buf, pitch, px, py, pw, fb_w, fb_h, content_y, content_h,
              "Audio", items, kOptA_COUNT);
  // Volume bar — drawn to the left of the right-aligned value text
  int rx = px + pw - kPanelPad;
  int y = content_y;
  int rh = kLineH + 1;
  for (int i = g_sub_scroll; i < kOptA_COUNT && i < g_sub_scroll + (content_h / rh); i++) {
    if (i == kOptA_Volume) {
      int bw = 40;
      int bx = rx - bw - 6 - (int)strlen(vv) * kFontW;  // bar to the left of value
      int fill = (vol * bw) / 100;
      DrawRectSafe(buf, pitch, bx, y + 2, bw, 5, kCol_BarBg, fb_w, fb_h);
      if (fill > 0)
        DrawRectSafe(buf, pitch, bx, y + 2, fill, 5, kCol_BarFill, fb_w, fb_h);
    }
    y += rh;
  }
}

// --- Game sub-page ---

enum {
  kOptG_EnhancedMode7, kOptG_NewRenderer, kOptG_ExtendY, kOptG_NoSpriteLimits,
  kOptG_Autosave, kOptG_DisplayPerf, kOptG_DisableFrameDelay, kOptG_Back, kOptG_COUNT
};

static void DrawGamePage(uint8 *buf, int pitch, int px, int py, int pw, int fb_w, int fb_h,
                         int content_y, int content_h) {
  char on[4] = "ON", off[4] = "OFF";
  SubPageItem items[] = {
    {"Enhanced Mode7", g_config.enhanced_mode7 ? on : off},
    {"New Renderer", g_config.new_renderer ? on : off},
    {"Extend Y (240p)", g_config.extend_y ? on : off},
    {"No Sprite Limits", g_config.no_sprite_limits ? on : off},
    {"Autosave", g_config.autosave ? on : off},
    {"Display Perf", g_config.display_perf_title ? on : off},
    {"Disable Frame Delay", g_config.disable_frame_delay ? on : off},
    {"Back", NULL},
  };
  DrawSubPage(buf, pitch, px, py, pw, fb_w, fb_h, content_y, content_h,
              "Game", items, kOptG_COUNT);
}

// --- Features sub-page ---

enum {
  kOptF_SwitchLR, kOptF_TurnWhileDashing, kOptF_MiscBugFixes,
  kOptF_Back, kOptF_COUNT
};

static void DrawFeaturesPage(uint8 *buf, int pitch, int px, int py, int pw, int fb_w, int fb_h,
                             int content_y, int content_h) {
  char on[4] = "ON", off[4] = "OFF";
  SubPageItem items[] = {
    {"Switch LR", (g_config.features0 & kFeatures0_SwitchLR) ? on : off},
    {"Turn While Dashing", (g_config.features0 & kFeatures0_TurnWhileDashing) ? on : off},
    {"Misc Bug Fixes", (g_config.features0 & kFeatures0_MiscBugFixes) ? on : off},
    {"Back", NULL},
  };
  DrawSubPage(buf, pitch, px, py, pw, fb_w, fb_h, content_y, content_h,
              "Features", items, kOptF_COUNT);
}

// ====================================================================
//  CONTROLS PAGE
// ====================================================================

// Build readable key name from internal keycode (reverse of REMAP_SDL_KEYCODE)
static void KeyName(uint16 key, char *out, int out_size) {
  out[0] = 0;
  if (key & kKeyMod_Ctrl)  strcat(out, "Ctrl+");
  if (key & kKeyMod_Shift) strcat(out, "Shft+");
  if (key & kKeyMod_Alt)   strcat(out, "Alt+");
  int code = key & (kKeyMod_ScanCode - 1);
  SDL_Keycode kc = code;
  const char *n = SDL_GetKeyName(kc);
  if (!n || !*n) n = "???";
  strncat(out, n, out_size - strlen(out) - 1);
}

enum {
  kCtrl_ResetDefaults,
  kCtrl_Back,
  kCtrl_COUNT
};

enum { kCtrl_ActionCount = 16 };  // the 16 game key bindings

static const struct { const char *act; int kid; } kCtrlActions[] = {
  {"Up",kKeys_Controls},{"Down",kKeys_Controls+1},{"Left",kKeys_Controls+2},{"Right",kKeys_Controls+3},
  {"Select",kKeys_Controls+4},{"Start",kKeys_Controls+5},{"A",kKeys_Controls+6},{"B",kKeys_Controls+7},
  {"X",kKeys_Controls+8},{"Y",kKeys_Controls+9},{"L",kKeys_Controls+10},{"R",kKeys_Controls+11},
  {"Fullscreen",kKeys_Fullscreen},{"Pause",kKeys_Pause},{"Turbo",kKeys_Turbo},
  {"Settings",kKeys_Settings},
};

// Total items = 16 actions + reset + back = 18
enum { kCtrl_TotalItems = kCtrl_ActionCount + 2 };

// Waiting-for-key state
static bool g_ctrl_waiting;
static int  g_ctrl_wait_cmd;

// Pot carry cheat: shared between pre-frame and post-frame
static bool g_potcarry_was_lifted;
// RAM flag used by Ancilla_TerminateSelectInteractives to preserve the
// picked-up ancilla across room transitions when pot carry cheat is active.
#define kRam_PotCarryPreserveAncilla 0x647

static void DrawControlsPage(uint8 *buf, int pitch, int px, int py, int pw, int fb_w, int fb_h,
                             int content_y, int content_h) {
  int lx = px + kPanelPad;
  int rx = px + pw - kPanelPad;
  int rh = kLineH + 2;
  int lines_fit = (content_h > 0) ? content_h / rh : 1;
  if (lines_fit < 1) lines_fit = 1;

  // Clamp scroll
  if (g_cursor >= 0 && g_cursor < kCtrl_TotalItems) {
    if (g_cursor < g_sub_scroll)
      g_sub_scroll = g_cursor;
    else if (g_cursor >= g_sub_scroll + lines_fit)
      g_sub_scroll = g_cursor - lines_fit + 1;
  }
  if (g_sub_scroll > kCtrl_TotalItems - lines_fit)
    g_sub_scroll = (kCtrl_TotalItems > lines_fit) ? kCtrl_TotalItems - lines_fit : 0;
  if (g_sub_scroll < 0) g_sub_scroll = 0;

  // Title
  DrawString(buf, pitch, lx, content_y - kFontH - 4, "Controls", kCol_Title);
  DrawRectSafe(buf, pitch, px + kPanelPad, content_y - 2, pw - kPanelPad * 2, 1,
               kCol_Sep, fb_w, fb_h);

  int y = content_y;
  for (int vi = g_sub_scroll; vi < kCtrl_TotalItems && vi < g_sub_scroll + lines_fit; vi++) {
    if (vi < kCtrl_ActionCount) {
      // Action row
      int kid = kCtrlActions[vi].kid;
      uint16 key = GetKeyForCmd(kid);
      char kn[40] = {0};
      if (g_ctrl_waiting && g_ctrl_wait_cmd == kid)
        snprintf(kn, sizeof(kn), "Press key...");
      else if (key)
        KeyName(key, kn, sizeof(kn));
      else
        snprintf(kn, sizeof(kn), "(unbound)");

      int sel = (vi == g_cursor);
      if (sel)
        DrawRectSafe(buf, pitch, px + kPanelPad, y, pw - kPanelPad * 2, kLineH, kCol_Highlight, fb_w, fb_h);
      if (sel)
        DrawChar(buf, pitch, lx - kFontW - 2, y + 3, '>', kCol_HiAccent);
      DrawString(buf, pitch, lx, y + 3, kCtrlActions[vi].act, kCol_Label);
      bool waiting = (g_ctrl_waiting && g_ctrl_wait_cmd == kid);
      DrawStringRight(buf, pitch, rx, y + 3, kn, waiting ? kCol_Cheat : kCol_Value);
    } else if (vi == kCtrl_ActionCount) {
      // Reset to defaults
      int sel = (vi == g_cursor);
      if (sel)
        DrawRectSafe(buf, pitch, px + kPanelPad, y, pw - kPanelPad * 2, kLineH, kCol_Highlight, fb_w, fb_h);
      if (sel)
        DrawChar(buf, pitch, lx - kFontW - 2, y + 3, '>', kCol_HiAccent);
      DrawString(buf, pitch, lx, y + 3, "Reset to Defaults", kCol_Action);
    } else {
      // Back
      int sel = (vi == g_cursor);
      if (sel)
        DrawRectSafe(buf, pitch, px + kPanelPad, y, pw - kPanelPad * 2, kLineH, kCol_Highlight, fb_w, fb_h);
      if (sel)
        DrawChar(buf, pitch, lx - kFontW - 2, y + 3, '>', kCol_HiAccent);
      DrawString(buf, pitch, lx, y + 3, "Back", kCol_Close);
    }
    y += rh;
  }

  bool scroll_up = (g_sub_scroll > 0);
  bool scroll_dn = (g_sub_scroll + lines_fit < kCtrl_TotalItems);
  if (scroll_up)
    DrawString(buf, pitch, lx, content_y - kFontH - 14, "^", kCol_Back);
  if (scroll_dn)
    DrawString(buf, pitch, lx, content_y + content_h - 10, "v", kCol_Back);
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
  int rx = px + pw - kPanelPad;
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
      DrawStringRight(buf, pitch, rx, y + 3, items[i].val, on ? kCol_On : kCol_Off);
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
    for (int i = 0; i < kCheat_COUNT; i++) {
      if (i == kCheat_Sep1 || i == kCheat_Sep2 || i == kCheat_Sep3) sep_count++;
    }
  } else if (g_page == kPage_Video) {
    item_count = kOptV_COUNT;
  } else if (g_page == kPage_Audio) {
    item_count = kOptA_COUNT;
  } else if (g_page == kPage_Game) {
    item_count = kOptG_COUNT;
  } else if (g_page == kPage_Features) {
    item_count = kOptF_COUNT;
  } else if (g_page == kPage_Controls) {
    item_count = kCtrl_TotalItems;
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
  int tx = px + (pw - 8 * 7) / 2;
  DrawString(buf, pitch, tx, py + kPanelPad, "SETTINGS", kCol_Title);
  DrawRectSafe(buf, pitch, px + kPanelPad, py + kPanelPad + kFontH + 2,
               pw - kPanelPad * 2, 1, 0xFF555577, fb_w, fb_h);

  // Page content
  int content_y = py + 7 * kFontH + 8;
  int content_h = py + ph - kPanelPad - kFontH - 4 - content_y;
  switch (g_page) {
  case kPage_Main:
    DrawMainPage(buf, pitch, px, py, pw, fb_w, fb_h, content_y, content_h);
    break;
  case kPage_Video:
    DrawVideoPage(buf, pitch, px, py, pw, fb_w, fb_h, content_y, content_h);
    break;
  case kPage_Audio:
    DrawAudioPage(buf, pitch, px, py, pw, fb_w, fb_h, content_y, content_h);
    break;
  case kPage_Game:
    DrawGamePage(buf, pitch, px, py, pw, fb_w, fb_h, content_y, content_h);
    break;
  case kPage_Features:
    DrawFeaturesPage(buf, pitch, px, py, pw, fb_w, fb_h, content_y, content_h);
    break;
  case kPage_Controls:
    DrawControlsPage(buf, pitch, px, py, pw, fb_w, fb_h, content_y, content_h);
    break;
  case kPage_Cheats:
    DrawCheatsPage(buf, pitch, px, py, pw, fb_w, fb_h, content_y, content_h);
    break;
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

static void ToggleBool(bool *val) { *val = !*val; }
static void ToggleFeaturesBit(uint32 bit) {
  g_config.features0 ^= bit;
  g_wanted_zelda_features ^= bit;
}

// --- Main page input ---

static void HandleMainInput(int key_code, int key_mod, bool pressed) {
  (void)key_mod;
  if (!pressed) return;
  switch (key_code) {
  case SDLK_UP:    g_cursor = (g_cursor - 1 + kOpt_MAIN_COUNT) % kOpt_MAIN_COUNT; break;
  case SDLK_DOWN:  g_cursor = (g_cursor + 1) % kOpt_MAIN_COUNT; break;
  case SDLK_RETURN:
  case SDLK_KP_ENTER:
    switch (g_cursor) {
    case kOpt_Video:    g_page = kPage_Video;    g_cursor = 0; g_sub_scroll = 0; break;
    case kOpt_Audio:    g_page = kPage_Audio;    g_cursor = 0; g_sub_scroll = 0; break;
    case kOpt_Game:     g_page = kPage_Game;     g_cursor = 0; g_sub_scroll = 0; break;
    case kOpt_Features: g_page = kPage_Features; g_cursor = 0; g_sub_scroll = 0; break;
    case kOpt_Controls: g_page = kPage_Controls; g_cursor = 0; g_sub_scroll = 0; g_ctrl_waiting = false; break;
    case kOpt_Cheats:   g_page = kPage_Cheats;   g_cursor = 0; g_cheat_scroll = 0; break;
    case kOpt_Update:
      if (g_update_available && Updater_IsReady()) Updater_Apply();
      else { Updater_Check(); g_update_available = false; }
      break;
    case kOpt_Close: SettingsMenu_Toggle(); break;
    }
    break;
  }
}

// --- Generic sub-page input handler ---

static void HandleSubPageInput(int key_code, int key_mod, bool pressed,
                                int back_page, int back_cursor, int item_count,
                                void (*on_enter)(int cursor), void (*on_change)(int cursor, int delta)) {
  (void)key_mod;
  if (!pressed) return;
  switch (key_code) {
  case SDLK_UP:    g_cursor = (g_cursor - 1 + item_count) % item_count; break;
  case SDLK_DOWN:  g_cursor = (g_cursor + 1) % item_count; break;
  case SDLK_LEFT:  if (on_change) on_change(g_cursor, -1); break;
  case SDLK_RIGHT: if (on_change) on_change(g_cursor, 1); break;
  case SDLK_RETURN:
  case SDLK_KP_ENTER:
    if (g_cursor == item_count - 1) { // Back
      g_page = back_page; g_cursor = back_cursor; g_sub_scroll = 0;
    } else if (on_enter) {
      on_enter(g_cursor);
    }
    break;
  }
}

// --- Video sub-page handlers ---

static void VideoChange(int opt, int delta) {
  switch (opt) {
  case kOptV_WindowScale: {
    int ns = g_current_window_scale + delta;
    if (ns >= 1 && ns <= 10)
      for (int i = 0; i < (delta > 0 ? delta : -delta); i++)
        ChangeWindowScale(delta > 0 ? 1 : -1);
    break;
  }
  case kOptV_Fullscreen:
    if (delta > 0) {
      if (g_win_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        g_win_flags &= ~SDL_WINDOW_FULLSCREEN_DESKTOP;
        g_config.fullscreen = 0;
      } else {
        g_win_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        g_config.fullscreen = 1;
      }
      SDL_SetWindowFullscreen(g_window, g_win_flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    break;
  case kOptV_AspectRatio: {
    int idx = GetAspectIndex();
    idx = (idx + delta + kAspectCount) % kAspectCount;
    g_config.extended_aspect_ratio = kAspectValues[idx];
    break;
  }
  case kOptV_StretchToFill: {
    ToggleBool(&g_config.ignore_aspect_ratio);
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
  case kOptV_LinearFilter:
    ToggleBool(&g_config.linear_filtering);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, g_config.linear_filtering ? "best" : "nearest");
    break;
  case kOptV_FpsCounter:
    g_config.fps_counter = (g_config.fps_counter + delta + 5) % 5;
    break;
  case kOptV_Vsync:
    ToggleBool(&g_config.vsync);
    break;
  case kOptV_OutputMethod: {
    int idx = 0;
    for (int i = 0; i < kOutputCount; i++)
      if (g_config.output_method == kOutputValues[i]) { idx = (i + delta + kOutputCount) % kOutputCount; break; }
    g_config.output_method = kOutputValues[idx];
    break;
  }
  case kOptV_Shader: {
    int idx = GetShaderIndex();
    idx = (idx + delta + kShaderCount) % kShaderCount;
    g_config.shader = kShaderPaths[idx];
    break;
  }
  case kOptV_UpdateShaders:
    if (delta > 0) system("./fetch-shaders.sh &");
    break;
  }
}

static void HandleVideoInput(int key_code, int key_mod, bool pressed) {
  HandleSubPageInput(key_code, key_mod, pressed, kPage_Main, kOpt_Video, kOptV_COUNT, NULL, VideoChange);
}

// --- Audio sub-page handlers ---

static void AudioChange(int opt, int delta) {
  switch (opt) {
  case kOptA_Volume: {
    int vol = (g_sdl_audio_mixer_volume * 100) / SDL_MIX_MAXVOLUME;
    vol += delta * 5;
    if (vol < 0) vol = 0;
    if (vol > 100) vol = 100;
    g_sdl_audio_mixer_volume = (vol * SDL_MIX_MAXVOLUME) / 100;
    break;
  }
  case kOptA_EnableAudio:
    ToggleBool(&g_config.enable_audio);
    break;
  }
}

static void HandleAudioInput(int key_code, int key_mod, bool pressed) {
  HandleSubPageInput(key_code, key_mod, pressed, kPage_Main, kOpt_Audio, kOptA_COUNT, NULL, AudioChange);
}

// --- Game sub-page handlers ---

static void GameChange(int opt, int delta) {
  (void)delta;
  switch (opt) {
  case kOptG_EnhancedMode7:   ToggleBool(&g_config.enhanced_mode7); break;
  case kOptG_NewRenderer:     ToggleBool(&g_config.new_renderer); break;
  case kOptG_ExtendY:         ToggleBool(&g_config.extend_y); break;
  case kOptG_NoSpriteLimits:  ToggleBool(&g_config.no_sprite_limits); break;
  case kOptG_Autosave:        ToggleBool(&g_config.autosave); break;
  case kOptG_DisplayPerf:     ToggleBool(&g_config.display_perf_title); break;
  case kOptG_DisableFrameDelay: ToggleBool(&g_config.disable_frame_delay); break;
  }
}

static void HandleGameInput(int key_code, int key_mod, bool pressed) {
  HandleSubPageInput(key_code, key_mod, pressed, kPage_Main, kOpt_Game, kOptG_COUNT, NULL, GameChange);
}

// --- Features sub-page handlers ---

static void FeaturesChange(int opt, int delta) {
  (void)delta;
  switch (opt) {
  case kOptF_SwitchLR:         ToggleFeaturesBit(kFeatures0_SwitchLR); break;
  case kOptF_TurnWhileDashing: ToggleFeaturesBit(kFeatures0_TurnWhileDashing); break;
  case kOptF_MiscBugFixes:     ToggleFeaturesBit(kFeatures0_MiscBugFixes); break;
  }
}

static void HandleFeaturesInput(int key_code, int key_mod, bool pressed) {
  HandleSubPageInput(key_code, key_mod, pressed, kPage_Main, kOpt_Features, kOptF_COUNT, NULL, FeaturesChange);
}

// --- Controls page input ---

static void HandleControlsInput(int key_code, int key_mod, bool pressed) {
  if (!pressed) return;

  // If waiting for a key rebind
  if (g_ctrl_waiting) {
    if (key_code == SDLK_ESCAPE) {
      g_ctrl_waiting = false;
      return;
    }
    // Build internal key from pressed key + modifiers
    int new_key = 0;
    if (key_mod & KMOD_ALT)   new_key |= kKeyMod_Alt;
    if (key_mod & KMOD_CTRL)  new_key |= kKeyMod_Ctrl;
    if (key_mod & KMOD_SHIFT) new_key |= kKeyMod_Shift;
    // Don't bind meta-only keys (modifier keys alone)
    if (key_code == SDLK_LALT || key_code == SDLK_RALT ||
        key_code == SDLK_LCTRL || key_code == SDLK_RCTRL ||
        key_code == SDLK_LSHIFT || key_code == SDLK_RSHIFT) {
      return;
    }
    new_key |= REMAP_SDL_KEYCODE(key_code);
    // Remove old binding for this command
    RemoveKeyForCmd(g_ctrl_wait_cmd);
    // If target key is already used by another command, free it first
    int other_cmd = FindCmdForSdlKey(key_code, key_mod);
    if (other_cmd && other_cmd != g_ctrl_wait_cmd)
      RemoveKeyForCmd(other_cmd);
    KeyMapHash_Add(new_key, g_ctrl_wait_cmd);
    g_ctrl_waiting = false;
    return;
  }

  switch (key_code) {
  case SDLK_UP:
    g_cursor = (g_cursor - 1 + kCtrl_TotalItems) % kCtrl_TotalItems;
    break;
  case SDLK_DOWN:
    g_cursor = (g_cursor + 1) % kCtrl_TotalItems;
    break;
  case SDLK_RETURN:
  case SDLK_KP_ENTER:
    if (g_cursor >= 0 && g_cursor < kCtrl_ActionCount) {
      // Start rebinding this action
      g_ctrl_waiting = true;
      g_ctrl_wait_cmd = kCtrlActions[g_cursor].kid;
    } else if (g_cursor == kCtrl_ActionCount) {
      // Reset to defaults
      RegisterDefaultKeys();
    } else {
      // Back
      g_page = kPage_Main;
      g_cursor = kOpt_Controls;
      g_sub_scroll = 0;
    }
    break;
  case SDLK_F12:
  case SDLK_ESCAPE:
    g_ctrl_waiting = false;
    g_page = kPage_Main;
    g_cursor = kOpt_Controls;
    g_sub_scroll = 0;
    break;
  case SDLK_DELETE:
  case SDLK_BACKSPACE:
    // Unbind: remove the current key
    if (g_cursor >= 0 && g_cursor < kCtrl_ActionCount) {
      RemoveKeyForCmd(kCtrlActions[g_cursor].kid);
    }
    break;
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
  // Escape goes back to main from sub-pages, closes from main
  if (pressed && (key_code == SDLK_F12 || key_code == SDLK_ESCAPE)) {
    if (g_page == kPage_Main)
      SettingsMenu_Toggle();
    else
      { g_page = kPage_Main; g_cursor = 0; g_sub_scroll = 0; }
    return;
  }
  switch (g_page) {
  case kPage_Main:     HandleMainInput(key_code, key_mod, pressed); break;
  case kPage_Video:    HandleVideoInput(key_code, key_mod, pressed); break;
  case kPage_Audio:    HandleAudioInput(key_code, key_mod, pressed); break;
  case kPage_Game:     HandleGameInput(key_code, key_mod, pressed); break;
  case kPage_Features: HandleFeaturesInput(key_code, key_mod, pressed); break;
  case kPage_Controls: HandleControlsInput(key_code, key_mod, pressed); break;
  case kPage_Cheats:   HandleCheatsInput(key_code, key_mod, pressed); break;
  }
}

// --- Per-frame cheat application ---

void SettingsMenu_ApplyCheats(void) {
  if (g_cheat_health) {
    g_ram[0xF36D] = g_ram[0xF36C]; // current = capacity
  }
  if (g_cheat_magic) {
    g_ram[0xF373] = 0x80; // magic full
  }
  // link_item_bombs (0xF343) is the real bomb count. Keep it pinned
  // at the max for the current upgrade level so it never decreases.
  if (g_cheat_bombs) {
    g_ram[0xF343] = kMaxBombsForLevel[g_ram[0xF370]];
  }
  // link_num_arrows (0xF377) is the real arrow count.
  if (g_cheat_arrows) {
    g_ram[0xF377] = kMaxArrowsForLevel[g_ram[0xF371]];
  }
  if (g_cheat_keys) {
    g_ram[0xF36F] = 1; // small keys
  }
  if (g_cheat_rupees) {
    uint16 r = *(uint16*)(g_ram + 0xF362);
    if (r < 999) {
      g_ram[0xF360] = 999 & 0xFF;
      g_ram[0xF361] = (999 >> 8) & 0xFF;
      g_ram[0xF362] = 999 & 0xFF;
      g_ram[0xF363] = (999 >> 8) & 0xFF;
    }
  }
  // Pot carry: restore hand state after room transitions.
  // The ancilla was preserved by Ancilla_TerminateSelectInteractives
  // checking kRam_PotCarryPreserveAncilla.
  if (g_cheat_potcarry) {
    uint8 cur_hand = g_ram[0x301];
    if (g_potcarry_was_lifted && !(cur_hand & 2) && link_z_coord == 0)
      g_ram[0x301] = cur_hand | 2;
    g_potcarry_was_lifted = (g_ram[0x301] & 2) ? 1 : 0;
    // Clear the preserve-ancilla flag for this frame
    g_ram[kRam_PotCarryPreserveAncilla] = 0;
  }
}

// Called BEFORE ZeldaRunFrame to hide pot state from the game
void SettingsMenu_PreFrameCheats(void) {
  if (g_cheat_config.pot_carry) {
    uint8 cur = g_ram[0x301];
    // If player is holding a pot, clear link_item_in_hand so the game's
    // door-transition code doesn't see it and block the door. Set a RAM
    // flag so Ancilla_TerminateSelectInteractives preserves the ancilla
    // across the room transition.
    if (cur & 2) {
      g_ram[0x301] = cur & ~2;
      g_ram[kRam_PotCarryPreserveAncilla] = 1;
      g_potcarry_was_lifted = true;
    }
  }
}
