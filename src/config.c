#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <locale.h>
#include <errno.h>
#include <wchar.h>
#include <wctype.h>
#include <sys/file.h>

#include "map.h"
#include "list.h"
#include "config.h"

#ifndef __STDC_ISO_10646__
#error "you compiler dos't support unicode."
#endif

static struct config *config_parse_object(struct config_parser *p);
static struct config *config_parse_array(struct config_parser *p);
static struct config *config_parse_simple(struct config_parser *p);
static struct config *config_parse_string(struct config_parser *p);
static struct config *config_parse_number(struct config_parser *p);
static struct config *config_parse_boolean(struct config_parser *p);
static struct config *config_parse_duration(struct config_parser *p);

static int keycmp(void *key1, void *key2) {
    return strcmp((char *)key1, (char *)key2);
}

static inline bool config_is_whitespace(wchar_t c) {
    return iswspace(c);
}

/*
 * skip unnecessary characters.
 *
 * @param p  config parse pfer.
 * @param mode skip which class characters. can be SKIP_WHITESPACE, SKIP_COMMENT,
 *             SKIP_SAPARATOR or any combination.
 */
static inline void config_skip(struct config_parser *p, int mode) {
    while (p->offset < p->length) {
        if (config_is_whitespace(p->buffer[p->offset]) && (SKIP_WHITESPACE & mode)) {
            p->offset++;
            if (p->offset == p->length) return;
        } else if ((p->buffer[p->offset] == '#' || (p->buffer[p->offset] == '/' &&
                   p->offset < p->length - 1 && p->buffer[p->offset + 1] == '/')) &&
                   (SKIP_COMMENT & mode)) {
                while (p->offset < p->length) {
                    if (p->buffer[p->offset++] == '\n') {
                        break;
                    }
                }
                if (p->offset == p->length) return;
        } else {
            return;
        }
    }
}

/*
 * skip unnecessary in current line.
 */
static inline void config_skip_inline(struct config_parser *p, int mode) {
    while (p->offset < p->length) {
        if (p->buffer[p->offset] == '\n') {
            return;
        } else if (config_is_whitespace(p->buffer[p->offset]) && (SKIP_WHITESPACE & mode)) {
            p->offset++;
            if (p->offset == p->length) return;
        } else if ((p->buffer[p->offset] == '#' || (p->buffer[p->offset] == '/' &&
                   p->offset < p->length - 1 && p->buffer[p->offset + 1] == '/')) &&
                   (SKIP_COMMENT & mode)) {
                while (p->offset < p->length) {
                    if (p->buffer[p->offset++] == '\n') {
                        p->offset--;
                        return;
                    }
                }
                if (p->offset == p->length) return;
        } else {
            return;
        }
    }
}

char *wcs_to_bs(wchar_t p[], size_t len) {
    char *str = NULL; 
    int strlen = 0;
    for (int i = 0; i < len; i++) {
        char bytes[MB_CUR_MAX];
        int n = wctomb(bytes, p[i]);
        str = realloc(str, strlen + n);
        strncpy(str + strlen, bytes, n);
        strlen += n;
    }
    str = realloc(str, strlen + 1);
    str[strlen] = '\0';
    return str;
}

static struct config *config_error(struct config_parser *p, const char *msg) {
    size_t start = p->offset, line_num = 1, col;
    while (start > 0) {
        if (p->buffer[start - 1] == '\n') {
            break;
        }
        start--;
    }
    int i = start;
    while (i >= 0) {
        if (p->buffer[i] == '\n') {
            line_num++;
        }
        i--;
    }
    p->err_line = line_num;
    p->err_col = p->offset - start + 1;
    p->err_msg = strdup(msg);
    return NULL;
}


/*
 * p->buffer[p->offset] is non-empty, non-comment.
 * key[0] can't be '.'.
 * escape character in quoted keys has no special meaning.
 */
