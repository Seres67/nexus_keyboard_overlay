#include <globals.hpp>

#include <unordered_map>
#include <utils.hpp>

std::string key_to_string(const UINT virtual_key, UINT scan_code)
{
    if (virtual_key >= VK_F1 && virtual_key <= VK_F24) {
        return "F" + std::to_string(virtual_key - VK_F1 + 1);
    }

    if (virtual_key == VK_SHIFT) {
        const UINT left_sc = MapVirtualKeyW(VK_LSHIFT, MAPVK_VK_TO_VSC);
        return (scan_code == left_sc) ? "LShift" : "RShift";
    }

    static const std::unordered_map<UINT, std::string> special_keys = {
        {VK_SPACE, "Space"},    {VK_RETURN, "Enter"},       {VK_TAB, "Tab"},         {VK_CAPITAL, "Caps Lock"},
        {VK_ESCAPE, "Escape"},  {VK_BACK, "Backspace"},     {VK_APPS, "Menu"},       {VK_UP, "Up"},
        {VK_DOWN, "Down"},      {VK_LEFT, "Left"},          {VK_RIGHT, "Right"},     {VK_INSERT, "Insert"},
        {VK_DELETE, "Delete"},  {VK_HOME, "Home"},          {VK_END, "End"},         {VK_NEXT, "Page Down"},
        {VK_PRIOR, "Page Up"},  {VK_SCROLL, "Scroll Lock"}, {VK_PAUSE, "Pause"},     {VK_LBUTTON, "Mouse1"},
        {VK_RBUTTON, "Mouse2"}, {VK_MBUTTON, "Mouse3"},     {VK_XBUTTON1, "Mouse4"}, {VK_XBUTTON2, "Mouse5"},
        {VK_OEM_3, "OEM3"}};

    if (const auto it = special_keys.find(virtual_key); it != special_keys.end()) {
        return it->second;
    }

    HKL__ *layout = GetKeyboardLayout(0);
    const UINT ch = MapVirtualKeyExW(virtual_key, MAPVK_VK_TO_CHAR, layout);
    if (ch != 0) {
        char buf[5] = {};
        std::string::size_type len =
            WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<LPCWCH>(&ch), 1, buf, sizeof(buf), nullptr, nullptr);
        if (len > 0)
            return {buf, len};
    }
    return {};
}
