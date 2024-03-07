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

void AddonLoad(AddonAPI *aApi);
void AddonUnload();
void AddonRender();
void AddonOptions();

HMODULE hSelf;

AddonDefinition AddonDef{};

std::filesystem::path AddonPath;
std::filesystem::path SettingsPath;

unsigned int keybindingToChange = UINT_MAX;
char newKeybindingName[10];
bool addingKeybinding = false;

ImVec2 windowPos;
unsigned int draggingButton = UINT_MAX;
ImVec2 dragPos;

std::map<unsigned int, Key> keys;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved)
{
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

extern "C" __declspec(dllexport) AddonDefinition *GetAddonDef()
{
    AddonDef.Signature = -918234798;
    AddonDef.APIVersion = NEXUS_API_VERSION;
    AddonDef.Name = "Keyboard Overlay";
    AddonDef.Version.Major = 0;
    AddonDef.Version.Minor = 7;
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

void KeyDown(WPARAM i_key, LPARAM lParam)
{
    unsigned int c = MapVirtualKey(i_key, MAPVK_VK_TO_VSC_EX);
    // NUMPAD, arrow keys, right alt, right ctrl, ins, del, home, end, page
    // up, page down, num lock, (ctrl + pause), print screen, divide
    // (numpad), enter (numpad)
    if (lParam >> 24 == 1)
        Log::debug("special key");
    for (auto &&key : keys) {
        if (!key.second.isKeyPressed() && c == key.first)
            key.second.keyDown();
    }
}

void KeyUp(WPARAM i_key)
{
    unsigned int c = MapVirtualKey(i_key, MAPVK_VK_TO_VSC_EX);
    for (auto &&key : keys)
        if (key.second.isKeyPressed() && c == key.first)
            key.second.keyUp();
}

void MouseButtonDown(WPARAM i_key)
{
    unsigned int c = GET_XBUTTON_WPARAM(i_key);
    c <<= 8;
    for (auto &&key : keys)
        if (!key.second.isKeyPressed() && c == key.first)
            key.second.keyDown();
}

void MouseButtonUp(WPARAM i_key)
{
    unsigned int c = GET_XBUTTON_WPARAM(i_key);
    c <<= 8;
    for (auto &&key : keys)
        if (key.second.isKeyPressed() && c == key.first)
            key.second.keyUp();
}

void setKeybinding(WPARAM key)
{
    unsigned int key_code = MapVirtualKey(key, MAPVK_VK_TO_VSC_EX);
    char c = MapVirtualKey(key, MAPVK_VK_TO_CHAR);
    keys[key_code].setKeyCode(key_code);
    if (key == VK_SPACE)
        keys[key_code].setKeyName("Space");
    else
        keys[key_code].setKeyName({c});
    keys[key_code].setPos(keys[keybindingToChange].getPos());
    keys[key_code].setDisplayName(keys[keybindingToChange].getDisplayName());
    keys[key_code].reset();
    keys.erase(keybindingToChange);
    keybindingToChange = UINT_MAX;
}

void addKeybinding(WPARAM key, LPARAM lParam)
{
    unsigned int key_code = MapVirtualKey(key, MAPVK_VK_TO_VSC_EX);
    char c = MapVirtualKey(key, MAPVK_VK_TO_CHAR);

    addingKeybinding = false;
    keys[key_code].setKeyCode(key_code);
    if (key == VK_SPACE)
        keys[key_code].setKeyName("Space");
    else {
        keys[key_code].setKeyName({c});
    }
    keys[key_code].setDisplayName(newKeybindingName);
    keys[key_code].reset();
    memset(newKeybindingName, 0, sizeof(newKeybindingName));
}

void setMouseKeybinding(WPARAM wParam)
{
    unsigned int c = GET_XBUTTON_WPARAM(wParam);
    c <<= 8;

    keys[c].setKeyCode(c);
    keys[c].setKeyName(c == 256 ? "Button4" : "Button5");
    keys[c].setPos(keys[keybindingToChange].getPos());
    keys[c].setDisplayName(keys[keybindingToChange].getDisplayName());
    keys[c].reset();
    keys.erase(keybindingToChange);
    keybindingToChange = UINT_MAX;
}

void addMouseButton(WPARAM wParam)
{
    unsigned int button_code = GET_XBUTTON_WPARAM(wParam);
    button_code <<= 8;
    addingKeybinding = false;
    keys[button_code].setKeyCode(button_code);
    keys[button_code].setKeyName(button_code == 256 ? "Button4" : "Button5");
    keys[button_code].setDisplayName(newKeybindingName);
    keys[button_code].reset();
    memset(newKeybindingName, 0, sizeof(newKeybindingName));
}

unsigned int WndProc(HWND hWnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
    if (addingKeybinding) {
        if (uMsg == WM_KEYDOWN)
            addKeybinding(wParam, lParam);
        else if (uMsg == WM_XBUTTONDOWN)
            addMouseButton(wParam);
    } else if (keybindingToChange == UINT_MAX) {
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
    if (draggingButton != UINT_MAX) {
        if (uMsg == WM_MOUSEMOVE) {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            keys[draggingButton].setPos({x - windowPos.x, y - windowPos.y});
        } else if (uMsg == WM_LBUTTONUP) {
            draggingButton = UINT_MAX;
        }
    }
    return uMsg;
}

void AddonLoad(AddonAPI *aApi)
{
    APIDefs = aApi;
    ImGui::SetCurrentContext(APIDefs->ImguiContext);
    ImGui::SetAllocatorFunctions(
        (void *(*)(size_t, void *))APIDefs->ImguiMalloc,
        (void (*)(void *, void *))APIDefs->ImguiFree); // on imgui 1.80+

    MumbleLink = (Mumble::Data *)APIDefs->GetResource("DL_MUMBLE_LINK");
    NexusLink = (NexusLinkData *)APIDefs->GetResource("DL_NEXUS_LINK");

    APIDefs->RegisterWndProc(WndProc);

    APIDefs->RegisterRender(ERenderType_Render, AddonRender);
    APIDefs->RegisterRender(ERenderType_OptionsRender, AddonOptions);

    AddonPath = APIDefs->GetAddonDirectory("keyboard_overlay");
    SettingsPath = APIDefs->GetAddonDirectory("keyboard_overlay/settings.json");
    std::filesystem::create_directory(AddonPath);
    Settings::Load(SettingsPath);

    if (!Settings::Settings["AllKeybindings"].is_null())
        Settings::Settings["AllKeybindings"].get_to(keys);

    Log::info("finished loading all settings!");
}

void AddonUnload()
{
    APIDefs->UnregisterRender(AddonOptions);
    APIDefs->UnregisterRender(AddonRender);

    APIDefs->UnregisterWndProc(WndProc);

    MumbleLink = nullptr;
    NexusLink = nullptr;

    json settings_json = keys;
    Settings::Settings["AllKeybindings"] = settings_json;
    Settings::Save(SettingsPath);
}

void displayKey(std::pair<unsigned int, Key> key)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ImVec4(0.15f, 0.15f, 0.15f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          ImVec4(0.15f, 0.15f, 0.15f, 0.8f));
    ImGui::SetCursorPos(key.second.getPos());
    if (key.second.isKeyPressed())
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.6f, 0.42f, 0.8f));
    else
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.298f, 0.298f, 0.298f, 0.8f));
    if (key.second.getKeyName() != "Space") {
        ImGui::Button(key.second.getDisplayName().c_str(),
                      {Settings::KeySize, Settings::KeySize});
    } else {
        ImGui::Button(key.second.getDisplayName().c_str(),
                      {Settings::KeySize * 2, Settings::KeySize});
    }
    ImGui::PopStyleColor();
    if (ImGui::IsItemActive()) {
        draggingButton = key.first;
        windowPos = ImGui::GetWindowPos();
    }
    if (Settings::ShowKeyTimers) {
        ImVec2 timerPos = key.second.getPos();
        timerPos.y += 55;
        // 7 = ~pixel size of a char
        // 3 = " ms" char count
        // 4 = average number of other chars
        timerPos.x += (Settings::KeySize - (7 * (3 + 4))) / 2;
        ImGui::SetCursorPos(timerPos);
        if (key.second.isKeyPressed()) {
            ImGui::Text("%lld ms", key.second.getPressedDuration());
        } else {
            ImGui::Text("%lld ms", key.second.getPressedDuration());
        }
    }
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
}

