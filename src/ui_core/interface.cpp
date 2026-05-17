#include "../constants.h"
#include "interface.h"

namespace ui {
sf::RenderWindow window;
void pre_initialize() {
  window = sf::RenderWindow(sf::VideoMode({ 640, 480 }), "NekoMino " GAME_VER_STRING);
}
void initialize() {

}
}
