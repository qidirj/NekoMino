#include "config.h"
#include "../util/error_handling.h"
#include "../util/util.h"
#include <iomanip>
#include <sstream>
#include <variant>

namespace config {
const std::string config_path = "res/config.txt";

namespace loader {
int errors, warns;

void load_ini_from_file(std::string filename, map_entry &var) {
  std::ifstream f(filename); std::string buffer;
  if (!f.is_open()) {
    logger::config.error("Cannot load file '" + filename + "'."), ++errors;
    return;
  }

  int line = 0; std::string prefix = "";
  while (std::getline(f, buffer)) {
    ++line;
    buffer = trim(buffer);
    if (buffer.empty() || buffer[0] == ':') continue;
    std::size_t pos;
    if ((pos = buffer.find('=')) == buffer.npos) logger::config.error("On config line " + std::to_string(line) + ": no value found (no '=')."), ++warns;
    std::string key = buffer.substr(0, pos), value = buffer.substr(pos + 1);
    if (key.empty()) logger::config.warn("On config line " + std::to_string(line) + ": key is empty."), ++warns;
    if (key[0] == '*') {
      key = key.substr(1);
      if (key == "prefix") prefix = value;
      else logger::config.warn("On config line " + std::to_string(line) + ": unknown meta key '*" + key + "'."), ++warns;
    } else {
      if (!prefix.empty()) key = prefix + key;
      auto path = split(key, ".");
      const map_entry *at_default = &default_config;
      map_entry *at_user = &config;
      bool ok = true;
      for (size_t i = 0; i + 1 < path.size(); ++i) {
        if (!at_default->a.count(path[i])) {
          logger::config.warn("On config line " + std::to_string(line) + ": key '" + key + "' not found."), ++warns;
          ok = false; break;
        } else {
          try { at_default = &std::get<map_entry>(at_default->a.at(path[i])); }
          catch (std::bad_variant_access e) {
            logger::config.warn("On config line " + std::to_string(line) + ": key '" + key + "' not found."), ++warns;
            ok = false; break;
          }
        }
        if (!at_user->a.count(path[i])) at_user->a.insert({ path[i], map_entry{} });
        at_user = &std::get<map_entry>(at_user->a.at(path[i])); // if there is a non-map, it should be reported the time when its inserted
      }
      if (!ok) continue;
      if (!at_default->a.count(path.back())) {
        logger::config.warn("On config line " + std::to_string(line) + ": key '" + key + "' not found."), ++warns;
        continue;
      }
      entry_t should_be;
      std::visit([&](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, bool>) {
          if (value == "true" || value == "1" || value == "on" || value == "yes" || value == "T" || value == "Y" || value == "t" || value == "y" || value == "+")
            should_be = true;
          else if (value == "false" || value == "0" || value == "off" || value == "no" || value == "F" || value == "N" || value == "f" || value == "n" || value == "-")
            should_be = false;
          else
            logger::config.warn("On config line " + std::to_string(line) + ": invalid value: need bool, found '" + value + "'."), ++warns, ok = false;
        } else if constexpr (std::is_same_v<T, long long>) {
          if (is_integer(value))
            should_be = std::stoll(value);
          else
            logger::config.warn("On config line " + std::to_string(line) + ": invalid value: need integer, found '" + value + "'."), ++warns, ok = false;
        } else if constexpr (std::is_same_v<T, double>) {
          if (is_float(value))
            should_be = std::stod(value);
          else
            logger::config.warn("On config line " + std::to_string(line) + ": invalid value: need float, found '" + value + "'."), ++warns, ok = false;
        } else if constexpr (std::is_same_v<T, std::string>) {
          should_be = value;
        } else if constexpr (std::is_same_v<T, list_entry>) {
          auto values = split(value);
          should_be = list_entry();
          for (auto i : values) std::get<list_entry>(should_be).a.push_back(i);
        } else if constexpr (std::is_same_v<T, map_entry>) {
          logger::config.warn("On config line " + std::to_string(line) + ": invalid value: need map, but map shouldn't be given directly in INI mode."), ++warns, ok = false;
        } else logger::config.error("On config line " + std::to_string(line) + ": this shouldn't be reached!!!!!"), ++errors, ok = false, assert(("This shouldn't be reached!", false));
      }, at_default->a.at(path.back()));
      if (!ok) continue;
      if (at_user->a.count(path.back())) {
        logger::config.warn("On config line " + std::to_string(line) + ": duplicate key '" + key + "'."); ++warns;
      }
      at_user->a[path.back()] = should_be;
    }
  }
}

void reset_errors() { errors = warns = 0; }
}

namespace writer {
int errors, warns;

// template<class _Tp, typename =
// std::enable_if_t<
//   std::is_same_v<_Tp, bool> ||
//   std::is_same_v<_Tp, long long> ||
//   std::is_same_v<_Tp, double> ||
//   std::is_same_v<_Tp, std::string>
// >>
template<class _Tp>
bool write_node_as_string_to_stream(std::ostream &file, const _Tp &node) {
  return std::visit([&](auto &&arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, bool>) file << (arg ? "true" : "false");
    else if constexpr (std::is_same_v<T, long long>) file << arg;
    else if constexpr (std::is_same_v<T, double>) file << std::setprecision(17) << arg;
    else if constexpr (std::is_same_v<T, std::string>) file << arg;
    else return false;
    return true;
  }, node);
}
template<class _Tp, typename =
std::enable_if_t<
  std::is_same_v<_Tp, bool> ||
  std::is_same_v<_Tp, long long> ||
  std::is_same_v<_Tp, double> ||
  std::is_same_v<_Tp, std::string> ||
  std::is_same_v<_Tp, list_entry>
>>
void write_node_ini_to_stream(std::ostream &file, std::string path, std::string name, int indent, const _Tp &node) {
  file << std::string(indent - 2, ' ') << (path.empty() ? "" : ".") << name << "=";
  if constexpr (std::is_same_v<_Tp, bool>) file << (node ? "true" : "false");
  else if constexpr (std::is_same_v<_Tp, long long>) file << node;
  else if constexpr (std::is_same_v<_Tp, double>) file << std::setprecision(17) << node;
  else if constexpr (std::is_same_v<_Tp, std::string>) file << node;
  else {
    bool first = true;
    for (auto i : node.a) {
      if (!first) file << ' ';
      first = false;
      if (!write_node_as_string_to_stream(file, i)) {
        logger::config.error("Node '" + path + "' cannot be converted to INI correctly (non-string list component)");
        ++errors; first = true;
      }
    }
  }
  file << '\n';
}
void write_node_ini_to_stream(std::ostream &file, std::string path, std::string name, int indent, const map_entry &node) {
  for (const auto &[ son_key, son ] : node.a) {
    std::string son_path;
    if (!path.empty()) son_path = path + "." + son_key;
    else son_path = son_key;

    std::visit([&](auto &&arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, map_entry>)
        file << std::string(indent, ' ') << "*prefix=" << son_path << '\n';
      write_node_ini_to_stream(file, son_path, son_key, indent + 2, arg);
      if constexpr (std::is_same_v<T, map_entry>)
        file << std::string(indent, ' ') << "*prefix=" << path << '\n';
    }, son);
  }
}
void write_ini_to_file(std::string filename, const map_entry &root) {
  std::ofstream file(filename);
  write_node_ini_to_stream(file, "", "", 0, root);
}
std::string write_ini_to_string(const map_entry &root) {
  std::ostringstream st;
  write_node_ini_to_stream(st, "", "", 0, root);
  return st.str();
}

void reset_errors() { errors = warns = 0; }
}
}
