#ifndef DEBUG_HANDLER_H
#define DEBUG_HANDLER_H

#define DEBUG_VER 2 // update when previous debug messages arent used anymore
#ifdef DEBUG
#define LOG(v) if constexpr (v >= DEBUG_VER) std::clog
#else
#define LOG(v) if constexpr (false) std::clog
#endif

#define GAME_VER_MAJOR 0
#define GAME_VER_MINOR 0
#define GAME_VER_PATCH 26051701

#endif
