#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <filesystem>
#include <mumble/Mumble.h>
#include <nexus/Nexus.h>
#include <nlohmann/json.hpp>
#include <string>

// handle to self hmodule
extern HMODULE self_module;
// addon definition
extern AddonDefinition addon_def;
// addon api
extern AddonAPI *api;

extern char addon_name[];

extern HWND game_handle;

extern Mumble::Data *mumble_link;
extern NexusLinkData *nexus_link;

extern std::filesystem::path textures_directory;

extern UINT pressed_vk;
extern std::string pressed_key;

extern bool adding_key;
extern bool recording_keypress;
extern UINT virtual_key_to_add;
extern UINT scan_code_to_add;

extern std::unordered_map<std::filesystem::path, nlohmann::json> configs;
#endif // GLOBALS_HPP
