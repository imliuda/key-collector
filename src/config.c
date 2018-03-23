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

/*
 * skip unnecessary characters.
 *
 * @param buf  config parse buffer.
 * @param mode skip which class characters. can be SKIP_WHITESPACE, SKIP_COMMENT,
 *             SKIP_SAPARATOR or any combination.
 */
static inline void config_skip(struct config_parse_buffer *buf, int mode) {
    while (buf->offset < buf->length) {
        if (config_is_whitespace(buf->buffer[buf->offset]) && (SKIP_WHITESPACE & mode)) {
            buf->offset++;
            if (buf->offset == buf->length) return;
        } else if ((buf->buffer[buf->offset] == '#' || (buf->buffer[buf->offset] == '/' &&
                   buf->offset < buf->length - 1 && buf->buffer[buf->offset + 1] == '/')) &&
                   (SKIP_COMMENT & mode)) {
                while (buf->offset < buf->length) {
                    if (buf->buffer[buf->offset++] == '\n') {
                        break;
                    }
                }
                if (buf->offset == buf->length) return;
        } else {
            return;
        }
    }
}

/*
 * skip unnecessary in current line.
 */
static inline void config_skip_inline(struct config_parse_buffer *buf, int mode) {
    while (buf->offset < buf->length) {
        if (buf->buffer[buf->offset] == '\n') {
            return;
        } else if (config_is_whitespace(buf->buffer[buf->offset]) && (SKIP_WHITESPACE & mode)) {
            buf->offset++;
            if (buf->offset == buf->length) return;
        } else if ((buf->buffer[buf->offset] == '#' || (buf->buffer[buf->offset] == '/' &&
                   buf->offset < buf->length - 1 && buf->buffer[buf->offset + 1] == '/')) &&
                   (SKIP_COMMENT & mode)) {
                while (buf->offset < buf->length) {
                    if (buf->buffer[buf->offset++] == '\n') {
                        buf->offset--;
                        return;
                    }
                }
                if (buf->offset == buf->length) return;
        } else {
            return;
        }
    }
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

static void config_error(struct config_parse_buffer *buf, char *msg) {
    size_t start = buf->offset, end = buf->offset, line_num = 1;
    while (start > 0) {
        if (buf->buffer[start - 1] == '\n') {
            break;
        }
        start--;
    }
    while (end < buf->length) {
        if (buf->buffer[end] == '\n') {
            break;
        }
        end++;
    }
    int i = start;
    while (i >= 0) {
        if (buf->buffer[i] == '\n') {
            line_num++;
        }
        i--;
    }
    char *line = wcs_to_bs(buf->buffer + start, end - start);
    fprintf(stderr, "configuration parsing error in line %d:\n%s\n%s\n", line_num, line, msg);
    exit(1);
}


/*
 * buf->buffer[buf->offset] is non-empty, non-comment.
 * key[0] can't be '.'.
 */
static char *config_parse_key(struct config_parse_buffer *buf) {
    size_t start = buf->offset, end = buf->offset;
    bool quote = false, match;
    while (buf->offset < buf->length) {
        if (buf->buffer[buf->offset] == '=' || buf->buffer[buf->offset] == ':') {
            break;
        } else if (buf->buffer[buf->offset] == '\n') {
            config_error(buf, "object key can't expand multi-line.");
        } else if (buf->buffer[buf->offset] == '"') {
            if (!quote) quote = true;
            else {
                if (match) config_error(buf, "invalid key.");
                else match = true;
            }
        } else {
            if (!config_is_whitespace(buf->buffer[buf->offset])) end++;
            buf->offset++;
        }
    }
    if (end - start == 0) {
        config_error(buf, "object key can't be empty.");
    }
    char *key =  wcs_to_bs(buf->buffer + start, end - start);
    printf("key: %s\n", key);
    if (key[0] == '.') {
        config_error(buf, "key's first character can't be '.'.");
    }
    return key;
}

static struct config *config_parse_object(struct config_parse_buffer *buf) {
    wchar_t c;
    bool has_fields = false, open_brace = false;
    char *key = NULL;
    struct config *value = NULL, *object;

    object = malloc(sizeof(struct config));
    object->type = CONFIG_OBJECT_TYPE;
    object->value = map_new(keycmp);

    while (buf->offset < buf->length) {
        config_skip(buf, SKIP_WHITESPACE | SKIP_COMMENT);
        /* buffer is done. check if has open brace and return */
        /* happen when parsing root object */
        if (buf->offset == buf->length) {
            if (open_brace) {
                config_error(buf, "close brace not found in the end.");
            }
            return object;
        }

        /* get current offset's value, non-empty, non-comment*/
        c = buf->buffer[buf->offset];

        if (c == '{' && open_brace == false) {
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
        } else {
            key = config_parse_key(buf);

            config_skip_inline(buf, SKIP_WHITESPACE);

            if (buf->offset == buf->length || (buf->buffer[buf->offset] != '=' && buf->buffer[buf->offset] != ':')) {
                config_error(buf, "can't find any separator of '=' or ':'.");
            }

            // found separator
            buf->offset++;
            config_skip_inline(buf, SKIP_WHITESPACE | SKIP_COMMENT);

            if (buf->offset == buf->length) {
                config_error(buf, "can't find object value.");
            }

            while (buf->offset < buf->length) {
                c = buf->buffer[buf->offset];

                if (c == '\n') {
                    config_error(buf, "value must start in the same line with key.");
                } else if (c == ',') {
                    config_error(buf, "value can't be empty.");
                } else if (c == '{') {
                    value = config_parse_object(buf);
                    break;
                } else if (c == '[') {
                    value = config_parse_array(buf);
                    break;
                } else {
                    value = config_parse_simple(buf);
                    break;
                }
            }
            assert(key != NULL);
            assert(value != NULL);
            has_fields = true;
            map_add(object->value, key, value);
        }
    }
}

/*
 * buf->buffer[buf->offset] == '['
 */
static struct config *config_parse_array(struct config_parse_buffer *buf) {
    printf("config_parse_array\n");
    bool open_bracket = false, has_values = false;
    struct config *array = malloc(sizeof(struct config));
    array->type = CONFIG_ARRAY_TYPE;
    array->value = list_new();

    while (buf->offset < buf->length) {
        if (buf->buffer[buf->offset] == '[') {
            if (open_bracket) {
                struct config *subarray = config_parse_array(buf);
                list_append(array->value, list_node(subarray));
                has_values = true;
            } else {
                open_bracket = true;
                buf->offset++;
            }
        } else if (buf->buffer[buf->offset] == ',') {
            if (has_values) {
                buf->offset++;
            } else {
                config_error(buf, "comma appears before first element of array.");
            }
        } else if (buf->buffer[buf->offset] == ']') {
            buf->offset++;
            return array;
        } else if (buf->buffer[buf->offset] == '{') {
            printf("array object\n");
            struct config *object = config_parse_object(buf);
            list_append(array->value, list_node(object));
            has_values = true;
        } else {
            config_skip(buf, SKIP_WHITESPACE | SKIP_COMMENT);
            if (buf->offset == buf->length) {
                config_error(buf, "can't find object value.");
            }

            struct config *simple = config_parse_simple(buf);
            list_append(array->value, list_node(simple));
            has_values = true;
        }
    }
}

/*
 * buf->buffer[buf->offset] is non-empty, non-comment.
 * simple value parse function must handle ',', '}' , ']' character and buffer end.
 * if simple value is single line, also need hanle '\n'.
 */
static struct config *config_parse_simple(struct config_parse_buffer *buf) {
    wchar_t c;
    struct config *simple = NULL;

    if (simple = config_parse_string(buf)) {
        return simple;
    } else if (simple = config_parse_number(buf)) {
        return simple;
    } else if (simple = config_parse_boolean(buf)) {
        return simple;
    } else if (simple = config_parse_duration(buf)) {
        return simple;
    }
    config_error(buf, "unknown value type.");
}

/*
 * string is double quoted unicode characters except '"', '\' and
 * control characters. escape characters: 
 * \", \\, \/, \b, \f, \n, \r, \t, \u .
 */
static struct config *config_parse_string(struct config_parse_buffer *buf) {
    size_t start = buf->offset, end = buf->offset, offset = buf->offset;
    bool match = false;

    if (buf->buffer[buf->offset] != '"') return NULL;
    buf->offset++;

    while (buf->offset < buf->length) {
        /* read end of line or object fields separator ',' */
        if (buf->buffer[buf->offset] == '\n' && !match) {
            config_error(buf, "string quotes don't match.");
        } else if (buf->buffer[buf->offset] == '"' && buf->buffer[buf->offset - 1] != '\\'){
            end = ++buf->offset;
            break;
        } else {
            buf->offset++;
        }
    }

    // skip qoutes
    start += 1; end -= 1;

    char *str = wcs_to_bs(buf->buffer + start, end - start);
    char *new = malloc(64);
    size_t index = 0, size = 0;
    for (char *p = str; *p != '\0';) {
        if ((*p == '"' || *p == '\\' || *p == '/' || *p == 'b' ||
            *p == 'f' || *p == 'n' || *p == 'r' || *p == 't') && (p > str) && *(p - 1) == '\\') {
            if (*p == '"') new[index - 1] = '\"';
            else if (*p == '\\') new[index - 1] = '\\';
            else if (*p == '/') new[index - 1] = '/';
            else if (*p == 'b') new[index - 1] = '\b';
            else if (*p == 'f') new[index - 1] = '\f';
            else if (*p == 'n') new[index - 1] = '\n';
            else if (*p == 'r') new[index - 1] = '\r';
            else if (*p == 't') new[index - 1] = '\t';
            p++;
        } else if (*p == 'u' && (p > str) && *(p - 1) == '\\') {
            p++;
            if (*p == '\0' || *(p + 1) == '\0' || *(p + 2) == '\0' || *(p + 3) == '\0') {
                config_error(buf, "unicode escape sequence must have 4 hexadecimal digit.");
            }

            char *e, us[5];
            strncpy(us, p, 4);
            us[4] = '\0';
            wchar_t wc = strtol(us, &e, 16);
            if (*e != '\0') {
                config_error(buf, "unicode escape sequence must have 4 hexadecimal digit.");
            }

            char bytes[MB_CUR_MAX];
            int nb = wcrtomb(bytes, wc, NULL);
            if (nb == (size_t) -1) {
                config_error(buf, "invalid unicode escape sequence.");
            }
            if (index + nb > size) {
                new = realloc(new, size + 64);
                size += 64;
            }
            strncpy(&new[--index], bytes, nb);
            index += nb;
            p += 4;
        } else {
            if (index == size) {
                new = realloc(new, size + 64);
                size += 64;
            }
            new[index++] = *p++;
        }
    }
    if (index == size) {
        new = realloc(new, size + 1);
    }
    new[index] = '\0';
    free(str);

    printf("string value: %s\n", new);
    struct config *simple = malloc(sizeof(struct config));
    simple->type = CONFIG_STRING_TYPE;
    simple->value = new;
    return simple;
}

static struct config *config_parse_number(struct config_parse_buffer *buf) {
    printf("config parse number\n");
    size_t start, end, offset = buf->offset;
    int sign = 1, base = 10;
    enum config_type type = CONFIG_INTEGER_TYPE;
    bool dot_parsed = false;
    struct config *simple;

    if (buf->buffer[buf->offset] == '+') {
        buf->offset++;
    } else if (buf->buffer[buf->offset] == '-') {
        sign = -1;
        buf->offset++;
    }

    if (buf->offset == buf->length) return NULL;

    if (buf->buffer[buf->offset] == '0') {
        buf->offset++;
        if (buf->offset == buf->length) {
            simple = malloc(sizeof(struct config));
            simple->type = CONFIG_INTEGER_TYPE;
            simple->value = 0;
            return simple;
        } else if (buf->buffer[buf->offset] == 'x') {
            base = 16;
            buf->offset++;
        } else if (iswdigit(buf->buffer[buf->offset])) {
            base = 8;
        } else {
            return NULL;
        }
    }

    if (buf->offset == buf->length && base == 16) return NULL;
 
    start = buf->offset; end = buf->offset;
    while (buf->offset < buf->length) {
        if (buf->buffer[buf->offset] == '.') {
            if (dot_parsed) {
                return NULL;
            } else {
                buf->offset++;
                end++;
                dot_parsed = true;
                type = CONFIG_DOUBLE_TYPE;
            }
        } else if (iswdigit(buf->buffer[buf->offset])) {
            buf->offset++;
            end++;
        } else {
            config_skip_inline(buf, SKIP_WHITESPACE | SKIP_COMMENT);
            if (buf->offset != buf->length && buf->buffer[buf->offset] != '\n' &&
                buf->buffer[buf->offset] != ',' && buf->buffer[buf->offset] != ']' &&
                buf->buffer[buf->offset] != '}') {
                buf->offset = offset;
                return NULL;
            } else {
                end++;
                break;
            }
        }
    }

    if (start == end) {
        buf->offset = offset;
        return NULL;
    }
    if (type == CONFIG_DOUBLE_TYPE && (buf->buffer[start] == '.' || buf->buffer[end - 1] == '.')) {
        buf->offset = offset;
        return NULL;
    }

    simple = malloc(sizeof(struct config));
    simple->type = type;
    char *str = wcs_to_bs(buf->buffer + start, end - start);
    printf("number value: %s\n", str);
    if (type == CONFIG_INTEGER_TYPE) {
        uint64_t *value = malloc(sizeof(uint64_t));
        *value = strtoll(str, NULL, base);
        simple->value = value;
    } else {
        double *value = malloc(sizeof(double));
        *value = atof(str);
        simple->value = value;
    }
    free(str);

    return simple;
}

static struct config *config_parse_boolean(struct config_parse_buffer *buf) {
    size_t start = buf->offset, end = buf->offset, offset = buf->offset, length = buf->length;
    wchar_t *buffer = buf->buffer;
    struct config *simple = NULL;

    if (offset <= length - 4 && buffer[offset] == 't' && buffer[offset + 1] == 'r' &&
        buffer[offset + 2] == 'u' && buffer[offset + 3] == 'e') {
        simple = malloc(sizeof(struct config));
        simple->type = CONFIG_BOOLEAN_TYPE;
        simple->value = malloc(sizeof(bool));
        *(bool *)(simple->value) = true;
        buf->offset += 4;
        printf("boolean value: true\n");
    }
    if (offset <= length - 5 && buffer[offset] == 'f' && buffer[offset + 1] == 'a' &&
        buffer[offset + 2] == 'l' && buffer[offset + 3] == 's' && buffer[offset + 4] == 'e') {
        simple = malloc(sizeof(struct config));
        simple->type = CONFIG_BOOLEAN_TYPE;
        simple->value = malloc(sizeof(bool));
        *(bool *)(simple->value) = false;
        buf->offset += 5;
        printf("boolean value: false\n");
    }
    return simple;
}

/*
 * parse duration type.
 *
 * supported unit: ns, us, ms, s, m, h, d.
 */
static struct config *config_parse_duration(struct config_parse_buffer *buf) {
    /*parse value or unit */
    size_t start, end, offset = buf->offset;

    start = end = buf->offset;
    while (buf->offset < buf->length) {
        if (iswdigit(buf->buffer[buf->offset])) end = ++buf->offset;
        else break;
    }
    if (start == end) {
         buf->offset = offset;
         return NULL;
    }
    char *value = wcs_to_bs(buf->buffer + start, end - start);
    config_skip_inline(buf, SKIP_WHITESPACE);
    if (buf->offset == buf->length) {
         buf->offset = offset;
         return NULL;
    }

    start = end = buf->offset;
    while (buf->offset < buf->length) {
        if (buf->buffer[buf->offset] == ',' || buf->buffer[buf->offset] == '}' ||
            buf->buffer[buf->offset] == ']' || buf->buffer[buf->offset] == '\n') {
            end++;
            break;
        } else {
            if (config_is_whitespace(buf->buffer[buf->offset])) {
                break;
            } else {
                end++;
            }
            buf->offset++;
        }
    }
    if (start == end) {
         buf->offset = offset;
        return NULL;
    }
    char *unit = wcs_to_bs(buf->buffer + start, end - start);

    config_skip_inline(buf, SKIP_WHITESPACE | SKIP_COMMENT);
    if (buf->offset != buf->length && buf->buffer[buf->offset] != '\n' && buf->buffer[buf->offset] != ',' &&
        buf->buffer[buf->offset] != ']' && buf->buffer[buf->offset] != '}') {
        buf->offset = offset;
        return NULL;
    }

    struct config *simple = malloc(sizeof(struct config));
    struct duration *duration = malloc(sizeof(struct duration));

    simple->type = CONFIG_DURATION_TYPE;
    simple->value = duration;
    duration->value = atol(value);

    if (strcmp(unit, "ns") == 0) {
        duration->unit = DURATION_NANO_SECOND;
    } else if (strcmp(unit, "us") == 0) {
        duration->unit = DURATION_MICRO_SECOND;
    } else if (strcmp(unit, "ms") == 0) {
        duration->unit = DURATION_MILLI_SECOND;
    } else if (strcmp(unit, "s") == 0) {
        duration->unit = DURATION_SECOND;
    } else if (strcmp(unit, "m") == 0) {
        duration->unit = DURATION_MINUTE;
    } else if (strcmp(unit, "h") == 0) {
        duration->unit = DURATION_HOUR;
    } else if (strcmp(unit, "d") == 0) {
        duration->unit = DURATION_DAY;
    } else {
        free(value);
        free(unit);
        free(simple);
        free(duration);
        buf->offset = offset;
        return NULL;
    }
    free(value);
    free(unit);
    return simple;
}

struct config *config_parse(char *buf, size_t len) {
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

    struct config_parse_buffer buffer = {wbuf, 0, wcslen};
    struct config *config = config_parse_object(&buffer);
    free(wbuf);
    return config;
}

struct config *config_load_file(const char *path) {
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

    struct config *config = config_parse(buf, len);
    free(buf);
    return config;
}

char *config_get_string(struct config *config, const char *name) {
    
}

uint64_t config_get_integer() {
}

double config_get_double() {
}

bool config_get_boolean() {
}

struct duration config_get_duration() {
}
