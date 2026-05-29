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
  float x, y;
  float w, h;
  bool fixed; // if fixed, its (x, y) is not affected by its container
  sf::Color cfg = sf::Color::White, cbrd = sf::Color::White, cbg = sf::Color::Black;
  virtual void clear();
  virtual sf::RenderTexture render();
};
class Interactive : public Object {
public:
  void clear() override;
  sf::RenderTexture render() override;
};
class Container : public Interactive {
public:
  float zoom;
  std::vector<std::unique_ptr<Object>> obj;
  void clear() override;
  sf::RenderTexture render() override;
};

}

#endif
