#ifndef SETTINGS_H
#define SETTINGS_H

#include <mutex>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

extern const char *IS_KEYBOARD_OVERLAY_VISIBLE;
extern const char *FORWARD_KEYBINDING;
extern const char *BACKWARDS_KEYBINDING;
extern const char *LEFT_KEYBINDING;
extern const char *RIGHT_KEYBINDING;
extern const char *JUMP_KEYBINDING;
extern const char *WEAPON_SWAP_KEYBINDING;
extern const char *HEAL_KEYBINDING;
extern const char *UTILIY1_KEYBINDING;
extern const char *UTILIY2_KEYBINDING;
extern const char *UTILIY3_KEYBINDING;
extern const char *ELITE_KEYBINDING;

namespace Settings {
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
extern float WidgetRangeDegrees;
extern float WidgetStepDegrees;

/* Widget derived */
extern float WidgetCenterX;
extern float WidgetOffsetPerDegree;

/* World/Agent */
extern bool IsAgentEnabled;

/* Indicator */
extern bool IsIndicatorEnabled;
extern bool IsIndicatorLocked;
extern std::string IndicatorPrefix;
extern char IndicatorPrefixC[64];
extern bool IndicatorLong;
extern bool IndicatorOutline;

// Keybindings
extern std::string ForwardKey;
} // namespace Settings

#endif
