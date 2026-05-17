#include "updater.h"
#include "settings_menu.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef CURRENT_VERSION
#define CURRENT_VERSION  "v0.0.0-unknown"
#endif
#define REPO_API        "https://api.github.com/repos/hyukishi/zelda3/releases/latest"

bool g_update_available;
const char *g_update_version;
bool g_update_notify_active;

static char g_download_url[512];
static char g_staged_path[512];
static char g_new_version[32];
static uint32 g_notify_start_time;
static SDL_Thread *g_update_thread;

// Minimal JSON string value extractor: find "key":"value" and copy value (no malloc)
static bool json_str(const char *json, const char *key, char *out, int outsz) {
  char search[64];
  snprintf(search, sizeof(search), "\"%s\":\"", key);
  const char *p = strstr(json, search);
  if (!p) return false;
  p += strlen(search);
  int i = 0;
  while (*p && *p != '"' && i < outsz - 1)
    out[i++] = *p++;
  out[i] = 0;
  return true;
}

// Find browser_download_url for the platform-appropriate asset in the JSON assets array.
// For AppImage: look for "zelda3-linux-x86_64.AppImage"
// For macOS: look for "zelda3" (but not "AppImage" or ".ini")
static bool find_asset_url(const char *json, char *url, int urlsz) {
#if defined(__APPLE__)
  // macOS: the binary is named "zelda3" — find the macOS artifact
  const char *pat = "\"browser_download_url\":\"";
  const char *p = json;
  while ((p = strstr(p, pat)) != NULL) {
    p += strlen(pat);
    // Skip if this URL contains "AppImage" or ".ini" or "source"
    const char *end = strchr(p, '"');
    if (!end) break;
    int len = end - p;
    if (len < urlsz - 1) {
      char tmp[512];
      memcpy(tmp, p, len);
      tmp[len] = 0;
      // macOS artifact is just "zelda3" — it's the binary
      if (strstr(tmp, "zelda3-macos") || (strstr(tmp, "zelda3") && !strstr(tmp, "AppImage") && !strstr(tmp, ".ini") && !strstr(tmp, "source"))) {
        memcpy(url, p, len);
        url[len] = 0;
        return true;
      }
    }
  }
#else
  // Linux: find the AppImage URL
  const char *pat = "\"browser_download_url\":\"";
  const char *p = json;
  while ((p = strstr(p, pat)) != NULL) {
    p += strlen(pat);
    const char *end = strchr(p, '"');
    if (!end) break;
    int len = end - p;
    if (len < urlsz - 1) {
      char tmp[512];
      memcpy(tmp, p, len);
      tmp[len] = 0;
      if (strstr(tmp, ".AppImage")) {
        memcpy(url, p, len);
        url[len] = 0;
        return true;
      }
    }
  }
#endif
  return false;
}

// Background thread: fetch latest release info without blocking the main loop
static int Updater_Thread(void *data) {
  (void)data;
  char cmd[512];
  snprintf(cmd, sizeof(cmd), "curl -sL %s 2>/dev/null", REPO_API);

  FILE *f = popen(cmd, "r");
  if (!f) return 0;

  char json[8192] = {0};
  int len = 0;
  while (len < (int)sizeof(json) - 1) {
    int c = fgetc(f);
    if (c == EOF) break;
    json[len++] = (char)c;
  }
  json[len] = 0;
  pclose(f);

  if (len < 10) return 0;

  char latest[32];
  if (!json_str(json, "tag_name", latest, sizeof(latest)))
    return 0;

  if (strcmp(latest, CURRENT_VERSION) <= 0)
    return 0;

  char url[512];
  if (!find_asset_url(json, url, sizeof(url)))
    return 0;

  strncpy(g_new_version, latest, sizeof(g_new_version) - 1);
  strncpy(g_download_url, url, sizeof(g_download_url) - 1);
  g_update_version = g_new_version;
  g_update_available = true;
  g_update_notify_active = true;
  g_notify_start_time = SDL_GetTicks();

  return 0;
}

void Updater_StartBackgroundCheck(void) {
  g_update_thread = SDL_CreateThread(Updater_Thread, "Updater", NULL);
  if (!g_update_thread)
    fprintf(stderr, "Warning: Could not create updater thread\n");
}

