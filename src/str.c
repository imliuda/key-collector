#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <iconv.h>

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

struct wcsbuf *wcsbufnew(size_t blksz) {
    struct wcsbuf *buf = malloc(sizeof(struct wcsbuf));
    buf->wcs = malloc(blksz * sizeof(wchar_t));
    buf->slen = 0;
    buf->blen = blksz;
    buf->blksz = blksz;
    return buf;
}

void wcsbuffree(struct wcsbuf *buf) {
    free(buf->wcs);
    free(buf);
}

wchar_t *wcsbufwcs(struct wcsbuf *buf) {
    return buf->wcs;
}

void wcsbufexts(struct wcsbuf *buf, wchar_t *str) {
    int slen = wcslen(str);
    if (slen >= (buf->blen - buf->slen)) {
        buf->wcs = realloc(buf->wcs, (slen / buf->blksz + 1) * buf->blksz * sizeof(wchar_t));
    }
    wcsncpy(&buf->wcs[buf->slen], str, slen);
    buf->slen += slen;
    buf->wcs[buf->slen] = L'\0';
}

void wcsbufextn(struct wcsbuf *buf, wchar_t *str, size_t n) {
    if (n >= (buf->blen - buf->slen)) {
        buf->wcs = realloc(buf->wcs, (n / buf->blksz + 1) * buf->blksz * sizeof(wchar_t));
    }
    wcsncpy(&buf->wcs[buf->slen], str, n);
    buf->slen += n;
    buf->wcs[buf->slen] = L'\0';
}

wchar_t *strutf8dec(const char *s) {
    iconv_t cd = iconv_open("WCHAR_T", "UTF-8");
    size_t slen = strlen(s);

    if (cd == (iconv_t) -1)
        return NULL;

    int i = 0;
    while (i < slen) {
        
    }

    iconv_close(cd);
}

const char *strutf8enc(wchar_t *s) {

}
