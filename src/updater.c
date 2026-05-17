#include "updater.h"
#include "settings_menu.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef CURRENT_VERSION
#define CURRENT_VERSION  "v0.0.0-unknown"
#endif

#define REPO_API           "https://api.github.com/repos/hyukishi/zelda3/releases/latest"
#define JSON_BUF_SIZE      8192
#define URL_BUF_SIZE        512
#define CMD_BUF_SIZE       1024
#define VERSION_BUF_SIZE     32
#define NOTIFY_TIMEOUT_MS 15000
#define BANNER_HEIGHT        28
#define FETCH_CMD_FMT       "curl -sL %s 2>/dev/null"

bool g_update_available;
const char *g_update_version;
bool g_update_notify_active;

static char g_download_url[URL_BUF_SIZE];
static char g_staged_path[URL_BUF_SIZE];
static char g_new_version[VERSION_BUF_SIZE];
static uint32 g_notify_start_time;
static SDL_Thread *g_update_thread;
static SDL_mutex *g_update_mutex;

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static void EnsureMutex(void) {
  if (!g_update_mutex)
    g_update_mutex = SDL_CreateMutex();
}

// Minimal JSON string value extractor: find "key":"value" and copy value
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

// Find browser_download_url for the platform-appropriate asset
static bool find_asset_url(const char *json, char *url, int urlsz) {
#if defined(__APPLE__)
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
      if (strstr(tmp, "zelda3-macos") || (strstr(tmp, "zelda3")
          && !strstr(tmp, "AppImage") && !strstr(tmp, ".ini")
          && !strstr(tmp, "source"))) {
        memcpy(url, p, len);
        url[len] = 0;
        return true;
      }
    }
  }
#else
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

// Fetch latest release JSON from GitHub API via curl. Returns bytes read or 0.
static int FetchReleaseJson(char *json_buf, int buf_size) {
  char cmd[CMD_BUF_SIZE];
  snprintf(cmd, sizeof(cmd), FETCH_CMD_FMT, REPO_API);

  FILE *f = popen(cmd, "r");
  if (!f) {
    fprintf(stderr, "Updater: curl not found or popen failed\n");
    return 0;
  }

  int len = 0;
  while (len < buf_size - 1) {
    int c = fgetc(f);
    if (c == EOF) break;
    json_buf[len++] = (char)c;
  }
  json_buf[len] = 0;

  int ret = pclose(f);
  if (ret != 0) {
    fprintf(stderr, "Updater: curl exited with code %d\n", ret);
    return 0;
  }
  return len;
}

// Parse release JSON. If a newer version with a download URL is found,
// copies version and URL into the output buffers. Returns true on success.
static bool ParseRelease(const char *json, int json_len,
                         char *version_out, int ver_sz,
                         char *url_out, int url_sz) {
  if (json_len < 10) return false;

  char latest[VERSION_BUF_SIZE];
  if (!json_str(json, "tag_name", latest, sizeof(latest)))
    return false;

  if (strcmp(latest, CURRENT_VERSION) <= 0)
    return false;

  if (!find_asset_url(json, url_out, url_sz))
    return false;

  strncpy(version_out, latest, ver_sz - 1);
  version_out[ver_sz - 1] = '\0';
  return true;
}

// Download a URL to a file via curl. On failure, removes any partial file.
static bool StageDownload(const char *url, const char *dest_path) {
  // Defense in depth: reject arguments containing single quotes
  if (strchr(url, '\'') || strchr(dest_path, '\''))
    return false;

  char cmd[CMD_BUF_SIZE];
  snprintf(cmd, sizeof(cmd),
           "curl -sL '%s' -o '%s' 2>/dev/null", url, dest_path);

  FILE *f = popen(cmd, "r");
  if (!f) return false;

  int ret = pclose(f);
  if (ret != 0) {
    unlink(dest_path);
    return false;
  }
  return true;
}

// Check that a file exists and has non-zero size (replaces system("test -s")).
static bool FileIsReadable(const char *path) {
  struct stat st;
  return stat(path, &st) == 0 && st.st_size > 0;
}

// ---------------------------------------------------------------------------
// Background thread
// ---------------------------------------------------------------------------

