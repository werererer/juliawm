#include "tile/tileTexture.h"
#include "client.h"
#include "container.h"
#include "utils/coreUtils.h"
#include "utils/stringUtils.h"
#include "utils/writeFile.h"
#include "tile/tileUtils.h"
#include "root.h"
#include <cairo/cairo.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/util/log.h>
#include <stdlib.h>

bool overlay = false;
// TODO: rewrite getPosition
/* static struct wlr_box getPosition(struct posTexture *ptexture) */
/* { */
/*     struct wlr_box container; */
/*     container.x = ptexture->x; */
/*     container.y = ptexture->y; */
/*     container.width = ptexture->texture->width; */
/*     container.height = ptexture->texture->height; */
/*     return container; */
/* } */

struct pos_texture *create_textbox(struct wlr_box box, float box_color[],
                                 float text_color[], char* text)
{
    cairo_format_t cFormat = CAIRO_FORMAT_ARGB32;

    int width = box.width;
    int height = box.height;
    int stride = cairo_format_stride_for_width(cFormat, width);

    cairo_surface_t *surface =
        cairo_image_surface_create(cFormat, width, height);
    //draw box
    cairo_t *cr = cairo_create(surface);
    cairo_set_line_width(cr, 0.1);
    cairo_set_source_rgba(cr,
            box_color[0], box_color[1], box_color[2], box_color[3]);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);
    cairo_surface_flush(surface);

    //write text
    cairo_select_font_face(cr, "serif", 
            CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 32.0);
    cairo_move_to(cr, width/2.0, height/2.0);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_source_rgba(cr,
            text_color[0], text_color[1], text_color[2], text_color[3]);
    cairo_show_text(cr, text);
    cairo_surface_flush(surface);

    unsigned char *cdata = cairo_image_surface_get_data(surface);

    struct wlr_texture *cTexture = 
        wlr_texture_from_pixels(drw, WL_SHM_FORMAT_ARGB8888, stride, 
                width, height, cdata);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    struct pos_texture *posTexture = calloc(1, sizeof(*posTexture));

    posTexture->texture = cTexture;
    posTexture->dataType = OVERLAY;
    posTexture->x = box.x;
    posTexture->y = box.y;
    posTexture->mon = selected_monitor;
    posTexture->ws = selected_monitor->ws;
    return posTexture;
}

void init_overlay()
{
    wlr_list_init(&render_data.textures);
    wlr_list_init(&render_data.base_textures);
}

void create_new_overlay()
{
    // recreate list
    wlr_list_clear(&render_data.textures);
    wlr_list_clear(&render_data.base_textures);
    create_overlay();
}

void create_overlay()
{
    struct container *con;

    char text[NUM_DIGITS];
    int i = 0;
    wl_list_for_each_reverse(con, &stack, slink) {
        if (con->client->type == LAYER_SHELL)
            continue;

        con->stack_position = i;
        i++;
        intToString(text, con->position);

        struct pos_texture *ptexture =
            create_textbox(con->geom, overlay_color, text_color, text);
        // sync properties
        ptexture->ws = con->client->ws;
        wlr_list_push(&render_data.textures, ptexture);
        wlr_list_push(&render_data.base_textures, ptexture);
    }
}

void update_container_overlay(struct container *con)
{
    if (overlay) {
        if (render_data.textures.length >= con->stack_position+1) {
            wlr_list_del(&render_data.textures, con->stack_position);
        }

        char text[NUM_DIGITS];

        intToString(text, con->position);

        struct pos_texture *ptexture =
            create_textbox(con->geom, overlay_color, text_color, text);
        ptexture->ws = con->client->ws;
        wlr_list_insert(&render_data.textures, con->stack_position, ptexture);
    } else {
        if (&render_data.textures.length > 0)
            wlr_list_clear(&render_data.textures);
    }
}

void update_overlay_count(size_t count)
{
    if (count == 0) {
        wlr_list_clear(&render_data.textures);
    } else {
        int len = render_data.textures.length;
        for (size_t i = count; i < len; i++) {
            wlr_list_del(&render_data.textures, count);
        }
    }
}

void update_overlay()
{
    if (overlay) {
        create_new_overlay();
    } else {
        wlr_list_clear(&render_data.textures);
    }
}

bool postexture_visible_on(struct pos_texture *ptexture, struct monitor *m, struct workspace *ws)
{
    if (!m || !ptexture)
        return false;
    if (ptexture->mon != m)
        return false;

    return ptexture->ws == ws;
}

void write_overlay(struct monitor *m, const char *layout)
{
    if (!overlay)
        return;
    char file[NUM_CHARS];
    char filetmp[NUM_CHARS];
    char filename[NUM_DIGITS];

    strcpy(file, get_config_layout_path());
    join_path(file, layout);
    mkdir(file, 0755);
    join_path(file, filename);
    mkdir(file, 0755);
    strcpy(filetmp, file);
    for (int i = 0; i < 8; i++) {
        strcpy(file, filetmp);
        intToString(filename, i+1);
        join_path(file, filename);

        int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        wlr_log(WLR_DEBUG, "create file %s", file);
        if (fd == -1) {
            wlr_log(WLR_INFO, "file didn't open correctly: %s", file);
            return;
        }

        int k = 0;

        for (int j = render_data.base_textures.length-1; j >= 0; j--) {
            // TODO: todo fix order
            struct pos_texture *ptexture = render_data.base_textures.items[j];
            if (postexture_visible_on(ptexture, m, get_workspace(i))) {
                // vector from root x/y -> monitor x/y
                int wdiff = selected_monitor->geom.x - m->root->geom.y;
                int hdiff = selected_monitor->geom.y - m->root->geom.y;
                struct wlr_box container = postexture_to_container(ptexture);
                // add vector to the container so that it is relative to
                // monitor again
                container.x += wdiff;
                container.y += hdiff;
                struct wlr_fbox box =
                    get_relative_box(container, selected_monitor->geom);
                write_container_to_file(fd, box);
                k++;
            }
        }

        close(fd);
        if (!k) {
            unlink(file);
        }
    }
}

struct wlr_box postexture_to_container(struct pos_texture *ptexture)
{
    struct wlr_box box;
    if (!ptexture) {
        box.x = 0;
        box.y = 0;
        box.width = 0;
        box.height = 0;
        return box;
    }

    box.x = ptexture->x;
    box.y = ptexture->y;
    box.width = ptexture->texture->width;
    box.height = ptexture->texture->height;
    return box;
}
