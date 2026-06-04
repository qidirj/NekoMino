#include <boost/locale.hpp>
#include <boost/locale/boundary/types.hpp>
#include <boost/locale/generator.hpp>
#include <iostream>

int main() {
  boost::locale::generator gen;     // 1. 创建生成器
  std::locale loc = gen("");        // 2. 生成 locale 对象
  std::locale::global(loc);         // 3. 将 locale 设置为全局
  
  using namespace boost::locale::boundary;
  std::string text = "Hello 世界, 今日は!";
  std::getline(std::cin, text);

  // 使用 "word" 边界分析
  // boost::locale::boundary::ssegment_index map(word, text.begin(), text.end());
  boost::locale::boundary::ssegment_index map(boundary_type::line, text.begin(), text.end());
  // 迭代并打印所有单词片段
  for (const auto &token : map) {
    std::cout << "[" << token << "] ";
  }
  return 0;
}
