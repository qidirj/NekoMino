#include "../constants.h"
#include "core.h"
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <random>
#include <SFML/Window/Keyboard.hpp>

std::vector<mino> seven_bag(int n) {
  static std::mt19937 rng(std::random_device{}());
  static std::vector<mino> bag = {
    mino(0, mI, mI),
    mino(0, mJ, mJ),
    mino(0, mL, mL),
    mino(0, mZ, mZ),
    mino(0, mS, mS),
    mino(0, mT, mT),
    mino(0, mO, mO),
  };
  static uint64_t idn = 0;
  std::vector<mino> res;
  for (int i = 1; i <= n; i += 7) {
    std::shuffle(bag.begin(), bag.end(), rng);
    for (int j = 0; j < 7; j++) {
      bag[j].id = ++idn;
      res.push_back(bag[j]);
    }
  }
  return res;
}

game::game() {
  g.generator = seven_bag;
  g.on_line_clear = [&](on_line_clear_argument) {
    charge_are += are_on_clear * (std::int64_t)(line > 0) + are_per_line * (std::int64_t)(line);
  };
  g.initialize();

  keybinding[oLeft] = { sf::Keyboard::Key::Left };
  keybinding[oRight] = { sf::Keyboard::Key::Right };
  keybinding[oSD] = { sf::Keyboard::Key::Down };
  keybinding[oHD] = { sf::Keyboard::Key::Space };
  keybinding[oCW] = { sf::Keyboard::Key::X };
  keybinding[oCCW] = { sf::Keyboard::Key::Z };
  keybinding[o180] = { sf::Keyboard::Key::A };
  keybinding[oHold] = { sf::Keyboard::Key::LShift, sf::Keyboard::Key::RShift };
}

