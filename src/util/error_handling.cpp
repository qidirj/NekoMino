#include "error_handling.h"
// #include <iostream>
namespace logger {
  // auto temp = (std::clog << "temp" << std::endl, 0);
  logger meta("logs/nekomino.log", true),
        localization("logs/localization.log"),
        rs_load("logs/rs_load.log"),
        tetris_core("logs/tetris_core.log"),
        interface("logs/ui_interface.log"),
        config("logs/config.log");
}