static wchar_t *config_parse_key(struct config_parser *p) {
    size_t start = p->offset, end = p->offset;
    while (p->offset < p->length) {
        if (p->buffer[p->offset] == '=' || p->buffer[p->offset] == ':') {
            break;
        } else if (p->buffer[p->offset] == '\n') {
            config_error(p, "object key can't expand multi-line.");
            return NULL;
        } else {
            if (!config_is_whitespace(p->buffer[p->offset])) end++;
            p->offset++;
        }
    }
    if (start == end) {
        config_error(p, "object key can't be empty.");
        return NULL;
    }

    if (p->buffer[start] == '.' || p->buffer[end - 1] == '.') {
        config_error(p, "key's first and last character can't be '.'.");
        return NULL;
    }

    bool quote = false, match = false;
    for (size_t i = start; i < end; i++) {
        if (p->buffer[i] == '"') {
            if (match) {
                config_error(p, "additional quotation mark."); 
                return NULL;
            }
            if (!quote) quote = true;
            else match = true;
            if (!match && i != start && p->buffer[i - 1] != '.') {
                config_error(p, "expecting '.' before start quotation mark.");
                return NULL;
            }
            if (match && i != end - 1 && p->buffer[i + 1] != '.') {
                config_error(p, "expecting '.' after end quotation mark.");
                return NULL;
            }
        }
    }

    if (quote && !match) {
        p->offset = end;
        config_error(p, "expecting close quotation mark.");
        return NULL;
    }

    wchar_t *key = malloc((end - start + 1) * sizeof(wchar_t));
    memcpy(key, p->buffer + start, (end - start) * sizeof(wchar_t));
    key[end - start] = L'\0';
    return key;
}

static struct config *config_parse_object(struct config_parser *p) {
    wchar_t c;
    bool has_fields = false, open_brace = false;
    struct config *value = NULL, *object;

    object = malloc(sizeof(struct config));
    object->type = CONFIG_OBJECT_TYPE;
    object->value = map_new(keycmp);

    while (p->offset < p->length) {
        config_skip(p, SKIP_WHITESPACE | SKIP_COMMENT);
        /* pfer is done. check if has open brace and return */
        /* happen when parsing root object */
        if (p->offset == p->length) {
            if (open_brace) {
                config_destroy(object);
                return config_error(p, "close brace not found in the end.");
            }
            return object;
        }

        /* get current offset's value, non-empty, non-comment*/
        c = p->buffer[p->offset];

        if (c == '{' && open_brace == false) {
            open_brace = true;
            p->offset += 1;
        } else if (c == '}') {
            if (open_brace) {
                p->offset += 1;
                return object;
            } else {
                config_destroy(object);
                return config_error(p, "unexpected close brace.");
            }
        } else if (c == ',') {
            if (has_fields) {
                p->offset += 1;
            } else {
                config_destroy(object);
                return config_error(p, "comma appears before first element of object.");
            }
        } else {
            size_t key_start = p->offset;
            wchar_t *key = config_parse_key(p);
            if (key == NULL) {
                config_destroy(object);
                return NULL;
            }

            config_skip_inline(p, SKIP_WHITESPACE);

            if (p->offset == p->length || (p->buffer[p->offset] != '=' && p->buffer[p->offset] != ':')) {
                free(key);
                config_destroy(object);
                return config_error(p, "can't find any separator of '=' or ':'.");
            }

            // found separator
            p->offset++;
            config_skip_inline(p, SKIP_WHITESPACE | SKIP_COMMENT);

            if (p->offset == p->length) {
                free(key);
                config_destroy(object);
                return config_error(p, "can't find object value.");
            }

            while (p->offset < p->length) {
                c = p->buffer[p->offset];

                if (c == '\n') {
                    free(key);
                    config_destroy(object);
                    return config_error(p, "value must start in the same line with key.");
                } else if (c == ',') {
                    free(key);
                    config_destroy(object);
                    return config_error(p, "value can't be empty.");
                } else if (c == '{') {
                    value = config_parse_object(p);
                    break;
                } else if (c == '[') {
                    value = config_parse_array(p);
                    break;
                } else {
                    value = config_parse_simple(p);
                    break;
                }
            }

            if (value == NULL) {
                free(key);
                config_destroy(object);
                return config_error(p, "unknown value type.");
            }

            size_t start = 0, end = 0;
            bool quote = false;
            char *pk = NULL;
            struct map *curr = object->value;

            for (int i = 0; i < wcslen(key); i++) {
                if (key[i] == '.' && quote == false) {
                    pk = wcs_to_bs(key + start, end - start);
                    if (!map_has(curr, pk)) {
                        struct config *child = malloc(sizeof(struct config));
                        child->type = CONFIG_OBJECT_TYPE;
                        child->value = map_new(keycmp);
                        map_add(curr, pk, child);
                        curr = child->value;
                    } else {
                        struct config *child;
                        map_get(curr, pk, (void **)&child);
                        curr = child->value;
                        free(pk);
                    }
                    start = i + 1;
                    end = i + 1;
                } else if (key[i] == '"') {
                    if (quote == false) quote = true;
                    else quote = false;
                    end++;
                } else {
                    end++;
                }
            }

            pk = wcs_to_bs(key + start, end - start);
            if (map_has(curr, pk)) {
                free(key);
                free(pk);
                config_destroy(value);
                config_destroy(object);
                p->offset = key_start;
                return config_error(p, "duplicated key.");
            } else {
                map_add(curr, pk, value);
            }

            free(key);
            has_fields = true;
        }
    }
}

