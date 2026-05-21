#include "board.h"
#include "../constants.h"
#include "../util/error_handling.h"
#include <cassert>
#include <iomanip>

mino::mino() : id(0), type(Minoes::m0), shape(), x(0), y(0), d(0) {}
mino::mino(uint64_t id, Minoes m_shape, Minoes m_display) : id(id), x(0), y(0), d(0) {
  type = m_shape;
  cx = rs::rotsys->rs[m_shape].center.first, cy = rs::rotsys->rs[m_shape].center.second;
  auto &oshape = mino_shapes[m_shape];
  shape.resize(oshape.size());
  for (size_t i = 0; i < shape.size(); i++) {
    shape[i].resize(oshape[i].size());
    for (size_t j = 0; j < shape[i].size(); j++) {
      if (oshape[i][j] == Minoes::m0) shape[i][j] = { id, Minoes::m0, 0, 0 };
      else shape[i][j] = { id, m_display, 0, 0 };
    }
  }
}
std::pair<std::pair<int, int>, std::pair<int, int>> mino::span() {
  int minx = 1e9, maxx = -1, miny = 1e9, maxy = -1;
  for (size_t i = 0; i < shape.size(); i++) {
    for (size_t j = 0; j < shape[i].size(); j++) {
      if (shape[i][j].type == Minoes::m0) continue;
      if (int(i) < minx) minx = i;
      if (int(i) > maxx) maxx = i;
      if (int(j) < miny) miny = j;
      if (int(j) > maxy) maxy = j;
    }
  }
  return {{minx * 2 - cx, maxx * 2 - cx}, {miny * 2 - cy, maxy * 2 - cy}};
}
void mino::rotate(int th) {
  d = (d + th + 4) % 4;
  for (int i = 0; i < th; i++) {
    std::vector<std::vector<cell>> new_shape(shape[0].size(), std::vector<cell>(shape.size()));
    for (size_t j = 0; j < shape.size(); j++) {
      for (size_t k = 0; k < shape[j].size(); k++) {
        new_shape[k][shape.size() - 1 - j] = shape[j][k];
      }
    }
    shape = new_shape;
    int old_cx = cx, old_cy = cy;
    cx = old_cy; cy = (shape.size() - 1) * 2 - old_cx;
  }
}

board::board(int w, int h) : width(w), height(h) {
  grid.resize(height);
  for (int i = 0; i < height; i++) {
    grid[i].resize(width);
    for (int j = 0; j < width; j++) {
      grid[i][j] = { 0, Minoes::m0, 0, 0 };
    }
  }
}

void game_handler::generate(int n) {
  auto new_minoes = generator(n);
  for (auto &m : new_minoes) {
    if (m.d == -1) m.d = rs::rotsys->rs[m.type].spawn_direction;
    next_queue.push_back(m);
  }
}
void game_handler::initialize() {
  current = mino();
  generate(next_limit);
}
void game_handler::spawn() {
  can_be_spin = true;
  is_mini_spin = false;
  last_kick_diff = 0;

  current = next_queue.front();
  next_queue.erase(next_queue.begin());
  if (next_queue.size() < next_limit) generate(next_limit - next_queue.size());

  auto span = current.span();
  current.x = b.height * 2 + span.first.second;
  current.y = ((b.width - ((span.second.second - span.second.first) / 2 + 1)) >> 1) * 2 - span.second.first;
}

bool game_handler::collide() {
  expand();
  for (size_t i = 0; i < current.shape.size(); i++) {
    for (size_t j = 0; j < current.shape[i].size(); j++) {
      if (current.shape[i][j].type == Minoes::m0) continue;
      int x = (current.x + current.cx - i * 2) / 2, y = (current.y + j * 2 - current.cy) / 2;
      if (x < 0 || y < 0 || y >= b.width) return true;
      if (b.grid[x][y].type != Minoes::m0) return true;
    }
  }
  return false;
}

int game_handler::move(int dx, int dy) {
  LOG(3) << "move: " << dx << ' ' << dy << std::endl;
#ifdef DEBUG
  assert((!!dx) ^ (!!dy)); // exactly one of dx and dy should be nonzero
#endif
  int ddx = dx > 0 ? 2 : -2, ddy = dy > 0 ? 2 : -2;
  int cx = rotsys->rs[current.type].center.first, cy = rotsys->rs[current.type].center.second;
  int n = dx / (ddx / 2) + dy / (ddy / 2), m = 0;
  if (!dx) ddx = 0; if (!dy) ddy = 0;
  while (m < n) {
    current.x += ddx; current.y += ddy;
    if (collide()) {
      current.x -= ddx; current.y -= ddy;
      if (m) can_be_spin = false;
      LOG(1) << "moved " << m << std::endl;
      return m;
    }
    ++m;
  }
  if (n) can_be_spin = false;
  LOG(1) << "moved " << n << std::endl;
  return n;
}
int game_handler::rotate(int delta) {
  int prev_d = current.d;
  current.rotate((delta + 4) % 4);
  int id = delta == -1 ? 0 : delta, m = 0; // LR2
  auto &kts = rotsys->rs[current.type].kick_table;
  for (auto kt : kts) {
    for (auto &test : kt->kt[prev_d][id]) {
      int dx = test.first, dy = test.second;
      current.x += dx * 2; current.y += dy * 2;
      if (!collide()) {
        can_be_spin = true;
        last_kick_diff = std::abs(dx) + std::abs(dy);
        return m;
      }
      current.x -= dx * 2; current.y -= dy * 2; ++m;
    }
  }
  current.rotate((-delta + 4) % 4);
  return -1;
}

