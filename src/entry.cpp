#include <chrono>
#include <cstdio>
#include <ctime>
#include <cwchar>
#include <filesystem>
#include <string>
#include <vector>
#include <windows.h>

#include "Settings.h"
#include "Shared.h"

#include "resource.h"

#include "imgui/imgui.h"
// #include "imgui/imgui_extensions.h"
#include "nexus/Nexus.h"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

void OnWindowResized(void *aEventArgs);
void ReceiveTexture(const char *aIdentifier, Texture *aTexture);

void AddonLoad(AddonAPI *aApi);
void AddonUnload();
void AddonRender();
void AddonOptions();

HMODULE hSelf;

AddonDefinition AddonDef{};

std::filesystem::path AddonPath;
std::filesystem::path SettingsPath;

Texture *hrTex = nullptr;

float padding = 5.0f;

std::unordered_map<char, Texture *> textures_pressed;
Texture *grid_texture;

int keybindIndexToChange = -1;
char newKeybindingName[20];
bool addingKeybinding = false;

const char *WINDOW_RESIZED = "EV_WINDOW_RESIZED";
const char *HR_TEX = "TEX_SEPARATOR_DETAIL";

struct m_key_s {
  std::string binding_name;
  std::string key_name;
  char code;
  bool pressed;
  std::chrono::time_point<std::chrono::steady_clock> start_pressing;
  std::chrono::time_point<std::chrono::steady_clock> end_pressing;
};

std::vector<struct m_key_s> KEYS;

void to_json(json &j, const struct m_key_s &p) {
  j = json{
      {"name", p.binding_name}, {"key_name", p.key_name}, {"key_code", p.code}};
}

void from_json(const json &j, struct m_key_s &p) {
  j.at("name").get_to(p.binding_name);
  j.at("key_name").get_to(p.key_name);
  j.at("key_code").get_to(p.code);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
    hSelf = hModule;
    break;
  case DLL_PROCESS_DETACH:
    break;
  case DLL_THREAD_ATTACH:
    break;
  case DLL_THREAD_DETACH:
    break;
  }
  return TRUE;
}

extern "C" __declspec(dllexport) AddonDefinition *GetAddonDef() {
  AddonDef.Signature = -918234798;
  AddonDef.APIVersion = NEXUS_API_VERSION;
  AddonDef.Name = "Keyboard Overlay";
  AddonDef.Version.Major = 0;
  AddonDef.Version.Minor = 4;
  AddonDef.Version.Build = 0;
  AddonDef.Version.Revision = 0;
  AddonDef.Author = "Seres67";
  AddonDef.Description = "Adds a simple keyboard overlay to the UI.";
  AddonDef.Load = AddonLoad;
  AddonDef.Unload = AddonUnload;
  AddonDef.Flags = EAddonFlags_None;
  AddonDef.Provider = EUpdateProvider_GitHub;
  AddonDef.UpdateLink = "https://github.com/Seres67/nexus_keyboard_overlay";

  return &AddonDef;
}

void KeyDown(WPARAM key) {
  for (int i = 0; i < KEYS.size(); ++i) {
    if (!KEYS[i].pressed &&
        key == VkKeyScanEx(KEYS[i].code, GetKeyboardLayout(0))) {
      KEYS[i].pressed = true;
      KEYS[i].start_pressing = std::chrono::steady_clock::now();
    }
  }
}

void KeyUp(WPARAM key) {
  for (int i = 0; i < KEYS.size(); ++i) {
    if (KEYS[i].pressed &&
        key == VkKeyScanEx(KEYS[i].code, GetKeyboardLayout(0))) {
      KEYS[i].pressed = false;
      KEYS[i].end_pressing = std::chrono::steady_clock::now();
    }
  }
}

void setKeybinding(WPARAM wParam) {
  char c = MapVirtualKey(wParam, MAPVK_VK_TO_CHAR);
  if (wParam >= 'A' && wParam <= 'Z')
    KEYS[keybindIndexToChange].code = c + 32;
  else
    KEYS[keybindIndexToChange].code = c;
  if (wParam == VK_SPACE)
    KEYS[keybindIndexToChange].key_name = std::string("Space");
  else {
    KEYS[keybindIndexToChange].key_name = c;
  }
  char keybinding_setting[30];
  sprintf(keybinding_setting, "%sKeybinding",
          KEYS[keybindIndexToChange].binding_name.c_str());
  APIDefs->Log(ELogLevel_INFO, keybinding_setting);
  Settings::Settings[keybinding_setting] = KEYS[keybindIndexToChange].key_name;
  char log[80];
  sprintf(log, "changing to '%c'", KEYS[keybindIndexToChange].code);
  APIDefs->Log(ELogLevel_INFO, log);
  keybindIndexToChange = -1;
}

