#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <locale.h>
#include <errno.h>
#include <wchar.h>
#include <wctype.h>
#include <sys/file.h>

#include "str.h"
#include "map.h"
#include "list.h"
#include "config.h"

#ifndef __STDC_ISO_10646__
#error "you compiler dos't support unicode."
#endif

const char *config_error_text[] = {
    [CONFIG_INVALID_ENCODING] = "invalid encoding",
    [CONFIG_OPEN_FILE_FAILED] = "open file failed",
    [CONFIG_INVALID_KEY] = "invalid key",
    [CONFIG_DUPLICATED_KEY] = "duplicated key",
    [CONFIG_ADDITION_QUOTATION] = "additional quotation mark '\"'",
    [CONFIG_EXPECTING_QUOTATION] = "expecting quotation mark '\"'",
    [CONFIG_EXPECTING_CLOSE_BRACE] = "expecting close brace '}'",
    [CONFIG_EXPECTING_VALUE] = "expecting a value",
    [CONFIG_EXPECTING_SEPERATOR] = "expecting seperator '=' or ':'",
    [CONFIG_EXPECTING_CLOSE_BRACKET] = "expecting close bracket ']'",
    [CONFIG_UNEXPECTED_OPEN_BRACE] = "unexpected open brace '{'",
    [CONFIG_UNEXPECTED_CLOSE_BRACE] = "unexpected close brace '}'",
    [CONFIG_UNEXPECTED_COMMA] = "unexpected comma ','",
    [CONFIG_UNEXPECTED_NEWLINE] = "unexpected newline",
    [CONFIG_UNKNOWN_VALUE] = "unkown value type"
};

