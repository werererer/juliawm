#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <stdio.h>
#include <stdlib.h>
#include <wlr/types/wlr_list.h>
#include "layout.h"
#include "container.h"

/* A tag is simply a workspace that can be focused (like a normal workspace)
 * and can selected: which just means that all clients on the selected tags
 * will be combined to be shown on the focused tag
 * using this struct requires to use tagsetCreate and later tagsetDestroy
 * */
struct workspace {
    size_t id;
    const char *name;
    struct layout layout[2];
    struct monitor *m;
};

struct workspace *create_workspace(const char *name, size_t id, struct layout lt);
void destroy_workspace(struct workspace *ws);

void update_workspace_ids(struct wlr_list *workspaces);

bool existon(struct container *con, struct wlr_list *workspaces, int ws_id);
bool is_workspace_occupied(struct workspace *ws);
bool hiddenon(struct container *con, struct wlr_list *workspaces, int ws_id);
bool visibleon(struct container *con, struct wlr_list *workspaces, int ws_id);
bool workspace_has_clients(struct workspace *ws);

int get_workspace_container_count(struct wlr_list *workspaces, size_t ws_id);
bool is_workspace_empty(struct wlr_list *workspaces, size_t ws_id);

struct workspace *find_next_unoccupied_workspace(struct wlr_list *workspaces, struct workspace *ws);
struct workspace *get_workspace(struct wlr_list *workspaces, int i);
struct workspace *get_next_empty_workspace(struct wlr_list *workspaces, size_t i);
struct workspace *get_prev_empty_workspace(struct wlr_list *workspaces, size_t i);

void focus_next_unoccupied_workspace(struct monitor *m, struct wlr_list *workspaces, struct workspace *ws);
void copy_layout_from_selected_workspace(struct wlr_list *workspaces);
void create_workspaces(struct wlr_list *workspaces, struct wlr_list tagNames, struct layout default_layout);
void destroy_workspaces(struct wlr_list *workspaces);
void delete_workspace(struct wlr_list *workspaces, size_t id);
void rename_workspace(size_t i, struct wlr_list *workspaces, const char *name);
void load_default_layout(lua_State *L, struct workspace *ws);
void load_layout(lua_State *L, struct workspace *ws, const char *layout_name, const char *layout_symbol);
void set_layout(lua_State *L, struct workspace *ws);
struct workspace *find_next_unoccupied_workspace(struct wlr_list *workspaces, struct workspace *ws);
void set_selected_layout(struct workspace *ws, struct layout layout);
void workspace_assign_monitor(struct workspace *ws, struct monitor *m);
/* sets the value of selTag[0] */
void focus_workspace(struct monitor *m, struct wlr_list *workspaces, int ws_id);
void push_workspace(int ws_ids[static 2], int ws_id);

#endif /* WORKSPACE_H */