ImGuiWindowFlags windowFlags =
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing |
    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar;
void AddonRender()
{
    if (Settings::IsBackgroundTransparent &&
        (windowFlags & ImGuiWindowFlags_NoBackground) == 0)
        windowFlags |=
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;
    else if (!Settings::IsBackgroundTransparent &&
             (windowFlags & ImGuiWindowFlags_NoBackground) != 0) {
        windowFlags = windowFlags & ~ImGuiWindowFlags_NoBackground;
        windowFlags = windowFlags & ~ImGuiWindowFlags_NoDecoration;
    }
    if (Settings::IsWidgetEnabled) {
        ImGui::PushFont(NexusLink->Font);
        if (ImGui::Begin("KEYBOARD_OVERLAY", (bool *)0, windowFlags)) {
            ImGui::SetWindowFontScale(Settings::WindowScale);
            for (auto &&key : keys)
                displayKey(key);
        }
        ImGui::PopFont();
        ImGui::End();
    }
}

void deleteKey(unsigned int code) { keys.erase(code); }

void AddonOptions()
{
    ImGui::Text("Keyboard Overlay");
    ImGui::TextDisabled("Widget");
    if (ImGui::Checkbox("Enabled##Widget", &Settings::IsWidgetEnabled)) {
        Settings::Settings[IS_KEYBOARD_OVERLAY_VISIBLE] =
            Settings::IsWidgetEnabled;
        Settings::Save(SettingsPath);
    }
    if (ImGui::Checkbox("Transparent Background##background",
                        &Settings::IsBackgroundTransparent)) {
        Settings::Settings[IS_BACKGROUND_TRANSPARENT] =
            Settings::IsBackgroundTransparent;
        Settings::Save(SettingsPath);
    }
    if (ImGui::Checkbox("Show Key Timers##KeyTimers",
                        &Settings::ShowKeyTimers)) {
        Settings::Settings[SHOW_KEY_TIMERS] = Settings::ShowKeyTimers;
        Settings::Save(SettingsPath);
    }
    if (ImGui::SliderFloat("Window Scale##Scale", &Settings::WindowScale, 0.1,
                           3.0)) {
        Settings::Settings[WINDOW_SCALE] = Settings::WindowScale;
        Settings::Save(SettingsPath);
    }
    if (ImGui::SliderFloat("KeySize##KeySize", &Settings::KeySize, 1, 200)) {
        Settings::Settings[KEY_SIZE] = Settings::KeySize;
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
                keybindingToChange = UINT_MAX;
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