static int Updater_Thread(void *data) {
  (void)data;

  char json[JSON_BUF_SIZE] = {0};
  int len = FetchReleaseJson(json, sizeof(json));
  if (len < 10) return 0;

  char latest[VERSION_BUF_SIZE];
  char url[URL_BUF_SIZE];
  if (!ParseRelease(json, len, latest, sizeof(latest), url, sizeof(url)))
    return 0;

  SDL_LockMutex(g_update_mutex);
  strncpy(g_new_version, latest, sizeof(g_new_version) - 1);
  g_new_version[sizeof(g_new_version) - 1] = '\0';
  strncpy(g_download_url, url, sizeof(g_download_url) - 1);
  g_download_url[sizeof(g_download_url) - 1] = '\0';
  g_update_version = g_new_version;
  g_update_available = true;
  g_update_notify_active = true;
  g_notify_start_time = SDL_GetTicks();
  SDL_UnlockMutex(g_update_mutex);

  return 0;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void Updater_StartBackgroundCheck(void) {
  EnsureMutex();
  g_update_thread = SDL_CreateThread(Updater_Thread, "Updater", NULL);
  if (!g_update_thread)
    fprintf(stderr, "Warning: Could not create updater thread\n");
}

void Updater_DrawNotify(uint8 *pixel_buffer, int pitch, int fb_w, int fb_h) {
  SDL_LockMutex(g_update_mutex);
  bool notify_active = g_update_notify_active;
  const char *version = g_update_version;
  if (notify_active) {
    if (SDL_GetTicks() - g_notify_start_time > NOTIFY_TIMEOUT_MS) {
      g_update_notify_active = false;
      notify_active = false;
    }
  }
  SDL_UnlockMutex(g_update_mutex);

  if (!notify_active || !version)
    return;

  // Semi-transparent banner at top of screen
  uint32 bg = 0xCC1A1A2E;
  int stride = pitch / 4;
  for (int row = 0; row < BANNER_HEIGHT; row++) {
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
  snprintf(buf, sizeof(buf), "%s available! Enter to update, Esc to dismiss", version);
  int text_w = (int)strlen(buf) * 9;
  int tx = (fb_w - text_w) / 2;
  int ty = (BANNER_HEIGHT - 8) / 2;
  if (tx < 0) tx = 0;
  DrawString(pixel_buffer, pitch, tx, ty, buf, 0xFF40FF80);
}

void Updater_NotifyInput(int key_code, bool pressed) {
  if (!pressed) return;

  SDL_LockMutex(g_update_mutex);
  bool notify_active = g_update_notify_active;
  SDL_UnlockMutex(g_update_mutex);

  if (!notify_active) return;

  if (key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER) {
    SDL_LockMutex(g_update_mutex);
    g_update_notify_active = false;
    snprintf(g_staged_path, sizeof(g_staged_path),
             "/tmp/zelda3_update_%s", g_new_version);
    SDL_UnlockMutex(g_update_mutex);

    if (StageDownload(g_download_url, g_staged_path) && Updater_IsReady())
      Updater_Apply();
    return;
  }

  if (key_code == SDLK_ESCAPE) {
    SDL_LockMutex(g_update_mutex);
    g_update_notify_active = false;
    SDL_UnlockMutex(g_update_mutex);
  }
}

void Updater_Check(void) {
  char json[JSON_BUF_SIZE] = {0};
  int len = FetchReleaseJson(json, sizeof(json));
  if (len < 10) return;

  char latest[VERSION_BUF_SIZE];
  char url[URL_BUF_SIZE];
  if (!ParseRelease(json, len, latest, sizeof(latest), url, sizeof(url)))
    return;

  SDL_LockMutex(g_update_mutex);
  strncpy(g_new_version, latest, sizeof(g_new_version) - 1);
  g_new_version[sizeof(g_new_version) - 1] = '\0';
  strncpy(g_download_url, url, sizeof(g_download_url) - 1);
  g_download_url[sizeof(g_download_url) - 1] = '\0';
  g_update_version = g_new_version;
  g_update_available = true;
  snprintf(g_staged_path, sizeof(g_staged_path),
           "/tmp/zelda3_update_%s", latest);
  SDL_UnlockMutex(g_update_mutex);

  StageDownload(g_download_url, g_staged_path);
}

bool Updater_IsReady(void) {
  SDL_LockMutex(g_update_mutex);
  bool available = g_update_available;
  bool has_path = g_staged_path[0] != '\0';
  SDL_UnlockMutex(g_update_mutex);

  if (!available || !has_path)
    return false;

  if (!FileIsReadable(g_staged_path))
    return false;

  if (chmod(g_staged_path, 0755) != 0)
    return false;

  return true;
}

void Updater_Apply(void) {
  SDL_LockMutex(g_update_mutex);
  char staged[URL_BUF_SIZE];
  strncpy(staged, g_staged_path, sizeof(staged) - 1);
  staged[sizeof(staged) - 1] = '\0';
  SDL_UnlockMutex(g_update_mutex);

  if (!staged[0]) return;

#if defined(__APPLE__)
  if (rename(staged, "./zelda3") != 0) {
    fprintf(stderr, "Updater: rename to ./zelda3 failed\n");
    return;
  }
  execl("./zelda3", "./zelda3", NULL);

#else
  const char *appimage = getenv("APPIMAGE");
  if (appimage && appimage[0]) {
    if (rename(staged, appimage) != 0) {
      fprintf(stderr, "Updater: rename to %s failed\n", appimage);
      return;
    }
    chmod(appimage, 0755);
    execl(appimage, appimage, NULL);
  } else {
    if (rename(staged, "./zelda3") != 0) {
      fprintf(stderr, "Updater: rename to ./zelda3 failed\n");
      return;
    }
    chmod("./zelda3", 0755);
    execl("./zelda3", "./zelda3", NULL);
  }
#endif
  fprintf(stderr, "Updater: exec failed\n");
}
