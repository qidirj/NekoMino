#ifndef UTIL_H
#define UTIL_H

#include <chrono>
#include <ctime>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

inline std::string inplace_trim(std::string &s) {
  size_t bg = 0;
  while (bg < s.size()) {
    if (s[bg] != ' ') break;
    ++bg;
  }
  s = s.substr(bg);
  while (!s.empty() && s.back() == ' ') s.pop_back();
  return s;
}
inline std::string trim(const std::string &s) {
  static std::string t;
  t = s; inplace_trim(t);
  return t;
}

inline std::vector<std::string> split(std::string s, std::string delim=" ") {
  std::vector<std::string> res;
  std::size_t pos;
  while ((pos = s.find(delim)) != s.npos) {
    res.push_back(s.substr(0, pos));
    s = s.substr(pos + delim.size());
  }
  res.push_back(s);
  return res;
}

template<bool strict = false>
inline bool is_integer(const std::string &s) {
  if (s.empty()) return false;
  if (s[0] == '-' || (s[0] == '+' && !strict)) {
    if (s.size() == 1) return false;
    return is_integer<strict>(s.substr(1));
  }
  if constexpr (strict) if (s.length() > 1 && s[0] == '0') return false;
  for (char c : s) {
    if (c < '0' || c > '9') return false;
  }
  return true;
}
template<bool strict = false>
inline bool is_float(const std::string &s) {
  if (s.empty()) return false;
  if (s[0] == '-' || s[0] == '+') {
    if (s.size() == 1) return false;
    return is_float<strict>(s.substr(1));
  }
  if constexpr (strict) if (s.length() > 1 && s[0] == '0' && s[1] != '.') return false;
  bool has_dot = false;
  for (std::size_t i = 0; i < s.length(); ++i) {
    char c = s[i];
    if (c < '0' || c > '9') {
      if (c == '.' && !has_dot) has_dot = true;
      else if (c == 'e') return is_integer<false>(s.substr(i + 1)); // e+0xx is always allowed
      else return false;
    }
  }
  return true;
}

inline std::string time_to_string(std::time_t t) {
  std::tm tm = *localtime(&t);
  static char buf[64];
  std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
  return buf;
}

template<class _Tp, class _Variant>
struct is_variant_member;

template<class _Tp, class... Ts>
struct is_variant_member<_Tp, std::variant<Ts...>>
    : std::disjunction<std::is_same<_Tp, Ts>...> {};

template<bool delta = false>
class Timer {};

template<>
class Timer<false> {
public:
  decltype(std::chrono::system_clock::now()) t0;
  Timer() { t0 = std::chrono::system_clock::now(); }
  ~Timer() { std::clog << "final, ", print(); }
  double operator()() { return (std::chrono::system_clock::now() - t0).count() * 1e-9;}
  void print() { std::clog << operator()() << std::endl; }
  void print(std::string _) { std::clog << _ << ": " << operator()() << std::endl; }
};

template<>
class Timer<true> {
public:
  decltype(std::chrono::system_clock::now()) t0, tmp;
  Timer() { t0 = std::chrono::system_clock::now(); }
  double operator()() { tmp = std::chrono::system_clock::now(); auto _ = (tmp - t0).count() * 1e-9; t0 = tmp; return _; }
  void print() { std::clog << operator()() << std::endl; }
  void print(std::string _) { std::clog << _ << ": " << operator()() << std::endl; }
};

#endif
