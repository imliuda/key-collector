#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>

#include "config.h"
#include "utils/list.h"
#include "utils/str.h"

static oc_list * restrict oc_config = NULL;

static void config_error(char buf[], int index, char *msg) {
    int line_num = 1;
    int char_num = 1;
    for (int i = 0; i != index; i++) {
        if (buf[i] == '\n') {
            line_num++;
            char_num = 1;
        } else {
            char_num++;
        }
    }
    printf("config error at line %d, character %d: %s\n", line_num, char_num, msg);
    exit(1);
}

static char *config_string(char *str, char c) {
    if (str == NULL) {
        str = malloc(64);
        str[0] = c;
        str[1] = '\0';
    } else {
        int len = strlen(str);
        if ((len + 1) % 64 == 0) {
            str = realloc(str, len + 1 + 64);
        }
        str[len] = c;
        str[len + 1] = '\0';
    }
    return str;
}

void config_dumps() {
    oc_list *p = oc_config;
    while (p = oc_list_next(oc_config, p)) {
        printf("section: %s, ", ((config_entry *)p->data)->section);
        printf("key: %s, ", ((config_entry *)p->data)->key);
        printf("value: %s\n", ((config_entry *)p->data)->value);
    }
}

void config_load(const char *path) {
    FILE *fp = fopen(path, "r");

    if (fp == NULL) {
        fprintf(stderr, "open config file %s failed: %s\n", path, strerror(errno));
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);
    char *buf = malloc(fsize);
    fread(buf, fsize, 1, fp);
    fclose(fp);

    cp_state s = CP_NONE;
    char *conmment = config_string(NULL, '\0');
    char *key = config_string(NULL, '\0');
    char *section = config_string(NULL, '\0');
    char *value = config_string(NULL, '\0');
    char error[512];
    oc_config = oc_list_new();

    for (int i=0; i<fsize; i++) {
        if (s == CP_NONE) {
            if (isspace(buf[i])) {
                continue;
            } else if (buf[i] == '#') {
                s == CP_COMMENT;
            } else if (buf[i] == '[') {
                s = CP_SECTION;
            } else if (buf[i] == '.') {
                if (strlen(key) == 0) {
                    config_error(buf, i, "key's first character can't be '.'.");
                } else {
                    s = CP_KEY;
                    config_string(key, buf[i]);
                }
            } else if (buf[i] == '=') {
                // it must be 0
                if (strlen(key) == 0) {
                    config_error(buf, i, "empty key name.");
                }
            } else {
                s = CP_KEY;
                config_string(key, buf[i]);
            }
        } else if (s == CP_COMMENT) {
            if (buf[i] == '\n') {
                s = CP_NONE;
            }
        } else if (s == CP_SECTION) {
            if (buf[i] == ']') {
                if (strlen(section) == 0) {
                    config_error(buf, i, "empty section name.");
                } else {
                    // got section name
                    s = CP_NONE;
                    section = strtrim(section);
                    section = config_string(NULL, '\0');
                }
            } else if (buf[i] == '.') {
                if (strlen(section) == 0) {
                    config_error(buf, i, "section's first character can't be '.'.");
                } else {
                    section = config_string(section, buf[i]);
                }
            } else {
                section = config_string(section, buf[i]);
            }
        } else if (s == CP_KEY) {
            if (buf[i] == '=') {
                if (strlen(key) == 0) {
                    config_error(buf, i , "empty key name.");
                } else {
                    // got a key
                    s = CP_VALUE;
                    key = strtrim(key);
                }
            } else if (buf[i] == '[' || buf[i] == ']') {
                config_error(buf, i, "invalid character '[' or ']' for a key.");
            } else {
                key = config_string(key, buf[i]);
            }
        } else if (s == CP_VALUE) {
            if (buf[i] == '\n') {
                // got a value
                s = CP_NONE;
                value = strtrim(value);
                printf("%s, %s, %s\n", section, key, value);

                // create a new config entry
                config_entry *entry = malloc(sizeof(config_entry));
                entry->section = section;
                entry->key = key;
                entry->value = value;
                oc_list *node = malloc(sizeof(oc_list));
                node->data = entry;
                oc_list_append(oc_config, node);

                // reset key and value
                key = config_string(NULL, '\0');
                value = config_string(NULL, '\0');
            } else {
                value = config_string(value, buf[i]);
            }
        }
    }
}
