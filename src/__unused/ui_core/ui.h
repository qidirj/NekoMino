/*
  every object is a RenderTarget.
  when render, render them recursively

  min-max width&height, flexible
*/

#ifndef UI_H
#define UI_H

#include <SFML/Graphics.hpp>
#include "ui_richtext.h"

namespace ui {

using ui_event = std::function<void ()>;

extern sf::Font default_font;
void initialize();

class Object {
public:
  Object() {}
  Object(unsigned w, unsigned h) :
    w(w), h(h) {}
  Object(unsigned w, unsigned h, float x, float y) :
    w(w), h(h), x(x), y(y) {}
  virtual ~Object() = default;

  unsigned w = 1, h = 1;
  float x = 0.0, y = 0.0;
  bool fixed = false; // if fixed, its (x, y) is not affected by its container (up to container)
  float opacity = 1.0; // todo: implemention of opacity
  float border = 1.0; // todo: implemention of border
  bool on_hover = false;
  sf::Color cfg = sf::Color::White, cbrd = sf::Color::White, cbg = sf::Color::Black;

  virtual sf::RenderTexture render();
};
class Interactive : public Object {
public:
  using Object::Object;
  bool on_focus = false;
  sf::RenderTexture render() override;
};
class Container : public Interactive {
public:
  using Interactive::Interactive;
  float zoom = 1.0;
  std::vector<std::unique_ptr<Object>> children = {};
  void add_element(std::unique_ptr<Object> child);
  sf::RenderTexture render() override;
};

class SingleLineLabel : public Object {
public:
  std::string text;
  ui::richtext::Style style;
  SingleLineLabel() {}
  SingleLineLabel(unsigned w, unsigned h, std::string text = "", ui::richtext::Style style = ui::richtext::Style{}) :
    Object(w, h), text(text), style(style) {}
  SingleLineLabel(unsigned w, unsigned h, float x, float y, std::string text = "", ui::richtext::Style style = ui::richtext::Style{}) :
    Object(w, h, x, y), text(text), style(style) {}
  sf::RenderTexture render() override;
};

class Label : public Object {
public:
  std::string text;
  ui::richtext::Style style;
  float linespace = 0, dynamic_linespace = 0.35;
  Label() {}
  Label(unsigned w, unsigned h, std::string text = "", ui::richtext::Style style = ui::richtext::Style{}) :
    Object(w, h), text(text), style(style) {}
  Label(unsigned w, unsigned h, float x, float y, std::string text = "", ui::richtext::Style style = ui::richtext::Style{}) :
    Object(w, h, x, y), text(text), style(style) {}
  sf::RenderTexture render() override;
};

extern Container meta;

void tick(double dt, sf::Vector2f mouse);

}

#endif
