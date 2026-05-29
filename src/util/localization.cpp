#include "../config/config.h"
#include "error_handling.h"
#include "localization.h"
#include <optional>
#include <filesystem>

namespace localization {

std::map<std::string, localization> languages = {
  {
    "en", {
      { "!language.name", "English (US)" },
      { "!language.name_local", "English (US)" },
      { "!language.id", "en" },
      { "about.gameTitle", "Neko Mino" },
      { "about.version", "Version [{#0}]" },
      { "frameFull", "Frame(s)" },
      { "dasFull", "Auto Shift Delay" },
      { "das", "ASD" },
      { "arrFull", "Auto Shift Period" },
      { "arr", "ASP" },
      { "configured", "Configured {#0}" }
    }
  }
};
std::vector<localization*> lang = { &languages["en"] }; // use pointer to prevent copying

std::optional<std::string> find_in_lang(const std::string &key) {
  for (auto *p : lang) {
    if (auto it = p->find(key); it != p->end()) return it->second;
  }
  return {};
}

std::string format(const std::string &key, const std::vector<std::string> &args, int recursion_depth) {
  if (recursion_depth >= 10) {
    logger::localization.warn("Recursion depth limit exceeded in localization format function.");
    logger::localization.note("The limit is hardcoded to 10 in current version.");
    return key; // Return the key as is to avoid infinite recursion
  }
  size_t ptr = 0, beg; std::string result = "";
  auto conv = [](const std::string &s) -> size_t {
    try {
      return std::stoul(s);
    } catch (...) {
      return -1;
    }
  };
  while ((beg = key.find('{', ptr)) != key.npos) {
    size_t end = key.find('}', ptr);
    if (end == key.npos) break;
    result += key.substr(ptr, beg - ptr);
    std::string subkey = key.substr(beg + 1, end - beg - 1), rres = "";
    /*
      花括号里支持的写法：
      {key}，不传参
      {#index}，取第 index 个参数，越界则为空
      {key*}，复制参数列表
      {key#index1[,index2[,...]]}，依次提取这些索引的参数作为参数列表
    */
    if (subkey[0] == '#') {
      // take corresponding argument
      if (size_t index = conv(subkey.substr(1)); index < args.size()) {
        rres = format(args[index], args, recursion_depth + 1);
      }
    } else {
      if (size_t pos = subkey.find('%'); pos != subkey.npos) subkey = subkey.substr(0, pos);
      std::string rkey = subkey; std::vector<std::string> rarg;
      if (size_t pos = subkey.find('*'); pos != subkey.npos) rkey = subkey.substr(0, pos), rarg = args;
      else if (size_t pos = subkey.find('#'); pos != subkey.npos) {
        rkey = subkey.substr(0, pos);
        // take arguments by indices
        std::vector<size_t> indices;
        size_t start = pos + 1, comma;
        while ((comma = subkey.find(',', start)) != subkey.npos) {
          if (size_t index = conv(subkey.substr(start, comma - start)); index < args.size()) {
            indices.push_back(index);
          }
          start = comma + 1;
        }
        if (size_t index = conv(subkey.substr(start)); index < args.size()) {
          indices.push_back(index);
        }
        for (const auto &i : indices) {
          if (!args[i].empty()) rarg.push_back(args[i]);
        }
      }
      auto it = find_in_lang(rkey);
      if (!it.has_value()) rres = key.substr(beg, end - beg + 1), logger::localization.warn("Missing localization for key '" + rkey + "'.");
      else rres = format(it.value(), rarg, recursion_depth + 1);
    }
    result += rres;
    ptr = end + 1;
  }
  result += key.substr(ptr);
  return result;
}

bool load_language_file(const std::string &filename) {
  localization lang;
  std::ifstream f(filename);
  if (!f.is_open()) {
    logger::localization.error("Failed to open language file when loading: " + filename + ".");
    return false;
  }
  std::string line;
  while (std::getline(f, line)) {
    inplace_trim(line);
    if (line.empty() || line.front() == ':') continue;
    size_t sep_pos = line.find(' ');
    if (sep_pos != std::string::npos) {
      std::string key = line.substr(0, sep_pos);
      std::string value = line.substr(sep_pos + 1);
      if (lang.count(key)) {
        logger::localization.warn("Duplicate localization key '" + key + "' in file '" + filename + "'.");
      }
      lang[key] = value;
    } else {
      logger::localization.warn("Value not given in file '" + filename + "'.");
    }
  }
  f.close();
  auto lang_id = filename.substr(filename.rfind('/') + 1);
  lang_id = lang_id.substr(0, filename.find('.'));
  if (lang.count("!language.id")) lang_id = lang["!language.id"];
  else lang["!language.id"] = lang_id, logger::localization.warn("Language in file '" + filename + "' is missing an ID.");
  if (!lang.count("!language.name")) lang["!language.name"] = "Unnamed language (" + lang_id + ")", logger::localization.warn("Language in file '" + filename + "' is missing a name.");
  if (!lang.count("!language.name_local")) lang["!language.name_local"] = lang["!language.name"], logger::localization.warn("Language in file '" + filename + "' is missing a localized name.");
  if (languages.count(lang_id)) {
    logger::localization.error("Language in file '" + filename + "' has a duplicate id '" + lang_id + "'.");
    return false;
  }
  languages[lang_id] = lang;
  return true;
}
bool export_language_file(const std::string &filename, const localization &lang) {
  std::ofstream f(filename);
  if (!f.is_open()) {
    logger::localization.error("Failed to open language file when exporting: " + filename);
    return false;
  }
  f << " Exported language file\n";
  for (const auto &pair : lang) {
    f << pair.first << " " << pair.second << "\n";
  }
  f.close();
  return true;
}

int load_all_from_path(const std::string &path) {
  if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
    logger::localization.error("Specified language file path does not exist: " + path);
    return -1;
  }
  int total_file_loaded = 0;
  try {
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
      if (std::filesystem::is_regular_file(entry.status())) {
        bool result = load_language_file(entry.path());
        if (result) ++total_file_loaded;
      }
    }
  } catch (std::filesystem::filesystem_error e) {
    logger::localization.error("Internal error when loading language file from path '" + path + "': " + e.what());
    return -1;
  }
  logger::localization.info(std::to_string(total_file_loaded) + " language files loaded.");
  return total_file_loaded;
}
bool set_language() {
  bool ok = true;
  lang.clear();
  auto l = config::get<config::list_entry>("system.lang");
  for (auto i : l.a) {
    auto la = std::get<std::string>(i);
    if (!languages.count(la))
      logger::localization.warn("Language '" + la + "' set in config not found."), ok = false;
    else lang.push_back(&languages[la]);
  }
  lang.push_back(&languages["en"]); // fallback
  return ok;
}
}
