#include "actions.h"
#include <unistd.h>
#include "client.h"
#include "tile/tileUtils.h"
#include "server.h"

static struct client *grabc = NULL;
static int grabcx, grabcy; /* client-relative */

static void setFloating(struct client *c, bool floating);
static void pointerfocus(struct client *c, struct wlr_surface *surface,
        double sx, double sy, uint32_t time);

static void pointerfocus(struct client *c, struct wlr_surface *surface,
        double sx, double sy, uint32_t time)
{
    /* Use top level surface if nothing more specific given */
    if (c && !surface)
        surface = getWlrSurface(c);

    /* If surface is NULL, clear pointer focus */
    if (!surface) {
        wlr_seat_pointer_notify_clear_focus(seat);
        return;
    }

    /* If surface is already focused, only notify of motion */
    if (surface == seat->pointer_state.focused_surface) {
        wlr_seat_pointer_notify_motion(seat, time, sx, sy);
        return;
    }
    /* Otherwise, let the client know that the mouse cursor has entered one
     * of its surfaces, and make keyboard focus follow if desired. */
    wlr_seat_pointer_notify_enter(seat, surface, sx, sy);

    if (c->type == X11Unmanaged)
        return;

    if (sloppyFocus)
        focusClient(selClient(), c, false);
}


int spawn(lua_State *L)
{
    if (fork() == 0) {
        setsid();
        const char *cmd = luaL_checkstring(L, -1);
        execl("/bin/sh", "/bin/sh", "-c", cmd, (void *)NULL);
    }
    lua_pop(L, 1);
    return 0;
}

int updateLayout(lua_State *L)
{
    struct layout l = getConfigLayout(L, "layout");
    printf("symbol %s: \n", l.symbol);
    setSelLayout(&selMon->tagset, l);
    arrange(selMon, true);
    return 0;
}

int focusOnStack(lua_State *L)
{
    struct client *c, *sel = selClient();
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    if (!sel)
        return 0;
    if (i > 0) {
        int j = 1;
        wl_list_for_each(c, &sel->link, link) {
            if (visibleon(c, selMon))
                break;  /* found it */
            j++;
        }
    } else {
        wl_list_for_each_reverse(c, &sel->link, link) {
            if (visibleon(c, selMon))
                break;  /* found it */
        }
    }
    /* If only one client is visible on selMon, then c == sel */
    focusClient(sel, c, true);
    return 0;
}

int focusOnHiddenStack(lua_State *L)
{
    struct client *c, *sel = selClient();
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    if (!sel)
        return 0;
    if (i > 0) {
        int j = 1;
        wl_list_for_each(c, &sel->link, link) {
            if (hiddenon(c, selMon))
                break;  /* found it */
            j++;
        }
    } else {
        wl_list_for_each_reverse(c, &sel->link, link) {
            if (hiddenon(c, selMon))
                break;  /* found it */
        }
    }
    if (sel && c) {
        if (sel == c)
            return 0;
        // replace current client with a hidden one
        wl_list_remove(&c->link);
        wl_list_insert(&sel->link, &c->link);
        wl_list_remove(&sel->link);
        wl_list_insert(clients.prev, &sel->link);
    }
    /* If only one client is visible on selMon, then c == sel */
    focusClient(sel, c, true);
    arrange(selMon, false);
    return 0;
}

int moveResize(lua_State *L)
{
    int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    grabc = xytoclient(server.cursor->x, server.cursor->y);
    if (!grabc)
        return 0;

    /* Float the window and tell motionnotify to grab it */
    setFloating(grabc, true);
    switch (server.cursorMode = ui) {
        case CurMove:
            grabcx = server.cursor->x - grabc->geom.x;
            grabcy = server.cursor->y - grabc->geom.y;
            wlr_xcursor_manager_set_cursor_image(server.cursorMgr,
                    "fleur", server.cursor);
            break;
        case CurResize:
            /* Doesn't work for X11 output - the next absolute motion event
             * returns the cursor to where it started */
            wlr_cursor_warp_closest(server.cursor, NULL,
                    grabc->geom.x + grabc->geom.width,
                    grabc->geom.y + grabc->geom.height);
            wlr_xcursor_manager_set_cursor_image(server.cursorMgr,
                    "bottom_right_corner", server.cursor);
            break;
        default:
            break;
    }
    return 0;
}

static void setFloating(struct client *c, bool floating)
{
    if (c->floating == floating)
        return;
    c->floating = floating;
    arrange(c->mon, false);
}

void motionnotify(uint32_t time)
{
    double sx = 0, sy = 0;
    struct wlr_surface *surface = NULL;
    struct client *c;

    /* Update selMon (even while dragging a window) */
    if (sloppyFocus)
        selMon = xytomon(server.cursor->x, server.cursor->y);

    /* If we are currently grabbing the mouse, handle and return */
    switch (server.cursorMode) {
        case CurMove:
            /* Move the grabbed client to the new position. */
            resize(grabc, server.cursor->x - grabcx, server.cursor->y - grabcy,
                    grabc->geom.width, grabc->geom.height, true);
            return;
            break;
        case CurResize:
            resize(grabc, grabc->geom.x, grabc->geom.y,
                    server.cursor->x - grabc->geom.x,
                    server.cursor->y - grabc->geom.y, true);
            return;
            break;
        default:
            break;
    }

    if ((c = xytoclient(server.cursor->x, server.cursor->y))) {
            surface = wlr_surface_surface_at(getWlrSurface(c),
                    server.cursor->x - c->geom.x - c->bw,
                    server.cursor->y - c->geom.y - c->bw, &sx, &sy);
    }

    /* If there's no client surface under the server.cursor, set the cursor image to a
     * default. This is what makes the cursor image appear when you move it
     * off of a client or over its border. */
    if (!surface) {
        wlr_xcursor_manager_set_cursor_image(server.cursorMgr,
                "left_ptr", server.cursor);
    }

    pointerfocus(c, surface, sx, sy, time);
}


int toggleFloating(lua_State *L)
{
    struct client *sel = selClient();
    if (!sel)
        return 0;
    /* return if fullscreen */
    setFloating(sel, !sel->floating /* || sel->isfixed */);
    return 0;
}

int moveClient(lua_State *L)
{
    resize(grabc, server.cursor->x - grabcx, server.cursor->y - grabcy,
            grabc->geom.width, grabc->geom.height, 1);
    return 0;
}

int resizeClient(lua_State *L)
{
    resize(grabc, grabc->geom.x, grabc->geom.y,
            server.cursor->x - grabc->geom.x,
            server.cursor->y - grabc->geom.y, 1);
    return 0;
}

int quit(lua_State *L)
{
    wl_display_terminate(server.display);
    return 0;
}

int zoom(lua_State *L)
{
    struct client *c, *old = selClient();

    if (!old || old->floating)
        return 0;

    /* Search for the first tiled window that is not sel, marking sel as
     * NULL if we pass it along the way */
    wl_list_for_each(c, &clients,
            link) if (visibleon(c, selMon) && !c->floating) {
        if (c != old)
            break;
        old = NULL;
    }

    /* Return if no other tiled window was found */
    if (&c->link == &clients)
        return 0;

    /* If we passed sel, move c to the front; otherwise, move sel to the
     * front */
    if (!old)
        old = c;
    wl_list_remove(&old->link);
    wl_list_insert(&clients, &old->link);

    focusClient(nextClient(), old, true);
    arrange(selMon, false);
    return 0;
}
