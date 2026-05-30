/*
  every object is a RenderTarget.
  when render, render them recursively

  min-max width&height, flexible
*/

#ifndef UI_H
#define UI_H

#include <SFML/Graphics.hpp>

namespace ui {

using ui_event = std::function<void ()>;

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
  std::vector<std::unique_ptr<Object>> children;
  void add_element(std::unique_ptr<Object> child);
  sf::RenderTexture render() override;
};

class Label : public Object {
public:
  std::string text;
  sf::RenderTexture render() override;
};

}

#endif
