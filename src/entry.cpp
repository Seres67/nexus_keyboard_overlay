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
    AddonDef.Version.Minor = 8;
    AddonDef.Version.Build = 4;
    AddonDef.Version.Revision = 2;
    AddonDef.Author = "Seres67";
    AddonDef.Description = "Adds a modular keyboard overlay to the UI.";
    AddonDef.Load = AddonLoad;
    AddonDef.Unload = AddonUnload;
    AddonDef.Flags = EAddonFlags_None;
    AddonDef.Provider = EUpdateProvider_GitHub;
    AddonDef.UpdateLink = "https://github.com/Seres67/nexus_keyboard_overlay";

    return &AddonDef;
}

void KeyDown(WPARAM i_key, LPARAM lParam)
{
    // NUMPAD, arrow keys, right alt, right ctrl, ins, del, home, end, page
    // up, page down, num lock, (ctrl + pause), print screen, divide
    // (numpad), enter (numpad)
    if (lParam >> 24 == 1)
        Log::debug("special key");
    for (auto &&key : keys)
        if (!key.second.isKeyPressed() && i_key == key.first)
            key.second.keyDown();
}

void KeyUp(WPARAM i_key)
{
    for (auto &&key : keys)
        if (key.second.isKeyPressed() && i_key == key.first)
            key.second.keyUp();
}

void LeftMouseButtonDown()
{
    if (keys.count(VK_LBUTTON))
        keys[VK_LBUTTON].keyDown();
}

void RightMouseButtonDown()
{
    if (keys.count(VK_RBUTTON))
        keys[VK_RBUTTON].keyDown();
}

void MiddleMouseButtonDown()
{
    if (keys.count(VK_MBUTTON))
        keys[VK_MBUTTON].keyDown();
}

void MouseXButtonDown(WPARAM wParam)
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
        keys[c].keyDown();
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

void MiddleMouseButtonUp()
{
    if (keys.count(VK_MBUTTON))
        keys[VK_MBUTTON].keyUp();
}

void MouseXButtonUp(WPARAM wParam)
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
    keys[key].setKeyCode(key_code);
    keys[key].setKeyName(buf);
    keys[key].setPos(keys[keybindingToChange].getPos());
    keys[key].setDisplayName(keys[keybindingToChange].getDisplayName());
    keys[key].setSize(keys[keybindingToChange].getSize());
    keys[key].reset();
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
    keys[key].setKeyCode(key_code);
    keys[key].setKeyName(buf);
    keys[key].setDisplayName(newKeybindingName);
    keys[key].setSize({SettingsVars::KeySize, SettingsVars::KeySize});
    keys[key].reset();
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
    } else if (wParam & MK_MBUTTON) {
        c = VK_MBUTTON;
        name = "Middle Mouse Button";
    }
    if (c == -1)
        return;
    keys[c].setKeyCode(c);
    keys[c].setKeyName(name);
    keys[c].setPos(keys[keybindingToChange].getPos());
    keys[c].setDisplayName(keys[keybindingToChange].getDisplayName());
    keys[c].setSize(keys[keybindingToChange].getSize());
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
    } else if (wParam & MK_MBUTTON) {
        c = VK_MBUTTON;
        name = "Middle Mouse Button";
    }
    if (c == -1)
        return;
    addingKeybinding = false;
    keys[c].setKeyCode(c);
    keys[c].setKeyName(name);
    keys[c].setDisplayName(newKeybindingName);
    keys[c].setSize({SettingsVars::KeySize, SettingsVars::KeySize});
    keys[c].reset();
    memset(newKeybindingName, 0, sizeof(newKeybindingName));
}

