#ifndef DEBUG_HANDLER_H
#define DEBUG_HANDLER_H

#define DEBUG_VER 4 // update when previous debug messages arent used anymore
#ifdef DEBUG
#define LOG(v) if constexpr (v >= DEBUG_VER) std::clog
#else
#define LOG(v) if constexpr (false) std::clog
#endif

#define GAME_VER_MAJOR 0
#define GAME_VER_MINOR 0
#define GAME_VER_PATCH 26051902
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define GAME_VER_STRING "v" STR(GAME_VER_MAJOR) "." STR(GAME_VER_MINOR) "." STR(GAME_VER_PATCH)

#endif
