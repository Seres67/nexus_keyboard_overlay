#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <ratio>
#include <string>
#include <vector>
#include <chrono>
#include <windows.h>

#include "Settings.h"
#include "Shared.h"

// #include "Version.h"

#include "imgui/imgui.h"
#include "imgui/imgui_extensions.h"
#include "nexus/Nexus.h"

void ProcessKeybind(const char *aIdentifier);
void OnWindowResized(void *aEventArgs);
void ReceiveTexture(const char *aIdentifier, Texture *aTexture);

void AddonLoad(AddonAPI *aApi);
void AddonUnload();
void AddonRender();
void AddonOptions();

// little helper function for compass render
std::string GetMarkerText(int aRotation, bool notch = true);
std::string GetClosestMarkerText(int aRotation, bool full = false);

HMODULE hSelf;

AddonDefinition AddonDef{};

std::filesystem::path AddonPath;
std::filesystem::path SettingsPath;

Texture *hrTex = nullptr;

float padding = 5.0f;

const char *FORWARD_PRESSED = "KB_FORWARD_PRESSED";
const char *WINDOW_RESIZED = "EV_WINDOW_RESIZED";
const char *MUMBLE_IDENITY_UPDATED = "EV_MUMBLE_IDENTITY_UPDATED";
const char *HR_TEX = "TEX_SEPARATOR_DETAIL";

bool Z_pressed = false;
std::chrono::time_point<std::chrono::steady_clock> Z_start_pressing;
std::chrono::time_point<std::chrono::steady_clock> Z_end_pressing;
bool Q_pressed = false;

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

  /* not necessary if hosted on Raidcore, but shown anyway for the example also
   * useful as a backup resource */
  AddonDef.Provider = EUpdateProvider_GitHub;
  AddonDef.UpdateLink = "https://github.com/Seres67/nexus-keyboard-overlay";

  return &AddonDef;
}

void KeyDown(WPARAM key) {
  if (key == 0x5A && !Z_pressed) {
    Z_pressed = true;
    Z_start_pressing = std::chrono::steady_clock::now();
  }
  else if (key == 0x51)
    Q_pressed = true;
}

void KeyUp(WPARAM key) {
  if (key == 0x5A && Z_pressed) {
    Z_pressed = false;
    Z_end_pressing = std::chrono::steady_clock::now();
  }
  else if (key == 0x51)
    Q_pressed = false;
}

UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

  // APIDefs->RegisterKeybindWithString(FORWARD_PRESSED, ProcessKeybind,
  // Settings::ForwardKey.c_str());

  OnWindowResized(nullptr); // initialise self
}
void AddonUnload() {
  APIDefs->UnregisterRender(AddonOptions);
  APIDefs->UnregisterRender(AddonRender);

  APIDefs->UnsubscribeEvent(WINDOW_RESIZED, OnWindowResized);

  APIDefs->UnregisterKeybind(FORWARD_PRESSED);

  MumbleLink = nullptr;
  NexusLink = nullptr;

  Settings::Save(SettingsPath);
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
      if (Z_pressed) {
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Z_start_pressing);
        ImGui::Text("Z pressed, %ld ms", duration.count());
      } else {
        ImGui::Text("Z not pressed, %ld ms", std::chrono::duration_cast<std::chrono::milliseconds>(Z_end_pressing - Z_start_pressing).count());
      }
      if (Q_pressed) {
        ImGui::Text("Q pressed");
      } else {
        ImGui::Text("Q not pressed");
      }
    }

    ImGui::PopFont();

    ImGui::End();
  }
}

void AddonOptions() {
  ImGui::Text("Keyboard Overlay");
  ImGui::TextDisabled("Widget");
  if (ImGui::Checkbox("Enabled##Widget", &Settings::IsWidgetEnabled)) {
    Settings::Settings[IS_KEYBOARD_OVERLAY_VISIBLE] = Settings::IsWidgetEnabled;
    Settings::Save(SettingsPath);
  }
  // if (ImGui::DragFloat("Vertical Offset##Widget", &Settings::WidgetOffsetV,
  //                      1.0f, NexusLink->Height * -1.0f,
  //                      NexusLink->Height * 1.0f)) {
  //   Settings::Settings[COMPASS_STRIP_OFFSET_V] = Settings::WidgetOffsetV;
  //   OnWindowResized(nullptr);
  //   Settings::Save(SettingsPath);
  // }

  // ImGui::Checkbox("Compass World", &IsWorldCompassVisible);

  // if (ImGui::Checkbox("Locked##Indicator", &Settings::IsIndicatorLocked)) {
  //   Settings::Settings[IS_COMPASS_INDICATOR_LOCKED] =
  //       Settings::IsIndicatorLocked;
  //   Settings::Save(SettingsPath);
  // }
}

void ProcessKeybind(const char *aIdentifier) {
  std::string str = aIdentifier;

  if (str == FORWARD_PRESSED) {
    APIDefs->Log(ELogLevel_INFO, "pressed forward");
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
