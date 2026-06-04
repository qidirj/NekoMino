#include "../ui_core/ui.h"

#include <SFML/Graphics/RenderTexture.hpp>
#include <iomanip>
#include <iostream>

/*
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.
*/

int main() {
  boost::locale::generator gen;
  std::locale loc = gen("");
  std::locale::global(loc);

  std::string s;
  std::getline(std::cin, s);

  auto font = sf::Font("font.otf");

  auto res = ui::richtext::wrap_line(ui::richtext::split_word(ui::richtext::parse_bbcode(s, ui::richtext::Style {.font = &font})), 640);
  // for (auto i : res) {
  //   auto bd = i.bounds();
  //   std::cout << std::quoted(i.content) << ": ";
  //   std::cout << bd.position.x << ' ' << bd.position.y << "; ";
  //   std::cout << bd.size.x << ' ' << bd.size.y << "\n";
  // }
  sf::RenderTexture rt({ 640, 480 });
  ui::richtext::render(rt, res, 0, 0.35);
  rt.display();
  sf::RenderWindow window(sf::VideoMode({ 640, 480 }), "UI Test");
  window.clear(sf::Color::White);
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
