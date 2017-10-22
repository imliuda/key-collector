#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>

#include "config.h"
#include "utils/list.h"

static oc_list *oc_config = NULL;

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

    for (int i=0; i<fsize; i++) {
        if (s == CP_NONE) {
            if (isspace(buf[i])) {
                continue;
            } else if (buf[i] == '#') {
                s == CP_COMMENT;
            } else if (buf[i] == '[') {
            } else if (buf[i] == '.') {
                if (strlen(key) == 0) {
                    snprintf(error, 512,  "key's first character can't be \".\".");
                    config_error(buf, i, error);
                } else {
                    s = CP_KEY;
                    config_string(key, buf[i]);
                }
            }
        } else if (s == CP_COMMENT) {
            if (buf[i] == '\n') {
                s = CP_NONE;
            }
        } else if (s == CP_SECTION) {
            
        } else if (s == CP_KEY) {
            if (buf[i] == '=') {
                /* get a key */
                printf("key: %s\n", key);
                s = CP_VALUE;
            }
        } else if (s == CP_VALUE) {
            if (buf[i] == '\n') {
                printf("value: %s\n", value);
                s = CP_NONE;
            } else {
                value = config_string(value, buf[i]);
            }
        }
    }
}