static struct config *config_parse_object(struct config_parser *p, struct config_error *e);
static struct config *config_parse_array(struct config_parser *p, struct config_error *e);
static struct config *config_parse_simple(struct config_parser *p);
static struct config *config_parse_string(struct config_parser *p);
static struct config *config_parse_number(struct config_parser *p);
static struct config *config_parse_boolean(struct config_parser *p);
static struct config *config_parse_duration(struct config_parser *p);
static struct config *config_parse_size(struct config_parser *p);

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
static void config_skip(struct config_parser *p, int mode) {
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

static void config_parse_error(struct config_parser *p, struct config_error *e, enum config_error_code code) {
    if (e == NULL) {
        return;
    }
    e->code = code;
    e->text = config_error_text[code];
    e->position = p ? p->offset + 1 : 1;
    e->line = 1;
    e->column = 1;
    if (p) {
        wchar_t *c = p->buffer + p->offset;
        while (c > p->buffer) {
            c--;
            e->column++;
            if (*c == 0xa) { /* new line '\n' */
                break;
            }
        }
        while (c > p->buffer) {
            c--;
            if (*c == 0xa) {
                e->line++;
            }
        }
    }
}


/*
 * p->buffer[p->offset] is non-empty, non-comment.
 * key[0] can't be '.'.
 * escape character in quoted keys has no special meaning.
 */
static wchar_t *config_parse_key(struct config_parser *p, struct config_error *e) {
    size_t start = p->offset, end;

    while (p->offset < p->length) {
        if (p->buffer[p->offset] == '=' || p->buffer[p->offset] == ':') {
            break;
        } else if (p->buffer[p->offset] == '\n') {
            config_parse_error(p, e, CONFIG_UNEXPECTED_NEWLINE);
            return NULL;
        }
        p->offset++;
    }

    end = p->offset - 1;

    while (end > start) {
        if (!config_is_whitespace(p->buffer[end])) {
            end++;
            break;
        }
        end--;
    }
    
    if (start == end) {
        p->offset = start;
        config_parse_error(p, e, CONFIG_INVALID_KEY);
        return NULL;
    }

    bool quote = false, match = true;
    for (size_t i = start; i < end; i++) {
        if (p->buffer[i] == '"') {
            if (!quote) {
                match = false;
                quote = true;
            } else {
                match = true;
                quote = false;
            }
            /* check before open quote if is dot */
            if (quote && i != start && p->buffer[i - 1] != '.') {
                config_parse_error(p, e, CONFIG_INVALID_KEY);
                p->offset = i;
                return NULL;
            }
            /* check after open quote if is dot */
            if (match && i != end - 1 && p->buffer[i + 1] != '.') {
                p->offset = i;
                config_parse_error(p, e, CONFIG_INVALID_KEY);
                return NULL;
            }
        } else if (p->buffer[i] == '.') {
            /* first and last character can't be '.' */
            if (i == start || i == end -1) {
                p->offset = i;
                config_parse_error(p, e, CONFIG_INVALID_KEY);
                return NULL;
            }
        }
    }

    if (!match) {
        config_parse_error(p, e, CONFIG_EXPECTING_QUOTATION);
        return NULL;
    }

    return wcsndup(p->buffer + start, end - start);
}

static struct config *config_parse_object(struct config_parser *p, struct config_error *e) {
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
                config_parse_error(p, e, CONFIG_EXPECTING_CLOSE_BRACE);
                return NULL;
            } else {
                return object;
            }
        }

        /* get current offset's value, non-empty, non-comment*/
        c = p->buffer[p->offset];

        if (c == '{') {
            if (!open_brace) {
                open_brace = true;
                p->offset += 1;
            } else {
                config_destroy(object);
                config_parse_error(p, e, CONFIG_UNEXPECTED_OPEN_BRACE);
                return NULL;
            }
        } else if (c == '}') {
            if (open_brace) {
                p->offset += 1;
                return object;
            } else {
                config_destroy(object);
                config_parse_error(p, e, CONFIG_UNEXPECTED_CLOSE_BRACE);
                return NULL;
            }
        } else if (c == ',') {
            if (has_fields) {
                p->offset += 1;
            } else {
                config_destroy(object);
                config_parse_error(p, e, CONFIG_UNEXPECTED_COMMA);
                return NULL;
            }
        } else {
            size_t key_start = p->offset;
            wchar_t *key = config_parse_key(p, e);
            if (key == NULL) {
                /* error has been set in config_parse_key() */
                config_destroy(object);
                return NULL;
            }

            config_skip_inline(p, SKIP_WHITESPACE);

            if (p->offset == p->length || (p->buffer[p->offset] != '=' && p->buffer[p->offset] != ':')) {
                free(key);
                config_destroy(object);
                config_parse_error(p, e, CONFIG_EXPECTING_SEPERATOR);
                return NULL;
            }

            // found separator
            p->offset++;
            config_skip_inline(p, SKIP_WHITESPACE | SKIP_COMMENT);

            if (p->offset == p->length) {
                free(key);
                config_destroy(object);
                config_parse_error(p, e, CONFIG_EXPECTING_VALUE);
                return NULL;
            }

            while (p->offset < p->length) {
                c = p->buffer[p->offset];

                if (c == '\n') {
                    free(key);
                    config_destroy(object);
                    config_parse_error(p, e, CONFIG_UNEXPECTED_NEWLINE);
                    return NULL;
                } else if (c == '{') {
                    value = config_parse_object(p, e);
                    break;
                } else if (c == '[') {
                    value = config_parse_array(p, e);
                    break;
                } else {
                    value = config_parse_simple(p);
                    break;
                }
            }

            if (value == NULL) {
                free(key);
                config_destroy(object);
                config_parse_error(p, e, CONFIG_UNKNOWN_VALUE);
                return NULL;
            }

            size_t start = 0, end = 0;
            bool quote = false;
            char *pk = NULL;
            struct map *curr = object->value;

            for (size_t i = 0; i < wcslen(key); i++) {
                if (key[i] == '.' && quote == false) {
                    pk = strutf8nenc(key + start, end - start);
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

            pk = strutf8nenc(key + start, end - start);
            if (map_has(curr, pk)) {
                free(key);
                free(pk);
                config_destroy(value);
                config_destroy(object);
                p->offset = key_start;
                config_parse_error(p, e, CONFIG_DUPLICATED_KEY);
                return NULL;
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
static struct config *config_parse_array(struct config_parser *p, struct config_error *e) {
    bool open_bracket = false, has_values = false;
    struct config *array = malloc(sizeof(struct config));
    array->type = CONFIG_ARRAY_TYPE;
    array->value = list_new();

    while (p->offset < p->length) {
        config_skip(p, SKIP_WHITESPACE | SKIP_COMMENT);
        if (p->buffer[p->offset] == '[') {
            if (open_bracket) {
                struct config *subarray = config_parse_array(p, e);
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
                config_parse_error(p, e, CONFIG_UNEXPECTED_COMMA);
                return NULL;
            }
        } else if (p->buffer[p->offset] == '{') {
            struct config *object = config_parse_object(p, e);
            if (object == NULL) {
                config_destroy(array);
                return NULL;
            }
            array->value = list_append(array->value, object);
            has_values = true;
        } else {
            if (p->offset == p->length) {
                config_destroy(array);
                config_parse_error(p, e, CONFIG_EXPECTING_CLOSE_BRACKET);
                return NULL;
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
    struct config *simple;

    if (simple = config_parse_string(p)) {
        return simple;
    } else if (simple = config_parse_number(p)) {
        return simple;
    } else if (simple = config_parse_boolean(p)) {
        return simple;
    } else if (simple = config_parse_duration(p)) {
        return simple;
    } else if (simple = config_parse_size(p)) {
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
        if (p->buffer[p->offset] == '\n') {
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

    char *str = strutf8nenc(p->buffer + start, end - start);
    char *new = malloc(64);
    size_t index = 0, size = 0;
    for (char *c = str; *c != '\0';) {
        if ((*c == '"' || *c == '\\' || *c == '/' || *c == 'b' ||
            *c == 'f' || *c == 'n' || *c == 'r' || *c == 't') && (c > str) && *(c - 1) == '\\') {
            if (*c == '"') new[index - 1] = '\"';
            else if (*c == '\\') new[index - 1] = '\\';
            else if (*c == '/') new[index - 1] = '/';
            else if (*c == 'b') new[index - 1] = '\b';
            else if (*c == 'f') new[index - 1] = '\f';
            else if (*c == 'n') new[index - 1] = '\n';
            else if (*c == 'r') new[index - 1] = '\r';
            else if (*c == 't') new[index - 1] = '\t';
            c++;
        } else if (*c == 'u' && (c > str) && *(c - 1) == '\\') {
            c++;
            if (*c == '\0' || *(c + 1) == '\0' || *(c + 2) == '\0' || *(c + 3) == '\0') {
                free(new);
                p->offset = offset;
                return NULL;
            }

            char *e, us[5];
            strncpy(us, c, 4);
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
            c += 4;
        } else {
            if (index == size) {
                new = realloc(new, size + 64);
                size += 64;
            }
            new[index++] = *c++;
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
        } else if (base == 16 && ((p->buffer[p->offset] >= 'a' && p->buffer[p->offset] <= 'f') ||
                   p->buffer[p->offset] >= 'A' && p->buffer[p->offset] <= 'F')) {
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
    char *str = strutf8nenc(p->buffer + start, end - start);
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
    char *value = strutf8nenc(p->buffer + start, end - start);
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
    char *unit = strutf8nenc(p->buffer + start, end - start);

    config_skip_inline(p, SKIP_WHITESPACE | SKIP_COMMENT);
    if (p->offset != p->length && p->buffer[p->offset] != '\n' && p->buffer[p->offset] != ',' &&
        p->buffer[p->offset] != ']' && p->buffer[p->offset] != '}') {
        free(value);
        free(unit);
        p->offset = offset;
        return NULL;
    }

    struct config *simple = malloc(sizeof(struct config));
    struct config_duration *duration = malloc(sizeof(struct config_duration));

    simple->type = CONFIG_DURATION_TYPE;
    simple->value = duration;
    duration->value = atoll(value);

    if (strcmp(unit, "ns") == 0) {
        duration->unit = CONFIG_NANO_SECOND;
    } else if (strcmp(unit, "us") == 0) {
        duration->unit = CONFIG_MICRO_SECOND;
    } else if (strcmp(unit, "ms") == 0) {
        duration->unit = CONFIG_MILLI_SECOND;
    } else if (strcmp(unit, "s") == 0) {
        duration->unit = CONFIG_SECOND;
    } else if (strcmp(unit, "m") == 0) {
        duration->unit = CONFIG_MINUTE;
    } else if (strcmp(unit, "h") == 0) {
        duration->unit = CONFIG_HOUR;
    } else if (strcmp(unit, "d") == 0) {
        duration->unit = CONFIG_DAY;
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

static struct config *config_parse_size(struct config_parser *p) {
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
    char *value = strutf8nenc(p->buffer + start, end - start);
    config_skip_inline(p, SKIP_WHITESPACE);

    /* has got a value, parse unit */
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
    char *unit = strutf8nenc(p->buffer + start, end - start);

    config_skip_inline(p, SKIP_WHITESPACE | SKIP_COMMENT);
    if (p->offset != p->length && p->buffer[p->offset] != '\n' && p->buffer[p->offset] != ',' &&
        p->buffer[p->offset] != ']' && p->buffer[p->offset] != '}') {
        free(value);
        free(unit);
        p->offset = offset;
        return NULL;
    }

    struct config *simple = malloc(sizeof(struct config));
    struct config_size *size = malloc(sizeof(struct config_size));

    simple->type = CONFIG_SIZE_TYPE;
    simple->value = size;
    size->value = atoll(value);

    if (strcmp(unit, "B") == 0) {
        size->unit = CONFIG_BYTE;
    } else if (strcmp(unit, "KB") == 0) {
        size->unit = CONFIG_KILO_BYTE;
    } else if (strcmp(unit, "MB") == 0) {
        size->unit = CONFIG_MEGA_BYTE;
    } else if (strcmp(unit, "GB") == 0) {
        size->unit = CONFIG_GIGA_BYTE;
    } else if (strcmp(unit, "TB") == 0) {
        size->unit = CONFIG_TERA_BYTE;
    } else if (strcmp(unit, "PB") == 0) {
        size->unit = CONFIG_PETA_BYTE;
    } else if (strcmp(unit, "EB") == 0) {
        size->unit = CONFIG_EXA_BYTE;
    } else if (strcmp(unit, "ZB") == 0) {
        size->unit = CONFIG_ZETTA_BYTE;
    } else if (strcmp(unit, "YB") == 0) {
        size->unit = CONFIG_YOTTA_BYTE;
    } else {
        free(value);
        free(unit);
        free(simple);
        free(size);
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
 * @param[output] e config error info.
 * @return if config parse success, return config object, and errmsg
 *         set to NULL. if parse failed, return NULL, and set errmsg
 *         to the reason. calling function need to free errmsg.
 */
struct config *config_parse(char *buf, size_t len, struct config_error *e) {
    /* remove utf-8 bom if it has */
    if (len >= 3 && buf[0] == 0xef && buf[1] == 0xbb && buf[2] == 0xbf) {
        buf += 3;
        len -= 3;
    }

    char *tmp = strndup(buf, len);
    wchar_t *wbuf = strutf8dec(tmp);
    free(tmp);

    if (!wbuf) {
        config_parse_error(NULL, e, CONFIG_INVALID_ENCODING);
        return NULL;
    }

    struct config_parser p;
    p.buffer = wbuf;
    p.offset = 0;
    p.length = wcslen(wbuf);

    struct config *config = config_parse_object(&p, e);

    free(wbuf);

    return config;
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
struct config *config_load(const char *path, struct config_error *e) {
    /* get file content to a p */
    FILE *fp = fopen(path, "r");
    if (!fp) {
        config_parse_error(NULL, e, CONFIG_OPEN_FILE_FAILED);
        return NULL;
    }
    flock(fileno(fp), LOCK_EX);
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buf = malloc(len);
    int nread = 0, total = 0;
    while(!feof(fp)) {
        nread = fread(buf + total, 1, 64, fp);
        total += nread;
    }
    flock(fileno(fp), LOCK_UN);
    fclose(fp);

    struct config *config = config_parse(buf, len, e);

    free(buf);

    return config;
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
        printf("%lld\n", *(uint64_t *)(config->value));
    } else if (config->type == CONFIG_DOUBLE_TYPE) {
        printf("%f\n", *(double *)(config->value));
    } else if (config->type == CONFIG_BOOLEAN_TYPE) {
        printf("%s\n", *(bool *)(config->value) ? "true" : "false");
    } else if (config->type == CONFIG_DURATION_TYPE) {
        struct config_duration *d = config->value;
        printf("%lld", d->value);
        if (d->unit == CONFIG_NANO_SECOND) {
            printf("ns");
        } else if (d->unit == CONFIG_MICRO_SECOND) {
            printf("us");
        } else if (d->unit == CONFIG_MILLI_SECOND) {
            printf("ms");
        } else if (d->unit == CONFIG_SECOND) {
            printf("s");
        } else if (d->unit == CONFIG_MINUTE) {
            printf("m");
        } else if (d->unit == CONFIG_HOUR) {
            printf("h");
        } else if (d->unit == CONFIG_DAY) {
            printf("d");
        }
        printf("\n");
    } else if (config->type == CONFIG_SIZE_TYPE) {
        struct config_size *v = config->value;
        printf("%lld", v->value);
        if (v->unit == CONFIG_BYTE) {
            printf("B");
        } else if (v->unit == CONFIG_KILO_BYTE) {
            printf("KB");
        } else if (v->unit == CONFIG_MEGA_BYTE) {
            printf("MB");
        } else if (v->unit == CONFIG_GIGA_BYTE) {
            printf("GB");
        } else if (v->unit == CONFIG_TERA_BYTE) {
            printf("TB");
        } else if (v->unit == CONFIG_PETA_BYTE) {
            printf("PB");
        } else if (v->unit == CONFIG_EXA_BYTE) {
            printf("EB");
        } else if (v->unit == CONFIG_ZETTA_BYTE) {
            printf("ZB");
        } else if (v->unit == CONFIG_YOTTA_BYTE) {
            printf("YB");
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
               config->type == CONFIG_DURATION_TYPE ||
               config->type == CONFIG_SIZE_TYPE) {
        free(config->value);
        free(config);
    } else {
        fprintf(stderr, "config destroy error, if you use config_load "
                        "series functions, there may be a bug.\n");
    }
}

enum config_type config_type(struct config *config) {
    return config->type;
}

struct config *config_object_get(struct config *config, const char *path) {
    bool quote = false;
    size_t start = 0, end = 0;
    struct map *curr = config->value;
    char pk[strlen(path) + 1];
    for (int i = 0; i < strlen(path); i++) {
        if (path[i] == '.' && quote == false) {
            strncpy(pk, path + start, end - start);
            pk[end - start] = '\0';
            struct config *child;
            if (!map_get(curr, pk, (void **)&child)) {
                return NULL;
            }
            if (child->type != CONFIG_OBJECT_TYPE) {
                return NULL;
            }
            curr = child->value;
            start = i + 1;
            end = i + 1;
        } else if (path[i] == '"') {
            if (quote == false) quote = true;
            else quote = false;
            end++;
        } else {
            end++;
        }
    }
    struct config *value;
    if (!map_get(curr, (void *)&path[start], (void **)&value)) {
        return NULL;
    }
    return value;
}

struct list *config_object_keys(struct config *config) {
    return map_keys(config->value);
}

size_t config_array_size(struct config *config) {
    return list_length(config->value);
}

struct config *config_array_get(struct config *config, size_t index) {
    struct list *p = config->value;
    size_t i = 0;
    while (p != NULL) {
        if (i == index)
            return list_data(p);
        i++;
        p = list_next(p);
    }
    return NULL;
}

const char *config_string_value(struct config *config) {
    return config->value;
}

long long config_integer_value(struct config *config) {
    return *(long long *)config->value;
}

double config_double_value(struct config *config) {
    return *(double *)config->value;
}

bool config_boolean_value(struct config *config) {
    return *(bool *)config->value;
}

static long long config_duration_value_convert(long long v, enum config_duration_unit from,
                                                 enum config_duration_unit to) {
    if (from < to) {
        if (from == CONFIG_NANO_SECOND) {
            return config_duration_value_convert(v / 1000, CONFIG_MICRO_SECOND, to);
        } else if (from == CONFIG_MICRO_SECOND) {
            return config_duration_value_convert(v / 1000, CONFIG_MILLI_SECOND, to);
        } else if (from == CONFIG_MILLI_SECOND) {
            return config_duration_value_convert(v / 1000, CONFIG_SECOND, to);
        } else if (from == CONFIG_SECOND) {
            return config_duration_value_convert(v / 60, CONFIG_MINUTE, to);
        } else if (from == CONFIG_MINUTE) {
            return config_duration_value_convert(v / 60, CONFIG_HOUR, to);
        } else if (from == CONFIG_HOUR) {
            return config_duration_value_convert(v / 24, CONFIG_DAY, to);
        } 
    } else if (from > to) {
        if (from == CONFIG_DAY) {
            return config_duration_value_convert(v * 24, CONFIG_HOUR, to);
        } else if (from == CONFIG_HOUR) {
            return config_duration_value_convert(v * 60, CONFIG_MINUTE, to);
        } else if (from == CONFIG_MINUTE) {
            return config_duration_value_convert(v * 60, CONFIG_SECOND, to);
        } else if (from == CONFIG_SECOND) {
            return config_duration_value_convert(v * 1000, CONFIG_MILLI_SECOND, to);
        } else if (from == CONFIG_MILLI_SECOND) {
            return config_duration_value_convert(v * 1000, CONFIG_MICRO_SECOND, to);
        } else if (from == CONFIG_MICRO_SECOND) {
            return config_duration_value_convert(v * 1000, CONFIG_NANO_SECOND, to);
        }
    } else {
        return v;
    }
}

long long config_duration_value(struct config *config, enum config_duration_unit unit) {
    struct config_duration *value = config->value;
    return config_duration_value_convert(value->value, value->unit, unit);
}

static long long config_size_value_convert(long long v, enum config_size_unit from,
                                             enum config_size_unit to) {
    if (from < to) {
        return config_size_value_convert(v / 1024, from + 1, to);
    } else if (from > to) {
        return config_size_value_convert(v * 1024, from - 1, to);
    } else {
        return v;
    }
}

long long config_size_value(struct config *config, enum config_size_unit unit) {
    struct config_size *value = config->value;
    return config_size_value_convert(value->value, value->unit, unit);
}
