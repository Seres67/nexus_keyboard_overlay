#include <UiKey.hpp>
#include <fstream>
#include <globals.hpp>
#include <gui.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <settings.hpp>
#include <tchar.h>
#include <utils.hpp>

void render_duration(const UIKey &val)
{
    auto text_size = ImGui::CalcTextSize(std::string(std::to_string(val.press_duration().count()) + "ms").c_str());
    ImGui::SetCursorPos({val.position()[0] + (val.size()[0] / 2) - text_size[0] / 2, val.position()[1]});
    ImGui::Text("%lldms", val.press_duration().count());
}

void render_key(UIKey &val)
{
    const auto &texture_id = val.pressed() ? val.pressed_texture_identifier() : val.released_texture_identifier();
    const auto *colors = val.pressed() ? val.pressed_colors() : val.released_colors();

    ImGui::SetCursorPos({val.position()[0], val.position()[1]});
    if (!texture_id.empty()) {
        const Texture *texture = api->Textures.Get(texture_id.c_str());
        if (!texture) {
            api->Textures.GetOrCreateFromFile(texture_id.c_str(),
                                              (textures_directory / texture_id.substr(17)).string().c_str());
        } else {
            ImGui::ImageButton(texture->Resource, {val.size()[0], val.size()[1]}, {0, 0}, {1, 1}, 0);
            if (Settings::edit_mode && ImGui::IsItemActive()) {
                const float pos[2] = {val.position()[0] + ImGui::GetIO().MouseDelta.x,
                                      val.position()[1] + ImGui::GetIO().MouseDelta.y};
                val.set_position(pos);
            }
            if (Settings::edit_mode && ImGui::IsItemDeactivated()) {
                Settings::json_config["Keys"] = Settings::keys;
                Settings::save_config();
            }
            if (Settings::show_durations) {
                render_duration(val);
            }
        }
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, {colors[0], colors[1], colors[2], colors[3]});
        ImGui::Button(val.display_text().empty() ? key_to_string(val.virtual_code(), val.scan_code()).c_str()
                                                 : val.display_text().c_str(),
                      {val.size()[0], val.size()[1]});
        ImGui::PopStyleColor();
        if (Settings::edit_mode && ImGui::IsItemActive()) {
            const float pos[2] = {val.position()[0] + ImGui::GetIO().MouseDelta.x,
                                  val.position()[1] + ImGui::GetIO().MouseDelta.y};
            val.set_position(pos);
        }
        if (Settings::edit_mode && ImGui::IsItemDeactivated()) {
            Settings::json_config["Keys"] = Settings::keys;
            Settings::save_config();
        }
        if (Settings::show_durations) {
            render_duration(val);
        }
    }
}

bool tmp_open = true;
void render_window()
{
    if (Settings::disable_when_map_open && (!nexus_link->IsGameplay || mumble_link->Context.IsMapOpen))
        return;
#ifndef NDEBUG
    ImGui::ShowDemoWindow();
#endif
    ImGui::SetNextWindowPos(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar;
    if (Settings::lock_window)
        window_flags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, {Settings::background_color[0], Settings::background_color[1],
                                              Settings::background_color[2], Settings::background_color[3]});
    if (ImGui::Begin("Keyboard Overlay##KeyboardOverlayMainWindow", &tmp_open, window_flags)) {
        ImGui::SetWindowFontScale(Settings::text_scaling);
#ifndef NDEBUG
        ImGui::Text("%u, %s", pressed_vk, pressed_key.c_str());
#endif
        for (auto &val : Settings::keys | std::views::values) {
            render_key(val);
        }
        ImGui::End();
    }
    ImGui::PopStyleColor();
}

void handle_texture(std::filesystem::path &texture_path)
{
    OPENFILENAME ofn{};
    TCHAR szFile[MAX_PATH]{};
    TCHAR initialDir[MAX_PATH]{};

    _tcscpy_s(initialDir, textures_directory.string().c_str());

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = static_cast<HWND>(nullptr);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All Files (*.*)\0*.*\0"
                      "Supported Images (*.png, *.jpg, *.jpeg, *.bmp, *.gif)\0"
                      "*.png;*.jpg;*.jpeg;*.bmp;*.gif\0"
                      "PNG (*.png)\0*.png\0"
                      "JPEG (*.jpg;*.jpeg)\0*.jpg;*.jpeg\0"
                      "Bitmap (*.bmp)\0*.bmp\0"
                      "GIF (*.gif)\0*.gif\0";
    ofn.nFilterIndex = 2;
    ofn.lpstrInitialDir = initialDir;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileName(&ofn) == TRUE) {
        if (!std::filesystem::exists(ofn.lpstrFile))
            return;
        texture_path = ofn.lpstrFile;
    }
}

