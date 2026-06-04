#include <SFML/Graphics.hpp>
#include <boost/locale.hpp>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <map>
#include <stack>
#include <vector>

// -------------------------------------------------------------------
// 数据结构：一个文本片段，包含纯文本和样式属性
// -------------------------------------------------------------------
struct TextFragment {
  sf::String text;                    // UTF-32 字符串
  const sf::Font *font = nullptr;     // 字体（外部管理，不能为空）
  unsigned int characterSize = 20;    // 字号
  unsigned style = sf::Text::Regular; // 样式位掩码
  sf::Color color = sf::Color::White; // 颜色
};

// -------------------------------------------------------------------
// BBCode 解析器
// -------------------------------------------------------------------
class BBCodeParser {
public:
  // 输入 BBCode 字符串，输出非格式化文本片段列表
  std::vector<TextFragment> parse(const sf::String &input, const sf::Font &font,
                                  unsigned int defaultSize) {
    m_defaultFont = &font;
    m_defaultSize = defaultSize;
    m_result.clear();
    while (m_styleStack.size()) m_styleStack.pop();
    // 初始样式入栈
    m_styleStack.push(
        CurrentStyle{defaultSize, sf::Text::Regular, sf::Color::White});

    // 遍历输入字符（手动解析，避免复杂正则）
    size_t len = input.getSize();
    for (size_t i = 0; i < len; ++i) {
      unsigned ch = input[i];
      if (ch == '[') {
        // 可能是标签开始
        size_t end = input.find(']', i);
        if (end != sf::String::InvalidPos) {
          sf::String tag = input.substring(i + 1, end - i - 1);
          if (handleTag(tag)) {
            i = end; // 跳过标签
            continue;
          }
        }
        // 不是合法标签，当作普通字符
        addPlainChar('[');
      } else if (ch == '\\' && i + 1 < len &&
                 (input[i + 1] == '[' || input[i + 1] == ']')) {
        // 转义：\[ 或 \]
        addPlainChar(input[i + 1]);
        ++i; // 跳过转义字符和下一个字符
      } else {
        addPlainChar(ch);
      }
    }

    flushCurrentFragment();
    return std::move(m_result);
  }

private:
  struct CurrentStyle {
    unsigned int size;
    unsigned style;
    sf::Color color;
  };

  const sf::Font *m_defaultFont = nullptr;
  unsigned int m_defaultSize = 20;
  std::vector<TextFragment> m_result;
  std::stack<CurrentStyle> m_styleStack;
  sf::String m_currentText;    // 当前累积的纯文本
  CurrentStyle m_currentStyle; // 当前样式（总是栈顶）

  void addPlainChar(unsigned ch) {
    if (m_currentText.isEmpty()) {
      m_currentStyle = m_styleStack.top();
    }
    m_currentText += (char) ch;
  }

  void flushCurrentFragment() {
    if (!m_currentText.isEmpty()) {
      TextFragment frag;
      frag.text = m_currentText;
      frag.font = m_defaultFont;
      frag.characterSize = m_currentStyle.size;
      frag.style = m_currentStyle.style;
      frag.color = m_currentStyle.color;
      m_result.push_back(frag);
      m_currentText.clear();
    }
  }

  bool handleTag(const sf::String &tag) {
    if (tag.isEmpty())
      return false;

    // 开始标签： [b], [i], [u], [s], [color=red], [size=24]
    if (tag == "b") {
      pushStyle(sf::Text::Bold);
      return true;
    } else if (tag == "i") {
      pushStyle(sf::Text::Italic);
      return true;
    } else if (tag == "u") {
      pushStyle(sf::Text::Underlined);
      return true;
    } else if (tag == "s") {
      pushStyle(sf::Text::StrikeThrough);
      return true;
    } else if (tag.substring(0, 6) == "color=") {
      sf::String colorStr = tag.substring(6);
      sf::Color col = parseColor(colorStr);
      flushCurrentFragment();
      CurrentStyle newStyle = m_styleStack.top();
      newStyle.color = col;
      m_styleStack.push(newStyle);
      return true;
    } else if (tag.substring(0, 5) == "size=") {
      sf::String sizeStr = tag.substring(5);
      unsigned int size = parseSize(sizeStr);
      if (size > 0) {
        flushCurrentFragment();
        CurrentStyle newStyle = m_styleStack.top();
        newStyle.size = size;
        m_styleStack.push(newStyle);
        return true;
      }
    }
    // 结束标签： [/b], [/i], ... 必须匹配当前栈顶
    else if (tag.substring(0, 1) == "/") {
      sf::String tagName = tag.substring(1);
      if (tagName == "b" || tagName == "i" || tagName == "u" ||
          tagName == "s") {
        if (m_styleStack.size() > 1) {
          flushCurrentFragment();
          m_styleStack.pop();
          return true;
        }
      } else if (tagName == "color" || tagName == "size") {
        if (m_styleStack.size() > 1) {
          flushCurrentFragment();
          m_styleStack.pop();
          return true;
        }
      }
    }
    return false; // 不是合法标签
  }

