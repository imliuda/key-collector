#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <iconv.h>
#include <locale.h>
#include <errno.h>
#include <wchar.h>
#include <wctype.h>
#include <sys/file.h>

#include "map.h"
#include "config.h"

#ifndef __STDC_ISO_10646__
#error "you compiler dos't support unicode."
#endif

static int keycmp(void *key1, void *key2) {
    return strcmp((char *)key1, (char *)key2);
}

struct config *config_new() {
    struct map *map = map_new(keycmp);
    struct config *config= malloc(sizeof(struct config));
    config->map = map;
    return config;
}

static void config_skip_comment(wchar_t buf[], size_t *cursor, size_t buflen) {
    while(c = buf[*cursor], (*cursor)++ < buflen) {
        if (c == '\n') break;
    }
}

static void config_skip_separator(wchar_t buf[], size_t *cursor, size_t buflen) {
    while (c = buf[*cursor], (*cursor)++ <buflen) {
        if (c == '=' || c == ':') break;
    }
}

static char *config_parse_key(wchar_t buf, size_t *cursor, size_t buflen) {
    size_t start = *cursor, end;
    char *key = 
    while (c = buf[*cursor], *cursor < buflen) {
        if (iswspace(c)) {
            *cursor += 1;
            start = *cursor;
        } else if (c == '#') {
            config_skip_comment(buf, cursor, buflen);
            start = *cursor;
        } else {
            end = start;
            while (c = buf[*cursor], *cursor < buflen - 1 && c != '=' && c != ':'){
                if (!iswspace(c)) {
                    end = *cursor;
                }
                *cursor += 1;
            }
            if (end - start == 0) {
                fprintf(stderr, "key can't be empty.\n");
                exit(1);
            }
            return strndup(buf[start], end - start);
        }
    }
}

static struct config *config_parse_object(wchar_t buf[], size_t *cursor, size_t buflen) {
    char *key;
    struct config_value *value, *object = malloc(sizeof(struct config_value));
    object->type = CONFIG_VALUE_OBJECT;
    object->value = map_new(keycmp);
    while (c = buf[*cursor], *cursor < buflen) {
        if (iswspace(c)) {
            *cursor += 1;
        } else if (c == '#') {
            config_skip_comment(buf, cursor, buflen);
        } else if (c == '}') {
            return ;
        } else if (*cursor = buflen - 1) {
            return;
        }
        key = config_parse_key(buf, cursor, buflen);
        config_skip_separator(buf, cursor, buflen);
        while (c = buf[*cursor], *cursor < buflen) {
            if (iswspace(c)) {
                *cursor += 1;
            } else if (c == '#') {
                config_skip_comment(buf, cursor, buflen);
            } else if (c == '{') {
                value = config_parse_object(buf, cursor, buflen);
                break;
            } else if (c == '[') {
                value = config_parse_array(buf, cursor, buflen);
                break;
            } else {
                value = config_parse_simple(buf, cursor, buflen);
                break;
            }
        }
        map_add(object->value, key, value);
    }
}

static struct config *config_parse_array(wchar_t buf, size_t *cursor, size_t buflen) {
    wchar_t c;
    struct config *array = malloc(sizeof(struct config));
    array->type = CONFIG_ARRAY_TYPE;
    array->value = list_new();
    while (c = buf[*cursor], *cursor < buflen) {
        if (iswspace(c)) {
            *cursor += 1;
        } else if (c == '#') {
            config_skip_comment();
        } else if (c == ',') {
            if (list_length(array->value) == 0) {
                fprintf(stderr, "comma before first array element.\n");
                exit(0);
            } else {
                *cursor += 1;
            }
        } else if (c == ']') {
            *cursor ++;
            return array;
        } else if (c == '{') {
            struct config *object = config_parse_object(buf, cursor, buflen);
            list_add(array->value, object);
        } else if (c == '[') {
            struct config *array = config_parse_array(buf, cursor, buflen);
            list_add(array->value, array);
        } else {
            struct config *simple = config_parse_simple(buf, cursor, buflen);
            list_add(array->value, simple);
        }
    }
}

static config_value *config_parse_simple() {

}

struct config *config_parse(struct config *config, wchar_t buf[], size_t buflen) {
    /* parse config, buf has buflen wchar_t character */
    wchar_t c;
    size_t cursor = 0;
    while (c = buf[cursor], cursor < buflen) {
        if (iswspace(c)) { /*skip whitespace*/
            n++;
        } else if (c == '#') { /* skip comment */
            config_skip_comment(buf, &cursor, buflen);
        } else if (c == '[') {
            fprintf(stderr, "root element must be an object, not an array.\n");
            exit(1);
        } else if (c == '{') { /* explicit root brace */
            n++;
            break;
        } else { /* key starts, implicit root brace */
            break;
        }
    }
    if (config == NULL) {
        return config_parse_object(buf, &cursor, buflen);
    } else {
        return config_merge(config, config_parse_object(buf, &cursor, buflen));
    }
}

struct config *config_load(const char *path) {
    /* get file content to a buf */
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "unable to open config file: %s\n", path);
        exit(1);
    }
    flock(fileno(fp), LOCK_EX);
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    unsigned char *buf = malloc(len);
    int nread = 0, total = 0;
    while(!feof(fp)) {
        nread = fread(buf + total, 1, 64, fp);
        total += nread;
    }
    flock(fileno(fp), LOCK_UN);
    fclose(fp);

    /* remove utf-8 bom if it has */
    if (len >= 3 && buf[0] == 0xef && buf[1] == 0xbb && buf[2] == 0xbf) {
        buf += 3;
        len -= 3;
    }

    /* convert char buf to wchar_t buf */
    mbstate_t mbs;
    wchar_t *wbuf = NULL, wc;
    size_t nbytes, wbuflen = 0, wcslen = 0;
    char *tbuf = buf;
    memset(&mbs, 0, sizeof(mbs));
    while (len > 0) {
        if ((nbytes = mbrtowc(&wc, tbuf, len, &mbs)) > 0) {
            if (nbytes >= (size_t) -2) {
                fprintf(stderr, "config file encoding error.\n");
                exit(1);
            }
            if (wcslen >= wbuflen) {
                wbuflen += 64;
                wbuf = realloc(wbuf, wbuflen * sizeof(wchar_t));
            }
            wbuf[wcslen++] = wc;
            len -= nbytes;
            tbuf += nbytes;
        } else {
            fprintf(stderr, "invalid '\0' character in config file.\n");
            exit(1);
        }
    }
    free(buf);

    struct config *config = config_parse(NULL, wbuf, wcslen);
    free(wbuf);
    return config;
}

char *config_get_string(struct config *config, const char *name) {
    
}

uint64_t config_get_integer() {
}

double config_get_float() {
}

bool config_get_boolean() {
}

struct duration config_get_duration() {
}
