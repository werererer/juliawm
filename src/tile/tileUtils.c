#include "tile/tileUtils.h"
#include <client.h>
#include <execinfo.h>
#include <string.h>
#include <sys/param.h>
#include <wayland-util.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>
#include <wlr/types/wlr_box.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <stdlib.h>
#include <wlr/util/log.h>

#include "container.h"
#include "root.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "utils/gapUtils.h"
#include "utils/parseConfigUtils.h"
#include "event_handler.h"

void arrange()
{
    struct monitor *m;
    wl_list_for_each(m, &mons, link) {
        arrange_monitor(m);
    }

    update_cursor(&server.cursor);
    root_damage_whole(m->root);
}

// TODO what does this fucntion even do?
static int get_layout_container_count(struct workspace *ws)
{
    struct layout *lt = &ws->layout[0];
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_copy_data_ref);

    int len = luaL_len(L, -1);
    int container_count = get_container_count(ws);
    int n = MAX(MIN(len, container_count), 1);

    lua_rawgeti(L, -1, n);

    // TODO refactor
    len = luaL_len(L, -1);
    n = MAX(MIN(len, n), 1);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lt->lua_layout_ref);
    lua_pop(L, 1);
    return n;
}

/* update layout and was set in the arrange function */
// TODO what does this fucntion even do?
static void update_layout_counters(lua_State *L, struct monitor *m)
{
    struct workspace *ws = get_workspace_on_monitor(m);
    struct layout *lt = get_layout_on_monitor(m);

    lt->n = get_layout_container_count(ws);
    lt->nmaster_abs = get_master_container_count(ws);
    lt->n_abs = lt->n + lt->nmaster_abs-1;
}

static struct wlr_fbox lua_unbox_layout_geom(lua_State *L, int i) {
    struct wlr_fbox geom;

    if (luaL_len(L, -1) < i)
        wlr_log(WLR_ERROR, "index to high: index %i len %lli", i, luaL_len(L, -1));

    lua_rawgeti(L, -1, i);

    lua_rawgeti(L, -1, 1);
    geom.x = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    lua_rawgeti(L, -1, 2);
    geom.y = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    lua_rawgeti(L, -1, 3);
    geom.width = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    lua_rawgeti(L, -1, 4);
    geom.height = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_pop(L, 1);
    return geom;
}

/* update layout and was set in the arrange function */
static void apply_nmaster_transformation(struct wlr_box *box, struct layout *lt, int position)
{
    if (position > lt->nmaster)
        return;

    // get layout
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->options.master_layout_data_ref);
    int len = luaL_len(L, -1);
    int g = MIN(lt->nmaster_abs, lt->nmaster);
    g = MAX(MIN(len, g), 1);
    lua_rawgeti(L, -1, g);
    int k = MIN(position, g);
    struct wlr_fbox geom = lua_unbox_layout_geom(L, k);
    lua_pop(L, 1);
    lua_pop(L, 1);

    struct wlr_box obox = get_absolute_box(geom, *box);
    memcpy(box, &obox, sizeof(struct wlr_box));
}

static struct wlr_box get_geom_in_layout(lua_State *L, struct layout *lt, struct root *root, int arrange_position)
{
    // relative position
    int n = MAX(0, arrange_position+1 - lt->nmaster) + 1;

    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_ref);
    struct wlr_fbox rel_geom = lua_unbox_layout_geom(L, n);
    lua_pop(L, 1);

    struct wlr_box box = get_absolute_box(rel_geom, root->geom);

    // TODO fix this function, hard to read
    apply_nmaster_transformation(&box, lt, arrange_position+1);
    return box;
}

int get_slave_container_count(struct workspace *ws)
{
    struct layout *lt = &ws->layout[0];
    int abs_count = get_tiled_container_count(ws);
    return MAX(abs_count - lt->nmaster, 0);
}

int get_master_container_count(struct workspace *ws)
{
    int abs_count = get_tiled_container_count(ws);
    int slave_container_count = get_slave_container_count(ws);
    return MAX(abs_count - slave_container_count, 0);
}

// amount of slave containers plus the one master area
int get_container_count(struct workspace *ws)
{
    return get_slave_container_count(ws) + 1;
}

void update_container_positions(struct monitor *m)
{
    struct container *con;
    int position = 0;

    wl_list_for_each(con, &containers, mlink) {
        if (!existon(con, &server.workspaces, m->ws_ids[0]))
            continue;
        if (con->floating)
            continue;
        if (con->client->type == LAYER_SHELL)
            continue;

        con->position = position;

        apply_rules(con);

        // then use the layout that may have been reseted
        position++;
    }

    wl_list_for_each(con, &containers, mlink) {
        if (!existon(con, &server.workspaces, m->ws_ids[0]))
            continue;
        if (!con->floating)
            continue;
        if (con->client->type == LAYER_SHELL)
            continue;

        con->position = position;

        apply_rules(con);

        // then use the layout that may have been reseted
        position++;
    }
}

