#include <globals.hpp>

HMODULE self_module = nullptr;
AddonDefinition addon_def{};
AddonAPI *api = nullptr;
char addon_name[] = "Keyboard Overlay";
HWND game_handle = nullptr;

Mumble::Data *mumble_link;
NexusLinkData *nexus_link;

std::filesystem::path textures_directory;

UINT pressed_vk;
std::string pressed_key;

bool adding_key;
bool recording_keypress;
UINT virtual_key_to_add;
UINT scan_code_to_add;

std::vector<std::filesystem::path> configs;
int current_config;
