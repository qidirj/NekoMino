/*
ui > base

implements: Object, Interactive, Container (todo)
*/

#include "ui.h"

namespace ui {
  void Object::clear() {} // default: do nothing (nothing can do)
  sf::RenderTexture Object::render() {
    sf::RenderTexture result({ w, h });
    result.clear(cbg);

    sf::RectangleShape brd({ float(w), float(h) });
    brd.setOutlineColor(cbrd);
    brd.setOutlineThickness(-2);
    brd.setPosition({ 0, 0 });
    result.draw(brd);

    result.display();
    return result;
  }

  void Interactive::clear() {}
  sf::RenderTexture Interactive::render() {
    sf::RenderTexture result({ w, h });
    result.clear(cbg);

    sf::RectangleShape brd({ float(w), float(h) });
    brd.setOutlineColor(cbrd);
    brd.setOutlineThickness(-2);
    if (on_focus) brd.setOutlineThickness(-4);
    brd.setPosition({ 0, 0 });
    result.draw(brd);

    result.display();
    return result;
  }
}
