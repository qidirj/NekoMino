#include "../util/error_handling.h"
#include "../util/util.h"
#include "config.h"
#include <variant>

namespace config {
namespace getset_impl {
template<class _Tp, typename =
std::enable_if_t<
  std::is_same_v<_Tp, bool> ||
  std::is_same_v<_Tp, long long> ||
  std::is_same_v<_Tp, double> ||
  std::is_same_v<_Tp, std::string> ||
  std::is_same_v<_Tp, list_entry>
>>
entry_t get(std::string path, const std::string &key, const std::vector<std::string> &keys, size_t idx, const _Tp &node, bool def = false) {
  if (idx != keys.size()) {
    logger::config.error("When getting config: key '" + key + "' does not exist because '" + path + "' is a non-map.");
    return entry_t();
  }
  return node;
}
entry_t get(std::string path, const std::string &key, const std::vector<std::string> &keys, size_t idx, const std::monostate &node, bool def = false) {
  logger::config.error("Node '" + path + "' is a monostate! You shouldn't be here!!!!!!!!!!!!");
  return entry_t();
}
entry_t get(std::string path, const std::string &key, const std::vector<std::string> &keys, size_t idx, const map_entry &node, bool def = false) {
  if (idx == keys.size()) return node;
  if (!node.a.count(keys[idx])) {
    if (def) logger::config.error("When getting config: key '" + keys[idx] + "' not found at '" + path + "' when trying to get '" + key + "'.");
    return entry_t();
  }
  std::string son_path;
  if (!path.empty()) son_path = path + "." + keys[idx];
  else son_path = keys[idx];
  return std::visit([&](auto &&arg) -> entry_t {
    return get(son_path, key, keys, idx + 1, arg, def);
  }, node.a.at(keys[idx]));
}

namespace validator {
template<class _Tp, typename =
std::enable_if_t<
  std::is_same_v<_Tp, bool> ||
  std::is_same_v<_Tp, long long> ||
  std::is_same_v<_Tp, double> ||
  std::is_same_v<_Tp, std::string>
>>
bool validate(std::string path, const _Tp &node, constraint_t passdown) {
  constraint_t cons = passdown;
  if (constraints.count(path)) cons = constraints.at(path);

  bool is_same = false;
  if constexpr (std::is_same_v<_Tp, bool>) is_same = (cons.first == "b");
  else if constexpr (std::is_same_v<_Tp, long long>) is_same = (cons.first == "i");
  else if constexpr (std::is_same_v<_Tp, double>) is_same = (cons.first == "f");
  else if constexpr (std::is_same_v<_Tp, std::string>) is_same = (cons.first == "s") || (is_legal_typename(cons.first));
  else logger::config.error("You shouldn't go here!!!!!!!!!!!!"), is_same = false;
  try {
    if (!is_same)
      return logger::config.error("When setting config: node '" + path + "' does not correspond to its (sub)type: " + cons.first), false;
    else if (!cons.second(node))
      return logger::config.error("When setting config: node '" + path + "' does not satisfy its constraint."), false;
  } catch (std::bad_variant_access e) {
    return logger::config.error("When setting config: node '" + path + "'s constraint function failed to run because of wrong type assumption."), false;
  }
  return true;
}
bool validate(std::string path, const std::monostate &node, constraint_t passdown) {
  logger::config.error("Node '" + path + "' is a monostate! You shouldn't be here!!!!!!!!!!!!");
  return false;
}
bool validate(std::string path, const map_entry &node, constraint_t passdown);
bool validate(std::string path, const list_entry &node, constraint_t passdown) {
  if (constraints.count(path.empty() ? "*" : path + ".*")) passdown = constraints.at(path.empty() ? "*" : path + ".*");
  for (std::size_t i = 0; i < node.a.size(); ++i) {
    auto &son = node.a[i];
    std::string son_path;
    if (!path.empty()) son_path = path + ".0";
    else son_path = "0"; // impossible because the root is always a map

    if (!std::visit([&](auto &&arg) -> bool {
      if (!validate(son_path, arg, passdown)) return false;
      return true;
    }, son)) return false;
  }
  return true;
}
bool validate(std::string path, const map_entry &node, constraint_t passdown) {
  if (constraints.count(path.empty() ? "*" : path + ".*")) passdown = constraints.at(path.empty() ? "*" : path + ".*");
  constraint_t key = default_constraint_key;
  if (constraints.count(path.empty() ? "#" : path + ".#")) key = constraints.at(path.empty() ? "#" : path + ".#");
  for (auto &[ son_key, son ] : node.a) {
    std::string son_path;
    if (!path.empty()) son_path = path + "." + son_key;
    else son_path = son_key;
    if (!key.second(son_key)) return false;
    else
      if (!std::visit([&](auto &&arg) -> bool {
        if (!validate(son_path, arg, passdown)) return false;
        return true;
      }, son)) return false;
  }
  return true;
}
}

int set(std::string key, const entry_t &val) {
  if (std::holds_alternative<map_entry>(val)) {
    logger::config.error("Cannot set config: you can't directly set a map.");
    return 2;
  }
  auto path = split(key, ".");
  const map_entry *at_default = &default_config;
  map_entry *at_user = &config;
  constraint_t cons = default_constraint_passdown;
  std::string current_path;
  for (size_t i = 0; i + 1 < path.size(); ++i) {
    if (!at_default->a.count(path[i])) {
      logger::config.error("When setting config: key '" + key + "' not found.");
      return 1;
    } else {
      try { at_default = &std::get<map_entry>(at_default->a.at(path[i])); }
      catch (std::bad_variant_access e) {
        logger::config.error("When setting config: key '" + key + "' not found.");
        return 1;
      }
    }
    if (constraints.count(current_path.empty() ? "*" : current_path + ".*")) cons = constraints.at(current_path.empty() ? "*" : current_path + ".*");
    if (!at_user->a.count(path[i])) at_user->a.insert({ path[i], map_entry{} });
    at_user = &std::get<map_entry>(at_user->a.at(path[i])); // if there is a non-map, it should be reported the time when its inserted (see config_io.cpp)
    if (!current_path.empty()) current_path = current_path + "." + path[i];
    else current_path = path[i];
  }
  if (!at_default->a.count(path.back())) {
    logger::config.error("When setting config: key '" + key + "' not found.");
    return 1;
  }
  if (std::holds_alternative<map_entry>(at_default->a.at(path.back())) || (std::holds_alternative<list_entry>(at_default->a.at(path.back())) ^ std::holds_alternative<list_entry>(val))) {
    logger::config.error("When setting config: type mismatch.");
    return 3;
  }
  if (!std::visit([&](auto &&arg) -> bool {
    return validator::validate(current_path, arg, cons);
  }, val)) {
    logger::config.error("When setting config: illegal value for '" + key + "'.");
    return 3;
  }
  at_user->a[path.back()] = val;
  return 0;
}


template<class _Tp, typename =
std::enable_if_t<
  std::is_same_v<_Tp, bool> ||
  std::is_same_v<_Tp, long long> ||
  std::is_same_v<_Tp, double> ||
  std::is_same_v<_Tp, std::string> ||
  std::is_same_v<_Tp, list_entry>
>>
bool erase(std::string path, const std::string &key, const std::vector<std::string> &keys, size_t idx, _Tp &node) {
  logger::config.error("When removing config: key '" + key + "' does not exist because '" + path + "' is a non-map.");
  return false;
}
bool erase(std::string path, const std::string &key, const std::vector<std::string> &keys, size_t idx, std::monostate &node) {
  logger::config.error("Node '" + path + "' is a monostate! You shouldn't be here!!!!!!!!!!!!");
  return false;
}
bool erase(std::string path, const std::string &key, const std::vector<std::string> &keys, size_t idx, map_entry &node) {
  if (!node.a.count(keys[idx])) return logger::config.error("When removing config: key '" + keys[idx] + "' not found at '" + path + "' when trying to remove '" + key + "'."), false;
  std::string son_path;
  if (!path.empty()) son_path = path + "." + keys[idx];
  else son_path = keys[idx];
  if (idx + 1 != keys.size())
    return std::visit([&](auto &&arg) ->  bool {
      return erase(son_path, key, keys, idx + 1, arg);
    }, node.a.at(keys[idx]));
  else return node.a.erase(keys[idx]);
}
}

entry_t getraw(std::string key) {
  auto keys = split(key, ".");
  auto res = getset_impl::get("", key, keys, 0, config, false);
  if (std::holds_alternative<std::monostate>(res)) return getset_impl::get("", key, keys, 0, default_config, true);
  else return res;
}
template<class _Tp, typename>
_Tp get(std::string key) {
  auto res = getraw(key);
  if (std::holds_alternative<_Tp>(res)) return std::get<_Tp>(res);
  else {
    logger::config.error("Key '" + key + "' does not correspond to the requested type.");
    return _Tp();
  }
}
int set(std::string key, const entry_t &val) {
  return getset_impl::set(key, val);
}
bool erase(std::string key) {
  auto keys = split(key, ".");
  return getset_impl::erase("", key, keys, 0, config);
}

#define TYPES (bool)(long long)(double)(std::string)(list_entry)(map_entry)
#define INSTANTIATE(r, data, type) template type get<type>(std::string);
BOOST_PP_SEQ_FOR_EACH(INSTANTIATE, ~, TYPES)
}
