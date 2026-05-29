#include "config.h"
#include "../constants.h"
#include "../util/error_handling.h"
#include <variant>
// #if DEBUG_VER == 5
#include <iostream>
// #endif

namespace config {
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
  { "system.lang.*", length(1, 64) },
  { "demo.sizemult", ranged(f, 0.25, 4.0) },
  { "demo.game.*", ranged(i, 0, 10000) },
  { "demo.game.gravity", ranged(i, -1, 10000) },
  { "demo.game.lock_delay", ranged(i, -1, 10000) },
  { "demo.game.irs", enum(IXSMode) },
  { "demo.game.ihs", enum(IXSMode) },
  { "demo.game.ims", { "IXSMode", header{ return std::get<std::string>(e) == "ixs_none" || std::get<std::string>(e) == "ixs_hold"; } } },
  { "demo.game.infinite_hold", always(b, true) },
  { "demo.game.lock_refresh", ranged(i, 0, 2147483647) },
  { "demo.game.lock_refresh_on_hold", ranged(i, 0, 2147483647) },
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
  LOG(8) << "validating literal '" << path << "'" << std::endl;
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
void validate(std::string path, const std::monostate &node, constraint_t passdown) {
  logger::config.error("Node '" + path + "' is a monostate! You shouldn't be here!!!!!!!!!!!!");
  ++errors;
}
void validate(std::string path, const map_entry &node, constraint_t passdown);
void validate(std::string path, const list_entry &node, constraint_t passdown) {
  LOG(8) << "validating list '" << path << "'" << std::endl;
  if (constraints.count(path)) logger::config.error("Assuming node '" + path + "' is a literal (by giving direct constraint), however it is a list."), ++errors;
  if (constraints.count(path.empty() ? "*" : path + ".*")) passdown = constraints.at(path.empty() ? "*" : path + ".*");
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
  LOG(8) << "validating map '" << path << "'" << std::endl;
  if (constraints.count(path)) logger::config.error("Assuming node '" + path + "' is a literal (by giving direct constraint), however it is a map."), ++errors;
  if (constraints.count(path.empty() ? "*" : path + ".*")) passdown = constraints.at(path.empty() ? "*" : path + ".*");
  constraint_t key = default_constraint_key;
  if (constraints.count(path.empty() ? "#" : path + ".#")) key = constraints.at(path.empty() ? "#" : path + ".#");
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
bool validate(std::string path, std::monostate &node, constraint_t passdown) {
  logger::config.warn("Node '" + path + "' is a monostate! You shouldn't be here!!!!!!!!!!!!");
  ++warns;
  return false;
}
bool validate(std::string path, map_entry &node, constraint_t passdown);
bool validate(std::string path, list_entry &node, constraint_t passdown) {
  if (constraints.count(path.empty() ? "*" : path + ".*")) passdown = constraints.at(path.empty() ? "*" : path + ".*");
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
  if (constraints.count(path.empty() ? "*" : path + ".*")) passdown = constraints.at(path.empty() ? "*" : path + ".*");
  constraint_t key = default_constraint_key;
  if (constraints.count(path.empty() ? "#" : path + ".#")) key = constraints.at(path.empty() ? "#" : path + ".#");
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
}
