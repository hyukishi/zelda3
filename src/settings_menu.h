#pragma once
#include "types.h"

extern bool g_settings_menu_active;

void SettingsMenu_Toggle(void);
void SettingsMenu_Draw(uint8 *pixel_buffer, int pitch, int fb_w, int fb_h);
void SettingsMenu_Input(int key_code, int key_mod, bool pressed);
void SettingsMenu_ApplyCheats(void);
