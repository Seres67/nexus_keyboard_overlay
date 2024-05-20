#ifndef NEXUS_KEYBOARD_OVERLAY_SETTINGS_H
#define NEXUS_KEYBOARD_OVERLAY_SETTINGS_H

#include <mutex>
#include "imgui/imgui.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

extern const char *IS_KEYBOARD_OVERLAY_ENABLED;
extern const char *IS_BACKGROUND_TRANSPARENT;
extern const char *SHOW_KEY_TIMERS;
extern const char *WINDOW_SCALE;
extern const char *KEY_SIZE;
extern const char *ALWAYS_DISPLAYED;
extern const char *PRESSED_KEY_COLOR;

class Settings
{
  public:
    static void Load(const std::filesystem::path &aPath);
    static void Save(const std::filesystem::path &aPath);

    static json m_json_settings;

  private:
    static std::mutex m_mutex;
};

namespace SettingsVars
{
extern bool IsKeyboardOverlayEnabled;
extern bool IsBackgroundTransparent;
extern bool ShowKeyTimers;
extern float WindowScale;
extern float KeySize;
extern bool AlwaysDisplayed;
extern bool DisableInChat;
extern ImU32 KeyPressedColor;
} // namespace SettingsVars

#endif // !NEXUS_KEYBOARD_OVERLAY_SETTINGS_H
