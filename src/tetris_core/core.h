#ifndef CORE_H
#define CORE_H

#include "board.h"
#include <map>
#include <vector>
#include <SFML/Window/Keyboard.hpp>

enum operation {
  oLeft, oRight, oSD, oHD, oCW, oCCW, o180, oHold, oLMost, oRMost, oID, o__Last
}; constexpr int num_operations = o__Last;
class game {
public:
  game_handler g;
  int das = 70, arr = 0, sddas = 0, sdarr = 0;
  std::vector<sf::Keyboard::Key> keybinding[num_operations];
  bool key_down[num_operations] = { 0 };

  void get_key_down_stat();
};

#endif
