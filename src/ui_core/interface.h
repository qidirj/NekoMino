#ifndef INTERFACE_H
#define INTERFACE_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace ui {
extern sf::RenderWindow window;
void pre_initialize(); // create an initial window
void initialize(); // resize and redraw the window based on loaded config.txt
}

#endif
