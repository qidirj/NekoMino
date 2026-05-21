#ifndef ENUMERATOR_H
#define ENUMERATOR_H

#define BOOST_PP_LIMIT_MAG 1024
#define BOOST_PP_LIMIT_TUPLE 256
#include "boost/preprocessor/cat.hpp"
#include "boost/preprocessor/seq/enum.hpp"
#include "boost/preprocessor/seq/for_each.hpp"
#include "boost/preprocessor/stringize.hpp"
#include "boost/preprocessor/variadic/to_seq.hpp"
#include <any>
#include <cassert>
#include <exception>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>

template <typename name> inline name from_string(const std::string &id);
template <typename name> inline bool is_legal_name(const std::string &id);
template <typename name> inline std::string to_string(name e);

inline std::unordered_map<std::string,
                          std::function<std::any(const std::string &)>> &
get_from_string_map() {
  static std::unordered_map<std::string,
                            std::function<std::any(const std::string &)>>
      map;
  return map;
}
inline std::unordered_map<std::string,
                          std::function<std::string(const std::any &)>> &
get_to_string_map() {
  static std::unordered_map<std::string,
                            std::function<std::string(const std::any &)>>
      map;
  return map;
}
inline bool is_legal_typename(const std::string &typeName) {
  return !!get_from_string_map().count(typeName);
}
inline std::any from_string(const std::string &typeName,
                            const std::string &id) {
  auto &map = get_from_string_map();
  auto it = map.find(typeName);
  if (it == map.end())
    throw std::invalid_argument("Unknown type name: " + typeName);
  return it->second(id);
}
inline std::string to_string(const std::string &typeName,
                             const std::any &value) {
  auto &map = get_to_string_map();
  auto it = map.find(typeName);
  if (it == map.end())
    throw std::invalid_argument("Unknown type name: " + typeName);
  return it->second(value);
}

#define ENUM_GET_IF(r, data, elem)                                             \
  if (id == BOOST_PP_STRINGIZE(elem))                                          \
    return data::elem;
#define ENUM_TEST_IF(r, data, elem)                                            \
  if (id == BOOST_PP_STRINGIZE(elem))                                          \
    return true;
#define ENUM_CASE(r, data, elem)                                               \
  case data::elem:                                                             \
    return BOOST_PP_STRINGIZE(elem);
#define declare_enum(name, ...)                                                \
  enum name { BOOST_PP_SEQ_ENUM(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) };      \
  template <> inline name from_string<name>(const std::string &id) {           \
    BOOST_PP_SEQ_FOR_EACH(ENUM_GET_IF, name,                                   \
                          BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))               \
    throw std::invalid_argument("Unknown enum value for " #name ": " + id);    \
  }                                                                            \
  template <> inline bool is_legal_name<name>(const std::string &id) {         \
    BOOST_PP_SEQ_FOR_EACH(ENUM_TEST_IF, name,                                  \
                          BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))               \
    return false;                                                              \
  }                                                                            \
  template <> inline std::string to_string<name>(name e) {                     \
    switch (e) {                                                               \
      BOOST_PP_SEQ_FOR_EACH(ENUM_CASE, name,                                   \
                            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))             \
    default:                                                                   \
      throw std::invalid_argument("Invalid enum value for " #name ": " +       \
                                  std::to_string((int)(e)));                   \
    }                                                                          \
  }                                                                            \
  register_enum(name)
#define register_enum(name)                                                    \
  namespace {                                                                  \
  struct BOOST_PP_CAT(Register_, name) {                                       \
    BOOST_PP_CAT(Register_, name)() {                                          \
      /* 注册 from_string 包装 */                                              \
      get_from_string_map()[BOOST_PP_STRINGIZE(name)] =                        \
                                [](const std::string &id) -> std::any {        \
        return from_string<name>(id);                                          \
      };                                                                       \
      /* 注册 to_string 包装 */                                                \
      get_to_string_map()[BOOST_PP_STRINGIZE(name)] =                          \
                              [](const std::any &value) -> std::string {       \
        return to_string(std::any_cast<name>(value));                          \
      };                                                                       \
    }                                                                          \
  };                                                                           \
  static BOOST_PP_CAT(Register_, name) BOOST_PP_CAT(register_, name);          \
  }

// file generated by Deepseek.

#endif