/*
 * p->buffer[p->offset] == '['
 */
static struct config *config_parse_array(struct config_parser *p) {
    bool open_bracket = false, has_values = false;
    struct config *array = malloc(sizeof(struct config));
    array->type = CONFIG_ARRAY_TYPE;
    array->value = list_new();

    while (p->offset < p->length) {
        config_skip(p, SKIP_WHITESPACE | SKIP_COMMENT);
        if (p->buffer[p->offset] == '[') {
            if (open_bracket) {
                struct config *subarray = config_parse_array(p);
                if (subarray == NULL) {
                    config_destroy(array);
                    return NULL;
                }
                array->value = list_append(array->value, subarray);
                has_values = true;
            } else {
                open_bracket = true;
                p->offset++;
            }
        } else if (p->buffer[p->offset] == ']') {
            p->offset++;
            return array;
        } else if (p->buffer[p->offset] == ',') {
            if (has_values) {
                p->offset++;
            } else {
                config_destroy(array);
                return config_error(p, "comma appears before first element of array.");
            }
        } else if (p->buffer[p->offset] == '{') {
            struct config *object = config_parse_object(p);
            if (object == NULL) {
                config_destroy(array);
                return NULL;
            }
            array->value = list_append(array->value, object);
            has_values = true;
        } else {
            if (p->offset == p->length) {
                config_destroy(array);
                return config_error(p, "no close bracket.");
            }
            struct config *simple = config_parse_simple(p);
            if (simple == NULL) {
                config_destroy(array);
                return NULL;
            }
            array->value = list_append(array->value, simple);
            has_values = true;
        }
    }
}

/*
 * p->buffer[p->offset] is non-empty, non-comment.
 * simple value parse function must handle ',', '}' , ']' character and pfer end.
 * if simple value is single line, also need hanle '\n'.
 */
static struct config *config_parse_simple(struct config_parser *p) {
    wchar_t c;
    struct config *simple;

    if (simple = config_parse_string(p)) {
        return simple;
    } else if (simple = config_parse_number(p)) {
        return simple;
    } else if (simple = config_parse_boolean(p)) {
        return simple;
    } else if (simple = config_parse_duration(p)) {
        return simple;
    } else {
        return NULL;
    }
}

/*
 * string is double quoted unicode characters except '"', '\' and
 * control characters. escape characters: 
 * \", \\, \/, \b, \f, \n, \r, \t, \u .
 */
static struct config *config_parse_string(struct config_parser *p) {
    size_t start = p->offset, end = p->offset, offset = p->offset;
    bool match = false;

    if (p->buffer[p->offset] != '"') {
        p->offset = offset;
        return NULL;
    }

    p->offset++;

    while (p->offset < p->length) {
        /* read end of line or object fields separator ',' */
        if (p->buffer[p->offset] == '\n' && !match) {
            p->offset = offset;
            return NULL;
        } else if (p->buffer[p->offset] == '"' && p->buffer[p->offset - 1] != '\\'){
            end = ++p->offset;
            break;
        } else {
            p->offset++;
        }
    }

    // skip qoutes
    start += 1; end -= 1;

