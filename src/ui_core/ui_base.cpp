/*
ui > base

implements: Object, Interactive, Container
*/

#include "ui.h"

namespace ui {
  sf::Font default_font;

  sf::RenderTexture Object::render() {
    sf::RenderTexture result({ w, h });
    result.clear(cbg);

    sf::RectangleShape brd({ float(w), float(h) });
    brd.setFillColor(sf::Color::Transparent);
    brd.setOutlineColor(cbrd);
    brd.setOutlineThickness(-2);
    brd.setPosition({ 0, 0 });
    result.draw(brd);

    result.display();
    return result;
  }

  sf::RenderTexture Interactive::render() {
    sf::RenderTexture result({ w, h });
    result.clear(cbg);

    sf::RectangleShape brd({ float(w), float(h) });
    brd.setFillColor(sf::Color::Transparent);
    brd.setOutlineColor(cbrd);
    brd.setOutlineThickness(-2);
    if (on_focus) brd.setOutlineThickness(-4);
    brd.setPosition({ 0, 0 });
    result.draw(brd);

    result.display();
    return result;
  }

  void Container::add_element(std::unique_ptr<Object> child) {
    children.push_back(std::move(child));
  }
  sf::RenderTexture Container::render() {
    sf::RenderTexture result({ w, h });
    result.clear(cbg);

    sf::RectangleShape brd({ float(w), float(h) });
    brd.setFillColor(sf::Color::Transparent);
    brd.setOutlineColor(cbrd);
    brd.setOutlineThickness(-2);
    if (on_focus) brd.setOutlineThickness(-4);
    brd.setPosition({ 0, 0 });
    result.draw(brd);

    for (auto &child : children) {
      auto tx = child->render().getTexture();
      sf::Sprite sprite(tx);
      sprite.setPosition({ child->x * zoom, child->y * zoom });
      sprite.setScale({ zoom, zoom });
      result.draw(sprite);
    }

    result.display();
    return result;
  }
}
