#ifndef NEXUS_KEYBOARD_OVERLAY_SETTINGS_H
#define NEXUS_KEYBOARD_OVERLAY_SETTINGS_H

#include <mutex>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

extern const char *IS_KEYBOARD_OVERLAY_VISIBLE;
extern const char *SHOW_KEY_LABELS;
extern const char *WINDOW_SCALE;
extern const char *KEY_SIZE;

namespace Settings
{
extern std::mutex Mutex;
extern json Settings;

/* Loads the settings. */
void Load(std::filesystem::path aPath);
/* Saves the settings. */
void Save(std::filesystem::path aPath);

/* Global */

/* Widget */
extern bool IsWidgetEnabled;
extern float WidgetOffsetV;
extern float WidgetWidth;

extern bool ShowKeyLabels;
extern float WindowScale;
extern float KeySize;

/* World/Agent */
extern bool IsAgentEnabled;

} // namespace Settings

#endif // !NEXUS_KEYBOARD_OVERLAY_SETTINGS_H
