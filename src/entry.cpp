#include <chrono>
#include <cstdio>
#include <ctime>
#include <cwchar>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>
#include <windowsx.h>

#include "Settings.h"
#include "Shared.h"

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

char keybindingToChange = -1;
char newKeybindingName[20];
bool addingKeybinding = false;

ImVec2 windowPos;
char draggingButton = -1;
ImVec2 dragPos;

const char *WINDOW_RESIZED = "EV_WINDOW_RESIZED";

struct m_key_s {
  std::string binding_name;
  std::string key_name;
  char code;
  bool pressed;
  ImVec2 pos;
  // std::weak_ptr<Texture> not_pressed_tex;
  // std::weak_ptr<Texture> pressed_tex;
  std::chrono::time_point<std::chrono::steady_clock> start_pressing;
  std::chrono::time_point<std::chrono::steady_clock> end_pressing;
};

std::map<char, struct m_key_s> keys;
std::unordered_map<char, Texture *> textures_not_pressed;
std::unordered_map<char, Texture *> textures_pressed;

void to_json(json &j, const struct m_key_s &p) {
  j = json{{"name", p.binding_name},
           {"key_name", p.key_name},
           {"key_code", p.code},
           {"pos_x", p.pos.x},
           {"pos_y", p.pos.y}};
}

void from_json(const json &j, struct m_key_s &p) {
  j.at("name").get_to(p.binding_name);
  j.at("key_name").get_to(p.key_name);
  j.at("key_code").get_to(p.code);
  j.at("pos_x").get_to(p.pos.x);
  j.at("pos_y").get_to(p.pos.y);
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
  AddonDef.Version.Minor = 5;
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

void KeyDown(WPARAM i_key) {
  char log[80];
  sprintf(log, "key down is %c", i_key);
  for (auto &&key : keys) {
    if (!key.second.pressed &&
        i_key == VkKeyScanEx(key.second.code, GetKeyboardLayout(0))) {
      key.second.pressed = true;
      key.second.start_pressing = std::chrono::steady_clock::now();
    }
  }
}

void KeyUp(WPARAM i_key) {
  for (auto &&key : keys) {
    if (key.second.pressed &&
        i_key == VkKeyScanEx(key.second.code, GetKeyboardLayout(0))) {
      key.second.pressed = false;
      key.second.end_pressing = std::chrono::steady_clock::now();
    }
  }
}

void setKeybinding(WPARAM wParam) {
  char c = MapVirtualKey(wParam, MAPVK_VK_TO_CHAR);
  if (wParam >= 'A' && wParam <= 'Z')
    keys[keybindingToChange].code = c + 32;
  else
    keys[keybindingToChange].code = c;
  if (wParam == VK_SPACE)
    keys[keybindingToChange].key_name = std::string("Space");
  else {
    keys[keybindingToChange].key_name = c;
  }
  keybindingToChange = -1;
}

void addKeybinding(WPARAM key) {
  char c = MapVirtualKey(key, MAPVK_VK_TO_CHAR);

  addingKeybinding = false;
  struct m_key_s new_key;
  if (key >= 'A' && key <= 'Z')
    keys[c].code = c + 32;
  else
    keys[c].code = c;
  if (key == VK_SPACE)
    keys[c].key_name = std::string("Space");
  else {
    keys[c].key_name = c;
  }
  keys[c].binding_name = strdup(newKeybindingName);
  memset(newKeybindingName, 0, sizeof(newKeybindingName));
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
  } else if (keybindingToChange == -1) {
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
  if (draggingButton != -1) {
    if (uMsg == WM_MOUSEMOVE) {
      int x = GET_X_LPARAM(lParam);
      int y = GET_Y_LPARAM(lParam);
      keys[draggingButton].pos = ImVec2(x - windowPos.x, y - windowPos.y);
    } else if (uMsg == WM_LBUTTONUP) {
      draggingButton = -1;
    }
  }
  if (uMsg == WM_XBUTTONDOWN) {
    char log[80];
    sprintf(log, "keystate: %zu, button %zu", GET_KEYSTATE_WPARAM(wParam),
            GET_XBUTTON_WPARAM(wParam));
    APIDefs->Log(ELogLevel_INFO, log);
  }
  return uMsg;
}

void LoadTextures() {
  char tex_name[19];
  char tex_pressed_name[15];
  int id = 103;
  for (char c = 'A'; c <= 'Z'; ++c, id += 2) {
    sprintf(tex_name, "TEX_%c_NOT_PRESSED", c);
    sprintf(tex_pressed_name, "TEX_%c_PRESSED", c);
    APIDefs->LoadTextureFromResource(tex_name, id, hSelf, ReceiveTexture);
    APIDefs->LoadTextureFromResource(tex_pressed_name, id + 1, hSelf,
                                     ReceiveTexture);
  }
  APIDefs->Log(ELogLevel_DEBUG, "finished loading all textures!");
}

void AddonLoad(AddonAPI *aApi) {
  APIDefs = aApi;
  ImGui::SetCurrentContext(APIDefs->ImguiContext);
  ImGui::SetAllocatorFunctions(
      (void *(*)(size_t, void *))APIDefs->ImguiMalloc,
      (void (*)(void *, void *))APIDefs->ImguiFree); // on imgui 1.80+

  LoadTextures();

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
    Settings::Settings["AllKeybindings"].get_to(keys);

  APIDefs->Log(ELogLevel_DEBUG, "finished loading all settings!");
  OnWindowResized(nullptr); // initialise self
}

void AddonUnload() {
  APIDefs->UnregisterRender(AddonOptions);
  APIDefs->UnregisterRender(AddonRender);

  APIDefs->UnsubscribeEvent(WINDOW_RESIZED, OnWindowResized);

  APIDefs->UnregisterWndProc(WndProc);

  MumbleLink = nullptr;
  NexusLink = nullptr;

  json settings_json = keys;
  Settings::Settings["AllKeybindings"] = settings_json;
  Settings::Save(SettingsPath);
}

void keyPressedText(struct m_key_s key) {
  if (key.pressed) {
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - key.start_pressing);
    ImGui::Text("%s pressed, %ld ms", key.binding_name.c_str(),
                duration.count());
  } else {
    ImGui::Text("%s not pressed, %ld ms", key.binding_name.c_str(),
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    key.end_pressing - key.start_pressing)
                    .count());
  }
}

