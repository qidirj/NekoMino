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
  sf::Time charge_gravity = sf::Time::Zero;
  sf::Time charge_lock = sf::Time::Zero;
  int lock_refresh = 0; int rot_amount = 0; // when >=5-th rotation, lock_refresh -1 even if in air
  bool began_lock = false;
  bool key_down[num_operations] = { false }, key_pressed[num_operations] = { false };
  int irs_stash = 0, ihs_stash = 0;
  bool held = false;

  void get_key_down_stat();

  int move_left(int amt = 1);
  int move_right(int amt = 1);
  int move_leftmost();
  int move_rightmost();
  int rotate_cw();
  int rotate_ccw();
  int rotate_180();
  void refresh_lockdelay();

public:
  game_handler g;
  sf::Time das = sf::milliseconds(83), arr = sf::milliseconds(8), dcd = sf::milliseconds(0);
  sf::Time are = sf::milliseconds(100), are_on_clear = sf::milliseconds(333), are_per_line = sf::milliseconds(0);
  sf::Time sddas = sf::milliseconds(0), sdarr = sf::milliseconds(0);
  sf::Time gravity = sf::milliseconds(0), lock_delay = sf::milliseconds(500); // -1: no gravity or no auto lock
  std::vector<sf::Keyboard::Key> keybinding[num_operations];
  IXSMode irs = ixs_hold, ihs = ixs_hold, ims = ixs_hold; // * ims cannot be tap
  bool infinite_hold = false;
  int lock_refresh_max = 15, lock_refresh_on_hold = 15;

  game();

  void tick(sf::Time dt);

  void print(std::ostream &s);
};

#endif
