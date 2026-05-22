#include <SFML/System/Clock.hpp>
#include <algorithm>
#include <iostream>
#include <random>

#include "config/config.h"
#include "tetris_core/board.h"
#include "tetris_core/core.h"
#include "tetris_core/minoes.h"

#include "ui_core/interface.h"

void initialize() {
  ui::pre_initialize();

  int config_result = config::initialize();
  if (config_result) {
    if (config_result == 1) ui::pre_initialize_render_error(true, std::to_string(config::validator::errors) + " errors generated when validating Default Config.\nPlease contact Daiari with your 'logs/config.log'."), std::exit(1);
    else if (config_result == 2) ui::pre_initialize_render_error(true, std::to_string(config::loader::errors) + " errors generated when loading Config.\nPlease verify your '" + config::config_path + "', 'logs/config.log' may help."), std::exit(2);
    else if (config_result == 3) ui::pre_initialize_render_error(false, std::to_string(config::loader::warns) + " warning generated when loading Config.\nPlease verify your '" + config::config_path + "', 'logs/config.log' may help.") ? std::exit(3) : void();
    else if (config_result == 4) ui::pre_initialize_render_error(false, std::to_string(config::user_validator::warns) + " warning generated when validating Config.\nPlease verify your '" + config::config_path + "', 'logs/config.log' may help.") ? std::exit(4) : void();
  }

  std::clog << config::writer::write_ini_to_string(config::default_config) << std::endl;

  rs::load_from_file("res/rotation_system/srs.txt");
  // rs::print(std::cout, rs::rotation_systems["SRS"], false);
  // rs::print(std::cout, rs::rotation_systems["SRS"], true);
  rotsys = &rs::rotation_systems["SRS"];

  ui::initialize();
}


std::vector<float> dt_a, dt_b;

int main() {
  initialize();

  sf::Clock clock; clock.restart();
  sf::Time render_cd, render_rate = sf::seconds(1.0 / 60.0);
  game g;
  while (ui::window.isOpen()) {
    auto dt = clock.restart();
    while (const std::optional event = ui::window.pollEvent()) {
      // "close requested" event: we close the window
      if (event->is<sf::Event::Closed>())
        ui::window.close();
    }
    g.tick(dt);
    if ((render_cd += dt) >= render_rate) {
      render_cd -= render_rate;
      ui::window.clear(sf::Color::Black);
      ui::render_temp(g);
      ui::window.display();
      dt_b.push_back(dt.asSeconds());
      // std::clog << dt.asSeconds() << std::endl;
    } else dt_a.push_back(dt.asSeconds());
    if (dt_a.size() == 200000) break;
    // if (dt_a.size() % 1000 == 0) std::clog << dt_a.size() << std::endl;
  }
  ui::window.close();
  
  double sum = 0, avr, var;
  for (auto i : dt_a) sum += i;
  avr = sum / dt_a.size();
  for (auto i : dt_a) var += (i - avr) * (i - avr);
  var = std::sqrt(var / dt_a.size());
  std::clog << "Logic  frame: " << dt_a.size() << ", time " << avr << "+-" << var << std::endl;

  sum = 0;
  for (auto i : dt_b) sum += i;
  avr = sum / dt_b.size();
  for (auto i : dt_b) var += (i - avr) * (i - avr);
  var = std::sqrt(var / dt_b.size());
  std::clog << "Render frame: " << dt_b.size() << ", time " << avr << "+-" << var << std::endl;
  return 0;
}
