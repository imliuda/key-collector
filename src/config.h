#ifndef __OSCLT_CONFIG_H__
#define __OSCLT_CONFIG_H__

#include "utils/list.h"

typedef enum cp_state cp_state;
typedef struct config_entry config_entry;

enum cp_state {
    CP_NONE,
    CP_COMMENT,
    CP_SECTION,
    CP_KEY,
    CP_VALUE
};

struct config_entry {
    char *section;
    char *key;
    char *value;
};

void config_dumps();
void config_load(const char *path);

#endif
