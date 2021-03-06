#ifndef TILEUTILS
#define TILEUTILS
#include "utils/coreUtils.h"
#include "container.h"
#include "monitor.h"

struct client *focustop(struct monitor *m);
void arrange();
void arrange_monitor(struct monitor *m);
void arrange_container(struct container *con, int arrange_position);
void resize(struct container *con, struct wlr_box geom);
void update_hidden_containers(struct monitor *m);
void update_container_positions(struct monitor *m);
int lib_get_this_container_count();
int get_container_count(struct workspace *ws);
int get_tiled_container_count(struct workspace *ws);
int get_slave_container_count(struct workspace *ws);
int get_master_container_count(struct workspace *ws);

#endif
