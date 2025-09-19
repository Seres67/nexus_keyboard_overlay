#ifndef NEXUS_KEYBOARD_OVERLAY_UIKEY_HPP
#define NEXUS_KEYBOARD_OVERLAY_UIKEY_HPP

#include <filesystem>
#include <imgui/imgui.h>
#include <nexus/Nexus.h>
#include <string>
#include <utility>

namespace Settings
{
extern float default_key_size;
extern bool show_durations;
} // namespace Settings

class UIKey
{
  public:
    UIKey() = default;
    UIKey(const UINT virtual_code, const UINT scan_code, const float released_colors[4], const float pressed_colors[4],
          std::string released_texture_identifier, std::string pressed_texture_identifier, std::string display_text)
        : m_virtual_key(virtual_code), m_scan_code(scan_code),
          m_released_texture_identifier(std::move(released_texture_identifier)),
          m_pressed_texture_identifier(std::move(pressed_texture_identifier)), m_display_text(std::move(display_text)),
          m_pressed(false)
    {
        m_pos[0] = 0;
        m_pos[1] = 0;
        m_size[0] = Settings::default_key_size;
        m_size[1] = Settings::default_key_size;
        m_released_colors[0] = released_colors[0];
        m_released_colors[1] = released_colors[1];
        m_released_colors[2] = released_colors[2];
        m_released_colors[3] = released_colors[3];
        m_pressed_colors[0] = pressed_colors[0];
        m_pressed_colors[1] = pressed_colors[1];
        m_pressed_colors[2] = pressed_colors[2];
        m_pressed_colors[3] = pressed_colors[3];
    }

    UIKey(const UINT virtual_code, const UINT scan_code, const float pos[2], const float size[2],
          const float released_colors[4], const float pressed_colors[4], std::string released_texture_identifier,
          std::string pressed_texture_identifier, std::string display_text)
        : m_virtual_key(virtual_code), m_scan_code(scan_code),
          m_released_texture_identifier(std::move(released_texture_identifier)),
          m_pressed_texture_identifier(std::move(pressed_texture_identifier)), m_display_text(std::move(display_text)),
          m_pressed(false)
    {
        m_pos[0] = pos[0];
        m_pos[1] = pos[1];
        m_size[0] = size[0];
        m_size[1] = size[1];
        m_released_colors[0] = released_colors[0];
        m_released_colors[1] = released_colors[1];
        m_released_colors[2] = released_colors[2];
        m_released_colors[3] = released_colors[3];
        m_pressed_colors[0] = pressed_colors[0];
        m_pressed_colors[1] = pressed_colors[1];
        m_pressed_colors[2] = pressed_colors[2];
        m_pressed_colors[3] = pressed_colors[3];
    }
    [[nodiscard]] UINT virtual_code() const { return m_virtual_key; }
    void set_virtual_code(const UINT vk) { m_virtual_key = vk; }

    [[nodiscard]] UINT scan_code() const { return m_scan_code; }
    void set_scan_code(const UINT scan_code) { m_scan_code = scan_code; }

    [[nodiscard]] const float *position() const { return m_pos; }
    [[nodiscard]] float *position() { return m_pos; }
    void set_position(const float pos[2])
    {
        m_pos[0] = pos[0];
        m_pos[1] = pos[1];
    }

    [[nodiscard]] const float *size() const { return m_size; }
    [[nodiscard]] float *size() { return m_size; }
    void set_size(const float size[2])
    {
        m_size[0] = size[0];
        m_size[1] = size[1];
    }

    [[nodiscard]] float *released_colors() { return m_released_colors; }
    [[nodiscard]] const float *released_colors() const { return m_released_colors; }
    void set_released_colors(const float colors[4])
    {
        m_released_colors[0] = colors[0];
        m_released_colors[1] = colors[1];
        m_released_colors[2] = colors[2];
        m_released_colors[3] = colors[3];
    }

    [[nodiscard]] float *pressed_colors() { return m_pressed_colors; }
    [[nodiscard]] const float *pressed_colors() const { return m_pressed_colors; }
    void set_pressed_colors(const float colors[4])
    {
        m_pressed_colors[0] = colors[0];
        m_pressed_colors[1] = colors[1];
        m_pressed_colors[2] = colors[2];
        m_pressed_colors[3] = colors[3];
    }

    [[nodiscard]] std::string released_texture_identifier() const { return m_released_texture_identifier; }
    [[nodiscard]] std::string &released_texture_identifier() { return m_released_texture_identifier; }
    void set_released_texture_identifier(const std::string &released_texture_identifier)
    {
        m_released_texture_identifier = released_texture_identifier;
    }

    [[nodiscard]] std::string pressed_texture_identifier() const { return m_pressed_texture_identifier; }
    [[nodiscard]] std::string &pressed_texture_identifier() { return m_pressed_texture_identifier; }
    void set_pressed_texture_identifier(const std::string &pressed_texture_identifier)
    {
        m_pressed_texture_identifier = pressed_texture_identifier;
    }

    [[nodiscard]] std::string display_text() const { return m_display_text; }
    [[nodiscard]] std::string &display_text() { return m_display_text; }
    void set_display_text(const std::string &display_text) { m_display_text = display_text; }

    [[nodiscard]] bool pressed() const { return m_pressed; }
    void set_pressed(const bool pressed)
    {
        if (Settings::show_durations && pressed && m_pressed != pressed)
            start_pressing();
        else if (Settings::show_durations && !pressed && m_pressed != pressed)
            end_pressing();
        m_pressed = pressed;
    }

    [[nodiscard]] const std::chrono::steady_clock::time_point &start_press_time() const { return m_start_press_time; }
    void start_pressing() { m_start_press_time = std::chrono::steady_clock::now(); }

    [[nodiscard]] const std::chrono::steady_clock::time_point &end_press_time() const { return m_end_press_time; }
    void end_pressing() { m_end_press_time = std::chrono::steady_clock::now(); }

    [[nodiscard]] std::chrono::milliseconds press_duration() const
    {
        if (m_pressed)
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() -
                                                                         m_start_press_time);
        return std::chrono::duration_cast<std::chrono::milliseconds>(m_end_press_time - m_start_press_time);
    }

  private:
    UINT m_virtual_key{};
    UINT m_scan_code;
    float m_pos[2];
    float m_size[2];
    float m_released_colors[4];
    float m_pressed_colors[4];
    std::string m_released_texture_identifier;
    std::string m_pressed_texture_identifier;
    std::string m_display_text;
    bool m_pressed;
    std::chrono::steady_clock::time_point m_start_press_time;
    std::chrono::steady_clock::time_point m_end_press_time;
};

struct OldKey
{
    std::string m_binding_name;
    std::string m_key_name;
    unsigned int m_code{};
    bool m_pressed = false;
    ImVec2 m_pos;
    ImVec2 m_size;
    std::chrono::time_point<std::chrono::steady_clock> m_start_pressing = {};
    std::chrono::time_point<std::chrono::steady_clock> m_end_pressing = {};
};

#endif // NEXUS_KEYBOARD_OVERLAY_UIKEY_HPP