void Updater_DrawNotify(uint8 *pixel_buffer, int pitch, int fb_w, int fb_h) {
  if (!g_update_notify_active || !g_update_version)
    return;

  if (SDL_GetTicks() - g_notify_start_time > 15000) {
    g_update_notify_active = false;
    return;
  }

  // Semi-transparent banner at top of screen
  int banner_h = 28;
  uint32 bg = 0xCC1A1A2E;
  int stride = pitch / 4;
  for (int row = 0; row < banner_h; row++) {
    uint32 *line = (uint32 *)pixel_buffer + row * stride;
    for (int col = 0; col < fb_w; col++) {
      uint32 p = line[col];
      uint8 a = bg >> 24, inv = 255 - a;
      uint8 r = (((bg >> 16) & 0xFF) * a + ((p >> 16) & 0xFF) * inv) / 255;
      uint8 g = (((bg >> 8) & 0xFF) * a + ((p >> 8) & 0xFF) * inv) / 255;
      uint8 b = ((bg & 0xFF) * a + (p & 0xFF) * inv) / 255;
      line[col] = (0xFF << 24) | (r << 16) | (g << 8) | b;
    }
  }

  char buf[128];
  snprintf(buf, sizeof(buf), "%s available! Enter to update, Esc to dismiss", g_update_version);
  int text_w = (int)strlen(buf) * 9;
  int tx = (fb_w - text_w) / 2;
  int ty = (banner_h - 8) / 2;
  if (tx < 0) tx = 0;
  DrawString(pixel_buffer, pitch, tx, ty, buf, 0xFF40FF80);
}

void Updater_NotifyInput(int key_code, bool pressed) {
  if (!g_update_notify_active || !pressed)
    return;

  if (key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER) {
    g_update_notify_active = false;
    snprintf(g_staged_path, sizeof(g_staged_path), "/tmp/zelda3_update_%s", g_new_version);
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
      "curl -sL '%s' -o '%s' 2>/dev/null", g_download_url, g_staged_path);
    int ret = system(cmd);
    if (ret == 0 && Updater_IsReady())
      Updater_Apply();
    return;
  }

  if (key_code == SDLK_ESCAPE) {
    g_update_notify_active = false;
    return;
  }
}

void Updater_Check(void) {
  // Fetch latest release info from GitHub
  char cmd[512];
  snprintf(cmd, sizeof(cmd),
    "curl -sL %s 2>/dev/null", REPO_API);

  FILE *f = popen(cmd, "r");
  if (!f) return;

  char json[8192] = {0};
  int len = 0;
  while (len < (int)sizeof(json) - 1) {
    int c = fgetc(f);
    if (c == EOF) break;
    json[len++] = (char)c;
  }
  json[len] = 0;
  pclose(f);

  if (len < 10) return;

  // Get latest version tag
  char latest[32];
  if (!json_str(json, "tag_name", latest, sizeof(latest)))
    return;

  // Compare versions
  if (strcmp(latest, CURRENT_VERSION) <= 0)
    return; // already up to date

  // Find the download URL for our platform
  char url[512];
  if (!find_asset_url(json, url, sizeof(url)))
    return;

  // New version available — store info
  strncpy(g_new_version, latest, sizeof(g_new_version) - 1);
  strncpy(g_download_url, url, sizeof(g_download_url) - 1);
  g_update_version = g_new_version;
  g_update_available = true;

  // Download in background so menu stays responsive
  snprintf(g_staged_path, sizeof(g_staged_path), "/tmp/zelda3_update_%s", latest);
  snprintf(cmd, sizeof(cmd),
    "curl -sL '%s' -o '%s' 2>/dev/null &", g_download_url, g_staged_path);
  system(cmd);
}

bool Updater_IsReady(void) {
  // Check that the download completed
  if (!g_update_available || !g_staged_path[0])
    return false;

  // Check file exists and has reasonable size
  char cmd[512];
  snprintf(cmd, sizeof(cmd), "test -s '%s'", g_staged_path);
  int ret = system(cmd);
  if (ret != 0) return false;

  // Make it executable
  snprintf(cmd, sizeof(cmd), "chmod +x '%s' 2>/dev/null", g_staged_path);
  system(cmd);
  return true;
}

void Updater_Apply(void) {
  if (!g_staged_path[0]) return;

#if defined(__APPLE__)
  // macOS: just move the new binary into place and exec
  char cmd[1024];
  snprintf(cmd, sizeof(cmd),
    "mv '%s' ./zelda3.new ; mv ./zelda3.new ./zelda3 ; exec ./zelda3",
    g_staged_path);
  system(cmd);
#else
  // Linux AppImage: replace and exec the new AppImage
  const char *appimage = getenv("APPIMAGE");
  if (appimage && appimage[0]) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
      "mv '%s' '%s' ; chmod +x '%s' ; exec '%s'",
      g_staged_path, appimage, appimage, appimage);
    system(cmd);
  } else {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
      "mv '%s' ./zelda3 ; chmod +x ./zelda3 ; exec ./zelda3",
      g_staged_path);
    system(cmd);
  }
#endif
}
