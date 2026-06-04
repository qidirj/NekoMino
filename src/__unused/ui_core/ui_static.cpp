/*
ui > static

implements: Label
requires: namespace richtext
*/

#include "ui.h"
#include "ui_richtext.h"

namespace ui {
  sf::RenderTexture SingleLineLabel::render() {
    if (style.font == nullptr) style.font = &default_font;
    sf::RenderTexture result({ w, h });
    result.clear(cbg);

    sf::RectangleShape brd({ float(w), float(h) });
    brd.setFillColor(sf::Color::Transparent);
    brd.setOutlineColor(cbrd);
    brd.setOutlineThickness(-2);
    brd.setPosition({ 0, 0 });
    result.draw(brd);

    auto tx = ui::richtext::render_singleline(text, style, w - 4, h - 4).getTexture();
    sf::Sprite sprite(tx);
    sprite.setPosition({ 2, 2 });
    result.draw(sprite);
    // todo: max_width = w - 2brd - 2padding & truncate to (w - ..., h - ...)

    result.display();
    return result;
  }
  sf::RenderTexture Label::render() {
    if (style.font == nullptr) style.font = &default_font;
    sf::RenderTexture result({ w, h });
    result.clear(cbg);

    sf::RectangleShape brd({ float(w), float(h) });
    brd.setFillColor(sf::Color::Transparent);
    brd.setOutlineColor(cbrd);
    brd.setOutlineThickness(-2);
    brd.setPosition({ 0, 0 });
    result.draw(brd);

    auto tx = ui::richtext::render(text, style, w - 4, h - 4, linespace, dynamic_linespace).getTexture();
    sf::Sprite sprite(tx);
    sprite.setPosition({ 2, 2 });
    result.draw(sprite);
    // todo: max_width = w - 2brd - 2padding & truncate to (w - ..., h - ...)

    result.display();
    return result;
  }
}