    char *str = wcs_to_bs(p->buffer + start, end - start);
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
                free(new);
                p->offset = offset;
                return NULL;
            }

            char *e, us[5];
            strncpy(us, p, 4);
            us[4] = '\0';
            wchar_t wc = strtol(us, &e, 16);
            if (*e != '\0') {
                free(new);
                p->offset = offset;
                return NULL;
            }

            char bytes[MB_CUR_MAX];
            int nb = wcrtomb(bytes, wc, NULL);
            if (nb == (size_t) -1) {
                free(new);
                p->offset = offset;
                return NULL;
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

    struct config *simple = malloc(sizeof(struct config));
    simple->type = CONFIG_STRING_TYPE;
    simple->value = new;
    return simple;
}

static struct config *config_parse_number(struct config_parser *p) {
    size_t start, end, offset = p->offset;
    int sign = 1, base = 10;
    enum config_type type = CONFIG_INTEGER_TYPE;
    bool dot_parsed = false;
    struct config *simple;

    if (p->buffer[p->offset] == '+') {
        p->offset++;
    } else if (p->buffer[p->offset] == '-') {
        sign = -1;
        p->offset++;
    }

    if (p->offset == p->length) {
        p->offset = offset;
        return NULL;
    }

    if (p->buffer[p->offset] == '0') {
        p->offset++;
        if (p->offset == p->length) {
            simple = malloc(sizeof(struct config));
            simple->type = CONFIG_INTEGER_TYPE;
            simple->value = 0;
            return simple;
        } else if (p->buffer[p->offset] == 'x' || p->buffer[p->offset] == 'X') {
            base = 16;
            p->offset++;
        } else if (iswdigit(p->buffer[p->offset])) {
            base = 8;
        } else {
            p->offset = offset;
            return NULL;
        }
    }

    if (p->offset == p->length && base == 16) {
        p->offset = offset;
        return NULL;
    }

    /*
     * 1. p->offset == p->length && (base == 8 || base == 10)
     *    following while will not run, return NULL.
     * 2. p->offset != p->length && (base == 8 || base == 10 || base == 16)
     *    if character is '.' or digit end++, p->offset++
     *    else if character may be whitespace, normal character and special end character.
     */ 
    start = p->offset; end = p->offset;
    while (p->offset < p->length) {
        if (p->buffer[p->offset] == '.') {
            if (dot_parsed) {
                p->offset = offset;
                return NULL;
            } else {
                p->offset++;
                end++;
                dot_parsed = true;
                type = CONFIG_DOUBLE_TYPE;
            }
        } else if (iswdigit(p->buffer[p->offset])) {
            p->offset++;
            end++;
        } else if (base == 16 && (p->buffer[p->offset] >= 'a' && p->buffer[p->offset] <= 'f') ||
                   p->buffer[p->offset] >= 'A' && p->buffer[p->offset] <= 'F') {
            p->offset++;
            end++;
        } else {
            config_skip_inline(p, SKIP_WHITESPACE | SKIP_COMMENT);
            if (p->offset == p->length || p->buffer[p->offset] == '\n' || p->buffer[p->offset] == ',' ||
                p->buffer[p->offset] == ']' || p->buffer[p->offset] == '}') {
                break;
            } else {
                p->offset = offset;
                return NULL;
            }
        }
    }
    if (start == end) {
        p->offset = offset;
        return NULL;
    }
    if (type == CONFIG_DOUBLE_TYPE && (p->buffer[start] == '.' || p->buffer[end - 1] == '.')) {
        p->offset = offset;
        return NULL;
    }

    simple = malloc(sizeof(struct config));
    simple->type = type;
    char *str = wcs_to_bs(p->buffer + start, end - start);
    if (type == CONFIG_INTEGER_TYPE) {
        uint64_t *value = malloc(sizeof(uint64_t));
        *value = strtoll(str, NULL, base) * sign;
        simple->value = value;
    } else {
        double *value = malloc(sizeof(double));
        *value = atof(str) * sign;
        simple->value = value;
    }
    free(str);

    return simple;
}

static struct config *config_parse_boolean(struct config_parser *p) {
    size_t start = p->offset, end = p->offset, offset = p->offset, length = p->length;
    wchar_t *pfer = p->buffer;
    struct config *simple = NULL;