void game::get_key_down_stat() {
  for (int i = 0; i < num_operations; ++i) {
    bool down = false;
    for (auto k : keybinding[i]) {
      if (sf::Keyboard::isKeyPressed(k)) down = true;
    }
    if (down && !key_down[i]) key_pressed[i] = true;
    else key_pressed[i] = false;
    key_down[i] = down;
    // if (down) LOG(2) << i << " down" << std::endl;
  }
}
// void game::press(operation id) {
// }
void game::change_dir(int nd) {
  if (nd == charge_move_dir) return;
  charge_move_dir = nd;
  charge_move = nd ? das : sf::Time::Zero;
}
void game::reset_opposite_das(int nd) {
  if (nd == charge_move_dir || !charge_move_dir) return;
  charge_move = das;
}
#define has_mino if (g.current.type != Minoes::m0)
#define hasnt_mino if (g.current.type == Minoes::m0)
#define stash(ixs) else if (ixs == ixs_tap)
void game::tick(sf::Time dt) {
  get_key_down_stat();

  // are, handle ihs, irs, ims
  hasnt_mino {
    charge_are -= dt;
    if (charge_are <= sf::Time::Zero) {
      LOG(3) << "Inside spawn" << std::endl;

      g.spawn(); held = false;
      
      charge_gravity = sf::Time::Zero;
      charge_lock = sf::Time::Zero;
      lock_refresh = lock_refresh_max;
      rot_amount = 0; began_lock = false;

      if ((ihs == ixs_hold && key_down[oHold]) || (ihs == ixs_tap && ihs_stash)) {
        g.hold(); held = true;
      }
      if (irs == ixs_hold || irs == ixs_tap) {
        int kc = 0, k = 0;
        if (irs == ixs_hold) {
          if (key_down[oCW]) ++kc, k = 1;
          if (key_down[oCCW]) ++kc, k = -1;
          if (key_down[o180]) ++kc, k = 2;
        } else kc = 1, k = irs_stash;
        if (kc == 1) {
          if (k == 1) rotate_cw();
          else if (k == -1) rotate_ccw();
          else if (k == 2) rotate_180();
        }
      }
      if (ims == ixs_hold) {
        if (key_pressed[oLMost] ^ key_pressed[oRMost]) {
          if (key_pressed[oLMost]) { move_leftmost(); reset_opposite_das(-1); }
          if (key_pressed[oRMost]) { move_rightmost(); reset_opposite_das(1); }
        }
        if (key_pressed[oLeft] ^ key_pressed[oRight]) {
          if (key_pressed[oLeft]) { move_left(); change_dir(-1); }
          if (key_pressed[oRight]) { move_right(); change_dir(1); }
          charge_move = std::max(charge_move, arr);
        }
      }
      irs_stash = 0; ihs_stash = 0;
      
      charge_are = sf::Time::Zero;

      LOG(3) << "Outside spawn" << std::endl;
    }
  }

  // gravity
  has_mino {
    if (gravity >= sf::Time::Zero) {
      charge_gravity += dt;
      if (charge_gravity >= gravity) {
        if (gravity > sf::Time::Zero) {
          int move_amount = std::floor(charge_gravity / gravity);
          g.soft_drop(move_amount);
          charge_gravity -= gravity * (std::int64_t)move_amount;
        } else if (gravity == sf::Time::Zero) {
          g.instant_drop();
          charge_gravity = sf::Time::Zero;
        }
      }
    }
  }

  // sd
  {
    if (key_pressed[oSD]) { has_mino g.soft_drop(); charge_sd_status = 1, charge_sd = sddas; }
    if (key_pressed[oID]) { has_mino g.instant_drop(); }
    if (charge_sd_status && !key_down[oSD]) charge_sd_status = 0, charge_sd = sf::Time::Zero;
    if (charge_sd_status) {
      charge_sd -= dt;
      if (charge_sd <= sf::Time::Zero) {
        has_mino {
          if (sdarr != sf::Time::Zero) {
            int move_amount = std::floor(-charge_sd / sdarr) + 1;
            g.soft_drop(move_amount);
            charge_sd += sdarr * (std::int64_t)move_amount;
          } else {
            g.instant_drop();
            charge_sd = sf::Time::Zero;
          }
        } else charge_sd = sf::Time::Zero;
      }
    }
  }
  LOG(3) << "1" << std::endl;

  // lock delay
  has_mino {
    if (lock_delay >= sf::Time::Zero) {
      if (g.touched_ground() || (lock_refresh == 0 && began_lock)) {
        began_lock = true;
        charge_lock += dt;
        if (charge_lock >= lock_delay) {
          g.hard_drop();
          charge_are += are, charge_move_dcd = dcd;
        }
      } else charge_lock = sf::Time::Zero;
    }
  }

  // move
  {
    if (key_pressed[oLeft]) { has_mino move_left(); change_dir(-1); }
    if (key_pressed[oRight]) { has_mino move_right(); change_dir(1); }
    if (key_pressed[oLMost]) { has_mino move_leftmost(); reset_opposite_das(-1); }
    if (key_pressed[oRMost]) { has_mino move_rightmost(); reset_opposite_das(1); }
    if (charge_move_dir == -1 && !key_down[oLeft]) {
      if (key_down[oRight]) change_dir(1);
      else change_dir(0);
    }
    if (charge_move_dir == 1 && !key_down[oRight]) {
      if (key_down[oLeft]) change_dir(-1);
      else change_dir(0);
    }
    if (charge_move_dir) {
      charge_move_dcd -= dt;
      if (charge_move_dcd <= sf::Time::Zero)
        charge_move += charge_move_dcd, charge_move_dcd = sf::Time::Zero;
      if (charge_move <= sf::Time::Zero) {
        has_mino {
          if (arr != sf::Time::Zero) {
            int move_amount = std::floor(-charge_move / arr) + 1;
            if (charge_move_dir == -1) move_left(move_amount);
            else move_right(move_amount);
            charge_move += arr * (std::int64_t)move_amount;
          } else {
            if (charge_move_dir == -1) move_leftmost();
            else move_rightmost();
            charge_move = sf::Time::Zero;
          }
        } else charge_move = sf::Time::Zero;
      }
    }
  }
  LOG(3) << "2" << std::endl;

  // rotate
  {
    if (key_pressed[oCW]) { has_mino rotate_cw(); stash(irs) irs_stash = 1; charge_move_dcd = dcd; }
    if (key_pressed[oCCW]) { has_mino rotate_ccw(); stash(irs) irs_stash = -1; charge_move_dcd = dcd; }
    if (key_pressed[o180]) { has_mino rotate_180(); stash(irs) irs_stash = 2; charge_move_dcd = dcd; }
  }

  // hd, hold
  {
    if (key_pressed[oHD]) { has_mino g.hard_drop(); charge_are += are, charge_move_dcd = dcd; }
    if (key_pressed[oHold]) {
      has_mino {
        if (!held || infinite_hold) {
          g.hold(), held = true;
          charge_gravity = sf::Time::Zero;
          charge_lock = sf::Time::Zero;
          lock_refresh = std::min(lock_refresh_max, lock_refresh + lock_refresh_on_hold);
          rot_amount = 0; began_lock = false;
        }
      } stash(ihs) ihs_stash = 1;
    }
  }
}

