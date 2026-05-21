#include "config.h"
#include "../constants.h"
#include "../util/error_handling.h"
#include "../util/util.h"
#include <variant>
// #if DEBUG_VER == 5
#include <iostream>
// #endif

namespace config {

const std::string config_path = "res/config.txt";

#define dict map_entry({{
#define _end }}) // to match brackets
#define list list_entry({{
#define end }})
const map_entry default_config = dict
  { "display", dict
    { "window", dict
      { "width", 320 },
      { "height", 240 },
    end }, 
  end },
  { "system", dict
    { "lang", "en" },
  end },
  { "demo", dict
    { "sizemult", 0.5 },
    { "game", dict
      { "das", 150 },
#if DEBUG_VER == 5
      { "break_it!" , -1 },
#endif
      { "arr", 50 },
      { "dcd", 33 },
      { "are", 0 },
      { "are_clear", 500 },
      { "are_line", 250 },
      { "sddas", 100 },
      { "sdarr", 100 },
      { "gravity", 1000 },
      { "lock_delay", 500 },
      { "irs", "ixs_hold" },
      { "ihs", "ixs_hold" },
      { "ims", "ixs_hold" },
      { "infinite_hold", false },
      { "lock_refresh", 15 },
      { "lock_refresh_hold", 15 },
      { "keybinding", dict
        { "op_left" , list { "key_Left" } end },
#if DEBUG_VER == 5
        { "break_it!" , list { "break_it!", 114514 } end },
#endif
        { "op_right", list { "key_Right" } end },
        { "op_sd"   , list { "key_Down" } end },
        { "op_hd"   , list { "key_Space" } end },
        { "op_cw"   , list { "key_X" } end },
        { "op_ccw"  , list { "key_Z" } end },
        { "op_180"  , list { "key_A" } end },
        { "op_hold" , list { "key_LShift", "key_RShift" } end },
      end },
    end },
  end },
end;
#undef dict
#undef list
#undef _end
#undef end


#define b bool
#define i long long
#define f double
#define s std::string
#define l list_entry
#define m map_entry
#define header [](const entry_t &e) -> bool
#define ranged(t, l, r) { #t, header{ return l <= std::get<t>(e) && std::get<t>(e) <= r; } }
#define length(l, r) { "s", header{ return l <= std::get<std::string>(e).length() && std::get<std::string>(e).length() <= r; } }
#define enum(t) { #t, header{ return is_legal_name<t>(std::get<std::string>(e)); } }
#define always(t, r) { #t, header{ return r; } }
const constraint_t default_constraint_passdown = always(i, true);
const constraint_t default_constraint_key = always(s, true);
const std::map<std::string, constraint_t> constraints = {
  { "display.window.width", ranged(i, 1, 8192) },
  { "display.window.height", ranged(i, 1, 8192) },
  { "system.lang", length(1, 64) },
  { "demo.sizemult", ranged(f, 0.25, 4.0) },
  { "demo.game.*", ranged(i, 0, 10000) },
  { "demo.game.gravity", ranged(i, -1, 10000) },
  { "demo.game.lock_delay", ranged(i, -1, 10000) },
  { "demo.game.irs", enum(IXSMode) },
  { "demo.game.ihs", enum(IXSMode) },
  { "demo.game.ims", { "IXSMode", header{ return std::get<std::string>(e) == "ixs_none" || std::get<std::string>(e) == "ixs_hold"; } } },
  { "demo.game.infinite_hold", always(b, true) },
  { "demo.game.lock_refresh", ranged(i, 0, 2147483647) },
  { "demo.game.lock_refresh_hold", ranged(i, 0, 2147483647) },
  { "demo.game.keybinding.#", enum(Operation) },
  { "demo.game.keybinding.*", enum(Key) },
};
// default to always(i, true).
// the path of a list's son is like path.to.list.0.
// list and map should only passes constraints to its son.
  // use path.to.them.* to do so, which is recursive.
  // use path.to.them.# to limit map keys, which should be str or enum.
#undef b
#undef i
#undef f
#undef s
#undef l
#undef m
#undef header
#undef ranged
#undef length
#undef enum
#undef always

map_entry config;

namespace validator {
int errors = 0;
template<class _Tp, typename =
std::enable_if_t<
  std::is_same_v<_Tp, bool> ||
  std::is_same_v<_Tp, long long> ||
  std::is_same_v<_Tp, double> ||
  std::is_same_v<_Tp, std::string>
>>
void validate(std::string path, const _Tp &node, constraint_t passdown) {
  LOG(5) << "validating literal '" << path << "'" << std::endl;
  constraint_t cons = passdown;
  if (constraints.count(path)) cons = constraints.at(path);
  if (constraints.count(path.empty() ? "*" : path + ".*")) logger::config.error("Assuming node '" + path + "' is an iterable (by giving passdown constraint), however it is a literal."), ++errors;
  if (constraints.count(path.empty() ? "#" : path + ".#")) logger::config.error("Assuming node '" + path + "' is a map (by giving key constraint), however it is a literal."), ++errors;

  bool is_same = false;
  if constexpr (std::is_same_v<_Tp, bool>) is_same = (cons.first == "b");
  else if constexpr (std::is_same_v<_Tp, long long>) is_same = (cons.first == "i");
  else if constexpr (std::is_same_v<_Tp, double>) is_same = (cons.first == "f");
  else is_same = (cons.first == "s") || (is_legal_typename(cons.first));
  try {
    if (!is_same)
      logger::config.error("Node '" + path + "' does not correspond to its (sub)type: " + cons.first), ++errors;
    else if (!cons.second(node))
      logger::config.error("Node '" + path + "' does not satisfy its constraint."), ++errors;
  } catch (std::bad_variant_access e) {
    logger::config.error("Node '" + path + "'s constraint function failed to run because of wrong type assumption."), ++errors;
  }
}
void validate(std::string path, const map_entry &node, constraint_t passdown);
void validate(std::string path, const list_entry &node, constraint_t passdown) {
  LOG(5) << "validating list '" << path << "'" << std::endl;
  if (constraints.count(path)) logger::config.error("Assuming node '" + path + "' is a literal (by giving direct constraint), however it is a list."), ++errors;
  if (constraints.count(path.empty() ? "*" : path + ".*")) passdown = constraints.at(path + ".*");
  if (constraints.count(path.empty() ? "#" : path + ".#")) logger::config.error("Assuming node '" + path + "' is a map (by giving key constraint), however it is a list."), ++errors;
  for (const auto &son : node.a) {
    std::string son_path;
    if (!path.empty()) son_path = path + ".0";
    else son_path = "0"; // impossible because the root is always a map

    std::visit([&](auto &&arg) {
      validate(son_path, arg, passdown);
    }, son);
  }
}
void validate(std::string path, const map_entry &node, constraint_t passdown) {
  LOG(5) << "validating map '" << path << "'" << std::endl;
  if (constraints.count(path)) logger::config.error("Assuming node '" + path + "' is a literal (by giving direct constraint), however it is a map."), ++errors;
  if (constraints.count(path.empty() ? "*" : path + ".*")) passdown = constraints.at(path + ".*");
  constraint_t key = default_constraint_key;
  if (constraints.count(path.empty() ? "#" : path + ".#")) key = constraints.at(path + ".#");
  if (!(key.first == "s" || is_legal_typename(key.first))) logger::config.error("Giving map node '" + path + "' 's key a non-string-or-enum type " + key.first + "."), ++errors;
  for (const auto &[ son_key, son ] : node.a) {
    std::string son_path;
    if (!path.empty()) son_path = path + "." + son_key;
    else son_path = son_key;
    if (!key.second(son_key)) logger::config.error("Key '" + son_key + "' does not satisfy the constraint given by map node '" + path + "'."), ++errors;

    std::visit([&](auto &&arg) {
      validate(son_path, arg, passdown);
    }, son);
  }
}
void reset_errors() { errors = 0; }
}

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

namespace user_validator {
int warns = 0;
template<class _Tp, typename =
std::enable_if_t<
  std::is_same_v<_Tp, bool> ||
  std::is_same_v<_Tp, long long> ||
  std::is_same_v<_Tp, double> ||
  std::is_same_v<_Tp, std::string>
>>
bool validate(std::string path, _Tp &node, constraint_t passdown) {
  constraint_t cons = passdown;
  if (constraints.count(path)) cons = constraints.at(path);

  bool is_same = false;
  if constexpr (std::is_same_v<_Tp, bool>) is_same = (cons.first == "b");
  else if constexpr (std::is_same_v<_Tp, long long>) is_same = (cons.first == "i");
  else if constexpr (std::is_same_v<_Tp, double>) is_same = (cons.first == "f");
  else is_same = (cons.first == "s") || (is_legal_typename(cons.first));
  try {
    if (!is_same)
      return logger::config.warn("Node '" + path + "' does not correspond to its (sub)type: " + cons.first), ++warns, false;
    else if (!cons.second(node))
      return logger::config.warn("Node '" + path + "' does not satisfy its constraint."), ++warns, false;
  } catch (std::bad_variant_access e) {
    return logger::config.warn("Node '" + path + "'s constraint function failed to run because of wrong type assumption."), ++warns, false;
  }
  return true;
}
bool validate(std::string path, map_entry &node, constraint_t passdown);
bool validate(std::string path, list_entry &node, constraint_t passdown) {
  if (constraints.count(path.empty() ? "*" : path + ".*")) passdown = constraints.at(path + ".*");
  std::vector<std::size_t> remove_index;
  for (std::size_t i = 0; i < node.a.size(); ++i) {
    auto &son = node.a[i];
    std::string son_path;
    if (!path.empty()) son_path = path + ".0";
    else son_path = "0"; // impossible because the root is always a map

    std::visit([&](auto &&arg) {
      if (!validate(son_path, arg, passdown)) remove_index.push_back(i);
    }, son);
  }
  for (std::size_t i = 0; i < remove_index.size(); ++i) {
    node.a.erase(node.a.begin() + remove_index[i] - i);
  }
  return true;
}
bool validate(std::string path, map_entry &node, constraint_t passdown) {
  if (constraints.count(path.empty() ? "*" : path + ".*")) passdown = constraints.at(path + ".*");
  constraint_t key = default_constraint_key;
  if (constraints.count(path.empty() ? "#" : path + ".#")) key = constraints.at(path + ".#");
  std::vector<std::string> remove_key;
  for (auto &[ son_key, son ] : node.a) {
    std::string son_path;
    if (!path.empty()) son_path = path + "." + son_key;
    else son_path = son_key;
    if (!key.second(son_key)) logger::config.warn("Key '" + son_key + "' does not satisfy the constraint given by map node '" + path + "'."), ++warns, remove_key.push_back(son_key);
    else
      std::visit([&](auto &&arg) {
        if (!validate(son_path, arg, passdown)) remove_key.push_back(son_key);
      }, son);
  }
  for (auto son_key : remove_key) node.a.erase(son_key);
  return true;
}
void reset_errors() { warns = 0; }
}

int initialize() {
  validator::reset_errors(); validator::validate("", default_config, default_constraint_passdown);
  if (validator::errors) {
    logger::config.note(std::to_string(validator::errors) + " error(s) generated in total when validating Default Config.");
    logger::config.note("Choose any of these methods to contact Daiari (please describe your situation and provide this file):");
    logger::config.note(AUTHOR_CONTACT);
    return 1;
  }

  loader::reset_errors(); loader::load_ini_from_file(config_path, config);
  if (loader::errors || loader::warns) {
    logger::config.note(std::to_string(loader::errors) + " error(s) and " + std::to_string(loader::warns) + " warning(s) generated in total when loading Config.");
    logger::config.note("You should check mentioned lines in '" + config_path + "'.");
    logger::config.note("If you have no clue, choose any of these methods to contact Daiari (please describe your situation and provide this file):");
    logger::config.note(AUTHOR_CONTACT);
  }
  if (loader::errors) return 2;
  else if (loader::warns) return 3;

  user_validator::reset_errors(); user_validator::validate("", config, default_constraint_passdown);
  if (user_validator::warns) {
    logger::config.note(std::to_string(user_validator::warns) + " warning(s) generated in total when validating Config.");
    logger::config.note("You should check mentioned keys in '" + config_path + "'.");
    logger::config.note("If you have no clue, choose any of these methods to contact Daiari (please describe your situation and provide this file):");
    logger::config.note(AUTHOR_CONTACT);
    return 4;
  }
  return 0;
}

}
