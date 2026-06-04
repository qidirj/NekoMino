#include <iostream>
#include "../ui_core/ui.h"

Timer<true> tmr, tmr_seg;

int main() {
  // sf::String test_string;
  // std::cout << test_string.toAnsiString() << std::endl;
  // std::string str = "测试111";
  // for (auto i : str) std::cout << (int)i << ' ';
  // std::cout << std::endl;
  // sf::String a;
  ui::richtext::initialize();
  assert(ui::default_font.openFromFile("../../res/font.ttf"));

  sf::RenderWindow window(sf::VideoMode({ 640, 480 }), "UI Test");

  tmr(); tmr_seg();
  double dt = 0;
  while (window.isOpen()) {
    std::clog << "Frame start" << std::endl; tmr_seg();
    while (const std::optional event = window.pollEvent()) {
      // "close requested" event: we close the window
      if (event->is<sf::Event::Closed>())
        window.close();
    }
    std::clog << "pollEvent " << tmr_seg() << std::endl;

    // std::clog << "Last frame took " + std::to_string(dt) + " ms" << std::endl;
    auto some_object = std::make_unique<ui::Object>(64, 32, 16, 16);
    auto some_interactive = std::make_unique<ui::Interactive>(128, 48, 24, 24);
    auto some_label = std::make_unique<ui::Label>(128, 64, 32, 128, "Miaowu!\ntest wordwrap meow meow meow meow meow meow", ui::richtext::Style{ .color = sf::Color::White, .font_size = 13 });
    auto timer_label = std::make_unique<ui::SingleLineLabel>(256, 32, 0, 224, "Last frame took " + std::to_string(dt * 1000) + " ms", ui::richtext::Style{ .color = sf::Color::White, .font_size = 13 });
    auto some_container = std::make_unique<ui::Container>(256, 256);
    std::clog << "makeControl " << tmr_seg() << std::endl;

    some_container->add_element(std::move(some_object));
    some_container->add_element(std::move(some_interactive));
    some_container->add_element(std::move(some_label));
    some_container->add_element(std::move(timer_label));
    std::clog << "addElement " << tmr_seg() << std::endl;
    
    window.clear();
    auto tx = some_container->render().getTexture();
    std::clog << "render " << tmr_seg() << std::endl;
    sf::Sprite sprite(tx);
    window.draw(sprite);
    window.display();
    std::clog << "display " << tmr_seg() << std::endl;

    // window.setFramerateLimit(5);
    
    dt = tmr();
  }
  window.close();

  return 0;
}