    if (offset <= length - 4 && pfer[offset] == 't' && pfer[offset + 1] == 'r' &&
        pfer[offset + 2] == 'u' && pfer[offset + 3] == 'e') {
        simple = malloc(sizeof(struct config));
        simple->type = CONFIG_BOOLEAN_TYPE;
        simple->value = malloc(sizeof(bool));
        *(bool *)(simple->value) = true;
        p->offset += 4;
    }
    if (offset <= length - 5 && pfer[offset] == 'f' && pfer[offset + 1] == 'a' &&
        pfer[offset + 2] == 'l' && pfer[offset + 3] == 's' && pfer[offset + 4] == 'e') {
        simple = malloc(sizeof(struct config));
        simple->type = CONFIG_BOOLEAN_TYPE;
        simple->value = malloc(sizeof(bool));
        *(bool *)(simple->value) = false;
        p->offset += 5;
    }
    return simple;
}

/*
 * parse duration type.
 *
 * supported unit: ns, us, ms, s, m, h, d.
 */
static struct config *config_parse_duration(struct config_parser *p) {
    /*parse value or unit */
    size_t start, end, offset = p->offset;

    start = end = p->offset;
    while (p->offset < p->length) {
        if (iswdigit(p->buffer[p->offset])) end = ++p->offset;
        else break;
    }
    if (start == end) {
         p->offset = offset;
         return NULL;
    }
    char *value = wcs_to_bs(p->buffer + start, end - start);
    config_skip_inline(p, SKIP_WHITESPACE);
    if (p->offset == p->length) {
         p->offset = offset;
         return NULL;
    }

    start = end = p->offset;
    while (p->offset < p->length) {
        if (!config_is_whitespace(p->buffer[p->offset])) {
            if (p->buffer[p->offset] == ',' || p->buffer[p->offset] == '}' ||
                p->buffer[p->offset] == ']' || p->buffer[p->offset] == '\n') {
                break;
            }
            end++;
            p->offset++;
        } else {
            break;
        }
    }
    if (start == end) {
         p->offset = offset;
        return NULL;
    }
    char *unit = wcs_to_bs(p->buffer + start, end - start);

    config_skip_inline(p, SKIP_WHITESPACE | SKIP_COMMENT);
    if (p->offset != p->length && p->buffer[p->offset] != '\n' && p->buffer[p->offset] != ',' &&
        p->buffer[p->offset] != ']' && p->buffer[p->offset] != '}') {
        free(value);
        free(unit);
        p->offset = offset;
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
        p->offset = offset;
        return NULL;
    }
    free(value);
    free(unit);
    return simple;
}

/*
 * parse utf-8 encoded byte string to config object.
 *
 * @param[input] p utf-8 encoded byte string.
 * @param len byte string length.
 * @param[output] errmsg error message.
 * @return if config parse success, return config object, and errmsg
 *         set to NULL. if parse failed, return NULL, and set errmsg
 *         to the reason. calling function need to free errmsg.
 */
struct config *config_parse(char *p, size_t len, char **errmsg) {
    /* remove utf-8 bom if it has */
    if (len >= 3 && p[0] == 0xef && p[1] == 0xbb && p[2] == 0xbf) {
        p += 3;
        len -= 3;
    }

    /* convert char p to wchar_t p */
    mbstate_t mbs;
    wchar_t *wp = NULL, wc;
    size_t nbytes, wplen = 0, wcslen = 0;
    char *tp = p;
    memset(&mbs, 0, sizeof(mbs));
    while (len > 0) {
        if ((nbytes = mbrtowc(&wc, tp, len, &mbs)) > 0) {
            if (nbytes >= (size_t) -2) {
                if (errmsg != NULL) {
                    *errmsg = strdup("config file encoding error.");
                    return NULL;
                }
            }
            if (wcslen >= wplen) {
                wplen += 64;
                wp = realloc(wp, wplen * sizeof(wchar_t));
            }
            wp[wcslen++] = wc;
            len -= nbytes;
            tp += nbytes;
        } else {
            if (errmsg != NULL)
            *errmsg = strdup("invalid '\0' character in config file.");
            return NULL;
        }
    }

    struct config_parser pfer;
    pfer.pfer = wp;
    pfer.offset = 0;
    pfer.length = wcslen;

    struct config *config = config_parse_object(&pfer);

