#include "error_handling.h"
// #include <iostream>
namespace logger {
  // auto temp = (std::clog << "temp" << std::endl, 0);
  logger meta("nekomino.log", true),
        localization("localization.log"),
        rs_load("rs_load.log"),
        tetris_core("tetris_core.log");
}
