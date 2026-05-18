#ifndef CORE_H
#define CORE_H

#include "board.h"
#include <map>
#include <vector>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Keyboard.hpp>

enum operation {
  oLeft, oRight, oSD, oHD, oCW, oCCW, o180, oHold, oLMost, oRMost, oID, o__Last
}; constexpr int num_operations = o__Last;
enum IRSMode {
  irs_none, irs_tap, irs_hold
};
class game {
  void change_dir(int nd);
  void reset_opposite_das(int nd);

public:
  game_handler g;
  sf::Time das = sf::milliseconds(70), arr = sf::milliseconds(0), dcd = sf::milliseconds(0),
           charge_move = sf::Time::Zero, charge_move_dcd = sf::Time::Zero,
           are = sf::milliseconds(0), are_on_clear = sf::milliseconds(0), are_per_line = sf::milliseconds(0),
           charge_are = sf::Time::Zero,
           sddas = sf::milliseconds(0), sdarr = sf::milliseconds(0),
           charge_sd = sf::Time::Zero;
           // todo: gravity
  int charge_move_dir = 0;
  std::vector<sf::Keyboard::Key> keybinding[num_operations];
  bool key_down[num_operations] = { 0 };
  IRSMode irs = irs_none;

  game();

  void get_key_down_stat();
  void press(operation id);
  void tick(sf::Time dt);
};

#endif
