/*
ui > richtext (header only)

implements: Color, Richtext
*/

#ifndef UI_RICHTEXT
#define UI_RICHTEXT

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Text.hpp>
#include <iostream>
#include <string>
#include <SFML/Graphics.hpp>
#include <boost/locale.hpp>
#include <boost/locale/boundary/types.hpp>
#include <boost/locale/generator.hpp>
// #include "../util/error_handling.h"
#ifndef UTIL_H
#include "util.h"
#endif

namespace ui {

inline sf::String to_sf(std::string s) {
  return sf::String::fromUtf8(s.begin(), s.end());
}

namespace color {
  inline int _get_hex(char ch) {
    return '0' <= ch && ch <= '9' ? ch ^ 48 : 'a' <= ch && ch <= 'f' ? 10 + (ch - 97) : 'A' <= ch && ch <= 'F' ? 10 + (ch - 65) : 0;
  }
  inline sf::Color from_hex_string(std::string color) {
    if (color.length() == 0) return sf::Color(0, 0, 0, 255);
    else if (color.length() == 1) {
      int x = _get_hex(color[0]) * 17;
      return sf::Color(x, x, x, 255);
    } else if (color.length() == 2) {
      int x = _get_hex(color[0]) * 17, y = _get_hex(color[1]) * 17;
      return sf::Color(x, x, x, y);
    } else if (color.length() == 3) {
      int r = _get_hex(color[0]) * 17, g = _get_hex(color[1]) * 17, b = _get_hex(color[2]) * 17;
      return sf::Color(r, g, b, 255);
    } else if (color.length() <= 5) {
      int r = _get_hex(color[0]) * 17, g = _get_hex(color[1]) * 17, b = _get_hex(color[2]) * 17, a = _get_hex(color[3]) * 16 + _get_hex(color.back());
      return sf::Color(r, g, b, a);
    } else if (color.length() == 6) {
      int r = _get_hex(color[0]) * 16 + _get_hex(color[1]);
      int g = _get_hex(color[2]) * 16 + _get_hex(color[3]);
      int b = _get_hex(color[4]) * 16 + _get_hex(color[5]);
      return sf::Color(r, g, b, 255);
    } else {
      int r = _get_hex(color[0]) * 16 + _get_hex(color[1]);
      int g = _get_hex(color[2]) * 16 + _get_hex(color[3]);
      int b = _get_hex(color[4]) * 16 + _get_hex(color[5]);
      int a = _get_hex(color[6]) * 16 + _get_hex(color.size() <= 8 ? color.back() : color[7]);
      return sf::Color(r, g, b, a);
    }
  }
  inline sf::Color from_string(std::string color) {
    if (color.substr(0, 1) == "#") return from_hex_string(color.substr(1));
    else if (color.substr(0, 2) == "0x") return from_hex_string(color.substr(2));
    else return sf::Color(0, 0, 0, 255);
  }
}

namespace richtext {

inline void initialize() {
  boost::locale::generator gen;
  std::locale loc = gen("");
  std::locale::global(loc);
}

struct Style {
  sf::Font *font = nullptr;
  unsigned style = sf::Text::Regular;
  sf::Color color = sf::Color::Black;
  unsigned font_size = 18;
};
struct StyledString {
  std::string content;
  Style style;

