#ifndef NEXUS_KEYBOARD_OVERLAY_KEY_HPP
#define NEXUS_KEYBOARD_OVERLAY_KEY_HPP

#include "imgui/imgui.h"
#include "nlohmann/json.hpp"
#include <chrono>
#include <string>
#include <utility>
#include <utils.h>

class Key
{
  public:
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Key, m_binding_name, m_key_name, m_code,
                                   m_pos.x, m_pos.y, m_size.x, m_size.y)

    Key() = default;

    Key(std::string binding_name, std::string key_name, unsigned int code,
        ImVec2 pos, ImVec2 size)
        : m_binding_name(std::move(binding_name)),
          m_key_name(std::move(key_name)), m_code(code), m_pos(pos),
          m_size(size)
    {
    }

    [[nodiscard]] bool isKeyPressed() const { return m_pressed; }

    [[nodiscard]] unsigned int getKeyCode() const { return m_code; }
    void setKeyCode(unsigned int code) { m_code = code; }

    [[nodiscard]] const std::string &getDisplayName() const
    {
        return m_binding_name;
    }
    void setDisplayName(const std::string &name) { m_binding_name = name; }

    [[nodiscard]] const std::string &getKeyName() const { return m_key_name; }
    void setKeyName(const std::string &name) { m_key_name = name; }

    [[nodiscard]] const ImVec2 &getPos() const { return m_pos; }
    void setPos(const ImVec2 &pos) { m_pos = pos; }

    [[nodiscard]] ImVec2 &getSize() { return m_size; }
    void setSize(const ImVec2 &size) { m_size = size; }

    void keyDown()
    {
        m_pressed = true;
        m_start_pressing = std::chrono::steady_clock::now();
    }

    void keyUp()
    {
        m_pressed = false;
        m_end_pressing = std::chrono::steady_clock::now();
    }

    long long int getPressedDuration()
    {
        if (m_pressed) {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - m_start_pressing)
                .count();
        } else {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                       m_end_pressing - m_start_pressing)
                .count();
        }
    }

    void reset()
    {
        m_pressed = false;
        m_start_pressing = {};
        m_end_pressing = {};
    }

  private:
    std::string m_binding_name;
    std::string m_key_name;
    unsigned int m_code{};
    bool m_pressed = false;
    ImVec2 m_pos;
    ImVec2 m_size;
    std::chrono::time_point<std::chrono::steady_clock> m_start_pressing = {};
    std::chrono::time_point<std::chrono::steady_clock> m_end_pressing = {};
};

#endif // !NEXUS_KEYBOARD_OVERLAY_KEY_HPP
