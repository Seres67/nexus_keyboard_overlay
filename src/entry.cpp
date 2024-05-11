#include <cstring>
#include <ctime>
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

unsigned int draggingButton = UINT_MAX;
ImVec2 initial_button_pos;
ImVec2 offset{-1, -1};

std::map<unsigned int, Key> keys;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        hSelf = hModule;
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
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
    unsigned int code = MapVirtualKey(i_key, MAPVK_VK_TO_VSC_EX);
    // NUMPAD, arrow keys, right alt, right ctrl, ins, del, home, end, page
    // up, page down, num lock, (ctrl + pause), print screen, divide
    // (numpad), enter (numpad)
    if (lParam >> 24 == 1)
        Log::debug("special key");
    for (auto &&key : keys) {
        if (!key.second.isKeyPressed() && code == key.first)
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
    int c = -1;
    if (i_key & MK_LBUTTON)
        c = VK_LBUTTON;
    else if (i_key & MK_RBUTTON)
        c = VK_RBUTTON;
    else if (i_key & MK_XBUTTON1)
        c = VK_XBUTTON1;
    else if (i_key & MK_XBUTTON2)
        c = VK_XBUTTON2;
    if (c == -1)
        return;
    for (auto &&key : keys)
        if (!key.second.isKeyPressed() && c == key.first)
            key.second.keyDown();
}

void LeftMouseButtonUp()
{
    if (keys.count(VK_LBUTTON))
        keys[VK_LBUTTON].keyUp();
}

void RightMouseButtonUp()
{
    if (keys.count(VK_RBUTTON))
        keys[VK_RBUTTON].keyUp();
}

void MouseButtonUp(WPARAM wParam)
{
    unsigned int button = HIWORD(wParam);
    int c = -1;
    if (button == 1)
        c = VK_XBUTTON1;
    else if (button == 2)
        c = VK_XBUTTON2;
    if (c == -1)
        return;
    if (keys.count(c))
        keys[c].keyUp();
}

void setKeybinding(WPARAM key)
{
    unsigned int key_code = MapVirtualKey(key, MAPVK_VK_TO_VSC_EX);
    char buf[32];
    long long_code = (long)key_code << 16;
    GetKeyNameTextA(long_code, buf, sizeof(buf));
    keys[key_code].setKeyCode(key_code);
    keys[key_code].setKeyName(buf);
    keys[key_code].setPos(keys[keybindingToChange].getPos());
    keys[key_code].setDisplayName(keys[keybindingToChange].getDisplayName());
    keys[key_code].reset();
    keys.erase(keybindingToChange);
    keybindingToChange = UINT_MAX;
}

void addKeybinding(WPARAM key)
{
    unsigned int key_code = MapVirtualKey(key, MAPVK_VK_TO_VSC_EX);
    char buf[32];
    long long_code = (long)key_code << 16;
    GetKeyNameTextA(long_code, buf, sizeof(buf));
    addingKeybinding = false;
    keys[key_code].setKeyCode(key_code);
    keys[key_code].setKeyName(buf);
    keys[key_code].setDisplayName(newKeybindingName);
    keys[key_code].reset();
    memset(newKeybindingName, 0, sizeof(newKeybindingName));
}

void setMouseKeybinding(WPARAM wParam)
{
    int c = -1;
    std::string name;
    if (wParam & MK_LBUTTON) {
        c = VK_LBUTTON;
        name = "Left Click";
    } else if (wParam & MK_RBUTTON) {
        c = VK_RBUTTON;
        name = "Right Click";
    } else if (wParam & MK_XBUTTON1) {
        c = VK_XBUTTON1;
        name = "Mouse Button 1";
    } else if (wParam & MK_XBUTTON2) {
        c = VK_XBUTTON2;
        name = "Mouse Button 2";
    }
    if (c == -1)
        return;
    keys[c].setKeyCode(c);
    keys[c].setKeyName(name);
    keys[c].setPos(keys[keybindingToChange].getPos());
    keys[c].setDisplayName(keys[keybindingToChange].getDisplayName());
    keys[c].reset();
    keys.erase(keybindingToChange);
    keybindingToChange = UINT_MAX;
}

void addMouseButton(WPARAM wParam)
{
    int c = -1;
    std::string name;
    if (wParam & MK_LBUTTON) {
        c = VK_LBUTTON;
        name = "Left Click";
    } else if (wParam & MK_RBUTTON) {
        c = VK_RBUTTON;
        name = "Right Click";
    } else if (wParam & MK_XBUTTON1) {
        c = VK_XBUTTON1;
        name = "Mouse Button 1";
    } else if (wParam & MK_XBUTTON2) {
        c = VK_XBUTTON2;
        name = "Mouse Button 2";
    }
    if (c == -1)
        return;
    addingKeybinding = false;
    keys[c].setKeyCode(c);
    keys[c].setKeyName(name);
    keys[c].setDisplayName(newKeybindingName);
    keys[c].reset();
    memset(newKeybindingName, 0, sizeof(newKeybindingName));
}

