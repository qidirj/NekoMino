#include "minoes.h"
Minoes get_mino_id(const std::string &name) {
  if (name == "I") return mI;
  if (name == "J") return mJ;
  if (name == "L") return mL;
  if (name == "Z") return mZ;
  if (name == "S") return mS;
  if (name == "T") return mT;
  if (name == "O") return mO;
  if (name == "I5") return mI5;
  if (name == "V5" || name == "V") return mV5;
  if (name == "T5") return mT5;
  if (name == "U5" || name == "U") return mU5;
  if (name == "W5" || name == "W") return mW5;
  if (name == "X5" || name == "X") return mX5;
  if (name == "J5") return mJ5;
  if (name == "L5") return mL5;
  if (name == "H5" || name == "H" || name == "N'") return mH5;
  if (name == "N5" || name == "N") return mN5;
  if (name == "Y5" || name == "Y") return mY5;
  if (name == "R5" || name == "R" || name == "Y'") return mR5;
  if (name == "P5" || name == "P" || name == "B5" || name == "B") return mP5;
  if (name == "Q5" || name == "Q" || name == "D5" || name == "D") return mQ5;
  if (name == "F5" || name == "F") return mF5;
  if (name == "E5" || name == "E" || name == "K5" || name == "K" || name == "F'") return mE5;
  if (name == "Z5") return mZ5;
  if (name == "S5") return mS5;
  if (name == "O1" || name == "I1" || name == ".") return mO1;
  if (name == "I2" || name == "-") return mI2;
  if (name == "I3" || name == "_") return mI3;
  if (name == "C3" || name == "C") return mC3;
  return m0;
}
std::string get_mino_name(Minoes id) {
  switch (id) {
    case mI: return "I";
    case mJ: return "J";
    case mL: return "L";
    case mZ: return "Z";
    case mS: return "S";
    case mT: return "T";
    case mO: return "O";
    case mI5: return "I5";
    case mV5: return "V5";
    case mT5: return "T5";
    case mU5: return "U5";
    case mW5: return "W5";
    case mX5: return "X5";
    case mJ5: return "J5";
    case mL5: return "L5";
    case mH5: return "H5";
    case mN5: return "N5";
    case mY5: return "Y5";
    case mR5: return "R5";
    case mP5: return "P5";
    case mQ5: return "Q5";
    case mF5: return "F5";
    case mE5: return "E5";
    case mZ5: return "Z5";
    case mS5: return "S5";
    case mO1: return "O1";
    case mI2: return "I2";
    case mI3: return "I3";
    case mC3: return "C3";
    default: return "?";
  }
}

std::map<Minoes, mino_shape_t> mino_shapes = {
  { mI, {
      { 0, 0, 0, 0 },
      { 1, 1, 1, 1 },
      { 0, 0, 0, 0 },
      { 0, 0, 0, 0 }
  } },
  { mJ, {
      { 1, 0, 0 },
      { 1, 1, 1 },
      { 0, 0, 0 }
  } },
  { mL, {
      { 0, 0, 1 },
      { 1, 1, 1 },
      { 0, 0, 0 }
  } },
  { mZ, {
      { 1, 1, 0 },
      { 0, 1, 1 },
      { 0, 0, 0 }
  } },
  { mS, {
      { 0, 1, 1 },
      { 1, 1, 0 },
      { 0, 0, 0 }
  } },
  { mT, {
      { 0, 1, 0 },
      { 1, 1, 1 },
      { 0, 0, 0 }
  } },
  { mO, {
      { 1, 1 },
      { 1, 1 }
  } },
  // todo: 1, 2, 3, 5-minoes
};
