#include <iostream>
#include "../ui_core/ui.h"

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

Timer<true> tmr, tmr_seg;

int main() {
  std::string text = ""

  return 0;
}
