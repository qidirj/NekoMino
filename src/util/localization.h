#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <string>
#include <map>
#include <vector>
using std::string, std::map, std::size_t;
using localization = std::map<string, string>;

extern std::map<string, localization> languages;
extern localization *lang;
string format(const string &key, const std::vector<string> &args, int recursion_depth = 0);

localization load_language_file(const string &filename);
void export_language_file(const string &filename, const localization &lang);

#endif
