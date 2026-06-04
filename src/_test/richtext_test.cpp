#include "../ui_core/ui.h"

#include <SFML/Graphics/RenderTexture.hpp>
#include <iomanip>
#include <iostream>

int main() {
  boost::locale::generator gen;
  std::locale loc = gen("");
  std::locale::global(loc);

  std::string s;
  std::getline(std::cin, s);
  s = s + "\n\n" + s;

  auto font = sf::Font("font.ttf");

  auto res = ui::richtext::split_word(ui::richtext::parse_bbcode(s, ui::richtext::Style {.font = &font}));
  for (auto i : res) {
    auto bd = i.bounds();
    std::cout << std::quoted(i.content) << ": ";
    std::cout << bd.position.x << ' ' << bd.position.y << "; ";
    std::cout << bd.size.x << ' ' << bd.size.y << "\n";
  }
  sf::RenderTexture rt({ 640, 480 });
  ui::richtext::render_line(rt, 0, res);
  rt.display();
  sf::RenderWindow window(sf::VideoMode({ 640, 480 }), "UI Test");
  window.clear();
  auto tx = rt.getTexture();
  sf::Sprite sprite(tx);
  window.draw(sprite);
  window.display();

  while (window.isOpen()) {
    while (const std::optional event = window.pollEvent()) {
      // "close requested" event: we close the window
      if (event->is<sf::Event::Closed>())
        window.close();
    }
  }
  window.close();
  return 0;
}
