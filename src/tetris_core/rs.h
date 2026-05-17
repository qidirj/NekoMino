#ifndef RS_H
#define RS_H

#include <map>
#include <set>
#include <string>
#include <vector>
#include "minoes.h"

namespace rs {
struct kick_table_t {
  std::string from_file, name;
  std::vector<std::pair<int, int>> kt[4][3]; // initial_direction, rotation, test, x/y
};
struct rotation_system_mino_t {
  std::pair<int, int> center; // *2 of original value; need to be the same parity
  int spawn_direction;
  std::vector<kick_table_t*> kick_table;
};
struct rotation_system_t {
  std::string from_file, name;
  std::map<Minoes, rotation_system_mino_t> rs;
};

extern std::map<std::string, kick_table_t> kick_tables;
extern std::map<std::string, rotation_system_t> rotation_systems;
extern std::set<std::string> loaded_file;

extern rotation_system_t *rotsys;

void reset_rs_data();
bool load_from_file(const std::string &filename);

void print(std::ostream &s, const kick_table_t &, bool);
void print(std::ostream &s, const rotation_system_t &, bool);
}
using rs::rotsys;

#endif
