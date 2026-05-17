#include <algorithm>
#include <iostream>
#include <random>

#include <conio.h>
#include <windows.h>

#include "tetris_core/board.h"
#include "tetris_core/minoes.h"

#include "ui_core/interface.h"

void _demo_1() {
  std::mt19937 rng(std::random_device{}());
  std::vector<mino> bag = {
    mino(0, mI, mI),
    mino(0, mJ, mJ),
    mino(0, mL, mL),
    mino(0, mZ, mZ),
    mino(0, mS, mS),
    mino(0, mT, mT),
    mino(0, mO, mO),
  };
  
  game_handler game;
  game.b = board();
  int idn = 0;
  game.generator = [&rng, &bag, &idn](int n) {
    std::vector<mino> res;
    for (int i = 1; i <= n; i += 7) {
      std::shuffle(bag.begin(), bag.end(), rng);
      for (int j = 0; j < 7; j++) {
        bag[j].id = ++idn;
        res.push_back(bag[j]);
      }
    }
    return res;
  };

  game.on_line_clear = [](on_line_clear_argument) {
    const std::string lines[] = { "VOID", "SINGLE", "DOUBLE", "TRIPLE", "QUAD" };
    if (spin) {
      if (spin == spin_mini) std::cout << "MINI ";
      std::cout << get_mino_name(used_mino.type) << "-SPIN ";
    }
    std::cout << lines[line] << ' ';
    if (pc) {
      if (pc == pc_full) std::cout << "PERFECT CLEAR ";
      else if (pc == pc_color) std::cout << "COLOR CLEAR ";
      else std::cout << "HALF CLEAR ";
    }
    std::cout << '\n';
  };

  game.initialize();
  // game.print(std::cout); system("pause");

  game.spawn();
  game.print(std::cout);

  while (true) {
    while (!_kbhit());
    char ch = _getch();
    // std::cout << ch << '\n';
    if (ch == 'K') game.move_left();
    else if (ch == 'M') game.move_right();
    else if (ch == 'P') game.soft_drop();
    else if (ch == ' ') game.hard_drop();
    else if (ch == 'x') game.rotate_cw();
    else if (ch == 'z') game.rotate_ccw();
    else if (ch == 'a') game.rotate_180();
    else if (ch == 'c') game.hold();
    else continue;
    game.print(std::cout);
  }
}

int main() {
  ui::pre_initialize();

  rs::load_from_file("res/rotation_system/srs.txt");
  // rs::print(std::cout, rs::rotation_systems["SRS"], false);
  // rs::print(std::cout, rs::rotation_systems["SRS"], true);
  rotsys = &rs::rotation_systems["SRS"];

  while (ui::window.isOpen()) {
    while (const std::optional event = ui::window.pollEvent()) {
        // "close requested" event: we close the window
        if (event->is<sf::Event::Closed>())
          ui::window.close();
    }
  }
  return 0;
}
