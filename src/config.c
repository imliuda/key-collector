#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "config.h"
#include "str.h"

char *config_file = NULL;

void config_init() {
    size_t nread, len;
    char *line, *name, *value;

    FILE *fp = fopen(config_file, "r");

    if (fp == NULL) {
        printf("failed to open config file: %s\n", config_file);
        exit(1);
    }

    do {
        line = NULL;
        len = 0;
        nread = getline(&line, &len, fp);
        if (nread != -1) {
            if (nread > 0 && line[nread - 1] == '\n') {
                line[nread - 1] = '\0';
            }
            value = line;
            name = strsep(&value, "=");
            if (value != NULL) {
                cfg_item *item = malloc(sizeof(cfg_item));
                item->name = strdup(strtrim(name, " \t"));
                item->value = strdup(strtrim(value, " \t"));
                item->next = config;
                config = item;
            }

            free(line);
        }
    } while (!feof(fp));

    fclose(fp);
}

static cfg_item *config_get_item(const char *name) {
    cfg_item *p = config;
    while (p != NULL) {
        if (strcmp(p->name, name) == 0) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

long int config_get_long(const char *name, long int def) {
    cfg_item *item = config_get_item(name);
    if (item) {
        return strtol(item->value, NULL, 10);
    } else {
        return def;
    }
}

bool config_get_bool(const char *name, bool def) {
    cfg_item *item = config_get_item(name);
    if (item) {
       if (strcasecmp(item->value, "true") == 0) {
           return true;
       } else {
           return false;
       }
    } else {
        return def;
    }
}

char *config_get_string(const char *name, char *def) {
    cfg_item *item = config_get_item(name);
    if(item) {
        return strdup(item->value);
    } else {
        return strdup(def);
    }
}
