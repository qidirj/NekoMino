#include <SFML/Graphics.hpp>

int main() {
  // 1. 创建离屏渲染目标 d (RenderTexture)
  // SFML 3.1 中，create() 方法不再返回 bool，若失败会抛出异常
  sf::RenderTexture renderTexture({400, 300});

  // 2. 在离屏目标 d 上绘制元素 a, b, c
  renderTexture.clear(sf::Color::Blue); // 蓝色背景

  // 圆形 a
  sf::CircleShape circleA(50.f);
  circleA.setFillColor(sf::Color::Green);
  circleA.setPosition({50, 50});
  renderTexture.draw(circleA);

  // 矩形 b
  sf::RectangleShape rectB({100.f, 80.f});
  rectB.setFillColor(sf::Color::Red);
  rectB.setPosition({150, 120});
  renderTexture.draw(rectB);

  // 文本 c（注意字体加载方式，SFML 3.1 仍可用 loadFromFile）
  sf::Font font;
  if (!font.openFromFile("../../res/font.ttf")) {
    return -1;
  }
  sf::Text textC(font, "Hello SFML 3.1!", 30);
  textC.setFillColor(sf::Color::White);
  textC.setPosition({250, 200});
  renderTexture.draw(textC);

  // 必须调用 display() 使绘制生效并更新内部纹理
  renderTexture.display();

  // 3. 创建精灵，持有纹理
  sf::Sprite sprite(renderTexture.getTexture());

  // 创建最终窗口 e
  sf::RenderWindow window(sf::VideoMode({800, 600}), "多级渲染示例");

  // 主循环（SFML 3.1 事件处理新方式）
  while (window.isOpen()) {
    // 使用 std::optional 风格的事件轮询
    while (const auto &event = window.pollEvent()) {
      if (event->is<sf::Event::Closed>())
        window.close();
    }

    window.clear();
    window.draw(sprite);
    // 可以继续绘制其他窗口元素...
    window.display();
  }

  return 0;
}
