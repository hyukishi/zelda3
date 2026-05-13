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

extern bool g_update_available;
extern const char *g_update_version;
