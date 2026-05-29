#include <SFML/System/Clock.hpp>
#include <algorithm>
#include <iostream>
#include <random>
#include <variant>

#include "config/config.h"
#include "tetris_core/board.h"
#include "tetris_core/core.h"
#include "tetris_core/minoes.h"

#include "ui/interface.h"
#include "util/localization.h"

void initialize() {
  ui_interface::pre_initialize::prepare();

  ui_interface::pre_initialize::show_step("Loading config");
  int config_result = config::initialize();
  if (config_result) {
    if (config_result == 1)      ui_interface::pre_initialize::deal_error("Loading config", true, std::to_string(config::validator::errors) + " errors generated when validating Default Config.\nPlease contact Daiari with your 'logs/config.log'."), std::exit(1);
    else if (config_result == 2) ui_interface::pre_initialize::deal_error("Loading config", true, std::to_string(config::loader::errors) + " errors generated when loading Config.\nPlease verify your '" + config::config_path + "', 'logs/config.log' may help."), std::exit(2);
    else if (config_result == 3) ui_interface::pre_initialize::deal_error("Loading config", false, std::to_string(config::loader::warns) + " warning generated when loading Config.\nPlease verify your '" + config::config_path + "', 'logs/config.log' may help.") ? std::exit(3) : void();
    else if (config_result == 4) ui_interface::pre_initialize::deal_error("Loading config", false, std::to_string(config::user_validator::warns) + " warning generated when validating Config.\nPlease verify your '" + config::config_path + "', 'logs/config.log' may help.") ? std::exit(4) : void();
  }
  
  LOG(8) << "Finish 1" << std::endl;
  sf::sleep(sf::seconds(1.0)); // to be deleted

  ui_interface::pre_initialize::show_step("Loading localization");
  localization::load_all_from_path("res/localization");
  localization::set_language();
  std::clog << localization::format("{about.game_title} in {!language.name_local}!") << std::endl;

  sf::sleep(sf::seconds(1.0)); // to be deleted
  
  rs::load_from_file("res/rotation_system/srs.txt");
  // rs::print(std::cout, rs::rotation_systems["SRS"], false);
  // rs::print(std::cout, rs::rotation_systems["SRS"], true);
  rotsys = &rs::rotation_systems["SRS"];

  ui_interface::initialize();
}

void demo_2() {
  std::string op, w;
  while (true) {
    std::cin >> op;
    if (op == "print") std::cout << config::writer::write_ini_to_string(config::config) << std::endl;
    else if (op == "end") break;
    else {
      std::cin >> w;
      if (op == "set") {
        std::string tp, val;
        std::cin >> tp >> val;
        config::entry_t val1;
        if (tp == "i") val1 = std::stoll(val);
        else if (tp == "f") val1 = std::stod(val);
        else if (tp == "b") val1 = (val == "true");
        else if (tp == "s") val1 = val;
        if (std::holds_alternative<std::monostate>(val1)) std::cout << "Unknown Type" << std::endl;
        else {
          auto res = config::set(w, val1);
          if (res == 1) std::cout << "Not found" << std::endl;
          else if (res == 2) std::cout << "Cannot set map" << std::endl;
          else if (res == 3) std::cout << "Incorrect value" << std::endl;
          else std::cout << "Success" << std::endl;
        }
      } else if (op == "get") {
        auto res = config::getraw(w);
        if (std::holds_alternative<std::monostate>(res)) std::cout << "Not found" << std::endl;
        else config::writer::write_node_as_string_to_stream(std::cout, res), std::cout << std::endl;
      } else if (op == "erase") {
        auto res = config::erase(w);
        if (res) std::cout << "Success" << std::endl;
        else std::cout << "Failed" << std::endl;
      }
    }
  }
  std::exit(0);
}

std::vector<float> dt_a, dt_b;

int main() {
  initialize();

  // demo_2();

  sf::Clock clock; clock.restart();
  sf::Time render_cd, render_rate = sf::seconds(1.0 / 60.0);
  game g; g.load_demo();
  int frame = 0;
  while (ui_interface::window.isOpen()) {
    auto dt = clock.restart();
    while (const std::optional event = ui_interface::window.pollEvent()) {
      // "close requested" event: we close the window
      if (event->is<sf::Event::Closed>())
        ui_interface::window.close();
    }
    g.tick(dt);
    if ((render_cd += dt) >= render_rate) {
      render_cd -= render_rate;
      ui_interface::window.clear(sf::Color::Black);
      ui_interface::render_temp(g);
      ui_interface::window.display();
      dt_b.push_back(dt.asSeconds());
      LOG(7) << "Render" << ++frame << std::endl;
      // std::clog << dt.asSeconds() << std::endl;
    } else {
      dt_a.push_back(dt.asSeconds());
      LOG(7) << "Logic " << ++frame << std::endl;;
    }
    // if (dt_a.size() == 200000) break;
    // if (dt_a.size() % 1000 == 0) std::clog << dt_a.size() << std::endl;
  }
  ui_interface::window.close();
  
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
