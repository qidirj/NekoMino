#ifndef INTERFACE_H
#define INTERFACE_H

#include "../tetris_core/core.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace ui_interface {
extern sf::RenderWindow window;
namespace pre_initialize { // before config and localization being loaded
void prepare(); // before anything can be drawn
void draw_step_text(std::string step); // draw text
bool deal_error(std::string step, bool fatal, std::string error); // draw text when errored
void show_step(std::string step); // clear and redraw
}
void initialize(); // resize and redraw the window based on loaded config.txt

void render_temp(game &g);
}

#endif