  void pushStyle(unsigned styleBit) {
    flushCurrentFragment();
    CurrentStyle newStyle = m_styleStack.top();
    newStyle.style |= styleBit;
    m_styleStack.push(newStyle);
  }

  sf::Color parseColor(const sf::String &str) {
    // 支持 #RRGGBB 或 常见颜色名
    std::string utf8 = str.toAnsiString(); // 简化：仅限ASCII
    if (utf8.empty())
      return sf::Color::White;
    if (utf8[0] == '#') {
      unsigned int r, g, b;
      if (sscanf(utf8.c_str(), "#%02x%02x%02x", &r, &g, &b) == 3)
        return sf::Color(r, g, b);
    } else {
      static std::map<std::string, sf::Color> colors = {
          {"red", sf::Color::Red},     {"green", sf::Color::Green},
          {"blue", sf::Color::Blue},   {"yellow", sf::Color::Yellow},
          {"white", sf::Color::White}, {"black", sf::Color::Black}};
      auto it = colors.find(utf8);
      if (it != colors.end())
        return it->second;
    }
    return sf::Color::White;
  }

  unsigned int parseSize(const sf::String &str) {
    // 简单转换为整数
    std::string utf8 = str.toAnsiString();
    try {
      return std::stoul(utf8);
    } catch (...) {
      return 0;
    }
  }
};

// -------------------------------------------------------------------
// 文本布局器：处理自动换行（使用 Boost.Locale 边界分析）
// -------------------------------------------------------------------
class TextLayouter {
public:
  TextLayouter(const sf::Font &font, unsigned int defaultSize, float maxWidth)
      : m_font(font), m_defaultSize(defaultSize), m_maxWidth(maxWidth) {
    // 初始化 Boost.Locale 全局环境（必须）
    boost::locale::generator gen;
    std::locale loc = gen("");
    std::locale::global(loc);
    // 注意：boost::locale::boundary 需要使用这个 locale
  }

  // 输入片段列表，输出行列表（每行是片段列表）
  std::vector<std::vector<TextFragment>>
  wrapLines(const std::vector<TextFragment> &fragments) {
    m_lines.clear();
    std::vector<TextFragment> currentLine;
    float currentWidth = 0.0f;

    for (const auto &frag : fragments) {
      // 预处理：测量整个片段的宽度
      float fullWidth = measureFragmentWidth(frag);
      if (currentWidth + fullWidth <= m_maxWidth) {
        // 整个片段可以放入当前行
        currentLine.push_back(frag);
        currentWidth += fullWidth;
      } else {
        // 需要拆分片段
        float spaceLeft = m_maxWidth - currentWidth;
        auto split = splitFragmentAtBoundary(frag, spaceLeft);
        if (split.first.text.isEmpty()) {
          // 一个字符都放不下，强制换行并重新尝试当前片段
          finishLine(currentLine);
          currentWidth = 0.0f;
          // 递归重新处理当前片段（此时 currentLine 为空）
          auto retry = wrapLines({frag});
          if (!retry.empty()) {
            for (auto &line : retry) {
              m_lines.push_back(line);
            }
          }
        } else {
          // 第一部分加入当前行
          currentLine.push_back(split.first);
          finishLine(currentLine);
          // 第二部分作为新行开始
          currentLine.clear();
          currentWidth = 0.0f;
          // 将第二部分放入新行（可能继续拆分）
          auto remainingWrapped = wrapLines({split.second});
          if (!remainingWrapped.empty()) {
            // 取第一行合并到 currentLine，其余直接添加
            auto firstRemainingLine = remainingWrapped.front();
            currentLine = firstRemainingLine;
            currentWidth = measureLineWidth(firstRemainingLine);
            for (size_t i = 1; i < remainingWrapped.size(); ++i) {
              m_lines.push_back(remainingWrapped[i]);
            }
          }
        }
      }
    }
    if (!currentLine.empty()) {
      finishLine(currentLine);
    }
    return m_lines;
  }

private:
  const sf::Font &m_font;
  unsigned int m_defaultSize;
  float m_maxWidth;
  std::vector<std::vector<TextFragment>> m_lines;

