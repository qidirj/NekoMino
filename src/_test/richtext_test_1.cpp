#include "../ui_core/ui.h"

#include <SFML/Graphics/RenderTexture.hpp>
#include <iomanip>
#include <iostream>

/*
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.
*/

Timer<true> tmr;

int main() {
  tmr();
  boost::locale::generator gen;
  std::locale loc = gen("");
  std::locale::global(loc);

  std::string s = R"([b]基本信息[/b]：这是一个 [i]多语言[/i] [u]BBCode[/u] 测试文本。包含 [s]废弃信息[/s] 正常信息。

[color=red]红色[/color] 和 [color=#00FF00]绿色[/color] 以及 [size=30]放大字号[/size] 的混合效果。

[b][i]粗斜体嵌套[/i][/b] 与 [b][u]粗体+下划线[/u][/b] 测试。

[color=blue][size=25]大号蓝色中文[/size][/color] 与 [color=purple]紫色英文 [b]bold nested[/b] 混合[/color]。

[size=20]多语言混排：[/size]
[color=brown]English: The quick brown fox jumps over the lazy dog.[/color]
[color=darkgreen]中文：静夜思 床前明月光，疑是地上霜。举头望明月，低头思故乡。[/color]
[color=magenta]日本語：吾輩は猫である。名前はまだ無い。どこで生れたかとんと見当がつかぬ。[/color]
[color=teal]한국어：안녕하세요? 오늘 날씨가 참 좋네요. BBCode 렌더링 테스트 중입니다.[/color]

[color=gray][size=18]数字与符号：0123456789 !@#$%^&*()_+{}[]|\\:;"'<>,.?/~`[/size][/color]

[b][color=red][size=28]多层嵌套测试：红色粗体大号字[/size][/color][/b] 后面跟随普通文本。

行内混排不同大小：正常字 [size=14]小号[/size] 和 [size=28]大号[/size] 以及 [b]粗体[/b] 基线对齐测试。

[color=navy]最后一段，包含手动换行符。
这一行应该是新的行。同时测试长单词断行，例如：Pneumonoultramicroscopicsilicovolcanoconiosis 这个超长单词是否被整体移动到下一行而不是中间切断。以及中文字符的换行规则——中文应该允许在每个字符后换行，但要避免标点符号孤立行首。[/color])";

  auto font = sf::Font("font.otf");
  tmr.print("initialize");

  // for (auto i : res) {
  //   auto bd = i.bounds();
  //   std::cout << std::quoted(i.content) << ": ";
  //   std::cout << bd.position.x << ' ' << bd.position.y << "; ";
  //   std::cout << bd.size.x << ' ' << bd.size.y << "\n";
  // }
  // sf::RenderTexture rt({ 800, 600 });
  // // ui::richtext::render_lines(rt, res, 0, 0.2);
  // tmr.print("make rt");
  // std::clog << s.size() << std::endl;;
  // const auto &step1 = ui::richtext::parse_bbcode(s, ui::richtext::Style {.font = &font});
  // tmr.print("parse_bbcode"); std::clog << step1.size() << std::endl;
  // // const auto &step2 = ui::richtext::split_word(step1);
  // // tmr.print("split_word");
  // auto res = ui::richtext::wrap_line(step1, 800);
  // tmr.print("wrap_line");
  // ui::richtext::render_lines(rt, res, 0, 0.2);
  // tmr.print("render_lines");
  // rt.display();
  // tmr.print("display");
  sf::RenderWindow window(sf::VideoMode({ 800, 700 }), "UI Test");
  window.clear(sf::Color::White);
  tmr.print("init");

  auto rt = ui::richtext::render(s, ui::richtext::Style {.font = &font}, 800, 600, 0, 0.2);
  tmr.print("render full");
  auto tx = rt.getTexture();
  sf::Sprite sprite(tx); sprite.setPosition({ 0, 100 });
  window.draw(sprite);
  tmr.print("draw full");

  rt = ui::richtext::render_singleline("[b]基本信息[/b]：这是一个 [i]多语言[/i] [u]BBCode[/u] 测试文本。包含 [s]废弃信息[/s] 正常信息。", ui::richtext::Style {.font = &font}, 800, 100);
  tmr.print("render singleline");
  tx = rt.getTexture();
  sf::Sprite sprite1(tx);
  window.draw(sprite1);
  tmr.print("draw singleline");

  rt = ui::richtext::render_plain("基本信息：这是一个 多语言 BBCode 测试文本。包含 废弃信息 正常信息。", ui::richtext::Style {.font = &font}, 800, 100);
  tmr.print("render singleline plain");
  tx = rt.getTexture();
  sf::Sprite sprite2(tx); sprite2.setPosition({ 0, 50 });
  window.draw(sprite2);
  tmr.print("draw singleline plain");

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
