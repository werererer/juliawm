#include "utils/writeFile.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <wlr/util/log.h>

int write_to_file(int fd, const char *content)
{
    if (fd < 0) {
        wlr_log(WLR_ERROR, "ERROR: file didn't open correctly");
        return -1;
    }

    if (write(fd, content, strlen(content)) != strlen(content)) {
        wlr_log(WLR_ERROR, "ERROR: failed to write content to file\n");
        return -1;
    }
    return 0;
}

static int write_double_to_file(int fd, double d)
{
    char content[NUM_CHARS];
    doubleToString(content, d);
    return write_to_file(fd, content);
}

void write_container_to_file(int fd, struct wlr_fbox box)
{
    wlr_log(WLR_DEBUG, "write container");
    write_double_to_file(fd, box.x);
    write_to_file(fd, " ");
    write_double_to_file(fd, box.y);
    write_to_file(fd, " ");
    write_double_to_file(fd, box.width);
    write_to_file(fd, " ");
    write_double_to_file(fd, box.height);
    write_to_file(fd, "\n");
}

void write_container_array_to_file(int fd, struct wlr_fbox box[], size_t length)
{
    for (int i = 0; i < length; i++) {
        write_container_to_file(fd, box[i]);
    }
}
