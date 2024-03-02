#include "Settings.h"

#include "Shared.h"

#include <filesystem>
#include <fstream>
#include <string>

const char *IS_KEYBOARD_OVERLAY_VISIBLE = "IsKeyboardOverlayVisible";
const char *FORWARD_KEYBINDING = "ForwardKeybinding";
const char *BACKWARDS_KEYBINDING = "BackwardsKeybinding";
const char *LEFT_KEYBINDING = "LeftKeybinding";
const char *RIGHT_KEYBINDING = "RightKeybinding";
const char *JUMP_KEYBINDING = "JumpKeybinding";
const char *WEAPON_SWAP_KEYBINDING = "WeaponSwapKeybinding";
const char *HEAL_KEYBINDING = "HealKeybinding";
const char *UTILITY1_KEYBINDING = "Utility1Keybinding";
const char *UTILITY2_KEYBINDING = "Utility2Keybinding";
const char *UTILITY3_KEYBINDING = "Utility3Keybinding";
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

  if (!Settings[IS_KEYBOARD_OVERLAY_VISIBLE].is_null()) {
    Settings[IS_KEYBOARD_OVERLAY_VISIBLE].get_to(IsWidgetEnabled);
  }
  if (!Settings[FORWARD_KEYBINDING].is_null()) {
    Settings[FORWARD_KEYBINDING].get_to(ForwardKey);
  }
  if (!Settings[LEFT_KEYBINDING].is_null()) {
    Settings[LEFT_KEYBINDING].get_to(LeftKey);
  }
  if (!Settings[RIGHT_KEYBINDING].is_null()) {
    Settings[RIGHT_KEYBINDING].get_to(RightKey);
  }
  if (!Settings[BACKWARDS_KEYBINDING].is_null()) {
    Settings[BACKWARDS_KEYBINDING].get_to(BackwardsKey);
  }
  if (!Settings[JUMP_KEYBINDING].is_null()) {
    Settings[JUMP_KEYBINDING].get_to(JumpKey);
  }
  if (!Settings[WEAPON_SWAP_KEYBINDING].is_null()) {
    Settings[WEAPON_SWAP_KEYBINDING].get_to(WeaponSwapKey);
  }
  if (!Settings[HEAL_KEYBINDING].is_null()) {
    Settings[HEAL_KEYBINDING].get_to(HealKey);
  }
  if (!Settings[UTILITY1_KEYBINDING].is_null()) {
    Settings[UTILITY1_KEYBINDING].get_to(Utility1Key);
  }
  if (!Settings[UTILITY2_KEYBINDING].is_null()) {
    Settings[UTILITY2_KEYBINDING].get_to(Utility2Key);
  }
  if (!Settings[UTILITY3_KEYBINDING].is_null()) {
    Settings[UTILITY3_KEYBINDING].get_to(Utility3Key);
  }
  if (!Settings[ELITE_KEYBINDING].is_null()) {
    Settings[ELITE_KEYBINDING].get_to(EliteKey);
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

bool IsWidgetEnabled = true;
float WidgetOffsetV = 0.0f;
float WidgetWidth = 600.0f;

// Keybindings
std::string ForwardKey;
std::string BackwardsKey;
std::string LeftKey;
std::string RightKey;
std::string JumpKey;
std::string WeaponSwapKey;
std::string HealKey;
std::string Utility1Key;
std::string Utility2Key;
std::string Utility3Key;
std::string EliteKey;

/* World/Agent */
bool IsAgentEnabled = true;

} // namespace Settings
