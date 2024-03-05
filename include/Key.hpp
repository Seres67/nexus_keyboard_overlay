#ifndef NEXUS_KEYBOARD_OVERLAY_KEY_HPP
#define NEXUS_KEYBOARD_OVERLAY_KEY_HPP

#include "Settings.h"
#include "imgui/imgui.h"
#include "nexus/Nexus.h"
#include "nlohmann/json.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <utils.h>

class Key {
public:
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Key, m_binding_name, m_key_name, m_code,
                                 m_pos.x, m_pos.y)

  Key() {}

  Key(std::string binding_name, std::string key_name, UINT code, ImVec2 pos)
      : m_binding_name(binding_name), m_key_name(key_name), m_code(code),
        m_pos(pos) {}

  // Key(const Key &key) {
  //   m_binding_name = key.m_binding_name;
  //   m_key_name = key.m_key_name;
  //   m_code = key.m_code;
  //   m_pos = key.m_pos;
  //   // m_not_pressed_tex = key.m_not_pressed_tex;
  //   // m_pressed_tex = key.m_pressed_tex;
  // }

  const bool isKeyPressed() const { return m_pressed; }

  const UINT getKeyCode() const { return m_code; }
  void setKeyCode(UINT code) { m_code = code; }

  const std::string &getDisplayName() const { return m_binding_name; }
  void setDisplayName(const std::string &name) { m_binding_name = name; }

  const std::string &getKeyName() const { return m_key_name; }
  void setKeyName(const std::string &name) { m_key_name = name; }

  const ImVec2 &getPos() const { return m_pos; }
  void setPos(const ImVec2 &pos) { m_pos = pos; }

  void keyDown() {
    Log::debug("pressed down");
    m_pressed = true;
    m_start_pressing = std::chrono::steady_clock::now();
  }

  void keyUp() {
    m_pressed = false;
    m_end_pressing = std::chrono::steady_clock::now();
  }

  long long int getPressedDuration() {
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

  void reset() {
    m_pressed = false;
    m_start_pressing = {};
    m_end_pressing = {};
  }

private:
  std::string m_binding_name;
  std::string m_key_name;
  UINT m_code;
  bool m_pressed;
  ImVec2 m_pos;
  std::chrono::time_point<std::chrono::steady_clock> m_start_pressing;
  std::chrono::time_point<std::chrono::steady_clock> m_end_pressing;
};

#endif // !NEXUS_KEYBOARD_OVERLAY_KEY_HPP
