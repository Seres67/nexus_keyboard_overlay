#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <UiKey.hpp>
#include <nlohmann/json.hpp>

void from_json(const nlohmann::json &j, UIKey &key);
void to_json(nlohmann::json &j, const UIKey &key);
void from_json(const nlohmann::json &j, OldKey &key);
void to_json(nlohmann::json &j, const OldKey &key);
namespace Settings
{

void load_settings();
void save_settings();
void load_config();
void save_config();

extern nlohmann::json json_settings;
extern nlohmann::json json_config;
extern std::filesystem::path settings_path;
extern std::filesystem::path config_path;

extern std::unordered_map<unsigned int, UIKey> keys;
extern float background_color[4];
extern float default_key_size;
extern float text_scaling;
extern bool disable_while_in_chat;
extern bool disable_when_map_open;
extern bool edit_mode;
extern bool lock_window;
extern bool show_durations;
} // namespace Settings

#endif // SETTINGS_HPP