    free(wp);

    if (config != NULL) {
        *errmsg = NULL;
        return config;
    }

    if (errmsg != NULL) {
        char *errfmt = "line %ld column %ld: %s";
        size_t errlen = snprintf(NULL, 0, errfmt, pfer.err_line, pfer.err_col, pfer.err_msg);
        errlen++;
        *errmsg = malloc(errlen);
        snprintf(*errmsg, errlen, errfmt, pfer.err_line, pfer.err_col, pfer.err_msg);
        free(pfer.err_msg);
    }

    return NULL;
}

/*
 * load config from file.
 *
 * @param path file path.
 * @param[output] errmsg error message.
 * @return if config parse success, return config object, and errmsg
 *         set to NULL. if parse failed, return NULL, and set errmsg
 *         to the reason. calling function need to free errmsg.
 */
struct config *config_load(const char *path, char **errmsg) {
    /* get file content to a p */
    FILE *fp = fopen(path, "r");
    if (!fp) {
        char *errfmt = "unable to open config file: %s";
        size_t errlen = snprintf(NULL, 0, errfmt, path);
        errlen++;
        *errmsg = malloc(errlen);
        snprintf(*errmsg, errlen, errfmt, path);
        return NULL;
    }
    flock(fileno(fp), LOCK_EX);
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char *p = malloc(len);
    int nread = 0, total = 0;
    while(!feof(fp)) {
        nread = fread(p + total, 1, 64, fp);
        total += nread;
    }
    flock(fileno(fp), LOCK_UN);
    fclose(fp);

    char *errp;
    struct config *config = config_parse(p, len, &errp);

    free(p);

    if (config != NULL) {
        *errmsg = NULL;
        return config;
    }

    if (errmsg != NULL) {
        char *errfmt = "config load error in file \"%s\":\n%s";
        size_t errlen = snprintf(NULL, 0, errfmt, path, errp);
        errlen++;
        *errmsg = malloc(errlen);
        snprintf(*errmsg, errlen, errfmt, path, errp);
        free(errp);
    }

    return NULL;
}

static void config_dumps_internal(struct config *config, int level) {
    if (config == NULL) return; 
    if (config->type == CONFIG_OBJECT_TYPE  ) {
        printf("{\n");
        struct list *key, *keys = map_keys(config->value);
        key = keys;
        while (key) {
            for (int i = 0; i < level; i++) {
                printf("    ");
            }
            printf("%s: ", key->data);
            void *data;
            map_get(config->value, key->data, &data);
            config_dumps_internal(data, level + 1);
            key = list_next(key);
        }
        list_destroy(keys);
        for (int i = 1; i < level; i++) {
            printf("    ");
        }
        printf("}\n");
    } else if (config->type == CONFIG_ARRAY_TYPE) {
        struct list *p, *vs = config->value;
        p = vs;
        printf("[\n");
        while (p) {
            for (int i = 0; i < level; i++) {
                printf("    ");
            }
            config_dumps_internal(p->data, level + 1);
            p = list_next(p);
        }
        for (int i = 1; i < level; i++) {
            printf("    ");
        }
        printf("]\n");
    } else if (config->type == CONFIG_STRING_TYPE) {
        printf("\"%s\"\n", config->value);
    } else if (config->type == CONFIG_INTEGER_TYPE) {
        printf("%ld\n", *(uint64_t *)(config->value));
    } else if (config->type == CONFIG_BOOLEAN_TYPE) {
        printf("%s\n", *(bool *)(config->value) ? "true" : "false");
    } else if (config->type == CONFIG_DURATION_TYPE) {
        struct duration *d = config->value;
        printf("%d", d->value);
        if (d->unit == DURATION_NANO_SECOND) {
            printf("ns");
        } else if (d->unit == DURATION_MICRO_SECOND) {
            printf("us");
        } else if (d->unit == DURATION_MILLI_SECOND) {
            printf("ms");
        } else if (d->unit == DURATION_SECOND) {
            printf("s");
        } else if (d->unit == DURATION_MINUTE) {
            printf("m");
        } else if (d->unit == DURATION_HOUR) {
            printf("h");
        } else if (d->unit == DURATION_DAY) {
            printf("d");
        }
        printf("\n");
    }
}

