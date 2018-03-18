#ifndef __OSCLT_CONFIG_H__
#define __OSCLT_CONFIG_H__

#include "map.h"

struct config {
    struct map *map;
};

struct duration {
    struct timespec time;
};

enum cp_state {
    CP_START,
    CP_OBJECT,
    CP_LIST,
    CP_KEY,
    CP_VALUE,
    CP_END
};

struct config *config_new();
void config_load(struct config *config, const char *path);

#endif
