#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <mutex>
#include <nlohmann/json.hpp>

class UIKey;
class OldKey;

void from_json(const nlohmann::json &j, UIKey &key);
void to_json(nlohmann::json &j, const UIKey &key);
void from_json(const nlohmann::json &j, OldKey &key);
void to_json(nlohmann::json &j, const OldKey &key);
namespace Settings
{

void load();
void save();

extern nlohmann::json json_settings;
extern std::filesystem::path settings_path;
extern std::mutex mutex;

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
