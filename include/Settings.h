#ifndef NEXUS_KEYBOARD_OVERLAY_SETTINGS_H
#define NEXUS_KEYBOARD_OVERLAY_SETTINGS_H

#include <mutex>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

extern const char *IS_KEYBOARD_OVERLAY_ENABLED;
extern const char *IS_BACKGROUND_TRANSPARENT;
extern const char *SHOW_KEY_TIMERS;
extern const char *WINDOW_SCALE;
extern const char *KEY_SIZE;

namespace Settings
{
extern std::mutex Mutex;
extern json Settings;

/* Loads the settings. */
void Load(const std::filesystem::path &aPath);
/* Saves the settings. */
void Save(const std::filesystem::path &aPath);

extern bool IsKeyboardOverlayEnabled;
extern bool IsBackgroundTransparent;
extern bool ShowKeyTimers;
extern float WindowScale;
extern float KeySize;
} // namespace Settings

#endif // !NEXUS_KEYBOARD_OVERLAY_SETTINGS_H