float pressed_key_colors[4] = {1, 1, 1, 0.933};
float released_key_colors[4] = {0.247, 0.302, 0.396, 0.933};
std::filesystem::path released_path;
std::filesystem::path pressed_path;
char display_text[64];
char config_name[64];
void render_options()
{
    ImGui::SetNextWindowSize({260, 270}, ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Add Key##KeyboardOverlayAddKeyModal")) {
        if (!recording_keypress) {
            if (ImGui::Button("Click to record key##KeyboardOverlayRecordNewKey")) {
                recording_keypress = true;
                virtual_key_to_add = 0;
                scan_code_to_add = 0;
            }
        }
        if (recording_keypress && virtual_key_to_add == 0) {
            ImGui::Button("Press key to add...##KeyboardOverlayPressKey");
        }
        if (virtual_key_to_add != 0) {
            const std::string key_str = key_to_string(virtual_key_to_add, scan_code_to_add);
            ImGui::SameLine();
            ImGui::Text("Adding key %s", key_str.c_str());
            recording_keypress = false;
        }
        ImGui::ColorEdit4("Color when key is released##KeyboardOverlayKeyColorPressed", released_key_colors,
                          ImGuiColorEditFlags_NoInputs);
        ImGui::ColorEdit4("Color when key is pressed##KeyboardOverlayKeyColorPressed", pressed_key_colors,
                          ImGuiColorEditFlags_NoInputs);
        if (!virtual_key_to_add) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        }
        if (ImGui::Button("Select released key texture")) {
            handle_texture(released_path);
        }
        if (!virtual_key_to_add) {
            ImGui::PopItemFlag();
        }
        if (!released_path.empty()) {
            const auto released_texture_identifier = ("KEYBOARD_OVERLAY_ " + released_path.string());
            const auto released_texture =
                api->Textures.GetOrCreateFromFile(released_texture_identifier.c_str(), released_path.string().c_str());
            if (released_texture) {
                ImGui::SameLine();
                ImGui::Image(released_texture->Resource, {Settings::default_key_size, Settings::default_key_size});
            }
        }
        if (!virtual_key_to_add) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        }
        if (ImGui::Button("Select pressed key texture")) {
            handle_texture(pressed_path);
        }
        if (!virtual_key_to_add) {
            ImGui::PopItemFlag();
        }
        if (!pressed_path.empty()) {
            const auto pressed_texture_identifier = ("KEYBOARD_OVERLAY_ " + pressed_path.string());
            const auto pressed_texture =
                api->Textures.GetOrCreateFromFile(pressed_texture_identifier.c_str(), pressed_path.string().c_str());
            if (pressed_texture) {
                ImGui::SameLine();
                ImGui::Image(pressed_texture->Resource, {Settings::default_key_size, Settings::default_key_size});
            }
        }

        ImGui::InputText("Display text", display_text, 64);
        if (ImGui::Button("Cancel##KeyboardOverlayAddKeyModalCancel")) {
            adding_key = false;
            virtual_key_to_add = 0;
            scan_code_to_add = 0;
            recording_keypress = false;
            memset(display_text, 0, 64);
            released_key_colors[0] = 0.247;
            released_key_colors[1] = 0.302;
            released_key_colors[2] = 0.396;
            released_key_colors[3] = 0.933;
            released_path.clear();
            pressed_path.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Save##KeyboardOverlayAddKeyModalSave") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            if (virtual_key_to_add == 0)
                return;

            Settings::keys[virtual_key_to_add] =
                UIKey(virtual_key_to_add, scan_code_to_add, released_key_colors, pressed_key_colors,
                      !released_path.empty() ? ("KEYBOARD_OVERLAY_" + released_path.filename().string()).c_str() : "",
                      !pressed_path.empty() ? ("KEYBOARD_OVERLAY_" + pressed_path.filename().string()).c_str() : "",
                      display_text);
            adding_key = false;
            virtual_key_to_add = 0;
            scan_code_to_add = 0;
            recording_keypress = false;
            memset(display_text, 0, 64);
            released_key_colors[0] = 0.247;
            released_key_colors[1] = 0.302;
            released_key_colors[2] = 0.396;
            released_key_colors[3] = 0.933;
            CopyFile(released_path.string().c_str(), (textures_directory / released_path.filename()).string().c_str(),
                     true);
            CopyFile(pressed_path.string().c_str(), (textures_directory / pressed_path.filename()).string().c_str(),
                     true);
            released_path.clear();
            pressed_path.clear();
            Settings::json_config["Keys"] = Settings::keys;
            Settings::save_config();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::ColorEdit4("Background Color##KeyboardOverlayBackgroundColor", Settings::background_color,
                          ImGuiColorEditFlags_NoInputs)) {
        Settings::json_settings["BackgroundColor"] = Settings::background_color;
        Settings::save_settings();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset background color##KeyboardOverlayResetBackgroundColor")) {
        Settings::background_color[0] = 0.075;
        Settings::background_color[1] = 0.086;
        Settings::background_color[2] = 0.11;
        Settings::background_color[3] = 0.933;
        Settings::json_settings["BackgroundColor"] = Settings::background_color;
        Settings::save_settings();
    }
    ImGui::Text("Default Key Size");
    ImGui::SameLine();
    ImGui::PushItemWidth(400);
    if (ImGui::SliderFloat("##KeyboardOverlayDefaultKeySize", &Settings::default_key_size, 1.f, 1000.f)) {
        Settings::json_settings["DefaultKeySize"] = Settings::default_key_size;
        Settings::save_settings();
    }
    ImGui::PopItemWidth();
    ImGui::Text("Text Scaling");
    ImGui::SameLine();
    ImGui::PushItemWidth(400);
    if (ImGui::SliderFloat("##KeyboardOverlayTextScaling", &Settings::text_scaling, 1.f, 10.f)) {
        Settings::json_settings["TextScaling"] = Settings::text_scaling;
        Settings::save_settings();
    }
    ImGui::PopItemWidth();
    if (ImGui::Checkbox("Disable while in chat##KeyboardOverlayDisableWhileInChat", &Settings::disable_while_in_chat)) {
        Settings::json_settings["DisableWhileInChat"] = Settings::disable_while_in_chat;
        Settings::save_settings();
    }
    if (ImGui::Checkbox("Hide when map is open##KeyboardOverlayDisableWhenMapIsOpen",
                        &Settings::disable_when_map_open)) {
        Settings::json_settings["DisableWhenMapOpen"] = Settings::disable_when_map_open;
        Settings::save_settings();
    }
    if (ImGui::Checkbox("Visual Edit Mode / WYSIWYG##KeyboardOverlayVisualEditMode", &Settings::edit_mode)) {
        Settings::json_settings["EditMode"] = Settings::edit_mode;
        Settings::save_settings();
    }
    if (ImGui::Checkbox("Lock Window##KeyboardOverlayLockWindow", &Settings::lock_window)) {
        Settings::json_settings["LockWindow"] = Settings::lock_window;
        Settings::save_settings();
    }

    std::vector<std::string> config_names;
    config_names.reserve(configs.size());
    for (const auto &path : configs) {
        config_names.push_back(path.string());
    }

    std::vector<const char *> config_cstrs;
    config_cstrs.reserve(config_names.size());
    for (auto &s : config_names) {
        config_cstrs.push_back(s.c_str());
    }
    ImGui::InputText("Config name", config_name, 64);
    ImGui::SameLine();
    if (ImGui::Button("New config")) {
        std::filesystem::path settings_path = api->Paths.GetAddonDirectory("keyboard_overlay");
        std::filesystem::path new_config_path = settings_path / (std::string(config_name) + std::string(".json"));
        std::ofstream config(new_config_path);
        config.write("{}", 2);
        config.close();
        configs.clear();
        for (auto &file : std::filesystem::directory_iterator(settings_path)) {
            if (file.path().extension() != ".json" || file.path().filename() == "settings.json")
                continue;
            configs.emplace_back(file.path());
        }
        Settings::config_path = new_config_path;
        Settings::json_settings["ConfigPath"] = Settings::config_path;
        Settings::save_settings();
        Settings::load_config();
        memset(config_name, 0, 64);
    }
    ImGui::SameLine();
    if (ImGui::Button("Duplicate current config")) {
        std::filesystem::path settings_path = api->Paths.GetAddonDirectory("keyboard_overlay");
        std::filesystem::path new_config_path = settings_path / (std::string(config_name) + std::string(".json"));
        std::ofstream config(new_config_path);
        if (config.is_open()) {
            auto settings = Settings::json_config.dump(1, '\t');
            config.write(settings.c_str(), settings.size());
            config.close();
        }
        configs.clear();
        for (auto &file : std::filesystem::directory_iterator(settings_path)) {
            if (file.path().extension() != ".json" || file.path().filename() == "settings.json")
                continue;
            configs.emplace_back(file.path());
        }
        Settings::config_path = new_config_path;
        Settings::json_settings["ConfigPath"] = Settings::config_path;
        Settings::save_settings();
        Settings::load_config();
        memset(config_name, 0, 64);
    }
    if (ImGui::Combo("Configs", &current_config, config_cstrs.data(), static_cast<int>(config_cstrs.size()))) {
        Settings::config_path = config_names[current_config];
        Settings::json_settings["ConfigPath"] = Settings::config_path;
        Settings::save_settings();
        Settings::load_config();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload configs")) {
        std::filesystem::path settings_path = api->Paths.GetAddonDirectory("keyboard_overlay");
        configs.clear();
        for (auto &file : std::filesystem::directory_iterator(settings_path)) {
            if (file.path().extension() != ".json" || file.path().filename() == "settings.json")
                continue;
            configs.emplace_back(file.path());
        }
    }
    if (ImGui::CollapsingHeader("Config##KeyboardOverlayConfigHeader")) {
        if (ImGui::Checkbox("Show key press durations##KeyboardOverlayShowDurations", &Settings::show_durations)) {
            Settings::json_config["ShowDurations"] = Settings::show_durations;
            Settings::save_config();
        }
        if (ImGui::Button("Add key##KeyboardOverlayOpenAddKeyModal")) {
            adding_key = true;
            ImGui::OpenPopup("Add Key##KeyboardOverlayAddKeyModal");
        }
        if (ImGui::CollapsingHeader("Keys##KeyboardOverlayKeysHeader")) {
            if (ImGui::BeginTable("KeysTable##KeyboardOverlayKeysTable", 7,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 80.f);
                ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Display Text", ImGuiTableColumnFlags_WidthFixed, 100.f);
                ImGui::TableSetupColumn("Released Color", ImGuiTableColumnFlags_WidthFixed, 50.f);
                ImGui::TableSetupColumn("Pressed Color", ImGuiTableColumnFlags_WidthFixed, 50.f);
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 50.f);
                ImGui::TableHeadersRow();

                for (auto key = Settings::keys.begin(); key != Settings::keys.end();) {
                    UIKey &k = key->second;
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    std::string key_str = key_to_string(k.virtual_code(), k.scan_code());
                    if (k.pressed()) {
                        ImGui::TextColored(ImVec4(k.pressed_colors()[0], k.pressed_colors()[1], k.pressed_colors()[2],
                                                  k.pressed_colors()[3]),
                                           "%s", key_str.c_str());
                    } else {
                        ImGui::Text("%s", key_str.c_str());
                    }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushItemWidth(-1);
                    if (ImGui::SliderFloat2(("##Size" + std::to_string(k.virtual_code())).c_str(), k.size(), 1.0f,
                                            1000.0f)) {
                        Settings::json_config["Keys"] = Settings::keys;
                        Settings::save_config();
                    }
                    ImGui::PopItemWidth();

                    ImGui::TableSetColumnIndex(2);
                    ImGui::PushItemWidth(-1);
                    if (ImGui::SliderFloat2(("##Position" + std::to_string(k.virtual_code())).c_str(), k.position(),
                                            0.0f, 5000.0f)) {
                        Settings::json_config["Keys"] = Settings::keys;
                        Settings::save_config();
                    }
                    ImGui::PopItemWidth();

                    ImGui::TableSetColumnIndex(3);
                    ImGui::PushItemWidth(-1);
                    if (ImGui::InputText(("##Display Text" + std::to_string(k.virtual_code())).c_str(),
                                         &k.display_text())) {
                        Settings::json_config["Keys"] = Settings::keys;
                        Settings::save_config();
                    }
                    ImGui::PopItemWidth();

                    ImGui::TableSetColumnIndex(4);
                    if (key->second.released_texture_identifier().empty()) {
                        if (ImGui::ColorEdit4(("##ReleasedColor" + std::to_string(k.virtual_code())).c_str(),
                                              k.released_colors(), ImGuiColorEditFlags_NoInputs)) {
                            Settings::json_config["Keys"] = Settings::keys;
                            Settings::save_config();
                        }
                    } else {
                        if (!api->Textures.Get(key->second.released_texture_identifier().c_str())) {
                            const auto released_texture_identifier =
                                ("KEYBOARD_OVERLAY_" + released_path.filename().string());
                            api->Textures.GetOrCreateFromFile(
                                released_texture_identifier.c_str(),
                                (textures_directory / released_path.filename()).string().c_str());
                        } else if (ImGui::ImageButton(
                                       api->Textures.Get(key->second.released_texture_identifier().c_str())->Resource,
                                       {key->second.size()[0], key->second.size()[1]}, {0, 0}, {1, 1}, 0)) {
                            handle_texture(released_path);
                            CopyFile(released_path.string().c_str(),
                                     (textures_directory / released_path.filename()).string().c_str(), true);
                            const auto released_texture_identifier =
                                ("KEYBOARD_OVERLAY_" + released_path.filename().string());
                            api->Textures.GetOrCreateFromFile(
                                released_texture_identifier.c_str(),
                                (textures_directory / released_path.filename()).string().c_str());
                            key->second.set_released_texture_identifier(released_texture_identifier);
                            released_path.clear();
                            Settings::json_config["Keys"] = Settings::keys;
                            Settings::save_config();
                        }
                    }

                    ImGui::TableSetColumnIndex(5);
                    if (key->second.pressed_texture_identifier().empty()) {
                        if (ImGui::ColorEdit4(("##PressedColor" + std::to_string(k.virtual_code())).c_str(),
                                              k.pressed_colors(), ImGuiColorEditFlags_NoInputs)) {
                            Settings::json_config["Keys"] = Settings::keys;
                            Settings::save_config();
                        }
                    } else {
                        if (!api->Textures.Get(key->second.pressed_texture_identifier().c_str())) {
                            const auto pressed_texture_identifier =
                                ("KEYBOARD_OVERLAY_" + pressed_path.filename().string());
                            api->Textures.GetOrCreateFromFile(
                                pressed_texture_identifier.c_str(),
                                (textures_directory / pressed_path.filename()).string().c_str());
                        } else if (ImGui::ImageButton(
                                       api->Textures.Get(key->second.pressed_texture_identifier().c_str())->Resource,
                                       {key->second.size()[0], key->second.size()[1]}, {0, 0}, {1, 1}, 0)) {
                            handle_texture(pressed_path);
                            CopyFile(pressed_path.string().c_str(),
                                     (textures_directory / pressed_path.filename()).string().c_str(), true);
                            const auto pressed_texture_identifier =
                                ("KEYBOARD_OVERLAY_" + pressed_path.filename().string());
                            api->Textures.GetOrCreateFromFile(
                                pressed_texture_identifier.c_str(),
                                (textures_directory / pressed_path.filename()).string().c_str());
                            key->second.set_pressed_texture_identifier(pressed_texture_identifier);
                            pressed_path.clear();
                            Settings::json_config["Keys"] = Settings::keys;
                            Settings::save_config();
                        }
                    }

                    ImGui::TableSetColumnIndex(6);
                    if (ImGui::Button(("Delete##" + std::to_string(k.virtual_code())).c_str())) {
                        key = Settings::keys.erase(key);
                        Settings::json_config["Keys"] = Settings::keys;
                        Settings::save_config();
                        continue;
                    }

                    ++key;
                }
                ImGui::EndTable();
            }
        }
    }
}
