#include "Settings.h"

#include "Shared.h"

#include <filesystem>
#include <fstream>
#include <string>

const char *IS_KEYBOARD_OVERLAY_VISIBLE = "IsKeyboardOverlayVisible";

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

/* World/Agent */
bool IsAgentEnabled = true;

} // namespace Settings
