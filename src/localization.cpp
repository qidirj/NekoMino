#include "error_handling.h"
#include "localization.h"

std::map<string, localization> languages = {
  {
    "en", {
      { "gameTitle", "Neko Mino" },
      { "frameFull", "Frame(s)" },
      { "dasFull", "Auto Shift Delay" },
      { "das", "ASD" },
      { "arrFull", "Auto Shift Period" },
      { "arr", "ASP" },
      { "configured", "Configured {#0}" }
    }
  }
};
localization *lang = &languages["en"]; // use pointer to prevent copying

string format(const string &key, const std::vector<string> &args, int recursion_depth) {
  if (recursion_depth >= 10) {
    logger::localization.warn("Recursion depth limit exceeded in localization format function.");
    logger::localization.note("The limit is hardcoded to 10 in current version.");
    return key; // Return the key as is to avoid infinite recursion
  }
  size_t ptr = 0, beg; string result = "";
  auto conv = [](const string &s) -> size_t {
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
    string subkey = key.substr(beg + 1, end - beg - 1), rres = "";
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
      string rkey = subkey; std::vector<string> rarg;
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
      auto it = lang->find(rkey);
      if (it == lang->end()) rres = key.substr(beg, end - beg + 1), logger::localization.warn("Missing localization for key '" + rkey + "'.");
      else rres = format(it->second, rarg, recursion_depth + 1);
    }
    result += rres;
    ptr = end + 1;
  }
  result += key.substr(ptr);
  return result;
}

localization load_language_file(const string &filename) {
  localization lang;
  std::ifstream f(filename);
  if (!f.is_open()) {
    logger::localization.error("Failed to open language file when loading: " + filename);
    return lang;
  }
  std::string line;
  while (std::getline(f, line)) {
    size_t sep_pos = line.find(' ');
    if (sep_pos != std::string::npos) {
      std::string key = line.substr(0, sep_pos);
      std::string value = line.substr(sep_pos + 1);
      if (!key.empty() && key[0] != ':') {
        if (lang.count(key)) {
          logger::localization.warn("Duplicate localization key '" + key + "' in file: " + filename);
        }
        lang[key] = value;
      }
    }
  }
  f.close();
  return lang;
}
void export_language_file(const string &filename, const localization &lang) {
  std::ofstream f(filename);
  if (!f.is_open()) {
    logger::localization.error("Failed to open language file when exporting: " + filename);
    return;
  }
  f << " Exported language file\n";
  for (const auto &pair : lang) {
    f << pair.first << " " << pair.second << "\n";
  }
  f.close();
  return;
}
