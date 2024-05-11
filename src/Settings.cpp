#include <Settings.h>

#include "Shared.h"

#include <filesystem>
#include <fstream>

const char *IS_KEYBOARD_OVERLAY_ENABLED = "IsKeyboardOverlayEnabled";
const char *IS_BACKGROUND_TRANSPARENT = "IsBackgroundTransparent";
const char *SHOW_KEY_TIMERS = "ShowKeyTimers";
const char *KEY_SIZE = "KeySize";
const char *WINDOW_SCALE = "WindowScale";

namespace Settings
{
std::mutex Mutex;
json Settings = json::object();

void Load(const std::filesystem::path &aPath)
{
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
            APIDefs->Log(
                ELogLevel_WARNING,
                "Keyboard Overlay: Settings.json could not be parsed.");
            APIDefs->Log(ELogLevel_WARNING, ex.what());
        }
    }
    Settings::Mutex.unlock();
    if (!Settings[IS_KEYBOARD_OVERLAY_ENABLED].is_null())
        Settings[IS_KEYBOARD_OVERLAY_ENABLED].get_to(IsKeyboardOverlayEnabled);
    if (!Settings[IS_BACKGROUND_TRANSPARENT].is_null())
        Settings[IS_BACKGROUND_TRANSPARENT].get_to(IsBackgroundTransparent);
    if (!Settings[SHOW_KEY_TIMERS].is_null())
        Settings[SHOW_KEY_TIMERS].get_to(ShowKeyTimers);
    if (!Settings[WINDOW_SCALE].is_null())
        Settings[WINDOW_SCALE].get_to(WindowScale);
    if (!Settings[KEY_SIZE].is_null())
        Settings[KEY_SIZE].get_to(KeySize);
}

void Save(const std::filesystem::path &aPath)
{
    Settings::Mutex.lock();
    {
        std::ofstream file(aPath);
        file << Settings.dump(1, '\t') << std::endl;
        file.close();
    }
    Settings::Mutex.unlock();
}

bool IsKeyboardOverlayEnabled = true;
bool IsBackgroundTransparent = false;
bool ShowKeyTimers = true;
float WindowScale = 0.9f;
float KeySize = 72;
} // namespace Settings
