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
}

void game::get_key_down_stat() {
  for (int i = 0; i < num_operations; ++i) {
    bool down = false;
    for (auto k : keybinding[i]) {
      if (sf::Keyboard::isKeyPressed(k)) down = true;
    }
    if (down && !key_down[i]) press((operation)(i));
    key_down[i] = down;
    if (down) std::clog << i << " down" << std::endl;
  }
}
void game::press(operation id) {
  if (irs == irs_none && g.current.type == Minoes::m0) return; // when no irs and in ARE, refuse any operation
  switch (id) {
    case oLeft: g.move_left(); change_dir(-1); break;
    case oRight: g.move_right(); change_dir(1); break;
    case oSD: g.soft_drop(); break; // todo: das-related code for sd
    case oHD: g.hard_drop(); charge_are += are, charge_move_dcd = dcd; break;
    case oCW: g.rotate_cw(); charge_move_dcd = dcd; break;
    case oCCW: g.rotate_ccw(); charge_move_dcd = dcd; break;
    case o180: g.rotate_180(); charge_move_dcd = dcd; break;
    case oHold: g.hold(); break; // todo: check hold settings(whether infinite)
    case oLMost: g.move_leftmost(); reset_opposite_das(-1); break;
    case oRMost: g.move_rightmost(); reset_opposite_das(1); break;
    case oID: g.instant_drop(); break;
    default: break; // placeholder for o__Last
  }
}
void game::change_dir(int nd) {
  if (nd == charge_move_dir) return;
  charge_move_dir = nd;
  charge_move = nd ? das : sf::Time::Zero;
}
void game::reset_opposite_das(int nd) {
  if (nd == charge_move_dir || !charge_move_dir) return;
  charge_move = das;
}
void game::tick(sf::Time dt) {
  get_key_down_stat();

  // are
  if (g.current.type == Minoes::m0) {
    charge_are -= dt;
    if (charge_are <= sf::Time::Zero) {
      g.spawn();
      charge_are = sf::Time::Zero;
    }
  }

  // todo: handle irs -> gravity -> sd -> rotate

  // move
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
      if (g.current.type != Minoes::m0) {
        if (arr != sf::Time::Zero) {
          int move_amount = std::floor(-charge_move / arr) + 1;
          if (charge_move_dir == -1) g.move_left(move_amount);
          else g.move_right(move_amount);
          charge_move += arr * (std::int64_t)move_amount;
        } else {
          if (charge_move_dir == -1) g.move_leftmost();
          else g.move_rightmost();
          charge_move = sf::Time::Zero;
        }
      } else charge_move = sf::Time::Zero;
    }
  }
}
