#ifndef CONFIG_H
#define CONFIG_H

#include <any>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace config {

extern const std::string config_path;

struct list_entry;
struct map_entry;
// using enum_entry = std::pair<std::string, std::string>;
using entry_t = std::variant<bool, long long, double, std::string, list_entry, map_entry>;
using constraint_checker_t = std::function<bool (const entry_t &)>;
using constraint_t = std::pair<std::string, constraint_checker_t>;
struct list_entry {
  std::vector<entry_t> a;
};
struct map_entry {
  std::map<std::string, entry_t> a;
}; // todo: make config screen structure using a similar approach

extern const map_entry default_config;
extern const std::map<std::string, constraint_t> constraints;
extern map_entry config;

namespace validator {
extern int errors;
}
namespace loader {
extern int errors, warns;
}
namespace user_validator {
extern int warns;
}

int initialize();
entry_t get(std::string key);

}

#endif
