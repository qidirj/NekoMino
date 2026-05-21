#include "rs.h"
#include "../util/error_handling.h"
#include "../util/util.h"
#include <iomanip>
#include <iostream>
#include <sstream>

std::vector<std::string> split(const std::string &s, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(s);
  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}
std::string toupper(const std::string &s) {
  std::string res = s;
  for (char &c : res) {
    if (c >= 'a' && c <= 'z') c = c - ('a' - 'A');
  }
  return res;
}

namespace rs {
std::map<std::string, kick_table_t> kick_tables;
std::map<std::string, rotation_system_t> rotation_systems;
std::set<std::string> loaded_file;
std::set<std::string> loading_file;

rotation_system_t *rotsys;

void reset_rs_data() {
  kick_tables.clear();
  rotation_systems.clear();
  loaded_file.clear();
}
bool load_from_file(const std::string &filename) {
  if (loaded_file.count(filename)) return logger::rs_load.warn("File already loaded: " + filename), false;
  if (loading_file.count(filename)) return logger::rs_load.warn("Recursive reference: " + filename), false;
  std::ifstream f(filename);
  if (!f.is_open()) {
    logger::rs_load.warn("Failed to open file: " + filename);
    return false;
  }
  loading_file.insert(filename);
  kick_table_t kick_buffer;
  rotation_system_t rs_buffer;
  int current_state = 0; std::string current_name = "";
  auto save = [&](){
    if (current_state == 0) return;
    if (current_name.empty()) {
      logger::rs_load.warn("Unnamed kick_table in file: " + filename);
      return;
    }
    if (current_state == 1) {
      if (kick_tables.count(current_name)) {
        logger::rs_load.warn("Duplicate kick_table name " + current_name + " in file: " + kick_tables[current_name].from_file + " and " + filename);
      }
      kick_buffer.name = current_name;
      kick_buffer.from_file = filename;
      kick_tables[current_name] = kick_buffer;
    }
    if (current_state == 2) {
      if (rotation_systems.count(current_name)) {
        logger::rs_load.warn("Duplicate rotation_system name " + current_name + " in file: " + rotation_systems[current_name].from_file + " and " + filename);
      }
      rs_buffer.name = current_name;
      rs_buffer.from_file = filename;
      rotation_systems[current_name] = rs_buffer;
    }
  };
  int unnamed_count = 0;
  bool reverse = false;
  while (f.good()) {
    std::string line;
    std::getline(f, line);
    auto tokens = split(line, ' ');
    if (tokens.empty()) continue;
    else if (tokens[0] == "comment") continue;
    else if (tokens[0] == "kick_table") save(), kick_buffer = kick_table_t(), current_state = 1, current_name = tokens.size() >= 2 ? tokens[1] : "";
    else if (tokens[0] == "rotation_system") save(), rs_buffer = rotation_system_t(), current_state = 2, current_name = tokens.size() >= 2 ? tokens[1] : "_" + filename + "_rs" + std::to_string(unnamed_count++);
    else if (tokens[0] == "end") save(), current_state = 0;
    else if (tokens[0] == "require") {
      if (tokens.size() < 2) {
        logger::rs_load.warn("Invalid require entry in file: " + filename);
        continue;
      }
      std::string req_file = tokens[1];
      if (!loaded_file.count(req_file) && !load_from_file(req_file)) {
        logger::rs_load.warn("Failed to load required file: " + req_file + " in file: " + filename);
        continue;
      }
    }
    else if (tokens[0] == "config") {
      if (tokens.size() < 2) {
        logger::rs_load.warn("Invalid config entry in file: " + filename);
        continue;
      }
      auto word = tokens[1];
      if (word == "col_row") reverse = true;
      else if (word == "row_col") reverse = false;
      else logger::rs_load.warn("Unknown config entry " + word + " in file: " + filename);
    }
    else if (tokens[0] == "kick") {
      if (current_state != 1) {
        logger::rs_load.warn("kick entry outside kick_table in file: " + filename);
        continue;
      }
      if (tokens.size() < 2) {
        logger::rs_load.warn("Invalid kick entry in file: " + filename);
        continue;
      }
      auto _ = split(tokens[1], ',');
      if (_.size() != 2) {
        logger::rs_load.warn("Invalid kick entry in file: " + filename);
        continue;
      }
      if (_[0].size() != 1 || _[0][0] < '0' || _[0][0] > '3') {
        logger::rs_load.warn("Invalid kick entry in file: " + filename);
        continue;
      }
      if (_[1].size() != 1 || (_[1][0] != 'L' && _[1][0] != 'R' && _[1][0] != '2')) {
        logger::rs_load.warn("Invalid kick entry in file: " + filename);
        continue;
      }
      int id = _[0][0] - 48, rot = (_[1][0] == 'L' ? 0 : (_[1][0] == 'R' ? 1 : 2));
      for (size_t i = 2; i < tokens.size(); ++i) {
        _ = split(tokens[i], ',');
        if (_.size() != 2 || !is_integer(_[0]) || !is_integer(_[1])) {
          logger::rs_load.warn("Invalid kick movement entry in file: " + filename);
          continue;
        }
        int x = std::stoi(_[0]), y = std::stoi(_[1]);
        if (reverse) std::swap(x, y);
        kick_buffer.kt[id][rot].emplace_back(x, y);
      }
    } else if (tokens[0] == "mino") {
      if (tokens.size() < 2) {
        logger::rs_load.warn("Invalid mino entry in file: " + filename);
        continue;
      }
      Minoes mino_id;
      if (is_integer(tokens[1])) mino_id = Minoes(std::stoi(tokens[1]));
      else mino_id = get_mino_id(toupper(tokens[1]));
      if (mino_id == m0) {
        logger::rs_load.warn("Unknown mino name " + tokens[1] + " in file: " + filename);
        continue;
      }
      int kt_begin = 2;
      rs_buffer.rs[mino_id].center = {2, 2};
      rs_buffer.rs[mino_id].spawn_direction = 0;
      while (tokens.size() >= kt_begin + 2 && (tokens[kt_begin] == "center" || tokens[kt_begin] == "spawn")) {
        if (tokens[kt_begin] == "center") {
          auto _ = split(tokens[kt_begin + 1], ',');
          if (_.size() != 2 || !is_integer(_[0]) || !is_integer(_[1])) {
            logger::rs_load.warn("Invalid center entry for " + tokens[1] + " in file: " + filename);
            continue;
          }
          int cx = std::stoi(_[0]), cy = std::stoi(_[1]);
          if (reverse) std::swap(cx, cy);
          rs_buffer.rs[mino_id].center = {cx, cy};
          kt_begin += 2;
        }
        if (tokens[kt_begin] == "spawn") {
          auto _ = tokens[kt_begin + 1];
          if (!is_integer(_)) {
            logger::rs_load.warn("Invalid spawn entry for " + tokens[1] + " in file: " + filename);
            continue;
          }
          int d = std::stoi(_) & 3;
          rs_buffer.rs[mino_id].spawn_direction = d;
          kt_begin += 2;
        }
      }
      for (size_t i = kt_begin; i < tokens.size(); ++i) {
        auto it = kick_tables.find(tokens[i]);
        if (it == kick_tables.end()) {
          logger::rs_load.warn("Unknown kick_table name " + tokens[i] + " in file: " + filename);
          continue;
        }
        rs_buffer.rs[mino_id].kick_table.push_back(&it->second);
      }
    }
  }
  save();
  f.close();
  loaded_file.insert(filename);
  loading_file.erase(filename);
  return true;
}

void print(std::ostream &s, const kick_table_t &kt, bool readable = false) {
  if (readable) s << "Kick Table " << kt.name << " from file: " << kt.from_file << '\n';
  else s << "kick_table " << kt.name << '\n';
  for (int id = 0; id < 4; ++id) {
    for (int rot = 0; rot < 3; ++rot) {
      if (kt.kt[id][rot].empty()) continue;
      if (readable) s << "  Direction: " << id << ", Rotation: " << (rot == 0 ? "CCW" : (rot == 1 ? " CW" : "180")) << " - ";
      else s << "kick " << id << ',' << (rot == 0 ? "L" : (rot == 1 ? "R" : "2")) << ' ';
      bool first = true;
      for (const auto &p : kt.kt[id][rot]) {
        if (!first) {
          if (readable) s << "; ";
          else s << ' ';
        }
        first = false;
        if (readable) s << "(" << std::setw(2) << p.first << ", " << std::setw(2) << p.second << ")";
        else s << p.first << ',' << p.second;
      }
      s << '\n';
    }
  }
  if (!readable) s << "end\n";
  s.flush();
}
void print(std::ostream &s, const rotation_system_t &rs, bool readable = false) {
  if (readable) s << "Rotation System " << rs.name << " from file: " << rs.from_file << '\n' << "\nKick Tables Used:\n\n";

  std::set<kick_table_t*> used;
  for (const auto &pair : rs.rs) {
    for (const auto &kt : pair.second.kick_table) {
      used.insert(kt);
    }
  }
  for (const auto &kt : used) {
    print(s, *kt, readable); s << '\n';
  }

  if (!readable) s << "rotation_system " << rs.name << '\n';
  if (readable) s << "Mino Configuration:\n";
 
  for (const auto &pair : rs.rs) {
    const auto &mino = pair.first;
    const auto &data = pair.second;
    if (readable) s << "  " << get_mino_name(mino) << ", Center: (" << data.center.first << ", " << data.center.second << "), Spawn direction: " << data.spawn_direction << '\n';
    else s << "mino " << get_mino_name(mino) << ' ' << "center " << data.center.first << ',' << data.center.second << ' ' << "spawn " << data.spawn_direction;
    if (readable) s << "    Kick Table: ";
    bool first = true;
    for (const auto &kt : data.kick_table) {
      if (readable) {
        if (!first) s << ", ";
        s << kt->name;
      } else s << ' ' << kt->name;
      first = false;
    }
    s << '\n';
  }
  if (!readable) s << "\nend\n";
  s.flush();
}
}