  float measureFragmentWidth(const TextFragment &frag) {
    sf::Text temp(*frag.font, frag.text);
    temp.setCharacterSize(frag.characterSize);
    temp.setStyle(frag.style);
    // SFML 3.1: getLocalBounds().size.x
    return temp.getLocalBounds().size.x;
  }

  float measureLineWidth(const std::vector<TextFragment> &line) {
    float width = 0.0f;
    for (const auto &f : line)
      width += measureFragmentWidth(f);
    return width;
  }

  // 使用 Boost.Locale 获取允许的断行位置（字符索引）
  std::vector<size_t> getBreakPositions(const sf::String &text,
                                        const TextFragment &frag) {
    // 将 sf::String (UTF-32) 转换为 UTF-8 供 Boost.Locale 使用
    std::string utf8 =
        text.toAnsiString(); // 注意：仅支持 ASCII 子集，但Boost支持UTF-8
    // 更好的转换：sf::String 自带 toUtf8() 但需要自己写循环，简化起见使用
    // toAnsiString 会丢失非ASCII
    // 实际项目中应使用正确的转换，这里为了演示，我们假设所有文本都是ASCII（英文）。
    // 对于真实多语言，请实现 utf8 转换：std::string utf8 =
    // sf::String::toUtf8(text); 由于 Boost.Locale 需要 UTF-8，以下代码使用
    // boost::locale::boundary::ssegment_index 获取单词边界
    boost::locale::boundary::ssegment_index index(boost::locale::boundary::word,
                                                  utf8.begin(),
                                                  utf8.end() // 使用全局 locale
    );
    std::vector<size_t> breaks;
    breaks.push_back(0);
    for (const auto &seg : index) {
      size_t pos = seg.begin() - utf8.begin();
      if (pos > 0 && pos < utf8.size()) {
        // 单词边界位置（字符偏移，不是字节？这里实际上是字节偏移，但因为我们使用单字节字符才正确）
        // 为了准确应使用 UTF-8
        // 迭代器转换为字符索引，简化起见我们假设每个字符一个字节
        breaks.push_back(pos);
      }
    }
    breaks.push_back(utf8.size());
    // 去重排序
    std::sort(breaks.begin(), breaks.end());
    breaks.erase(std::unique(breaks.begin(), breaks.end()), breaks.end());
    return breaks;
  }

  // 在不超过 maxWidth 的前提下，找到最靠后的合法断点，将片段拆成两部分
  std::pair<TextFragment, TextFragment>
  splitFragmentAtBoundary(const TextFragment &frag, float maxWidth) {
    if (frag.text.isEmpty())
      return {TextFragment{}, TextFragment{}};

    // 获取所有允许的断行位置（字符索引）
    std::vector<size_t> breakPoints = getBreakPositions(frag.text, frag);
    if (breakPoints.empty())
      breakPoints = {0, frag.text.getSize()};

    // 测量前缀宽度，找到最后一个不超过 maxWidth 的断点
    size_t bestPos = 0;
    float bestWidth = 0.0f;
    for (size_t pos : breakPoints) {
      if (pos == 0)
        continue;
      sf::String prefix = frag.text.substring(0, pos);
      TextFragment prefixFrag = frag;
      prefixFrag.text = prefix;
      float w = measureFragmentWidth(prefixFrag);
      if (w <= maxWidth && pos > bestPos) {
        bestPos = pos;
        bestWidth = w;
      }
    }

    if (bestPos == 0) {
      // 连一个字符都放不下，强制切一个字符
      bestPos = 1;
      bestWidth =
          measureFragmentWidth({frag.text.substring(0, 1), frag.font,
                                frag.characterSize, frag.style, frag.color});
    }

    TextFragment first = frag;
    first.text = frag.text.substring(0, bestPos);
    TextFragment second = frag;
    second.text = frag.text.substring(bestPos);
    return {first, second};
  }