unsigned int WndProc(HWND hWnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
    //    if (uMsg != 132 && uMsg != 512 && uMsg != 32) {
    //        char log[80];
    //        sprintf(log, "event: %d", uMsg);
    //        Log::debug(log);
    //    }
    if (addingKeybinding) {
        if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN)
            addKeybinding(wParam);
        else if (uMsg == WM_XBUTTONDOWN || uMsg == WM_LBUTTONDOWN ||
                 uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN)
            addMouseButton(wParam);
    } else if (keybindingToChange == UINT_MAX) {
        switch (uMsg) {
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            KeyDown(wParam, lParam);
            break;
        case WM_SYSKEYUP:
        case WM_KEYUP:
            KeyUp(wParam);
            break;
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
            LeftMouseButtonDown();
            break;
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
            RightMouseButtonDown();
            break;
        case WM_MBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
            MiddleMouseButtonDown();
            break;
        case WM_XBUTTONDOWN:
        case WM_XBUTTONDBLCLK:
            MouseXButtonDown(wParam);
            break;
        case WM_LBUTTONUP:
            LeftMouseButtonUp();
            break;
        case WM_RBUTTONUP:
            RightMouseButtonUp();
            break;
        case WM_MBUTTONUP:
            MiddleMouseButtonUp();
            break;
        case WM_XBUTTONUP:
            MouseXButtonUp(wParam);
            break;
        default:
            break;
        }
    } else {
        if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN)
            setKeybinding(wParam);
        else if (uMsg == WM_XBUTTONDOWN || uMsg == WM_LBUTTONDOWN ||
                 uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN)
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

    AddonPath = APIDefs->GetAddonDirectory("keyboard_overlay");
    SettingsPath = APIDefs->GetAddonDirectory("keyboard_overlay/settings.json");
    std::filesystem::create_directory(AddonPath);
    Settings::Load(SettingsPath);
    if (!Settings::m_json_settings["AllKeybindings"].is_null()) {
        for (auto &key : Settings::m_json_settings["AllKeybindings"]) {
            if (key[1]["m_size.x"].is_null())
                key[1]["m_size.x"] = SettingsVars::KeySize;
            if (key[1]["m_size.y"].is_null())
                key[1]["m_size.y"] = SettingsVars::KeySize;
        }
        Settings::m_json_settings["AllKeybindings"].get_to(keys);
    }

    Log::info("finished applying all settings!");

    APIDefs->RegisterWndProc(WndProc);

    APIDefs->RegisterRender(ERenderType_Render, AddonRender);
    APIDefs->RegisterRender(ERenderType_OptionsRender, AddonOptions);
}

void AddonUnload()
{
    APIDefs->UnregisterRender(AddonOptions);
    APIDefs->UnregisterRender(AddonRender);

    APIDefs->UnregisterWndProc(WndProc);

    MumbleLink = nullptr;
    NexusLink = nullptr;

    json settings_json = keys;
    Settings::m_json_settings["AllKeybindings"] = settings_json;
    Settings::Save(SettingsPath);
}

void showTimers(std::pair<unsigned int, Key> key, ImVec2 &timerPos)
{
    timerPos.y += 55;
    // 7 = average pixel size of a char
    // 3 = " ms" char count
    // 4 = average number of other chars
    timerPos.x += (key.second.getSize().x - (7 * (3 + 4))) / 2;
    ImGui::SetCursorPos(timerPos);
    ImGui::Text("%lld ms", key.second.getPressedDuration());
}