  auto bounds() {
    sf::Text text(*style.font, to_sf(content));
    text.setStyle(style.style);
    text.setCharacterSize(style.font_size);
    return text.getLocalBounds();
  }
  sf::Vector2f size() { return bounds().size; }
};
inline std::vector<StyledString> parse_bbcode(std::string input, const Style &default_style) {
  std::vector<std::pair<std::string, Style>> stylestack = { { "root", default_style } };
  std::vector<StyledString> result;
  std::string current_content;

  auto handle_tag = [&](std::string tag) -> bool {
    if (tag.empty()) return false;
    if (tag.front() == '/') { // ending
      tag = tag.substr(1);
      if (tag == "root" || stylestack.back().first != tag) return false; // root cannot be popped
      if (!current_content.empty()) result.push_back({ current_content, stylestack.back().second });
      current_content.clear();
      stylestack.pop_back();
      return true;
    } else {
      auto style = stylestack.back().second;
      if (tag == "b") style.style |= sf::Text::Bold;
      else if (tag == "i") style.style |= sf::Text::Italic;
      else if (tag == "u") style.style |= sf::Text::Underlined;
      else if (tag == "s") style.style |= sf::Text::StrikeThrough;
      else if (tag.substr(0, 6) == "color=") style.color = color::from_string(tag.substr(6)), tag = tag.substr(0, 5);
      else if (tag.substr(0, 5) == "size=") {
        auto sizetext = tag.substr(5);
        if (sizetext.empty()) return false;
        if (sizetext.back() == 'x') {
          sizetext.pop_back();
          if (!is_float(sizetext)) return false;
          style.font_size = std::round(stylestack.back().second.font_size * std::stof(sizetext));
        } else {
          if (!is_integer(sizetext)) return false;
          style.font_size = std::stoi(sizetext);
        }
        tag = tag.substr(0, 4);
      } else return false; // todo: [font=FontName] support
      if (!current_content.empty()) result.push_back({ current_content, stylestack.back().second });
      current_content.clear();
      stylestack.push_back({ tag, style });
      return true;
    }
  };

  std::size_t len = input.size();
  for (std::size_t i = 0; i < len; ++i) {
    auto ch = input[i];
    if (ch == '[') {
      auto ed = input.find(']', i);
      if (ed != input.npos) {
        auto tag = input.substr(i + 1, ed - i - 1);
        if (handle_tag(tag)) {
          i = ed;
          continue;
        }
      }
      current_content.push_back(ch);
    } else if (ch == '\\' && i + 1 < len) {
      current_content.push_back(input[i + 1]);
      ++i;
    } else current_content.push_back(ch);
  }
  if (!current_content.empty()) result.push_back({ current_content, stylestack.back().second });
  return result;
}

inline std::vector<StyledString> split_word(const std::vector<StyledString> &input) {
  std::vector<StyledString> result;
  for (auto input_seg : input) {
    boost::locale::boundary::ssegment_index map(boost::locale::boundary::boundary_type::line, input_seg.content.begin(), input_seg.content.end());
    for (const auto &token : map) {
      result.push_back({ token, input_seg.style });
    }
  }
  return result;
}

inline std::vector<std::vector<StyledString>> wrap_line(const std::vector<StyledString> &input, float max_width) {
  std::vector<std::vector<StyledString>> result;
  std::vector<StyledString> current_line; float current_width = 0;
  for (auto input_seg : input) {
    auto without_spc = input_seg; inplace_trim(without_spc.content);
    float seg_width_min = without_spc.size().x, seg_width_max = input_seg.size().x;
    if (current_width + seg_width_min > max_width) { // cannot put this seg
      result.push_back(current_line), current_line.clear(); current_width = 0;
    }
    int new_line_count = 0;
    while (!input_seg.content.empty() && input_seg.content.back() == '\n') {
      input_seg.content.pop_back(); ++new_line_count;
    }
    current_line.push_back(input_seg); current_width += seg_width_max;
    while (new_line_count--) {
      result.push_back(current_line), current_line.clear(); current_width = 0;
      current_line.push_back({ "", input_seg.style });
    }
    #ifdef ERROR_HANDLING_H
    if (seg_width_min > max_width) logger::interface.warn("fuck up why did someone put a such loooooooooong word here?");
    #endif
  }
  result.push_back(current_line);
  return result;
}

// returns max height
inline float render_line(sf::RenderTexture &rt, int y0, const std::vector<StyledString> &line) {
  float x0 = 0; unsigned max_sz = 0;
  for (auto seg : line) max_sz = std::max(max_sz, seg.style.font_size);
  for (auto seg : line) {
    sf::Text text(*seg.style.font, to_sf(seg.content));
    text.setStyle(seg.style.style);
    text.setFillColor(seg.style.color);
    text.setCharacterSize(seg.style.font_size);
    text.setPosition({ x0, (float) std::round(y0 + (max_sz - seg.style.font_size)) });
    rt.draw(text);
    x0 += text.getLocalBounds().size.x;
  }
  return max_sz;
}
inline void render_lines(sf::RenderTexture &rt, const std::vector<std::vector<StyledString>> &lines, float linespace = 0, float dynamic_linespace = 0) {
  float y0 = 0;
  for (const auto &line : lines) {
    y0 += render_line(rt, y0, line) * (1 + dynamic_linespace) + linespace;
  }
  return;
}

inline sf::RenderTexture render(std::string text, const Style &style, unsigned max_x, unsigned max_y, float linespace = 0, float dynamic_linespace = 0) {
  sf::RenderTexture rt({ max_x, max_y });
  render_lines(rt, wrap_line(split_word(parse_bbcode(text, style)), max_x), linespace, dynamic_linespace);
  rt.display();
  return rt;
}

}
}

#endif
