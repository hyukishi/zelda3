#include "updater.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CURRENT_VERSION  "v0.9.2"
#define REPO_API        "https://api.github.com/repos/hyukishi/zelda3/releases/latest"

bool g_update_available;
const char *g_update_version;

static char g_download_url[512];
static char g_staged_path[512];
static char g_new_version[32];

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
