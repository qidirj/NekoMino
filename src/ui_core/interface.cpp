#include "../constants.h"
#include "../util/error_handling.h"
#include <sstream>
#include "interface.h"

namespace ui {
sf::RenderWindow window;
sf::Font font;
void pre_initialize() {
  window = sf::RenderWindow(sf::VideoMode({ 640, 480 }), "NekoMino " GAME_VER_STRING);
  window.clear(sf::Color::Black);
  if (!font.openFromFile("res/font.ttf")) logger::interface.error("Cannot open default font file res/font.ttf.");
  sf::Text text(font, "Loading config", 48); text.setFillColor(sf::Color::White);
  text.setOrigin(text.getLocalBounds().getCenter()); text.setPosition({ 320, 240 });
  window.draw(text);
  window.display();
  // sf::sleep(sf::seconds(1)); // pretend to be loading smth
  // we've got smth to load! congrats
}
bool pre_initialize_render_error(bool fatal, std::string error) {
  window.clear(sf::Color::Black);
  sf::Text text(font, "Loading config", 48); text.setFillColor(sf::Color::White);
  text.setOrigin(text.getLocalBounds().getCenter()); text.setPosition({ 320, 240 });
  window.draw(text);
  text = sf::Text(font, error + (fatal ? "\nPress any key or close the window to terminate" : "\nPress any key to continue, Close the window to terminate"), 14); text.setFillColor(fatal ? sf::Color::Red : sf::Color::Yellow); text.setStyle(sf::Text::Bold);
  text.setOrigin({ text.getLocalBounds().getCenter().x, 0 }); text.setPosition({ 320, 280 });
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
}

void initialize() {
  // window.setFramerateLimit(60);
}
void render_temp(game &g) {
  std::ostringstream ss;
  g.print(ss);
  sf::Text text(font, ss.str(), 12); text.setFillColor(sf::Color::White);
  text.setPosition({ 0, 0 });
  window.draw(text);
}
}
