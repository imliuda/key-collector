#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "config.h"
#include "utils/str.h"

config_plugin *config_list = NULL;
json_t *config = NULL;

void config_register(const char *name, void (*load)(const char *param)) {
    config_plugin *plugin = malloc(sizeof(config_plugin));
    plugin->name = strdup(name);
    plugin->load = load;
    plugin->next = config_list;
    config_list = plugin;
}

void config_load(const char *name, const char *param) {
    config_plugin *plugin = config_list;
    while (plugin != NULL) {
        if (strcmp(plugin->name, name) == 0) {
            plugin->load(param);
            break;
        }
        plugin = plugin->next;
    }

    if (plugin == NULL) {
        printf("failed to load config plugin: %s, no such plugin\n", name);
    }
}