void game_handler::expand() {
  if (current.type == Minoes::m0) return;
  size_t bound = (current.x + (current.shape.size() * 2 - current.cx)) / 2;
  if (bound > 1024) {
    logger::tetris_core.error("When expanding board, the needed height is too high: " + std::to_string(bound) + ". This may be caused by a bug in the code, or an invalid rotation system data. To prevent potential oom exception, the height is limited to 1024.");
    bound = 1024;
  }
  if (bound < b.grid.size()) return;
  logger::tetris_core.debugv(4, "Expanding board to " + std::to_string(bound));
  while (b.grid.size() <= bound) {
    b.grid.push_back(std::vector<cell>(b.width, { 0, Minoes::m0, 0, 0 }));
  }
}
void game_handler::test_clear_line() {
  int lowest = (current.x - current.span().first.second) / 2;
  bool can_be_hpc = true, can_be_cpc = true, can_be_pc = true;
  std::vector<std::vector<cell>> new_grid, cleared;
  for (size_t i = 0; i < b.grid.size(); i++) {
    bool full = true, empty = true, garbage = true;
    for (size_t j = 0; j < b.grid[i].size(); j++) {
      if (b.grid[i][j].type == Minoes::m0) {
        full = false;
      } else {
        empty = false;
        if (b.grid[i][j].type != Minoes::mG) garbage = false;
      }
    }
    if (full) cleared.push_back(b.grid[i]);
    else new_grid.push_back(b.grid[i]);
    if (!full && !empty && i >= lowest) can_be_hpc = false;
    if (!full && !empty && !garbage) can_be_cpc = false;
    if (!full && !empty) can_be_pc = false;
  }
  if (cleared.empty()) return;
  if (zone != ZoneType::zone_stack) {
    b.grid.clear();
    if (zone == ZoneType::zone_sink)
      for (auto &line : cleared) b.grid.push_back(line);
    for (auto &line : new_grid) b.grid.push_back(line);
  }
  int line = cleared.size() - (zone && !(zone & zone_ended) ? zone_line : 0);
  if (zone && !(zone & zone_ended)) zone_line = cleared.size();
  SpinType spin = can_be_spin ? (is_mini_spin ? spin_mini : spin_full) : spin_none;
  PCType pc = can_be_pc ? pc_full : (can_be_hpc ? (can_be_cpc ? pc_color : pc_half) : pc_none);
  on_line_clear(line, current, spin, pc, zone);
}
void game_handler::start_zone(ZoneType z) {
  if (zone) end_zone();
  zone = z;
  zone_line = 0;
}
void game_handler::end_zone() {
  if (zone) {
    zone |= zone_ended;
    test_clear_line();
    zone = zone_none; zone_line = 0;
  }
}
void game_handler::fix() {
  if (current.type == Minoes::mT && can_be_spin) {
    bool is_real_T = current.cx == 2 && current.cy == 2 && current.shape.size() == 3 && current.shape[1][1].type != Minoes::m0 &&
                      ((current.shape[0][1].type == Minoes::m0) + (current.shape[2][1].type == Minoes::m0) + (current.shape[1][0].type == Minoes::m0) + (current.shape[1][2].type == Minoes::m0) == 1) && 
                      (current.shape[0][0].type == Minoes::m0 && current.shape[0][2].type == Minoes::m0 && current.shape[2][0].type == Minoes::m0 && current.shape[2][2].type == Minoes::m0);
    if (is_real_T) {
      int x = current.x / 2, y = current.y / 2;
      int zx = -1, zy = 0;
      if (current.shape[2][1].type == Minoes::m0) zx = 1, zy = 0;
      else if (current.shape[1][0].type == Minoes::m0) zx = 0, zy = -1;
      else if (current.shape[1][2].type == Minoes::m0) zx = 0, zy = 1;

      auto test = [&] (int nx, int ny) {
        nx = x - nx, ny = y + ny;
        if (nx < 0 || ny < 0 || ny >= b.width) return true;
        if (nx >= b.grid.size()) return false;
        if (b.grid[nx][ny].type != Minoes::m0) return true;
        return false;
      };

      bool a1 = test(zx ? -zx : -1, zy ? -zy : -1), a2 = test(zx ? -zx : 1, zy ? -zy : 1);
      bool b1 = test(zx ? zx : -1, zy ? zy : -1), b2 = test(zx ? zx : 1, zy ? zy : 1);
      if (a1 + a2 + b1 + b2 < 3) is_mini_spin = true;
      else if (!a1 || !a2) is_mini_spin = true;
      if (a1 + a2 + b1 + b2 >= 3) can_be_spin = true;
      if (last_kick_diff >= 3) can_be_spin = can_be_spin || is_mini_spin, is_mini_spin = false;
      LOG(1) << "T-spin test: " << a1 << ' ' << a2 << ' ' << b1 << ' ' << b2 << std::endl;
    }
  } else can_be_spin = can_be_spin &&
                      !moveable(-1, 0) && !moveable(1, 0) &&
                      !moveable(0, -1) && !moveable(0, 1);

  expand();
  for (size_t i = 0; i < current.shape.size(); i++) {
    for (size_t j = 0; j < current.shape[i].size(); j++) {
      if (current.shape[i][j].type == Minoes::m0) continue;
      int x = (current.x + current.cx - i * 2) / 2, y = (current.y + j * 2 - current.cy) / 2;
      b.grid[x][y] = current.shape[i][j];
    }
  }
  test_clear_line();
  // spawn(); // done: let game handle every spawn (for potential ARE)
  current = mino(); // reset mino; check if its a m0
}
bool game_handler::moveable(int dx, int dy) {
  current.x += dx * 2; current.y += dy * 2;
  bool res = !collide();
  current.x -= dx * 2; current.y -= dy * 2;
  LOG(1) << "moveable: " << dx << ' ' << dy << ' ' << res << std::endl;
  return res;
}
bool game_handler::touched_ground() {
  return !moveable(-1, 0);
}

