#include "Key.hpp"

// Key Key::from_json(const json &j) {
//   float x = j.at("pos_x").get<float>();
//   float y = j.at("pos_y").get<float>();
//   return {j.at("name").get<std::string>(),
//   j.at("key_name").get<std::string>(),
//           j.at("key_code").get<char>(), ImVec2(x, y)};
// }
//
// void Key::to_json(json &j, Key p) {
//   j = json{{"name", p.getDisplayName()},
//            {"key_name", p.getKeyName()},
//            {"key_code", p.getKeyCode()},
//            {"pos_x", p.getPos().x},
//            {"pos_y", p.getPos().y}};
// }
