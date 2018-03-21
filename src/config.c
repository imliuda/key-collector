#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <locale.h>
#include <errno.h>
#include <wchar.h>
#include <wctype.h>
#include <assert.h>
#include <sys/file.h>

#include "map.h"
#include "list.h"
#include "config.h"

#ifndef __STDC_ISO_10646__
#error "you compiler dos't support unicode."
#endif

static int keycmp(void *key1, void *key2) {
    return strcmp((char *)key1, (char *)key2);
}

static inline bool config_is_whitespace(wchar_t c) {
    return iswspace(c);
}

static inline void config_skip_whitespace(wchar_t buf[], size_t *cursor, size_t buflen) {
    while(*cursor < buflen && config_is_whitespace(buf[*cursor])) *cursor += 1;
}

static inline void config_skip_comment(wchar_t buf[], size_t *cursor, size_t buflen) {
    wchar_t c;
    while(c = buf[*cursor], *cursor < buflen) {
        if (c == '\n') {
            *cursor += 1;
            break;
        } else if (c == '\r' && (*cursor < buflen - 1 && buf[*cursor + 1] == '\n')) {
            *cursor += 2;
            break;
        }
        *cursor += 1;
    }
}

static inline void config_skip_separator(wchar_t buf[], size_t *cursor, size_t buflen) {
    wchar_t c;
    while (c = buf[*cursor], (*cursor)++ <buflen) {
        if (c == '=' || c == ':') return;
    }
    fprintf(stderr, "can't find separator '=' or ':'.\n");
    exit(1);
}

static inline bool config_is_newline(wchar_t buf[], size_t *cursor, size_t buflen) {
    if (buf[*cursor] == '\n' || (*cursor < buflen - 1 && buf[*cursor] == '\r' && buf[*cursor + 1] == '\n')) {
        return true;
    }
    return false;
}

static inline void config_skip(struct config_parse_buffer *buf, int mode) {
    
}

static inline void config_skip_inline(struct config_parse_buffer *buf, int mode) {
    
}

static void config_error(struct config_parse_buffer *buf, char *msg) {

}

char *wcs_to_bs(wchar_t buf[], size_t len) {
    char *str = NULL; 
    int strlen = 0;
    for (int i = 0; i < len; i++) {
        char bytes[MB_CUR_MAX];
        int n = wctomb(bytes, buf[i]);
        str = realloc(str, strlen + n);
        strncpy(str + strlen, bytes, n);
        strlen += n;
    }
    str = realloc(str, strlen + 1);
    str[strlen] = '\0';
    return str;
}

static char *config_parse_key(struct config_parse_buffer *buf) {
    wchar_t c;
    while (c = buf[*cursor], *cursor < buflen) {
        if (config_is_whitespace(c)) {
            config_skip_whitespace(buf, cursor, buflen);
        } else if (c == '#') {
            config_skip_comment(buf, cursor, buflen);
        } else {
            size_t start = *cursor, end = *cursor;
            while (c = buf[*cursor], *cursor < buflen) {
                if (config_is_newline(buf, cursor, buflen)) {
                    fprintf(stderr, "keys can't expand multi-line.\n");
                    exit(1);
                }
                if (c == '=' || c == ':'){
                    break;
                }
                if (!config_is_whitespace(c)) {
                    end = *cursor;
                }
                *cursor += 1;
            }
            if (end - start == 0) {
                fprintf(stderr, "key can't be empty.\n");
                exit(1);
            }
            printf("key: %s\n", wcs_to_bs(buf + start, end - start + 1));
            return wcs_to_bs(buf + start, end - start + 1);
        }
    }
}

static struct config *config_parse_object(struct config_parse_buffer *buf) {
    printf("config_parse_object\n");
    wchar_t c;
    bool has_fields = false, open_brace = false;
    char *key = NULL;
    struct config *value = NULL, *object;

    object = malloc(sizeof(struct config));
    object->type = CONFIG_OBJECT_TYPE;
    object->value = map_new(keycmp);