void displayKey(std::pair<const unsigned int, Key> &key)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ImVec4(0.8f, 0.8f, 0.8f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.8f, 0.8f, 1.f));
    int style_cout = 2;
    ImGui::SetCursorPos(key.second.getPos());
    if (key.second.isKeyPressed()) {
        if (!SettingsVars::DisableInChat ||
            !MumbleLink->Context.IsTextboxFocused) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  SettingsVars::KeyPressedColor);
            style_cout += 2;
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.8f));
            ++style_cout;
            if (SettingsVars::IsBackgroundTransparent) {
                ImGui::PushStyleColor(
                    ImGuiCol_Button,
                    ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
                ++style_cout;
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button,
                                      ImVec4(0.298f, 0.298f, 0.298f, 0.8f));
                ++style_cout;
            }
        }
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.8f));
        ++style_cout;
        if (SettingsVars::IsBackgroundTransparent) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
            ++style_cout;
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0.298f, 0.298f, 0.298f, 0.8f));
            ++style_cout;
        }
    }
    ImGui::Button(key.second.getDisplayName().c_str(), key.second.getSize());
    ImGui::PopStyleColor();
    --style_cout;
    if (ImGui::IsItemActive()) {
        draggingButton = key.first;
        initial_button_pos = key.second.getPos();
    }
    if (SettingsVars::ShowKeyTimers) {
        if (!SettingsVars::DisableInChat ||
            !MumbleLink->Context.IsTextboxFocused) {
            ImVec2 timerPos = key.second.getPos();
            showTimers(key, timerPos);
        }
    }
    ImGui::PopStyleColor(style_cout);
    ImGui::PopStyleVar();
}

ImGuiWindowFlags windowFlags =
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing |
    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar;
void AddonRender()
{
    if (SettingsVars::IsBackgroundTransparent &&
        (windowFlags & ImGuiWindowFlags_NoBackground) == 0)
        windowFlags |= ImGuiWindowFlags_NoBackground |
                       ImGuiWindowFlags_NoDecoration |
                       ImGuiWindowFlags_NoInputs;
    else if (!SettingsVars::IsBackgroundTransparent &&
             (windowFlags & ImGuiWindowFlags_NoBackground) != 0) {
        windowFlags = windowFlags & ~ImGuiWindowFlags_NoBackground;
        windowFlags = windowFlags & ~ImGuiWindowFlags_NoDecoration;
        windowFlags = windowFlags & ~ImGuiWindowFlags_NoInputs;
        windowFlags |= ImGuiWindowFlags_NoTitleBar;
    }
    ImGui::SetNextWindowSizeConstraints({40, 40}, {FLT_MAX, FLT_MAX});
    if (SettingsVars::IsKeyboardOverlayEnabled) {
        if (SettingsVars::AlwaysDisplayed || NexusLink->IsGameplay) {
            ImGui::PushFont(NexusLink->Font);
            if (ImGui::Begin("KEYBOARD_OVERLAY", nullptr, windowFlags)) {
                ImGui::SetWindowFontScale(SettingsVars::WindowScale);
                auto pressed_keys =
                    keys |
                    std::views::filter([&](const auto &pair)
                                       { return pair.second.isKeyPressed(); });
                auto unpressed_keys =
                    keys |
                    std::views::filter([&](const auto &pair)
                                       { return !pair.second.isKeyPressed(); });
                for (auto &key : unpressed_keys)
                    displayKey(key);
                for (auto &key : pressed_keys)
                    displayKey(key);
            }
            ImGui::PopFont();
            ImGui::End();
        }
    }
}

void deleteKey(unsigned int code) { keys.erase(code); }

namespace ImGui
{
bool ColorEdit4U32(const char *label, ImU32 *color,
                   ImGuiColorEditFlags flags = 0)
{
    float col[4];
    col[0] = (float)((*color >> 0) & 0xFF) / 255.0f;
    col[1] = (float)((*color >> 8) & 0xFF) / 255.0f;
    col[2] = (float)((*color >> 16) & 0xFF) / 255.0f;
    col[3] = (float)((*color >> 24) & 0xFF) / 255.0f;

    bool result = ColorEdit4(label, col, flags);

    *color = ((ImU32)(col[0] * 255.0f)) | ((ImU32)(col[1] * 255.0f) << 8) |
             ((ImU32)(col[2] * 255.0f) << 16) |
             ((ImU32)(col[3] * 255.0f) << 24);

    return result;
}
} // namespace ImGui