void config_dumps(struct config *config) {
    config_dumps_internal(config, 1);
}

void config_destroy(struct config *config) {
    if (config == NULL) return;
    if (config->type == CONFIG_OBJECT_TYPE) {
        struct list *key, *keys = map_keys(config->value);
        key = keys;
        while (key) {
            void *orig_key, *orig_data;
            map_remove(config->value, list_data(key), &orig_key, &orig_data);
            free(orig_key);
            config_destroy(orig_data);
            key = list_next(key);
        }
        map_destroy(config->value);
        list_destroy(keys);
        free(config);
    } else if (config->type == CONFIG_ARRAY_TYPE) {
        struct list *p, *vs = config->value;
        p = vs;
        while (p) {
            config_destroy(p->data);
            p = list_next(p);
        }
        list_destroy(vs);
        free(config);
    } else if (config->type == CONFIG_STRING_TYPE ||
               config->type == CONFIG_INTEGER_TYPE ||
               config->type == CONFIG_DOUBLE_TYPE ||
               config->type == CONFIG_BOOLEAN_TYPE ||
               config->type == CONFIG_DURATION_TYPE) {
        free(config->value);
        free(config);
    } else {
        fprintf(stderr, "config destroy error, if you use config_load "
                        "series functions, there may be a bug.\n");
        exit(EXIT_FAILURE);
    }
}

static struct config *config_get(struct config *config, const char *key) {
    if (config == NULL || config->type != CONFIG_OBJECT_TYPE) return NULL;
    bool quote = false;
    size_t start = 0, end = 0;
    struct map *curr = config->value;
    for (int i = 0; i < strlen(key); i++) {
        if (key[i] == '.' && quote == false) {
            char *k = strndup(key + start, end - start);
            struct config *child;
            if (!map_get(curr, k, (void **)&child)) {
                free(k);
                return NULL;
            }
            if (child->type != CONFIG_OBJECT_TYPE) {
                free(k);
                return NULL;
            }
            free(k);
            curr = child->value;
            start = i + 1;
            end = i + 1;
        } else if (key[i] == '"') {
            if (quote == false) quote = true;
            else quote = false;
            end++;
        } else {
            end++;
        }
    }
    struct config *value;
    if (!map_get(curr, (void *)&key[start], (void **)&value)) {
        return NULL;
    }
    return value;
}

struct config *config_get_object(struct config *config, const char *key) {
    struct config *value = config_get(config, key);
    if (!value || value->type != CONFIG_OBJECT_TYPE) {
        return NULL;
    }
    return value;
}

struct list *config_get_object_keys(struct config *config) {
    if (!config || config->type != CONFIG_OBJECT_TYPE) {
        return NULL;
    }
    return map_keys(config->value);
}

struct config *config_get_array(struct config *config, const char *key) {
    struct config *value = config_get(config, key);
    if (!value || value->type != CONFIG_ARRAY_TYPE) {
        return NULL;
    }
    return value;
}

const char *config_get_string(struct config *config, const char *key, const char *def) {
    struct config *value = config_get(config, key);
    if (!value || value->type != CONFIG_STRING_TYPE) {
        return def;
    }
    return value->value;
}

long long config_get_integer(struct config *config, const char *key, long long def) {
    struct config *value = config_get(config, key);
    if (!value || value->type != CONFIG_INTEGER_TYPE) {
        return def;
    }
    return *(long long *)value->value;
}

double config_get_double(struct config *config, const char *key, double def) {
    struct config *value = config_get(config, key);
    if (!value || value->type != CONFIG_DOUBLE_TYPE) {
        return def;
    }
    return *(double *)value->value;
}

bool config_get_boolean(struct config *config, const char *key, bool def) {
    struct config *value = config_get(config, key);
    if (!value || value->type != CONFIG_BOOLEAN_TYPE) {
        return def;
    }
    return *(bool *)value->value;
}

struct duration config_get_duration(struct config *config, const char *key, long long value, enum duration_unit unit) {
    struct config *d = config_get(config, key);
    if (!d || d->type != CONFIG_DURATION_TYPE) {
        struct duration r;
        r.value = value;
        r.unit = unit;
        return r;
    }
    return *(struct duration *)d->value;
}
