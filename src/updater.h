#pragma once
#include "types.h"

// Call at startup to check for updates in background. If a newer version
// is available, sets g_update_available and stores the download URL.
void Updater_Check(void);

// Returns true if an update was found and downloaded successfully.
// The new binary is staged in a temp location, ready to replace the current one.
bool Updater_IsReady(void);

// Applies the staged update: replaces the current binary, then restarts.
void Updater_Apply(void);

// Start a non-blocking background thread to check for updates. When a new
// version is detected, sets g_update_notify_active to show a banner.
void Updater_StartBackgroundCheck(void);

// Draw update notification banner if active (called each frame).
void Updater_DrawNotify(uint8 *pixel_buffer, int pitch, int fb_w, int fb_h);

// Handle input when notification is showing: Enter = download+apply, Esc = dismiss.
void Updater_NotifyInput(int key_code, bool pressed);

extern bool g_update_available;
extern const char *g_update_version;
extern bool g_update_notify_active;
