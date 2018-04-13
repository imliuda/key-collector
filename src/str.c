#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "str.h"

void strltrim(char *str) {
    char *p = str;
    char *s = str;
    while (*p != '\0') {
        if (!isspace(*p)) {
            while (*p != '\0') {
                *s++ = *p++;
            }
            *s = '\0';
            break;
        }
        p++;
    }
}

void strrtrim(char *str) {
    char *p = str;
    while (*p != '\0') p++;
    while (--p >= str) {
        if (!isspace(*p)) {
            *++p = '\0';
            break;
        }
    };
}

void strtrim(char *str) {
    strltrim(str);
    strrtrim(str);
    str;
}

struct strbuf *strbufnew(size_t blksz) {
    struct strbuf *buf = malloc(sizeof(struct strbuf));
    buf->str = malloc(blksz);
    buf->slen = 0;
    buf->blen = blksz;
    buf->blksz = blksz;
    return buf;
}

void strbuffree(struct strbuf *buf) {
    free(buf->str);
    free(buf);
}

const char *strbufstr(struct strbuf *buf) {
    return buf->str;
}

void strbufexts(struct strbuf *buf, const char *str) {
    int slen = strlen(str);
    if (slen >= (buf->blen - buf->slen)) {
        buf->str = realloc(buf->str, (slen / buf->blksz + 1) * buf->blksz);
    }
    strncpy(&buf->str[buf->slen], str, slen);
    buf->slen += slen;
    buf->str[buf->slen] = '\0';
}

void strbufextn(struct strbuf *buf, const char *str, size_t n) {

}

void strbufextf(struct strbuf *buf, const char *fmt, ...) {

}
