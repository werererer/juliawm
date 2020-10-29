#ifndef CLIENT
#define CLIENT
#include <wlr/types/wlr_layer_shell_v1.h>
#include <X11/Xlib.h>
#include <wlr/xwayland.h>

#include "tagset.h"
#include "parseConfig.h"

enum shell { XDGShell, X11Managed, X11Unmanaged, LayerShell }; /* client types */

struct client {
    struct wl_list link;
    struct wl_list flink;
    struct wl_list slink;
    struct wl_list llink;
    union {
        struct wlr_xdg_surface *xdg;
        struct wlr_layer_surface_v1 *layer;
        struct wlr_xwayland_surface *xwayland;
    } surface;
    struct wl_listener activate;
    struct wl_listener commit;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wlr_box geom;  /* layout-relative, includes border */
    struct monitor *mon;
    enum shell type;
    struct tagset tagset;
    int bw;
    bool floating;
    uint32_t resize; /* configure serial of a pending resize */
};

void createClient(struct client *c);
void destroyClient(struct client *c);

void applybounds(struct client *c, struct wlr_box bbox);
/* set title and floating status */
bool visibleon(struct client *c, struct monitor *m);
bool visibleonTag(struct client *c, struct monitor *m, size_t focusedTag);
struct client *nextClient();
struct client *selClient();
void applyrules(struct client *c);
void focusClient(struct client *old, struct client *c, bool lift);
void focusTopClient(struct client *old, bool lift);
struct client *getClient(int i);

extern struct wl_list clients; /* tiling order */
extern struct wl_list focusStack;  /* focus order */
extern struct wl_list stack;   /* stacking z-order */
extern struct wl_list layerStack;   /* stacking z-order */
extern struct wlr_output_layout *output_layout;
extern struct wlr_box sgeom;

struct wlr_surface *getWlrSurface(struct client *c);
#endif
