#include "../constants.h"
#include "../error_handling.h"
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
  sf::sleep(sf::seconds(1)); // pretend to be loading smth
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