void addKeybinding(WPARAM key) {
  APIDefs->Log(ELogLevel_INFO, "begin");
  char c = MapVirtualKey(key, MAPVK_VK_TO_CHAR);
  addingKeybinding = false;
  struct m_key_s new_key;
  if (key >= 'A' && key <= 'Z')
    new_key.code = c + 32;
  else
    new_key.code = c;
  if (key == VK_SPACE)
    new_key.key_name = std::string("Space");
  else {
    new_key.key_name = c;
  }
  new_key.binding_name = strdup(newKeybindingName);
  memset(newKeybindingName, 0, sizeof(newKeybindingName));
  KEYS.emplace_back(new_key);
}

// void setMouseKeybinding(int index, WPARAM button) {
//   waitForKeybindings[index] = false;
//   KEYS[]
//   if (wParam >= 'A' && wParam <= 'Z')
//     KEYS[index].code = c + 32;
//   else
//     KEYS[index].code = c;
//   if (wParam == VK_SPACE)
//     KEYS[index].key_name = strdup("Space");
//   else {
//     KEYS[index].key_name[0] = c;
//     KEYS[index].key_name[1] = 0;
//   }
// }

UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (addingKeybinding) {
    if (uMsg == WM_KEYDOWN)
      addKeybinding(wParam);
  } else if (keybindIndexToChange == -1) {
    switch (uMsg) {
    case WM_KEYDOWN:
      KeyDown(wParam);
      break;
    case WM_KEYUP:
      KeyUp(wParam);
      break;
    // case WM_XBUTTONDOWN:
    //   MouseButtonDown(wParam);
    //   break;
    // case WM_XBUTTONUP:
    //   MouseButtonUp(wParam);
    //   break;
    default:
      break;
    }
  } else {
    if (uMsg == WM_KEYDOWN)
      setKeybinding(wParam);
    // else if (uMsg == WM_XBUTTONDOWN)
    //   setMouseKeybinding(i, wParam);
  }

  if (uMsg == WM_XBUTTONDOWN) {
    char log[80];
    sprintf(log, "keystate: %zu, button %zu", GET_KEYSTATE_WPARAM(wParam),
            GET_XBUTTON_WPARAM(wParam));
    APIDefs->Log(ELogLevel_INFO, log);
  }
  return uMsg;
}

void AddonLoad(AddonAPI *aApi) {
  APIDefs = aApi;
  ImGui::SetCurrentContext(APIDefs->ImguiContext);
  ImGui::SetAllocatorFunctions(
      (void *(*)(size_t, void *))APIDefs->ImguiMalloc,
      (void (*)(void *, void *))APIDefs->ImguiFree); // on imgui 1.80+

  MumbleLink = (Mumble::Data *)APIDefs->GetResource("DL_MUMBLE_LINK");
  NexusLink = (NexusLinkData *)APIDefs->GetResource("DL_NEXUS_LINK");

  APIDefs->RegisterWndProc(WndProc);
  APIDefs->SubscribeEvent(WINDOW_RESIZED, OnWindowResized);

  APIDefs->RegisterRender(ERenderType_Render, AddonRender);
  APIDefs->RegisterRender(ERenderType_OptionsRender, AddonOptions);

  AddonPath = APIDefs->GetAddonDirectory("keyboard_overlay");
  SettingsPath = APIDefs->GetAddonDirectory("keyboard_overlay/settings.json");
  std::filesystem::create_directory(AddonPath);
  Settings::Load(SettingsPath);

  if (!Settings::Settings["AllKeybindings"].is_null())
    Settings::Settings["AllKeybindings"].get_to(KEYS);

  OnWindowResized(nullptr); // initialise self
}

