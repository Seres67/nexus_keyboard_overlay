#include <Settings.h>

#include "Shared.h"
#include "utils.h"

#include <filesystem>
#include <fstream>

const char *IS_KEYBOARD_OVERLAY_ENABLED = "IsKeyboardOverlayEnabled";
const char *IS_BACKGROUND_TRANSPARENT = "IsBackgroundTransparent";
const char *SHOW_KEY_TIMERS = "ShowKeyTimers";
const char *KEY_SIZE = "KeySize";
const char *WINDOW_SCALE = "WindowScale";
const char *ALWAYS_DISPLAYED = "AlwaysDisplayed";
const char *DISABLE_IN_CHAT = "DisableInChat";

json Settings::m_json_settings;
std::mutex Settings::m_mutex;

void Settings::Load(const std::filesystem::path &aPath)
{
    m_json_settings = json::object();
    if (!std::filesystem::exists(aPath)) {
        return;
    }

    {
        std::lock_guard lock(m_mutex);
        try {
            std::ifstream file(aPath);
            if (file.is_open()) {
                m_json_settings = json::parse(file);
                file.close();
            }
        } catch (json::parse_error &ex) {
            APIDefs->Log(
                ELogLevel_WARNING,
                "Keyboard Overlay: Settings.json could not be parsed.");
            APIDefs->Log(ELogLevel_WARNING, ex.what());
        }
    }
    if (!m_json_settings[IS_KEYBOARD_OVERLAY_ENABLED].is_null())
        m_json_settings[IS_KEYBOARD_OVERLAY_ENABLED].get_to(
            SettingsVars::IsKeyboardOverlayEnabled);
    if (!m_json_settings[IS_BACKGROUND_TRANSPARENT].is_null())
        m_json_settings[IS_BACKGROUND_TRANSPARENT].get_to(
            SettingsVars::IsBackgroundTransparent);
    if (!m_json_settings[SHOW_KEY_TIMERS].is_null())
        m_json_settings[SHOW_KEY_TIMERS].get_to(SettingsVars::ShowKeyTimers);
    if (!m_json_settings[WINDOW_SCALE].is_null())
        m_json_settings[WINDOW_SCALE].get_to(SettingsVars::WindowScale);
    if (!m_json_settings[KEY_SIZE].is_null())
        m_json_settings[KEY_SIZE].get_to(SettingsVars::KeySize);
    if (!m_json_settings[ALWAYS_DISPLAYED].is_null())
        m_json_settings[ALWAYS_DISPLAYED].get_to(SettingsVars::AlwaysDisplayed);
    if (!m_json_settings[DISABLE_IN_CHAT].is_null())
        m_json_settings[DISABLE_IN_CHAT].get_to(SettingsVars::DisableInChat);
}

void Settings::Save(const std::filesystem::path &aPath)
{
    if (m_json_settings.is_null()) {
        Log::debug("settings is null");
        return;
    }
    {
        std::lock_guard lock(m_mutex);
        std::ofstream file(aPath);
        if (file.is_open()) {
            file << m_json_settings.dump(1, '\t') << std::endl;
            file.close();
        }
    }
}

namespace SettingsVars
{
bool IsKeyboardOverlayEnabled = true;
bool IsBackgroundTransparent = false;
bool ShowKeyTimers = true;
float WindowScale = 0.9f;
float KeySize = 72;
bool AlwaysDisplayed = false;
bool DisableInChat = true;
} // namespace SettingsVars