unsigned int WndProc(HWND hWnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
    if (addingKeybinding) {
        if (uMsg == WM_KEYDOWN)
            addKeybinding(wParam);
        else if (uMsg == WM_XBUTTONDOWN || uMsg == WM_LBUTTONDOWN ||
                 uMsg == WM_RBUTTONDOWN)
            addMouseButton(wParam);
    } else if (keybindingToChange == UINT_MAX) {
        switch (uMsg) {
        case WM_KEYDOWN:
            KeyDown(wParam, lParam);
            break;
        case WM_KEYUP:
            KeyUp(wParam);
            break;
        case WM_LBUTTONDOWN:
        case WM_XBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_XBUTTONDBLCLK:
            MouseButtonDown(wParam);
            break;
        case WM_LBUTTONUP:
            LeftMouseButtonUp();
            break;
        case WM_RBUTTONUP:
            RightMouseButtonUp();
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
            if (offset.x == -1)
                offset = {static_cast<float>(x) - initial_button_pos.x,
                          static_cast<float>(y) - initial_button_pos.y};
            keys[draggingButton].setPos({static_cast<float>(x) - offset.x,
                                         static_cast<float>(y) - offset.y});
        } else if (uMsg == WM_LBUTTONUP) {
            draggingButton = UINT_MAX;
            offset = {-1, -1};
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

void showTimers(std::pair<unsigned int, Key> key, ImVec2 &timerPos)
{
    timerPos.y += 55;
    // 7 = average pixel size of a char
    // 3 = " ms" char count
    // 4 = average number of other chars
    timerPos.x += (Settings::KeySize - (7 * (3 + 4))) / 2;
    ImGui::SetCursorPos(timerPos);
    ImGui::Text("%lld ms", key.second.getPressedDuration());
}

void displayKey(const std::pair<unsigned int, Key> &key)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ImVec4(0.8f, 0.8f, 0.8f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.8f, 0.8f, 1.f));
    ImGui::SetCursorPos(key.second.getPos());
    if (key.second.isKeyPressed()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.694f, 0.612f, 0.851f, 0.8f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.298f, 0.298f, 0.298f, 0.8f));
    }
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
        initial_button_pos = key.second.getPos();
    }
    if (Settings::ShowKeyTimers) {
        ImVec2 timerPos = key.second.getPos();
        showTimers(key, timerPos);
    }
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
}

ImGuiWindowFlags windowFlags =
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing |
    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar;
void AddonRender()
{
    if (Settings::IsBackgroundTransparent &&
        (windowFlags & ImGuiWindowFlags_NoBackground) == 0)
        windowFlags |= ImGuiWindowFlags_NoBackground |
                       ImGuiWindowFlags_NoDecoration |
                       ImGuiWindowFlags_NoInputs;
    else if (!Settings::IsBackgroundTransparent &&
             (windowFlags & ImGuiWindowFlags_NoBackground) != 0) {
        windowFlags = windowFlags & ~ImGuiWindowFlags_NoBackground;
        windowFlags = windowFlags & ~ImGuiWindowFlags_NoDecoration;
        windowFlags = windowFlags & ~ImGuiWindowFlags_NoInputs;
        windowFlags |= ImGuiWindowFlags_NoTitleBar;
    }
    if (Settings::IsKeyboardOverlayEnabled) {
        ImGui::PushFont(NexusLink->Font);
        if (ImGui::Begin("KEYBOARD_OVERLAY", nullptr, windowFlags)) {
            ImGui::SetWindowFontScale(Settings::WindowScale);
            for (const auto &key : keys)
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
    if (ImGui::Checkbox("Enabled##Widget",
                        &Settings::IsKeyboardOverlayEnabled)) {
        Settings::Settings[IS_KEYBOARD_OVERLAY_ENABLED] =
            Settings::IsKeyboardOverlayEnabled;
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
    int key_to_delete = -1;
    for (const auto &key : keys) {
        ImGui::PushID(static_cast<int>(key.first));
        ImGui::Text("%s Key", key.second.getDisplayName().c_str());
        ImGui::SameLine();
        if (keybindingToChange == key.first) {
            ImGui::Button("Press key to bind");
        } else if (ImGui::Button(key.second.getKeyName().c_str())) {
            keybindingToChange = key.first;
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete Key")) {
            char log[80];
            sprintf(log, "deleting %d\n", key.first);
            Log::debug(log);
            key_to_delete = static_cast<int>(key.first);
            if (keybindingToChange == key.first)
                keybindingToChange = UINT_MAX;
        }
        ImGui::PopID();
    }
    if (key_to_delete != -1)
        deleteKey(key_to_delete);
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
