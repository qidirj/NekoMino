#ifndef INTERFACE_H
#define INTERFACE_H

#include "../tetris_core/core.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace ui {
extern sf::RenderWindow window;
void pre_initialize(); // create an initial window
bool pre_initialize_render_error(bool fatal, std::string error); // display some error text before initialized
void initialize(); // resize and redraw the window based on loaded config.txt

void render_temp(game &g);
}

#endif
