#ifndef MINOES_H
#define MINOES_H

#include <map>
#include <string>
#include <vector>

enum Minoes {
  m0, mG, mBn, // solid, garbage, bone
  mI = 0x10, mJ, mL, mZ, mS, mT, mO, // 7 4-minoes
  mI5 = 0x20, mV5, mT5, mU5, mW5, mX5, mJ5, mL5, mH5, mN5, mY5, mR5, mP5, mQ5, mF5, mE5, mZ5, mS5, // 18 5-minoes
  mO1 = 0x40, mI2, mI3, mC3, // 1+1+2 1~3-minoes
  // 6-minoes todo
};
Minoes get_mino_id(const std::string &name);
std::string get_mino_name(Minoes id);

using mino_shape_t = std::vector<std::vector<int>>;
extern std::map<Minoes, mino_shape_t> mino_shapes;

#endif
