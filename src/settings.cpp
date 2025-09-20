#include <filesystem>
#include <fstream>
#include <globals.hpp>
#include <nexus/Nexus.h>
#include <nlohmann/json.hpp>
#include <settings.hpp>

void from_json(const nlohmann::json &j, UIKey &key)
{
    key.set_virtual_code(j.at("virtual_code").get<UINT>());
    key.set_scan_code(j.at("scan_code").get<UINT>());
    const float x = j.at("x").get<float>();
    const float y = j.at("y").get<float>();
    const float pos[2] = {x, y};
    key.set_position(pos);
    const float width = j.at("width").get<float>();
    const float height = j.at("height").get<float>();
    const float size[2] = {width, height};
    key.set_size(size);
    const std::string display_text = j.at("display_text").get<std::string>();
    key.set_display_text(display_text);
    if (j.contains("released_texture")) {
        const std::string released_texture_id = j.at("released_texture").get<std::string>();
        key.set_released_texture_identifier("KEYBOARD_OVERLAY_" + released_texture_id);
    } else {
        const float released_red = j.at("released_red").get<float>();
        const float released_green = j.at("released_green").get<float>();
        const float released_blue = j.at("released_blue").get<float>();
        const float released_alpha = j.at("released_alpha").get<float>();
        const float released_colors[4] = {released_red, released_green, released_blue, released_alpha};
        key.set_released_colors(released_colors);
    }
    if (j.contains("pressed_texture")) {
        const std::string pressed_texture_id = j.at("pressed_texture").get<std::string>();
        key.set_pressed_texture_identifier("KEYBOARD_OVERLAY_" + pressed_texture_id);
    } else {
        const float pressed_red = j.at("pressed_red").get<float>();
        const float pressed_green = j.at("pressed_green").get<float>();
        const float pressed_blue = j.at("pressed_blue").get<float>();
        const float pressed_alpha = j.at("pressed_alpha").get<float>();
        const float pressed_colors[4] = {pressed_red, pressed_green, pressed_blue, pressed_alpha};
        key.set_pressed_colors(pressed_colors);
    }
}

void to_json(nlohmann::json &j, const UIKey &key)
{

    j = nlohmann::json{
        {"virtual_code", key.virtual_code()},
        {"scan_code", key.scan_code()},
        {"x", key.position()[0]},
        {"y", key.position()[1]},
        {"width", key.size()[0]},
        {"height", key.size()[1]},
        {"display_text", key.display_text()},
    };
    if (!key.released_texture_identifier().empty()) {
        j += {"released_texture", key.released_texture_identifier().substr(17)};
    } else {
        j += {"released_red", key.released_colors()[0]};
        j += {"released_green", key.released_colors()[1]};
        j += {"released_blue", key.released_colors()[2]};
        j += {"released_alpha", key.released_colors()[3]};
    }
    if (!key.pressed_texture_identifier().empty()) {
        j += {"pressed_texture", key.pressed_texture_identifier().substr(17)};
    } else {
        j += {"pressed_red", key.pressed_colors()[0]};
        j += {"pressed_green", key.pressed_colors()[1]};
        j += {"pressed_blue", key.pressed_colors()[2]};
        j += {"pressed_alpha", key.pressed_colors()[3]};
    }
}

void from_json(const nlohmann::json &j, OldKey &key)
{
    key.m_code = j.at("m_code").get<UINT>();
    const float x = j.at("m_pos.x").get<float>();
    const float y = j.at("m_pos.y").get<float>();
    key.m_pos = {x, y};
    const float width = j.at("m_size.x").get<float>();
    const float height = j.at("m_size.y").get<float>();
    key.m_size = {width, height};
    const std::string display_text = j.at("m_binding_name").get<std::string>();
    key.m_binding_name = display_text;
    const std::string key_name = j.at("m_key_name").get<std::string>();
    key.m_key_name = key_name;
}