int game_handler::move_left(int amt) { return move(0, -amt); }
int game_handler::move_right(int amt) { return move(0, amt); }
int game_handler::move_leftmost() { return move(0, -b.width); }
int game_handler::move_rightmost() { return move(0, b.width); }
int game_handler::soft_drop(int amt) { return move(-amt, 0); }
int game_handler::instant_drop() { return move(-std::max(1, (int)(current.x / 2 + current.shape.size() + 1)), 0); }
int game_handler::hard_drop() {
  int amt = instant_drop();
  fix();
  return amt;
}
int game_handler::hold() {
  if (!hold_limit) return -1;
  current.rotate((4 - current.d) % 4);
  int amt = hold_queue.size();
  auto prev = current;
  if (amt == hold_limit) {
    current = hold_queue.front();
    hold_queue.erase(hold_queue.begin());
    hold_queue.push_back(prev);

    auto span = current.span();
  current.x = b.height * 2 + span.first.second;
  current.y = ((b.width - ((span.second.second - span.second.first) / 2 + 1)) >> 1) * 2 - span.second.first;
  } else {
    hold_queue.push_back(prev);
    spawn();
  }
  return amt;
}
int game_handler::rotate_cw() { return rotate(1); }
int game_handler::rotate_ccw() { return rotate(-1); }
int game_handler::rotate_180() { return rotate(2); }

void game_handler::print(std::ostream &s) {
  s << "Hold: ";
  for (auto i : hold_queue) s << get_mino_name(i.type) << ' ';
  s << '\n';
  s << "Next: ";
  for (auto i : next_queue) s << get_mino_name(i.type) << ' ';
  s << '\n';
  s << current.x << ' ' << current.y << ' ' << current.d << '\n';
  s << current.cx << ' ' << current.cy << '\n';
  expand();
  auto g = b.grid;
  if (current.type != Minoes::m0) {
    for (size_t i = 0; i < current.shape.size(); i++) {
      for (size_t j = 0; j < current.shape[i].size(); j++) {
        if (current.shape[i][j].type == Minoes::m0) continue;
        int x = (current.x + current.cx - i * 2) / 2, y = (current.y + j * 2 - current.cy) / 2;
        g[x][y] = current.shape[i][j];
      }
    }
  }
  for (int i = g.size() - 1; i >= 0; i--) {
    s << std::setw(2) << i << " | ";
    for (size_t j = 0; j < g[i].size(); j++) {
      if (g[i][j].type == Minoes::m0) s << ".";
      else s << get_mino_name(g[i][j].type);
    }
    s << '\n';
  }
  s.flush();
}

void game_handler::recalculate_texture() {
  // todo: add texture code
}

/*
 TODO: texture (mino & game_handler, different types), end zone when top out (maybe go to game)
 */