void AddonOptions()
{
    ImGui::Text("Keyboard Overlay");
    ImGui::TextDisabled("Widget");
    if (ImGui::Checkbox("Enabled##Widget",
                        &SettingsVars::IsKeyboardOverlayEnabled)) {
        Settings::m_json_settings[IS_KEYBOARD_OVERLAY_ENABLED] =
            SettingsVars::IsKeyboardOverlayEnabled;
        Settings::Save(SettingsPath);
    }
    if (ImGui::Checkbox("Always Displayed##always",
                        &SettingsVars::AlwaysDisplayed)) {
        Settings::m_json_settings[ALWAYS_DISPLAYED] =
            SettingsVars::AlwaysDisplayed;
        Settings::Save(SettingsPath);
    }
    if (ImGui::Checkbox("Transparent Background##background",
                        &SettingsVars::IsBackgroundTransparent)) {
        Settings::m_json_settings[IS_BACKGROUND_TRANSPARENT] =
            SettingsVars::IsBackgroundTransparent;
        Settings::Save(SettingsPath);
    }
    if (ImGui::Checkbox("Disable while typing in chat##background",
                        &SettingsVars::DisableInChat)) {
        Settings::m_json_settings[IS_BACKGROUND_TRANSPARENT] =
            SettingsVars::IsBackgroundTransparent;
        Settings::Save(SettingsPath);
    }
    if (ImGui::Checkbox("Show Key Timers##KeyTimers",
                        &SettingsVars::ShowKeyTimers)) {
        Settings::m_json_settings[SHOW_KEY_TIMERS] =
            SettingsVars::ShowKeyTimers;
        Settings::Save(SettingsPath);
    }
    if (ImGui::SliderFloat("Window Scale##Scale", &SettingsVars::WindowScale,
                           0.1, 3.0)) {
        Settings::m_json_settings[WINDOW_SCALE] = SettingsVars::WindowScale;
        Settings::Save(SettingsPath);
    }
    if (ImGui::SliderFloat("Default key size##KeySize", &SettingsVars::KeySize, 1,
                           200)) {
        Settings::m_json_settings[KEY_SIZE] = SettingsVars::KeySize;
        Settings::Save(SettingsPath);
    }
    if (ImGui::ColorEdit4U32("##KeyColor", &SettingsVars::KeyPressedColor,
                             ImGuiColorEditFlags_NoInputs |
                                 ImGuiColorEditFlags_NoLabel)) {
        Settings::m_json_settings[PRESSED_KEY_COLOR] =
            SettingsVars::KeyPressedColor;
        Settings::Save(SettingsPath);
    }
    ImGui::SameLine();
    ImGui::Text("Pressed keys color");
    int key_to_delete = -1;
    if (ImGui::BeginTable("Keys##keys_table", 5)) {
        for (auto &key : keys) {
            ImGui::TableNextRow();

            ImGui::PushID(static_cast<int>(key.first));
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s Key", key.second.getDisplayName().c_str());
            ImGui::TableSetColumnIndex(1);
            if (keybindingToChange == key.first) {
                ImGui::Button("Press key to bind");
            } else if (ImGui::Button(key.second.getKeyName().c_str())) {
                keybindingToChange = key.first;
            }
            ImGui::TableSetColumnIndex(2);
            if (ImGui::Button("Delete Key")) {
                key_to_delete = static_cast<int>(key.first);
                if (keybindingToChange == key.first)
                    keybindingToChange = UINT_MAX;
            }
            ImGui::TableSetColumnIndex(3);
            ImGui::PushItemWidth(60.f);
            ImGui::InputFloat("Width", &key.second.getSize().x);
            ImGui::PopItemWidth();
            ImGui::TableSetColumnIndex(4);
            ImGui::PushItemWidth(60.f);
            ImGui::InputFloat("Height", &key.second.getSize().y);
            ImGui::PopItemWidth();
            ImGui::PopID();
        }
        ImGui::EndTable();
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
