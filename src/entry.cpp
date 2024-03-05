#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cwchar>
#include <filesystem>
#include <map>
#include <string>
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>

#include "Key.hpp"
#include "Settings.h"
#include "Shared.h"

#include "imgui/imgui.h"
#include "nexus/Nexus.h"
#include "nlohmann/json.hpp"
#include "utils.h"

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

ImVec2 image_size = ImVec2(72, 72);

UINT keybindingToChange = -1;
char newKeybindingName[10];
bool addingKeybinding = false;

ImVec2 windowPos;
UINT draggingButton = -1;
ImVec2 dragPos;

const char *WINDOW_RESIZED = "EV_WINDOW_RESIZED";

std::map<UINT, Key> keys;
std::unordered_map<char, Texture *> textures_not_pressed;
std::unordered_map<char, Texture *> textures_pressed;

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
  AddonDef.Version.Minor = 6;
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

void KeyDown(WPARAM i_key, LPARAM lParam) {
  UINT c = MapVirtualKey(i_key, MAPVK_VK_TO_VSC_EX);
  char log[80];
  if (lParam >> 24 == 1) {
    Log::debug("numpad key");
    // NUMPAD, arrow keys, right alt, right ctrl, ins, del, home, end, page up,
    // page down, num lock, (ctrl + pause), print screen, divide (numpad), enter
    // (numpad)
  }

  for (auto &&key : keys) {
    sprintf(log, "received %d, checking against %d", c,
            key.second.getKeyCode());
    Log::debug(log);

    if (!key.second.isKeyPressed() && c == key.second.getKeyCode()) {
      key.second.keyDown();
    }
  }
}

void KeyUp(WPARAM i_key) {
  UINT c = MapVirtualKey(i_key, MAPVK_VK_TO_VSC_EX);
  for (auto &&key : keys) {
    if (key.second.isKeyPressed() && c == key.second.getKeyCode()) {
      key.second.keyUp();
    }
  }
}

void MouseButtonDown(WPARAM i_key) {
  for (auto &&key : keys) {
    if (!key.second.isKeyPressed() &&
        GET_XBUTTON_WPARAM(i_key) == key.second.getKeyCode()) {
      key.second.keyDown();
    }
  }
}

void MouseButtonUp(WPARAM i_key) {
  for (auto &&key : keys) {
    if (key.second.isKeyPressed() &&
        GET_XBUTTON_WPARAM(i_key) == key.second.getKeyCode()) {
      key.second.keyUp();
    }
  }
}

void setKeybinding(WPARAM key) {
  UINT key_code = MapVirtualKey(key, MAPVK_VK_TO_VSC_EX);
  char c = MapVirtualKey(key, MAPVK_VK_TO_CHAR);
  // if (wParam >= 'A' && wParam <= 'Z')
  //   keys[c].setKeyCode(c + 32);
  // else
  keys[key_code].setKeyCode(key_code);
  if (key == VK_SPACE)
    keys[key_code].setKeyName("Space");
  else {
    keys[key_code].setKeyName({c});
  }
  keys[key_code].setPos(keys[keybindingToChange].getPos());
  keys[key_code].setDisplayName(keys[keybindingToChange].getDisplayName());
  keys[key_code].reset();
  keys.erase(keybindingToChange);
  keybindingToChange = -1;
}

void addKeybinding(WPARAM key, LPARAM lParam) {
  UINT key_code = MapVirtualKey(key, MAPVK_VK_TO_VSC_EX);
  char c = MapVirtualKey(key, MAPVK_VK_TO_CHAR);

  addingKeybinding = false;
  // if (key >= 'A' && key <= 'Z')
  //   keys[c].setKeyCode(c + 32);
  // else
  keys[key_code].setKeyCode(key_code);
  if (key == VK_SPACE)
    keys[key_code].setKeyName("Space");
  else {
    keys[key_code].setKeyName({c});
  }
  keys[key_code].setDisplayName(newKeybindingName);
  memset(newKeybindingName, 0, sizeof(newKeybindingName));
}

void setMouseKeybinding(WPARAM wParam) {
  UINT c = GET_XBUTTON_WPARAM(wParam);

  keys[c].setKeyCode(c);
  keys[c].setKeyName(c == 1 ? "Button4" : "Button5");
  keys[c].setPos(keys[keybindingToChange].getPos());
  keys[c].setDisplayName(keys[keybindingToChange].getDisplayName());
  keys[c].reset();
  keys.erase(keybindingToChange);
  keybindingToChange = -1;
}

void addMouseButton(WPARAM wParam) {
  UINT button_code = GET_XBUTTON_WPARAM(wParam);
  addingKeybinding = false;
  keys[button_code].setKeyCode(button_code);
  keys[button_code].setKeyName(button_code == 1 ? "Button4" : "Button5");
  keys[button_code].setDisplayName(newKeybindingName);
  memset(newKeybindingName, 0, sizeof(newKeybindingName));
}

UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (addingKeybinding) {
    if (uMsg == WM_KEYDOWN)
      addKeybinding(wParam, lParam);
    else if (uMsg == WM_XBUTTONDOWN)
      addMouseButton(wParam);
  } else if (keybindingToChange == -1) {
    switch (uMsg) {
    case WM_KEYDOWN:
      KeyDown(wParam, lParam);
      break;
    case WM_KEYUP:
      KeyUp(wParam);
      break;
    case WM_XBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
      MouseButtonDown(wParam);
      break;
    case WM_XBUTTONUP:
      MouseButtonUp(wParam);
      break;
    default:
      break;
    }
  } else {
    if (uMsg == WM_KEYDOWN)
      setKeybinding(wParam);
    else if (uMsg == WM_XBUTTONDOWN || uMsg == WM_XBUTTONDBLCLK)
      setMouseKeybinding(wParam);
  }
  if (draggingButton != -1) {
    if (uMsg == WM_MOUSEMOVE) {
      int x = GET_X_LPARAM(lParam);
      int y = GET_Y_LPARAM(lParam);
      keys[draggingButton].setPos({x - windowPos.x, y - windowPos.y});
    } else if (uMsg == WM_LBUTTONUP) {
      draggingButton = -1;
    }
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

void keyPressedText(Key &key) {
  if (key.isKeyPressed()) {

    ImGui::Text("%s pressed, %lld ms", key.getDisplayName().c_str(),
                key.getPressedDuration());
  } else {
    ImGui::Text("%s not pressed, %lld ms", key.getDisplayName().c_str(),
                key.getPressedDuration());
  }
}

ImVec2 pos;
void displayKey(std::unordered_map<char, Texture *> textures,
                std::pair<char, Key> key) {
  // TODO: this is wrong
  if (textures[key.second.getKeyName()[0]] &&
      textures[key.second.getKeyName()[0]]->Resource) {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
    ImGui::SetCursorPos(key.second.getPos());
    ImGui::ImageButton(textures[key.second.getKeyName()[0]]->Resource,
                       image_size);
    if (ImGui::IsItemActive()) {
      draggingButton = key.first;
      windowPos = ImGui::GetWindowPos();
    }
    if (Settings::ShowKeyLabels) {
      ImVec2 timerPos = key.second.getPos();
      ImVec2 labelPos = key.second.getPos();
      timerPos.y += 55;
      // 7 = ~pixel size of a char
      // 3 = " ms" char count
      // 4 = average number of other chars
      timerPos.x += (image_size.x - (7 * (3 + 4))) / 2;
      labelPos.x +=
          (image_size.x - (7 * key.second.getDisplayName().size())) / 2;
      ImGui::SetCursorPos(labelPos);
      ImGui::Text("%s", key.second.getDisplayName().c_str());
      ImGui::SetCursorPos(timerPos);
      if (key.second.isKeyPressed()) {
        ImGui::Text("%lld ms", key.second.getPressedDuration());
      } else {
        ImGui::Text("%lld ms", key.second.getPressedDuration());
      }
    }
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
  } else {
    keyPressedText(keys[key.first]);
  }
}

void AddonRender() {
  if (Settings::IsWidgetEnabled) {
    pos = ImGui::GetCursorPos();
    ImGui::PushFont(NexusLink->Font);
    if (ImGui::Begin("KEYBOARD_OVERLAY", (bool *)0,
                     // ImGuiWindowFlags_NoBackground |
                     // ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_NoScrollbar)) {
      ImGui::SetWindowFontScale(0.9f);
      for (auto &&key : keys) {
        if (key.second.isKeyPressed())
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
  if (ImGui::Checkbox("Show Key Labels##KeyLabels", &Settings::ShowKeyLabels)) {
    Settings::Settings[SHOW_KEY_LABELS] = Settings::ShowKeyLabels;
    Settings::Save(SettingsPath);
  }
  for (auto &&key : keys) {
    ImGui::PushID(key.first);
    ImGui::Text("%s Key", key.second.getDisplayName().c_str());
    ImGui::SameLine();
    if (keybindingToChange == key.first) {
      ImGui::Button("Press key to bind");
    } else if (ImGui::Button(key.second.getKeyName().c_str())) {
      keybindingToChange = key.first;
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete Key")) {
      deleteKey(key.first);
      if (keybindingToChange == key.first)
        keybindingToChange = -1;
    }
    ImGui::PopID();
  }
  ImGui::Text("New Key Name: ");
  ImGui::SameLine();
  ImGui::InputText("##newKeyName", newKeybindingName, 10);
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