    while (buf->offset < buf->length) {
        config_skip_whitespace_and_comment(buf);
        /* buffer is done. check if has open brace and return */
        /* happen when parsing root object */
        if (buf->offset == buf->length) {
            if (open_brace) {
                config_error(buf, "close brace not found in the end.");
            }
            return object;
        }

        /* get current offset's value */
        c = buf->buffer[buf->offset];

        if (c == '{') {
            buf->offset += 1;
            open_brace = true;
        } else if (c == '}') {
            buf->offset += 1;
            return object;
        } else if (c == ',') {
            if (has_fields) {
                buf->offset += 1;
            } else {
                config_error(buf, "comma appears before first element of object.");
            }
        }

        key = config_parse_key(buf);

        config_skip_separator_inline(buf);
        config_skip_whitespace_inline(buf);

        while (buf->offset < buf->length) {
            c = buf->buffer[buf->offset];

            if (c == '\n') {
                config_error(buf, "value must start in the same line with key.");
            } else if (c == ',' || c == '#') {
                config_error(buf, "value can't be empty.");
            } else if (c == '{') {
                value = config_parse_object(buf);
                break;
            } else if (c == '[') {
                value = config_parse_array(buf, cursor, buflen);
                break;
            } else {
                value = config_parse_simple(buf, cursor, buflen);
                break;
            }
        }
        assert(key != NULL);
        assert(value != NULL);
        map_add(object->value, key, value);
    }
}

static struct config *config_parse_array(wchar_t buf[], size_t *cursor, size_t buflen) {
    printf("config_parse_array\n");
    wchar_t c;
    struct config *array = malloc(sizeof(struct config));
    array->type = CONFIG_ARRAY_TYPE;
    array->value = list_new();
    while (c = buf[*cursor], *cursor < buflen) {
        if (config_is_whitespace(c)) {
            config_skip_whitespace(buf, cursor, buflen);
        } else if (c == '#') {
            config_skip_comment(buf, cursor, buflen);
        } else if (c == ',') {
            if (list_length(array->value) == 0) {
                fprintf(stderr, "comma before first element of array.\n");
                exit(1);
            } else {
                *cursor += 1;
            }
        } else if (c == ']') {
            *cursor += 1;
            return array;
        } else if (c == '{') {
            *cursor += 1;
            struct config *object = config_parse_object(buf, cursor, buflen, true);
            list_append(array->value, list_node(object));
        } else if (c == '[') {
            *cursor += 1;
            struct config *array = config_parse_array(buf, cursor, buflen);
            list_append(array->value, list_node(array));
        } else {
            struct config *simple = config_parse_simple(buf, cursor, buflen);
            list_append(array->value, list_node(simple));
        }
    }
}

static struct config *config_parse_simple(wchar_t buf[], size_t *cursor, size_t buflen) {
    printf("config_parse_simple\n");
    wchar_t c;
    struct config *simple = malloc(sizeof(struct config));
    simple->type = CONFIG_STRING_TYPE;
    simple->value = NULL;
    while (c = buf[*cursor], *cursor < buflen) {
        if (config_is_whitespace(c)) {
            config_skip_whitespace(buf, cursor, buflen);
        } else {
            int start = *cursor, end = *cursor;
            while (!config_is_newline(buf, cursor, buflen)) {
                if (!config_is_whitespace(buf[*cursor])) {
                    end = *cursor;
                }
                *cursor += 1;
            }
            printf("string value: %s\n", wcs_to_bs(buf + start, end - start + 1));
            simple->value = wcs_to_bs(buf + start, end - start + 1);
            return simple;
        }
    }
}

struct config *config_parse(struct config *config, wchar_t buf[], size_t buflen) {
    /* parse config, buf has buflen wchar_t character */
    printf("config_parse\n");
    wchar_t c;
    size_t cursor = 0;
    bool open_brace = false;
    while (c = buf[cursor], cursor < buflen) {
        if (config_is_whitespace(c)) { /* skip whitespace */
            config_skip_whitespace(buf, &cursor, buflen);
        } else if (c == '#') { /* skip comment */
            config_skip_comment(buf, &cursor, buflen);
        } else if (c == '[') {
            fprintf(stderr, "root element must be an object, not an array.\n");
            exit(1);
        } else if (c == '{') { /* explicit root brace */
            open_brace = true;
            cursor += 1;
            break;
        } else { /* key starts, implicit root brace */
            break;
        }
    }
    if (config == NULL) {
        return config_parse_object(buf, &cursor, buflen, open_brace);
    } else {
        //return config_merge(config, config_parse_object(buf, &cursor, buflen));
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