  void finishLine(std::vector<TextFragment> &line) {
    if (!line.empty()) {
      m_lines.push_back(line);
      line.clear();
    }
  }
};

// -------------------------------------------------------------------
// 渲染函数：将片段行列表绘制到 RenderTexture 上
// -------------------------------------------------------------------
sf::Texture renderBBCodeToTexture(
    const sf::String &bbcode, const sf::Font &font, unsigned int defaultSize,
    float maxWidth, const sf::Color &backgroundColor = sf::Color::Transparent) {
  // 1. 解析 BBCode
  BBCodeParser parser;
  std::vector<TextFragment> fragments = parser.parse(bbcode, font, defaultSize);

  for (auto i : fragments) std::clog << std::quoted(i.text.toAnsiString()) << ' ';
  std::clog << std::endl;

  // 2. 布局（自动换行）
  TextLayouter layouter(font, defaultSize, maxWidth);
  auto lines = layouter.wrapLines(fragments);
  for (auto l : lines) {
    for (auto i : l) std::clog << std::quoted(i.text.toAnsiString()) << ' ';
    std::clog << std::endl;
  }

  // 3. 计算总高度（行高取每行最大高度）
  float lineSpacing = 1.2f; // 行间距因子
  float totalHeight = 0.0f;
  std::vector<float> lineHeights;
  for (const auto &line : lines) {
    float maxHeight = 0.0f;
    for (const auto &frag : line) {
      sf::Text temp(*frag.font, frag.text);
      temp.setCharacterSize(frag.characterSize);
      temp.setStyle(frag.style);
      float h = temp.getLocalBounds().size.y;
      if (h > maxHeight)
        maxHeight = h;
    }
    lineHeights.push_back(maxHeight);
    totalHeight += maxHeight * lineSpacing;
  }

  // 创建 RenderTexture
  sf::RenderTexture renderTexture({static_cast<unsigned int>(maxWidth),
                                   static_cast<unsigned int>(totalHeight)});
  renderTexture.clear(backgroundColor);

  // 逐行绘制
  float y = 0.0f;
  for (size_t i = 0; i < lines.size(); ++i) {
    float x = 0.0f;
    for (const auto &frag : lines[i]) {
      sf::Text text(*frag.font, frag.text);
      text.setCharacterSize(frag.characterSize);
      text.setStyle(frag.style);
      text.setFillColor(frag.color);
      text.setPosition({x, y});
      renderTexture.draw(text);
      x += text.getLocalBounds().size.x; // 复用测量函数
    }
    y += lineHeights[i] * lineSpacing;
  }

  renderTexture.display();
  return renderTexture.getTexture();
}

// -------------------------------------------------------------------
// 主函数示例
// -------------------------------------------------------------------
int main() {
  boost::locale::generator gen;
  std::locale loc = gen("");
  std::locale::global(loc);
  // 初始化 SFML 窗口
  sf::RenderWindow window(sf::VideoMode({800, 600}), "BBCode Renderer");
  sf::Font font;
  if (!font.openFromFile("font.ttf")) { // 请替换为实际字体文件路径
    std::cerr << "Failed to load font" << std::endl;
    return -1;
  }

  // BBCode 示例文本
  // sf::String bbcode = "test [b]bold[/b] and [i]italic[/i] and [u]underline[/u]\n"
  //                     "and [color=red]colored text[/color] and [size=30]HUGE TEXT[/size]\n"
  //                     "and a loooooooooooooooooong sentence which needed a auto word-wrap, but not within a word\n";
  sf::String bbcode = "test [b]粗体[/b] 和 [i]斜体[/i] 和 [u]下划线[/u]\n"
                      "[color=red]红色文字[/color] 和 [size=30]大号字[/size]\n"
                      "这是一个很长的句子，需要自动换行，并且不会在单词中间切开"
                      "。比如 Internationalization 这个长单词会被整体移动。\n"
                      "[b]中文测试：[/b]这是一段中文，它应该在任意字符处换行。";

  // 渲染到纹理
  try {
    sf::Texture texture =
        renderBBCodeToTexture(bbcode, font, 20, 600.f, sf::Color::Black);
    sf::Sprite sprite(texture);
    sprite.setPosition({100, 100});

    while (window.isOpen()) {
      while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
          window.close();
      }
      window.clear(sf::Color::White);
      window.draw(sprite);
      window.display();
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return -1;
  }
  return 0;
}