static void update_container_focus_stack_positions(struct monitor *m)
{
    int position = 0;
    struct container *con;
    wl_list_for_each(con, &focus_stack, flink) {
        if (!visibleon(con, &server.workspaces, m->ws_ids[0]))
            continue;
        if (con->floating)
            continue;
        if (con->client->type == LAYER_SHELL)
            continue;

        con->focus_stack_position = position;
        // then use the layout that may have been reseted
        position++;
    }
}

void arrange_monitor(struct monitor *m)
{
    struct layout *lt = get_layout_on_monitor(m);

    set_root_area(m->root, m->geom);
    container_surround_gaps(&m->root->geom, lt->options.outer_gap);

    update_layout_counters(L, m);
    call_update_function(&lt->options.event_handler, lt->n);

    update_hidden_containers(m);
    update_container_focus_stack_positions(m);
    update_container_positions(m);

    struct container *con;
    if (lt->options.arrange_by_focus) {
        wl_list_for_each(con, &focus_stack, flink) {
            if (!visibleon(con, &server.workspaces, m->ws_ids[0]))
                continue;
            if (con->floating)
                continue;
            if (con->client->type == LAYER_SHELL)
                continue;

            arrange_container(con, con->focus_stack_position);
        }
    } else {
        wl_list_for_each(con, &containers, mlink) {
            if (!visibleon(con, &server.workspaces, m->ws_ids[0]))
                continue;
            if (con->floating)
                continue;

            arrange_container(con, con->position);
        }
    }
}

void arrange_container(struct container *con, int arrange_position)
{
    if (con->floating || con->hidden)
        return;

    struct monitor *m = con->m;
    struct workspace *ws = get_workspace_on_monitor(m);
    struct layout *lt = &ws->layout[0];
    // the 1 added represents the master area

    struct wlr_box geom = get_geom_in_layout(L, lt, m->root, arrange_position);

    container_surround_gaps(&geom, lt->options.inner_gap);
    // since gaps are halfed we need to multiply it by 2
    container_surround_gaps(&geom, 2*lt->options.tile_border_px);

    resize(con, geom);
}

void resize(struct container *con, struct wlr_box geom)
{
    /*
     * Note that I took some shortcuts here. In a more fleshed-out
     * compositor, you'd wait for the client to prepare a buffer at
     * the new size, then commit any movement that was prepared.
     */
    set_container_geom(con, geom);
    con->client->resize = true;

    bool preserve_ratio = con->ratio != 0;

    if (preserve_ratio) {
        /* calculated biggest container where con->geom.width and
         * con->geom.height = con->geom.width * con->ratio is inside geom.width
         * and geom.height
         * */
        float max_height = geom.height/con->ratio;
        con->geom.width = MIN(geom.width, max_height);
        con->geom.height = con->geom.width * con->ratio;
        // TODO make a function out of that 
        // center in x direction
        con->geom.x += (geom.width - con->geom.width)/2;
        // center in y direction
        con->geom.y += (geom.height - con->geom.height)/2;
    }

    apply_bounds(con, *wlr_output_layout_get_box(server.output_layout, NULL));

    /* wlroots makes this a no-op if size hasn't changed */
    switch (con->client->type) {
        case XDG_SHELL:
            wlr_xdg_toplevel_set_size(con->client->surface.xdg,
                    con->geom.width, con->geom.height);
            break;
        case LAYER_SHELL:
            wlr_layer_surface_v1_configure(con->client->surface.layer,
                    con->m->geom.width,
                    con->m->geom.height);
            break;
        case X11_UNMANAGED:
        case X11_MANAGED:
            wlr_xwayland_surface_configure(con->client->surface.xwayland,
                    con->geom.x, con->geom.y, con->geom.width,
                    con->geom.height);
    }
}

void update_hidden_containers(struct monitor *m)
{
    struct container *con;
    struct workspace *ws = get_workspace_on_monitor(m);
    // because the master are is included in n aswell as nmaster we have to
    // subtract the solution by one to count
    struct layout *lt = &ws->layout[0];

    int i = 1;
    if (ws->layout[0].options.arrange_by_focus) {
        wl_list_for_each(con, &focus_stack, flink) {
            if (con->floating)
                continue;
            if (!existon(con, &server.workspaces, ws->id))
                continue;
            if (con->client->type == LAYER_SHELL)
                continue;

            con->hidden = i > lt->n_abs;
            i++;
        }
    } else {
        wl_list_for_each(con, &containers, mlink) {
            if (con->floating)
                continue;
            if (!existon(con, &server.workspaces, ws->id))
                continue;
            if (con->client->type == LAYER_SHELL)
                continue;

            con->hidden = i > lt->n_abs;
            i++;
        }
    }
}

int get_tiled_container_count(struct workspace *ws)
{
    struct container *con;
    int n = 0;

    wl_list_for_each(con, &containers, mlink) {
        if (con->floating)
            continue;
        if (!existon(con, &server.workspaces, ws->id))
            continue;
        if (con->client->type == LAYER_SHELL)
            continue;
        n++;
    }
    return n;
}
