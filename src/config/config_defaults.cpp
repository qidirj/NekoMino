#include "config.h"
#include "../constants.h"

namespace config {
#define dict map_entry({{
#define _end }}) // to match brackets
#define list list_entry({{
#define end }})
const map_entry default_config = dict
  { "display", dict
    { "window", dict
      { "width", 320 },
      { "height", 240 },
    end }, 
  end },
  { "system", dict
    { "lang", "en" },
  end },
  { "demo", dict
    { "sizemult", 0.5 },
    { "game", dict
      { "das", 150 },
#if DEBUG_VER == 5
      { "break_it!" , -1 },
#endif
      { "arr", 50 },
      { "dcd", 33 },
      { "are", 0 },
      { "are_clear", 500 },
      { "are_line", 250 },
      { "sddas", 100 },
      { "sdarr", 100 },
      { "gravity", 1000 },
      { "lock_delay", 500 },
      { "irs", "ixs_hold" },
      { "ihs", "ixs_hold" },
      { "ims", "ixs_hold" },
      { "infinite_hold", false },
      { "lock_refresh", 15 },
      { "lock_refresh_hold", 15 },
      { "keybinding", dict
        { "op_left" , list { "key_Left" } end },
#if DEBUG_VER == 5
        { "break_it!" , list { "break_it!", 114514 } end },
#endif
        { "op_right", list { "key_Right" } end },
        { "op_sd"   , list { "key_Down" } end },
        { "op_hd"   , list { "key_Space" } end },
        { "op_cw"   , list { "key_X" } end },
        { "op_ccw"  , list { "key_Z" } end },
        { "op_180"  , list { "key_A" } end },
        { "op_hold" , list { "key_LShift", "key_RShift" } end },
      end },
    end },
  end },
end;
#undef dict
#undef list
#undef _end
#undef end
}
