#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <string>
#include <map>
#include <optional>
#include <vector>

namespace localization {
using localization = std::map<std::string, std::string>;

extern std::map<std::string, localization> languages;
extern std::vector<localization*> lang;

std::optional<std::string> find_in_lang(const std::string &key);
std::string format(const std::string &key, const std::vector<std::string> &args = {}, int recursion_depth = 0);

bool load_language_file(const std::string &filename);
bool export_language_file(const std::string &filename, const localization &lang);

int load_all_from_path(const std::string &path);
bool set_language(); // based on config
}

#endif
