#ifndef DEBUG_HANDLER_H
#define DEBUG_HANDLER_H

#include "util/enumerator.h"
#include "tetris_core/minoes.h" // for enum Minoes
#include <SFML/Window/Keyboard.hpp>

#define DEBUG_VER 6 // update when previous debug messages arent used anymore
#ifdef DEBUG
#define LOG(v) if constexpr (v >= DEBUG_VER) std::clog
#else
#define LOG(v) if constexpr (false) std::clog
#endif

#define GAME_VER_MAJOR 0
#define GAME_VER_MINOR 0
#define GAME_VER_PATCH 26052101
#define GAME_VER_PATCH_FULL "20260521a 'config update but half done'"
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define GAME_VER_STRING "v" STR(GAME_VER_MAJOR) "." STR(GAME_VER_MINOR) "." GAME_VER_PATCH_FULL

#define AUTHOR_CONTACT "QQ. 3846938522; Github. @qidirj (repo: qidirj/NekoMino); Mail: nekodaiari@outlook.com | Available language: Chinese(preferred), English"

declare_enum(Key, key_A, key_B, key_C, key_D, key_E, key_F, key_G, key_H, key_I, key_J, key_K, key_L, key_M, key_N, key_O, key_P, key_Q, key_R, key_S, key_T, key_U, key_V, key_W, key_X, key_Y, key_Z, key_Num0, key_Num1, key_Num2, key_Num3, key_Num4, key_Num5, key_Num6, key_Num7, key_Num8, key_Num9, key_Escape, key_LControl, key_LShift, key_LAlt, key_LSystem, key_RControl, key_RShift, key_RAlt, key_RSystem, key_Menu, key_LBracket, key_RBracket, key_Semicolon, key_Comma, key_Period, key_Apostrophe, key_Slash, key_Backslash, key_Grave, key_Equal, key_Hyphen, key_Space, key_Enter, key_Backspace, key_Tab, key_PageUp, key_PageDown, key_End, key_Home, key_Insert, key_Delete, key_Add, key_Subtract, key_Multiply, key_Divide, key_Left, key_Right, key_Up, key_Down, key_Numpad0, key_Numpad1, key_Numpad2, key_Numpad3, key_Numpad4, key_Numpad5, key_Numpad6, key_Numpad7, key_Numpad8, key_Numpad9, key_F1, key_F2, key_F3, key_F4, key_F5, key_F6, key_F7, key_F8, key_F9, key_F10, key_F11, key_F12, key_F13, key_F14, key_F15, key_Pause)

declare_enum(SpinType, spin_none, spin_mini, spin_full)
declare_enum(PCType, pc_none, pc_half, pc_color, pc_full)
declare_enum(ZoneType, zone_none, zone_stack, zone_sink, zone_placeholder, zone_ended)
// zone_none, // no zone
// zone_stack, // do not clear the line
// zone_sink, // when clear, sink the cleared line to the bottom

declare_enum(Operation, op_left, op_right, op_sd, op_hd, op_cw, op_ccw, op_180, op_hold, op_lmost, op_rmost, op_id, op__Last)
declare_enum(IXSMode, ixs_none, ixs_tap, ixs_hold)

template <> inline Minoes from_string<Minoes>(const std::string &id) { return get_mino_id(id); }
template <> inline std::string to_string<Minoes>(Minoes e) { return get_mino_name(e); }
register_enum(Minoes)

#endif