void to_json(nlohmann::json &j, const OldKey &key)
{

    // j = json{
    //     {"virtual_code", key.virtual_code()},
    //     {"scan_code", key.scan_code()},
    //     {"x", key.position()[0]},
    //     {"y", key.position()[1]},
    //     {"width", key.size()[0]},
    //     {"height", key.size()[1]},
    //     {"display_text", key.display_text()},
    // };
    // if (!key.released_texture_identifier().empty()) {
    //     j += {"released_texture", key.released_texture_identifier().substr(17)};
    // } else {
    //     j += {"released_red", key.released_colors()[0]};
    //     j += {"released_green", key.released_colors()[1]};
    //     j += {"released_blue", key.released_colors()[2]};
    //     j += {"released_alpha", key.released_colors()[3]};
    // }
    // if (!key.pressed_texture_identifier().empty()) {
    //     j += {"pressed_texture", key.pressed_texture_identifier().substr(17)};
    // } else {
    //     j += {"pressed_red", key.pressed_colors()[0]};
    //     j += {"pressed_green", key.pressed_colors()[1]};
    //     j += {"pressed_blue", key.pressed_colors()[2]};
    //     j += {"pressed_alpha", key.pressed_colors()[3]};
    // }
}

namespace Settings
{
nlohmann::json json_settings;
nlohmann::json json_config;
std::mutex mutex;
std::filesystem::path settings_path;
std::filesystem::path config_path;

std::unordered_map<unsigned int, UIKey> keys;
float background_color[4] = {0.075, 0.086, 0.11, 0.933};
float default_key_size = 42;
float text_scaling = 1;
bool disable_while_in_chat = true;
bool disable_when_map_open = false;
bool edit_mode = false;
bool lock_window = false;
bool show_durations = false;

void load_settings()
{
    json_settings = nlohmann::json::object();
    if (!std::filesystem::exists(settings_path)) {
        return;
    }

    {
        std::lock_guard lock(mutex);
        try {
            if (std::ifstream file(settings_path); file.is_open()) {
                json_settings = nlohmann::json::parse(file);
                file.close();
            }
        } catch (nlohmann::json::parse_error &ex) {
            api->Log(ELogLevel_WARNING, addon_name, "settings.json could not be parsed.");
            api->Log(ELogLevel_WARNING, addon_name, ex.what());
        }
    }
    if (!json_settings["BackgroundColor"].is_null())
        json_settings["BackgroundColor"].get_to(background_color);
    if (!json_settings["DefaultKeySize"].is_null())
        json_settings["DefaultKeySize"].get_to(default_key_size);
    if (!json_settings["TextScaling"].is_null())
        json_settings["TextScaling"].get_to(text_scaling);
    if (!json_settings["DisableWhileInChat"].is_null())
        json_settings["DisableWhileInChat"].get_to(disable_while_in_chat);
    if (!json_settings["DisableWhenMapOpen"].is_null())
        json_settings["DisableWhenMapOpen"].get_to(disable_when_map_open);
    if (!json_settings["LockWindow"].is_null())
        json_settings["LockWindow"].get_to(lock_window);
    if (!json_settings["ConfigPath"].is_null())
        json_settings["ConfigPath"].get_to(config_path);
    api->Log(ELogLevel_INFO, addon_name, "settings loaded!");
}

void save_settings()
{
    if (!std::filesystem::exists(settings_path.parent_path())) {
        std::filesystem::create_directories(settings_path.parent_path());
    }
    {
        std::lock_guard lock(mutex);
        if (std::ofstream file(settings_path); file.is_open()) {
            file << json_settings.dump(1, '\t') << std::endl;
            file.close();
        }
        api->Log(ELogLevel_INFO, addon_name, "settings saved!");
    }
}

void load_config()
{
    json_config = nlohmann::json::object();
    if (!std::filesystem::exists(config_path)) {
        return;
    }

    {
        std::lock_guard lock(mutex);
        try {
            if (std::ifstream file(config_path); file.is_open()) {
                json_config = nlohmann::json::parse(file);
                file.close();
            }
        } catch (nlohmann::json::parse_error &ex) {
            api->Log(ELogLevel_WARNING, addon_name, "config could not be parsed.");
            api->Log(ELogLevel_WARNING, addon_name, ex.what());
        }
    }
    if (!json_config["Keys"].is_null())
        json_config["Keys"].get_to(keys);
    if (!json_config["ShowDurations"].is_null())
        json_config["ShowDurations"].get_to(show_durations);

    api->Log(ELogLevel_INFO, addon_name, "config loaded!");
}

void save_config()
{
    if (!std::filesystem::exists(config_path.parent_path())) {
        std::filesystem::create_directories(config_path.parent_path());
    }
    {
        std::lock_guard lock(mutex);
        if (std::ofstream file(config_path); file.is_open()) {
            file << json_config.dump(1, '\t') << std::endl;
            file.close();
        }
        api->Log(ELogLevel_INFO, addon_name, "config saved!");
    }
}

} // namespace Settings
