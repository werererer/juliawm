#ifndef LAYOUT_H
#define LAYOUT_H

#include <stdbool.h>
#include <lua.h>
#include <lauxlib.h>
#include "options.h"

#include "layout_set.h"

struct layout {
    const char *name;
    const char *symbol;
    // the amount of slave windows plus the master are (+1)
    int n;
    int n_abs;
    // the absolute amount of windows
    int nmaster_abs;
    // the amount master windows
    int nmaster;
    int lua_layout_ref;
    int lua_layout_copy_data_ref;
    int lua_layout_original_copy_data_ref;

    struct options options;
};

bool is_same_layout(struct layout layout, struct layout layout2);
bool lua_islayout_data(lua_State *L, const char *name);
void lua_copy_table(lua_State *L, int *ref);
struct resize_constraints lua_toresize_constrains(lua_State *L);
void push_layout(struct layout lt_stack[static 2], struct layout lt);
struct layout copy_layout(struct layout *src_lt);
struct layout get_default_layout();
#endif /* LAYOUT_H */
