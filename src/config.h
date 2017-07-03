#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <jansson.h>

typedef struct _config_plugin config_plugin;
struct _config_plugin {
    char            *name;
    void            (*load)(const char *param);
    config_plugin   *next;
};

extern config_plugin *config_list;
extern json_t *config;

void config_register(const char *name, void (*load)(const char *param));

#endif
