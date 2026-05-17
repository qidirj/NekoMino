#include "core.h"
#include <SFML/Window/Keyboard.hpp>

void game::get_key_down_stat() {
  for (int i = 0; i < num_operations; ++i) {
    bool down = false;
    for (auto k : keybinding[i]) {
      if (sf::Keyboard::isKeyPressed(k)) down = true;
    }
    if (down && !key_down[i]) press(i);
    key_down[i] = down;
  }
}
