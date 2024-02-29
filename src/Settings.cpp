#include "Settings.h"

#include "Shared.h"

#include <filesystem>
#include <fstream>

const char *IS_KEYBOARD_OVERLAY_VISIBLE = "IsKeyboardOverlayVisible";
const char *FORWARD_KEYBINDING = "ForwardKeybinding";
const char *BACKWARDS_KEYBINDING = "BackwardsKeybinding";
const char *LEFT_KEYBINDING = "LeftKeybinding";
const char *RIGHT_KEYBINDING = "RightKeybinding";
const char *JUMP_KEYBINDING = "JumpKeybinding";
const char *WEAPON_SWAP_KEYBINDING = "WeaponSwapKeybinding";
const char *HEAL_KEYBINDING = "HealKeybinding";
const char *UTILIY1_KEYBINDING = "Utility1Keybinding";
const char *UTILIY2_KEYBINDING = "Utility2Keybinding";
const char *UTILIY3_KEYBINDING = "Utility3Keybinding";
const char *ELITE_KEYBINDING = "EliteKeybinding";

namespace Settings {
std::mutex Mutex;
json Settings = json::object();

void Load(std::filesystem::path aPath) {
  if (!std::filesystem::exists(aPath)) {
    return;
  }

  Settings::Mutex.lock();
  {
    try {
      std::ifstream file(aPath);
      Settings = json::parse(file);
      file.close();
    } catch (json::parse_error &ex) {
      APIDefs->Log(ELogLevel_WARNING,
                   "Keyboard Overlay: Settings.json could not be parsed.");
      APIDefs->Log(ELogLevel_WARNING, ex.what());
    }
  }
  Settings::Mutex.unlock();

  /* Widget */
  if (!Settings[IS_KEYBOARD_OVERLAY_VISIBLE].is_null()) {
    Settings[IS_KEYBOARD_OVERLAY_VISIBLE].get_to<bool>(IsWidgetEnabled);
  }
  if (!Settings[FORWARD_KEYBINDING].is_null()) {
    Settings[FORWARD_KEYBINDING].get_to<char>(ForwardKey);
  }
  if (!Settings[LEFT_KEYBINDING].is_null()) {
    Settings[LEFT_KEYBINDING].get_to<char>(LeftKey);
  }
  if (!Settings[RIGHT_KEYBINDING].is_null()) {
    Settings[RIGHT_KEYBINDING].get_to<char>(RightKey);
  }
  if (!Settings[BACKWARDS_KEYBINDING].is_null()) {
    Settings[BACKWARDS_KEYBINDING].get_to<char>(BackwardsKey);
  }
  if (!Settings[JUMP_KEYBINDING].is_null()) {
    Settings[JUMP_KEYBINDING].get_to<char>(JumpKey);
  }
}

void Save(std::filesystem::path aPath) {
  Settings::Mutex.lock();
  {
    std::ofstream file(aPath);
    file << Settings.dump(1, '\t') << std::endl;
    file.close();
  }
  Settings::Mutex.unlock();
}

/* Global */

/* Widget */
bool IsWidgetEnabled = true;
float WidgetOffsetV = 0.0f;
float WidgetWidth = 600.0f;

// Keybindings
char ForwardKey = 0;
char BackwardsKey = 0;
char LeftKey = 0;
char RightKey = 0;
char JumpKey = 0;

/* Widget derived */
// float WidgetCenterX = WidgetWidth / 2;
// float WidgetOffsetPerDegree = WidgetWidth / WidgetRangeDegrees;

/* World/Agent */
bool IsAgentEnabled = true;

/* Indicator */
bool IsIndicatorEnabled = false;
bool IsIndicatorLocked = false;
} // namespace Settings
