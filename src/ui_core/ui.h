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
  bool fixed; // if fixed, its (x, y) is not affected by its container (up to container)
  unsigned w, h;
  float opacity; // todo: implemention of opacity
  float border; // todo: implemention of border
  sf::Color cfg = sf::Color::White, cbrd = sf::Color::White, cbg = sf::Color::Black;
  virtual void clear();
  virtual sf::RenderTexture render();
};
class Interactive : public Object {
public:
  bool on_focus;
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
