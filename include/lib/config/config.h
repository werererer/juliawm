#ifndef CONFIG_H
#define CONFIG_H

#include "options.h"
#include <lauxlib.h>
#include <lua.h>

int lib_reload(lua_State *L);
int lib_set_arrange_by_focus(lua_State *L);
int lib_set_border_color(lua_State *L);
int lib_set_buttons(lua_State *L);
int lib_set_default_layout(lua_State *L);
int lib_set_default_layout(lua_State *L);
int lib_set_float_borderpx(lua_State *L);
int lib_set_focus_color(lua_State *L);
int lib_set_inner_gaps(lua_State *L);
int lib_set_outer_gaps(lua_State *L);
int lib_set_keybinds(lua_State *L);
int lib_set_layout_constraints(lua_State *L);
int lib_create_layout_set(lua_State *L);
int lib_set_master_constraints(lua_State *L);
int lib_set_master_layout_data(lua_State *L);
int lib_set_mod(lua_State *L);
int lib_set_monrules(lua_State *L);
int lib_set_repeat_delay(lua_State *L);
int lib_set_repeat_rate(lua_State *L);
int lib_set_resize_data(lua_State *L);
int lib_set_resize_direction(lua_State *L);
int lib_set_root_color(lua_State *L);
int lib_set_rules(lua_State *L);
int lib_set_sloppy_focus(lua_State *L);
int lib_set_tile_borderpx(lua_State *L);
int lib_create_workspaces(lua_State *L);

#endif /* CONFIG_H */
