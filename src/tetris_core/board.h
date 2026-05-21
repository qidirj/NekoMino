#ifndef BOARD_H
#define BOARD_H

#include "rs.h"
#include "../util/enumerator.h"
#include "../constants.h"
#include <cstdint>
#include <iostream>
#include <functional>

struct cell {
  uint64_t id;
  Minoes type;
  int edge; // for connnected texture, 4bit
  int amount; // for garbage cell
};
struct mino {
  uint64_t id;
  Minoes type;
  std::vector<std::vector<cell>> shape;
  int x, y; // position, multiplied by 2
  int cx, cy; // center, multiplied by 2
  int d; // 0-3

  mino(); // with type 0 and empty shape
  mino(uint64_t id, Minoes m_shape, Minoes m_display); // with shape from mino m_shape, and appearance from mino m_display (eg. bone)

  std::pair<std::pair<int, int>, std::pair<int, int>> span(); // minimum rectangle that covers the shape

  void rotate(int th = 0); // clockwise
};
/*
row 19
row 18
...
row 1
row 0
col    0 1 ... 9
*/
struct board {
  std::vector<std::vector<cell>> grid;
  int width, height;

  board(int w = 10, int h = 20);
};


// accepts operation, emit line clear event, does not handle anything else
// template<>
#define on_line_clear_argument int line, mino &used_mino, SpinType spin, PCType pc, int zone// may change
class game_handler {
private:
  int move(int dx, int dy);
  int rotate(int delta);
  void expand(); // expand the board if current mino is too high
  void fix(); // freeze the current mino and deal with line clear
  void generate(int n = 1);
  void test_clear_line();

  bool moveable(int dx, int dy);

  int zone = zone_none; int zone_line = 0;
  bool can_be_spin = false, is_mini_spin = false; int last_kick_diff = 0;

public:
  board b;
  std::function<std::vector<mino> (int)> generator; // generate at least %0 minoes. if d=-1 use default dir instead
  std::function<void (on_line_clear_argument)> on_line_clear;

  mino current;
  std::vector<mino> next_queue, hold_queue; int next_limit = 6, hold_limit = 1;

  rs::rotation_system_t *rotsys = rs::rotsys;

  void initialize(); // generate 6 minoes initially
  void spawn(); // spawn the next mino, fill the next queue
  void start_zone(ZoneType z);
  void end_zone(); // exit zone, emit a line_clear
  void recalculate_texture();

  bool collide();
  bool touched_ground();

  // -1 for failed, others for amount (eg. cells moved, kick entry used, etc.)
  int move_left(int amt = 1); // arrow_left
  int move_right(int amt = 1); // arrow_right
  int move_leftmost(); // for arr=0
  int move_rightmost(); // for arr=0
  int soft_drop(int amt = 1); // arrow_down
  int instant_drop(); // for sdarr=0
  int hard_drop(); // space
  int hold(); // lshift, rshift -> return: previous hold amount (-1 for no hold)
  int rotate_cw(); // x, arrow_up
  int rotate_ccw(); // z
  int rotate_180(); // a, c

  void print(std::ostream &s);
};

#endif
