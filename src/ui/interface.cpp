#include "../constants.h"
#include "../config/config.h"
#include "../ui_core/ui.h"
#include "../util/error_handling.h"
#include <SFML/Graphics/Rect.hpp>
#include <sstream>
#include <stdexcept>
#include "interface.h"

namespace ui_interface {
sf::RenderWindow window;
// sf::Font font;

namespace pre_initialize {
void prepare() {
  window.create(sf::VideoMode({ 640, 480 }), "NekoMino " GAME_VER_STRING);
  if (!ui::default_font.openFromFile("res/font.ttf")) logger::interface.error("Cannot open default font file res/font.ttf.");
}
void draw_step_text(std::string step) {
  sf::Text text(ui::default_font, step, 32); text.setFillColor(sf::Color::White);
  text.setOrigin(text.getLocalBounds().getCenter()); text.setPosition({ 320, 280 });
  window.draw(text);
}
bool deal_error(std::string step, bool fatal, std::string error) {
  window.clear(sf::Color::Black);
  draw_step_text(step);
  sf::Text text(ui::default_font, error + (fatal ? "\nPress any key or close the window to terminate" : "\nPress any key to continue, Close the window to terminate"), 14); text.setFillColor(fatal ? sf::Color::Red : sf::Color::Yellow); text.setStyle(sf::Text::Bold);
  text.setOrigin({ text.getLocalBounds().getCenter().x, 0 }); text.setPosition({ 320, 320 });
  window.draw(text);
  window.display();
  while (window.isOpen()) {
    while (const std::optional event = window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) return window.close(), true;
      if (event->is<sf::Event::KeyPressed>()) {
        if (fatal) return window.close(), true;
        else return false;
      }
    }
  }
  return true;
}
void show_step(std::string step) {
  window.clear(sf::Color::Black);
  draw_step_text(step);
  window.display();
  // sf::sleep(sf::seconds(0.25)); // pretend to be loading smth
  // // we've got smth to load! congrats
  // // ^ way too fast lets rest for 250ms
}
}

void initialize() {
  unsigned int sizex = config::get<long long>("display.window.width");
  unsigned int sizey = config::get<long long>("display.window.height");
  window.setSize({ sizex, sizey });
  sf::View view(sf::FloatRect({ 0, 0 }, { (float)sizex, (float)sizey }));
  window.setView(view);
  // window.setFramerateLimit(60);
}
void initialize_step(std::string step) {
  unsigned int sizex = config::get<long long>("display.window.width");
  unsigned int sizey = config::get<long long>("display.window.height");
  window.setSize({ sizex, sizey });
  // window.setFramerateLimit(60);
}

void render_temp(game &g) {
  std::ostringstream ss;
  g.print(ss);
  sf::Text text(ui::default_font, ss.str(), 12 * config::get<double>("demo.sizemult")); text.setFillColor(sf::Color::White);
  text.setPosition({ 0, 0 });
  window.draw(text);
}
}
