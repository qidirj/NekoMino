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
enum IXSMode {
  ixs_none, ixs_tap, ixs_hold
};
class game {
  void change_dir(int nd);
  void reset_opposite_das(int nd);

  sf::Time charge_move = sf::Time::Zero, charge_move_dcd = sf::Time::Zero;
  int charge_move_dir = 0;
  sf::Time charge_are = sf::Time::Zero;
  sf::Time charge_sd = sf::Time::Zero;
  int charge_sd_status = 0;
  sf::Time charge_gravity = sf::Time::Zero; // -1: no gravity
  bool key_down[num_operations] = { false }, key_pressed[num_operations] = { false };
  int irs_stash = 0, ihs_stash = 0;
  bool held = false;

public:
  game_handler g;
  sf::Time das = sf::milliseconds(70), arr = sf::milliseconds(0), dcd = sf::milliseconds(0);
  sf::Time are = sf::milliseconds(0), are_on_clear = sf::milliseconds(0), are_per_line = sf::milliseconds(0);
  sf::Time sddas = sf::milliseconds(0), sdarr = sf::milliseconds(0);
  sf::Time gravity = sf::milliseconds(0);
  std::vector<sf::Keyboard::Key> keybinding[num_operations];
  IXSMode irs = ixs_none, ihs = ixs_none, ims = ixs_hold; // * ims cannot be tap
  bool infinite_hold = false;

  game();

  void get_key_down_stat();
  void press(operation id);
  void tick(sf::Time dt);
};

#endif
