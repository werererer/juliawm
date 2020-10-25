#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <wayland-util.h>

#include "tile/tile.h"
#include "utils/coreUtils.h"

//global variables
struct wl_list containers; /* tiling order */
struct wl_list focusStack;  /* focus order */
struct wl_list stack;   /* stacking z-order */
struct wlr_output_layout *output_layout;
struct wlr_box sgeom;

/* function implementations */
void applybounds(struct client *c, struct wlr_box bbox)
{
    /* set minimum possible */
    c->geom.width = MAX(1, c->geom.width);
    c->geom.height = MAX(1, c->geom.height);

    if (c->geom.x >= bbox.x + bbox.width)
        c->geom.x = bbox.x + bbox.width - c->geom.width;
    if (c->geom.y >= bbox.y + bbox.height)
        c->geom.y = bbox.y + bbox.height - c->geom.height;
    if (c->geom.x + c->geom.width + 2 * c->bw <= bbox.x)
        c->geom.x = bbox.x;
    if (c->geom.y + c->geom.height + 2 * c->bw <= bbox.y)
        c->geom.y = bbox.y;
}

void applyrules(struct client *c)
{
    const char *appid, *title;
    unsigned int i, newtags = 0;
    const struct rule *r;
    struct monitor *m; 
    /* rule matching */
    c->floating = false;
    switch (c->type) {
        case XDGShell:
            appid = c->surface.xdg->toplevel->app_id;
            title = c->surface.xdg->toplevel->title;
            break;
        case LayerShell:
            appid = "test";
            title = "test";
            break;
        case X11Managed:
        case X11Unmanaged:
            appid = c->surface.xwayland->class;
            title = c->surface.xwayland->title;
            break;
    }
    if (!appid)
        appid = "broken";
    if (!title)
        title = "broken";

    for (r = rules; r < END(rules); r++) {
        if ((!r->title || strstr(title, r->title))
                && (!r->id || strstr(appid, r->id))) {
            c->floating = r->floating;
            newtags |= r->tags;
            i = 0;
            wl_list_for_each(m, &mons, link)
                if (r->monitor == i++)
                    selMon = m;
        }
    }
}

struct client *selClient()
{
    if (wl_list_length(&focusStack))
    {
        struct client *c = wl_container_of(focusStack.next, c, flink);
        if (!visibleon(c, selMon))
            return NULL;
        else
            return c;
    } else {
        return NULL;
    }
}

struct client *nextClient()
{
    if (wl_list_length(&focusStack) >= 2)
    {
        struct client *c = wl_container_of(focusStack.next->next, c, flink);
        if (!visibleon(c, selMon))
            return NULL;
        else
            return c;
    } else {
        return NULL;
    }
}

struct client *prevClient()
{
    if (wl_list_length(&focusStack) >= 2)
    {
        struct client *c = wl_container_of(focusStack.prev, c, flink);
        if (!visibleon(c, selMon))
            return NULL;
        else
            return c;
    } else {
        return NULL;
    }
}

struct client *getClient(int i)
{
    struct client *c;

    if (abs(i) > wl_list_length(&focusStack))
        return NULL;
    if (i == 0)
    {
        c = selClient();
    } else if (i > 0) {
        struct wl_list *pos = &focusStack;
        while (i > 0) {
            if (pos->next)
                pos = pos->next;
            i--;
        }
        c = wl_container_of(pos, c, flink);
    } else { // i < 0
        struct wl_list *pos = &focusStack;
        while (i < 0) {
            pos = pos->prev;
            i++;
        }
        c = wl_container_of(pos, c, flink);
    }
    return c;
}

struct wlr_surface *getWlrSurface(struct client *c)
{
    switch (c->type) {
        case XDGShell:
            return c->surface.xdg->surface;
            break;
        case LayerShell:
            return c->surface.layer->surface;
            break;
        case X11Managed:
        case X11Unmanaged:
            return c->surface.xwayland->surface;
        default:
            printf("wlr_surface is not supported: \n");
            return NULL;
    }
}

bool visibleon(struct client *c, struct monitor *m)
{
    if (m && c) {
        bool sameMon = c->mon == m;
        bool sameTag = c->mon->tagset.selTags[0] & (1 << m->tagset.focusedTag);
        return sameMon && sameTag;
    }
    return false;
}

static void unfocusClient(struct client *c)
{
    if (c) {
        switch (c->type) {
            case XDGShell:
                wlr_xdg_toplevel_set_activated(c->surface.xdg, false);
                break;
            case X11Managed:
            case X11Unmanaged:
                wlr_xwayland_surface_activate(c->surface.xwayland, false);
                break;
            default:
                break;
        }
    }
}

void focusClient(struct client *old, struct client *c, bool lift)
{
    struct wlr_keyboard *kb = wlr_seat_get_keyboard(seat);
    /* Raise client in stacking order if requested */
    if (c && lift) {
        wl_list_remove(&c->slink);
        wl_list_insert(&stack, &c->slink);
    }
    unfocusClient(old);
    /* Update wlroots' keyboard focus */
    if (!c) {
        /* With no client, all we have left is to clear focus */
        wlr_seat_keyboard_notify_clear_focus(seat);
        return;
    }

    /* Have a client, so focus its top-level wlr_surface */
    wlr_seat_keyboard_notify_enter(seat, getWlrSurface(c), kb->keycodes, 
            kb->num_keycodes, &kb->modifiers);

    /* Put the new client atop the focus stack */
    wl_list_remove(&c->flink);
    wl_list_insert(&focusStack, &c->flink);

    /* Activate the new client */
    switch (c->type) {
        case XDGShell:
            wlr_xdg_toplevel_set_activated(c->surface.xdg, true);
            break;
        case X11Managed:
        case X11Unmanaged:
            wlr_xwayland_surface_activate(c->surface.xwayland, true);
            break;
        default:
            break;
    }
}

void focusTopClient(struct client *old, bool lift)
{
    struct client *c;
    bool focus = false;

    // focus_stack should not be changed while iterating
    wl_list_for_each(c, &focusStack, flink)
        if (visibleon(c, selMon)) {
            focus = true;
            break;
        }
    if (focus)
        focusClient(old, c, lift);
}
