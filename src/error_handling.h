#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <ctime>
#include <fstream>
#include <stdexcept>
#include <string>
namespace logger {
  class logger {
    std::ofstream f; bool workwell = true, last = false;
#ifdef DEBUG
    int mask = 0b11111; // fatal / error / warn / info / debug
#else
    int mask = 0b00111; // fatal / error / warn / info / debug
#endif
    public:
      logger(const std::string &filename, bool ismeta = false) : f(filename, std::ios::app) {
        workwell = workwell && f.is_open();
        if (ismeta && !workwell) throw std::runtime_error("Failed to open meta log file: " + filename);
        // else meta.error("Failed to open log file: " + filename);
      }
      bool ok() { return workwell; }
      std::ofstream &raw() { return f; }
      void setmask(int m) { mask = m & 0b11111; }

      void fatal(const std::string &msg) { if ((last = (mask & 0b00001))) f << time(0) << " [FATAL] " << msg << std::endl; }
      void error(const std::string &msg) { if ((last = (mask & 0b00010))) f << time(0) << " [ERROR] " << msg << std::endl; }
      void warn (const std::string &msg) { if ((last = (mask & 0b00100))) f << time(0) << " [WARN]  " << msg << std::endl; }
      void info (const std::string &msg) { if ((last = (mask & 0b01000))) f << time(0) << " [INFO]  " << msg << std::endl; }
      void debug(const std::string &msg) { if ((last = (mask & 0b10000))) f << time(0) << " [DEBUG] " << msg << std::endl; }
      void note (const std::string &msg) { if ( last                    ) f <<  "           [NOTE] " << msg << std::endl; }
  };
  extern logger meta, localization, rs_load, tetris_core, interface;
}

#endif