void AddonUnload() {
  APIDefs->UnregisterRender(AddonOptions);
  APIDefs->UnregisterRender(AddonRender);

  APIDefs->UnsubscribeEvent(WINDOW_RESIZED, OnWindowResized);

  APIDefs->UnregisterWndProc(WndProc);

  MumbleLink = nullptr;
  NexusLink = nullptr;

  json settings_json = KEYS;
  Settings::Settings["AllKeybindings"] = settings_json;
  Settings::Save(SettingsPath);
}

void keyPressedText(int index) {
  if (KEYS[index].pressed) {
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - KEYS[index].start_pressing);
    ImGui::Text("%s pressed, %ld ms", KEYS[index].binding_name.c_str(),
                duration.count());
  } else {
    ImGui::Text("%s not pressed, %ld ms", KEYS[index].binding_name.c_str(),
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    KEYS[index].end_pressing - KEYS[index].start_pressing)
                    .count());
  }
}

void AddonRender() {
  if (!NexusLink->IsGameplay) {
    return;
  }

  if (Settings::IsWidgetEnabled) {
    ImVec2 initialPos = ImGui::GetCursorPos();
    // ImGui::ShowUserGuide();
    // ImGui::ShowDemoWindow();
    // ImGui::ShowStyleEditor();
    // ImGui::ShowStyleSelector("label");
    // ImGui::ShowMetricsWindow();
    /* use Menomonia */
    ImGui::PushFont(NexusLink->Font);
    ImGui::PushItemWidth(1200.f);
    if (ImGui::Begin("KEYBOARD_OVERLAY", (bool *)0,
                     ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_NoScrollbar)) {

      for (int i = 0; i < KEYS.size(); ++i) {
        char code = KEYS[i].code;
        if (KEYS[i].pressed) {
          if (textures_pressed[code] && textures_pressed[code]->Resource) {
            ImGui::SetCursorPos(initialPos);
            ImGui::Image(textures_pressed[code]->Resource, ImVec2(50, 50));
          } else {
            keyPressedText(i);
            APIDefs->LoadTextureFromResource("TEX_Z_PRESSED", IDB_PNG1, hSelf,
                                             ReceiveTexture);
          }
        }
      }
      if (grid_texture && grid_texture->Resource) {
        ImGui::SetCursorPos(initialPos);
        ImGui::Image(grid_texture->Resource, ImVec2(600, 400));
      } else {
        APIDefs->LoadTextureFromFile("TEX_GRID", "grid.png", ReceiveTexture);
      }
    }
    ImGui::PopFont();
    ImGui::End();
  }
}

void deleteKey(int index) { KEYS.erase(KEYS.begin() + index); }

void AddonOptions() {
  ImGui::Text("Keyboard Overlay");
  ImGui::TextDisabled("Widget");
  if (ImGui::Checkbox("Enabled##Widget", &Settings::IsWidgetEnabled)) {
    Settings::Settings[IS_KEYBOARD_OVERLAY_VISIBLE] = Settings::IsWidgetEnabled;
    Settings::Save(SettingsPath);
  }
  for (int i = 0; i < KEYS.size(); ++i) {
    ImGui::PushID(i);
    ImGui::Text("%s Key", KEYS[i].binding_name.c_str());
    ImGui::SameLine();
    if (keybindIndexToChange == i) {
      ImGui::Button("Press key to bind");
    } else if (ImGui::Button(KEYS[i].key_name.c_str())) {
      keybindIndexToChange = i;
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete Key")) {
      deleteKey(i);
    }
    ImGui::PopID();
  }
  ImGui::Text("New Key Name: ");
  ImGui::SameLine();
  ImGui::InputText("##newKeyName", newKeybindingName, 20);
  ImGui::SameLine();
  if (addingKeybinding) {
    ImGui::Button("Press key to bind");
  } else if (ImGui::Button("Add Key")) {
    addingKeybinding = true;
  }
}

void OnWindowResized(void *aEventArgs) { /* event args are nullptr, ignore */
}

void ReceiveTexture(const char *aIdentifier, Texture *aTexture) {
  std::string str = aIdentifier;

  if (str == HR_TEX) {
    hrTex = aTexture;
  } else if (str == "TEX_GRID") {
    grid_texture = aTexture;
  } else if (str == "TEX_Z_PRESSED") {
    textures_pressed['z'] = aTexture;
  }
}
