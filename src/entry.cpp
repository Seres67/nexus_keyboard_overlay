#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <iterator>
#include <string>
#include <vector>
#include <windows.h>

#include "Settings.h"
#include "Shared.h"

// #include "Version.h"

#include "imgui/imgui.h"
#include "imgui/imgui_extensions.h"
#include "nexus/Nexus.h"

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

const char *WINDOW_RESIZED = "EV_WINDOW_RESIZED";
const char *HR_TEX = "TEX_SEPARATOR_DETAIL";

struct m_key_s {
  char *binding_name;
  char *key_name;
  char code;
  bool pressed;
  std::chrono::time_point<std::chrono::steady_clock> start_pressing;
  std::chrono::time_point<std::chrono::steady_clock> end_pressing;
};

std::vector<struct m_key_s> KEYS = {
    {strdup("Forward"), strdup("Z"), 'z', false,
     std::chrono::steady_clock::now(), std::chrono::steady_clock::now()},
    {strdup("Left"), strdup("Q"), 'q', false, std::chrono::steady_clock::now(),
     std::chrono::steady_clock::now()},
    {strdup("Backwards"), strdup("S"), 's', false,
     std::chrono::steady_clock::now(), std::chrono::steady_clock::now()},
    {strdup("Right"), strdup("D"), 'd', false, std::chrono::steady_clock::now(),
     std::chrono::steady_clock::now()},
    {strdup("Jump"), strdup("Space"), ' ', false,
     std::chrono::steady_clock::now(), std::chrono::steady_clock::now()}};

std::array<bool, 5> waitForKeybindings = {0};

char newKeybindingName[20];
bool addingKeybinding = false;

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
  AddonDef.Signature = 17;
  AddonDef.APIVersion = NEXUS_API_VERSION;
  AddonDef.Name = "Keyboard Overlay";
  AddonDef.Version.Major = 1;
  AddonDef.Version.Minor = 0;
  AddonDef.Version.Build = 0;
  AddonDef.Version.Revision = 0;
  AddonDef.Author = "Seres67";
  AddonDef.Description = "Adds a simple keyboard overlay to the UI.";
  AddonDef.Load = AddonLoad;
  AddonDef.Unload = AddonUnload;
  AddonDef.Flags = EAddonFlags_None;
  AddonDef.Provider = EUpdateProvider_GitHub;
  AddonDef.UpdateLink = "https://github.com/Seres67/nexus-keyboard-overlay";

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

void setKeybinding(int index, WPARAM wParam) {
  char c = MapVirtualKey(wParam, MAPVK_VK_TO_CHAR);
  waitForKeybindings[index] = false;
  if (wParam >= 'A' && wParam <= 'Z')
    KEYS[index].code = c + 32;
  else
    KEYS[index].code = c;
  if (wParam == VK_SPACE)
    KEYS[index].key_name = strdup("Space");
  else {
    KEYS[index].key_name[0] = c;
    KEYS[index].key_name[1] = 0;
  }
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
    new_key.key_name = strdup("Space");
  else {
    new_key.key_name = (char *)malloc(sizeof(char) * 2);
    if (!new_key.key_name)
      return;
    new_key.key_name[0] = c;
    new_key.key_name[1] = 0;
  }
  new_key.binding_name = strdup(newKeybindingName);
  char log[80];
  sprintf(log, "%zu", KEYS.size());
  APIDefs->Log(ELogLevel_INFO, log);
  KEYS.emplace_back(new_key);
  sprintf(log, "%zu", KEYS.size());
  APIDefs->Log(ELogLevel_INFO, log);
}

UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  auto it =
      std::find_if(std::begin(waitForKeybindings), std::end(waitForKeybindings),
                   [](auto &b) { return b; });
  if (addingKeybinding) {
    if (uMsg == WM_KEYDOWN)
      addKeybinding(wParam);
  } else if (it == std::end(waitForKeybindings)) {
    switch (uMsg) {
    case WM_KEYDOWN:
      KeyDown(wParam);
      break;
    case WM_KEYUP:
      KeyUp(wParam);
      break;
    default:
      break;
    }
  } else {
    for (int i = 0; i < KEYS.size(); ++i)
      if (it - std::begin(waitForKeybindings) == i)
        if (uMsg == WM_KEYDOWN) {
          setKeybinding(i, wParam);
        }
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

  OnWindowResized(nullptr); // initialise self
}
void AddonUnload() {
  APIDefs->UnregisterRender(AddonOptions);
  APIDefs->UnregisterRender(AddonRender);

  APIDefs->UnsubscribeEvent(WINDOW_RESIZED, OnWindowResized);

  APIDefs->UnregisterWndProc(WndProc);

  MumbleLink = nullptr;
  NexusLink = nullptr;

  Settings::Save(SettingsPath);
}

void keyPressedText(int index) {
  if (KEYS[index].pressed) {
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - KEYS[index].start_pressing);
    ImGui::Text("%s pressed, %ld ms", KEYS[index].binding_name,
                duration.count());
  } else {
    ImGui::Text("%s not pressed, %ld ms", KEYS[index].binding_name,
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
    ImGuiIO &io = ImGui::GetIO();

    /* use Menomonia */
    ImGui::PushFont(NexusLink->Font);

    /* set width and position */
    ImGui::PushItemWidth(1200.f);
    if (ImGui::Begin("KEYBOARD_OVERLAY", (bool *)0,
                     ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_NoScrollbar)) {

      /* use Menomonia but bigger */
      ImGui::PushFont(NexusLink->FontBig);
      for (int i = 0; i < KEYS.size(); ++i)
        keyPressedText(i);
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
    ImGui::Text("%s Key", KEYS[i].binding_name);
    ImGui::SameLine();
    if (ImGui::Button("...")) {
      waitForKeybindings[i] = true;
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
  if (ImGui::Button("Add Key")) {
    addingKeybinding = true;
  }
}

void OnWindowResized(void *aEventArgs) { /* event args are nullptr, ignore */
}

void ReceiveTexture(const char *aIdentifier, Texture *aTexture) {
  std::string str = aIdentifier;

  if (str == HR_TEX) {
    hrTex = aTexture;
  }
}
