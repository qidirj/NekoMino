#include <iostream>
#include "../ui_core/ui.h"

int main() {
  // sf::String test_string;
  // std::cout << test_string.toAnsiString() << std::endl;
  // std::string str = "测试111";
  // for (auto i : str) std::cout << (int)i << ' ';
  // std::cout << std::endl;
  // sf::String a;
  ui::richtext::initialize();
  assert(ui::default_font.openFromFile("../../res/font.ttf"));

  auto some_object = std::make_unique<ui::Object>(64, 32, 16, 16);
  auto some_interactive = std::make_unique<ui::Interactive>(128, 48, 24, 24);
  auto some_label = std::make_unique<ui::Label>(128, 64, 32, 128, "Miaowu!\ntest wordwrap meow meow meow meow meow meow", ui::richtext::Style{ .color = sf::Color::White, .font_size = 13 });
  auto some_container = std::make_unique<ui::Container>(256, 256);

  some_container->add_element(std::move(some_object));
  some_container->add_element(std::move(some_interactive));
  some_container->add_element(std::move(some_label));

  sf::RenderWindow window(sf::VideoMode({ 640, 480 }), "UI Test");
  window.clear();
  auto tx = some_container->render().getTexture();
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
