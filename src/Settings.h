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

/* World/Agent */
extern bool IsAgentEnabled;

// Keybindings
extern std::string ForwardKey;
extern std::string BackwardsKey;
extern std::string LeftKey;
extern std::string RightKey;
extern std::string JumpKey;
extern std::string WeaponSwapKey;
extern std::string HealKey;
extern std::string Utility1Key;
extern std::string Utility2Key;
extern std::string Utility3Key;
extern std::string EliteKey;

} // namespace Settings

#endif