int game::move_left(int amt) {
  int m = 0; bool tg = g.touched_ground();
  while (amt--) {
    if (!g.move_left()) return m;
    ++m;
    if (lock_refresh > 0) charge_lock = sf::Time::Zero;
    if (tg) lock_refresh = std::max(lock_refresh - 1, 0), began_lock = true;
    tg = g.touched_ground();
  }
  return m;
}
int game::move_right(int amt) {
  int m = 0; bool tg = g.touched_ground();
  while (amt--) {
    if (!g.move_right()) return m;
    ++m;
    if (lock_refresh > 0) charge_lock = sf::Time::Zero;
    if (tg) lock_refresh = std::max(lock_refresh - 1, 0), began_lock = true;
    tg = g.touched_ground();
  }
  return m;
}
int game::move_leftmost() {
  int amt = g.b.width, m = 0; bool tg = g.touched_ground();
  while (amt--) {
    if (!g.move_left()) return m;
    ++m;
    if (lock_refresh > 0) charge_lock = sf::Time::Zero;
    if (tg) lock_refresh = std::max(lock_refresh - 1, 0), began_lock = true;
    tg = g.touched_ground();
  }
  return m;
}
int game::move_rightmost() {
  int amt = g.b.width, m = 0; bool tg = g.touched_ground();
  while (amt--) {
    if (!g.move_right()) return m;
    ++m;
    if (lock_refresh > 0) charge_lock = sf::Time::Zero;
    if (tg) lock_refresh = std::max(lock_refresh - 1, 0), began_lock = true;
    tg = g.touched_ground();
  }
  return m;
}
int game::rotate_cw() {
  bool tg = g.touched_ground();
  int res = g.rotate_cw();
  if (res != -1 && lock_refresh > 0) charge_lock = sf::Time::Zero;
  if (res != -1 && (++rot_amount >= 5 || tg)) lock_refresh = std::max(lock_refresh - 1, 0), began_lock = true;
  return res;
}
int game::rotate_ccw() {
  bool tg = g.touched_ground();
  int res = g.rotate_ccw();
  if (res != -1 && lock_refresh > 0) charge_lock = sf::Time::Zero;
  if (res != -1 && (++rot_amount >= 5 || tg)) lock_refresh = std::max(lock_refresh - 1, 0), began_lock = true;
  return res;
}
int game::rotate_180() {
  bool tg = g.touched_ground();
  int res = g.rotate_180();
  if (res != -1 && lock_refresh > 0) charge_lock = sf::Time::Zero;
  if (res != -1 && (++rot_amount >= 5 || tg)) lock_refresh = std::max(lock_refresh - 1, 0), began_lock = true;
  return res;
}

void game::print(std::ostream &s) {
  g.print(s);
  auto ldr = 1 - (charge_lock / lock_delay); int n = std::round(10 * ldr);
  s << std::string(n, '&') << std::string(10 - n, ' ') << '|' << lock_refresh;
}