ImVec2 pos;
void displayKey(std::unordered_map<char, Texture *> textures,
                std::pair<char, struct m_key_s> key) {
  if (textures[key.first] && textures[key.first]->Resource) {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
    ImGui::SetCursorPos(key.second.pos);
    ImGui::ImageButton(textures[key.first]->Resource, ImVec2(48, 48));
    if (ImGui::IsItemActive()) {
      draggingButton = key.first;
      windowPos = ImGui::GetWindowPos();
    }
    ImGui::Text(keys[key.first].binding_name.c_str());
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
  } else {
    keyPressedText(keys[key.first]);
  }
}

void AddonRender() {
  if (Settings::IsWidgetEnabled) {
    pos = ImGui::GetCursorPos();
    // ImGui::ShowUserGuide();
    // ImGui::ShowDemoWindow();
    // ImGui::ShowStyleEditor();
    // ImGui::ShowStyleSelector("label");
    // ImGui::ShowMetricsWindow();
    /* use Menomonia */
    ImGui::PushFont(NexusLink->Font);
    ImGui::PushItemWidth(1200.f);
    if (ImGui::Begin("KEYBOARD_OVERLAY", (bool *)0,
                     // ImGuiWindowFlags_NoBackground |
                     // ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_NoScrollbar)) {
      for (auto &&key : keys) {
        if (key.second.pressed)
          displayKey(textures_pressed, key);
        else
          displayKey(textures_not_pressed, key);
      }
    }
    ImGui::PopFont();
    ImGui::End();
  }
}

void deleteKey(char code) { keys.erase(code); }

void AddonOptions() {
  ImGui::Text("Keyboard Overlay");
  ImGui::TextDisabled("Widget");
  if (ImGui::Checkbox("Enabled##Widget", &Settings::IsWidgetEnabled)) {
    Settings::Settings[IS_KEYBOARD_OVERLAY_VISIBLE] = Settings::IsWidgetEnabled;
    Settings::Save(SettingsPath);
  }
  for (auto &&key : keys) {
    ImGui::PushID(key.first);
    ImGui::Text("%s Key", key.second.binding_name.c_str());
    ImGui::SameLine();
    if (keybindingToChange == key.first) {
      ImGui::Button("Press key to bind");
    } else if (ImGui::Button(key.second.key_name.c_str())) {
      keybindingToChange = key.first;
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete Key")) {
      deleteKey(key.first);
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

  if (str == "TEX_Z_PRESSED") {
    textures_pressed['Z'] = aTexture;
  } else if (str == "TEX_Z_NOT_PRESSED") {
    textures_not_pressed['Z'] = aTexture;
  } else if (str == "TEX_Q_PRESSED") {
    textures_pressed['Q'] = aTexture;
  } else if (str == "TEX_Q_NOT_PRESSED") {
    textures_not_pressed['Q'] = aTexture;
  } else if (str == "TEX_S_PRESSED") {
    textures_pressed['S'] = aTexture;
  } else if (str == "TEX_S_NOT_PRESSED") {
    textures_not_pressed['S'] = aTexture;
  } else if (str == "TEX_D_PRESSED") {
    textures_pressed['D'] = aTexture;
  } else if (str == "TEX_D_NOT_PRESSED") {
    textures_not_pressed['D'] = aTexture;
  }
}
