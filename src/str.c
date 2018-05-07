#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <iconv.h>
#include <errno.h>
#include <stdarg.h>

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

wchar_t *wcsndup(wchar_t *s, size_t n) {
    size_t len = wcsnlen(s, n);
    wchar_t* wcs = malloc((len + 1) * sizeof(wchar_t));
    wmemcpy(wcs, s, len);
    wcs[len] = L'\0';
    return wcs;
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
        buf->blen += (slen / buf->blksz + 1) * buf->blksz;
        buf->str = realloc(buf->str, buf->blen);
    }
    strncpy(&buf->str[buf->slen], str, slen);
    buf->slen += slen;
    buf->str[buf->slen] = '\0';
}

void strbufextn(struct strbuf *buf, const char *str, size_t n) {
    if (n >= (buf->blen - buf->slen)) {
        buf->blen += (n / buf->blksz + 1) * buf->blksz;
        buf->str = realloc(buf->str, buf->blen);
    }
    strncpy(&buf->str[buf->slen], str, n);
    buf->slen += n;
    buf->str[buf->slen] = '\0';
}

void strbufextf(struct strbuf *buf, const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    size_t len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    char s[len + 1];
    va_start(args, fmt);
    vsnprintf(s, len + 1, fmt, args);
    va_end(args);

    strbufexts(buf, s);
}

struct wcsbuf *wcsbufnew(size_t blksz) {
    struct wcsbuf *buf = malloc(sizeof(struct wcsbuf));
    buf->wcs = malloc(blksz * sizeof(wchar_t));
    buf->slen = 0;
    buf->blen = blksz;
    buf->blksz = blksz;
    buf->wcs[0] = L'\0';
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
        buf->blen += (slen / buf->blksz + 1) * buf->blksz;
        buf->wcs = realloc(buf->wcs, buf->blen * sizeof(wchar_t));
    }
    wcsncpy(&buf->wcs[buf->slen], str, slen);
    buf->slen += slen;
    buf->wcs[buf->slen] = L'\0';
}

void wcsbufextn(struct wcsbuf *buf, wchar_t *str, size_t n) {
    if (n >= (buf->blen - buf->slen)) {
        buf->blen += (n / buf->blksz + 1) * buf->blksz;
        buf->wcs = realloc(buf->wcs, buf->blen * sizeof(wchar_t));
    }
    wcsncpy(&buf->wcs[buf->slen], str, n);
    buf->slen += n;
    buf->wcs[buf->slen] = L'\0';
}

wchar_t *strutf8dec(const char *s) {
    iconv_t cd = iconv_open("WCHAR_T", "UTF-8");

    if (cd == (iconv_t) -1)
        return NULL;

    struct wcsbuf *buf = wcsbufnew(64);
    wchar_t wcs[64];
    char *cwcs = (char *)wcs;
    size_t outleft = 64 * sizeof(wchar_t);
    size_t inleft = strlen(s);

    while (inleft != 0) {
        size_t nconv = iconv(cd, (char **)&s, &inleft, &cwcs, &outleft);
        if (nconv == (size_t) -1) {
            if (errno == E2BIG) {
                wcsbufextn(buf, wcs, (64 * sizeof(wchar_t) - outleft) / sizeof(wchar_t));
                outleft = 64 * sizeof(wchar_t);
                cwcs = (char *)wcs;
            } else {
                iconv_close(cd);
                wcsbuffree(buf);
                return NULL;
            }
        }
    }
    wcsbufextn(buf, wcs, (64 * sizeof(wchar_t) - outleft) / sizeof(wchar_t));
    iconv_close(cd);
    wchar_t *r = wcsdup(wcsbufwcs(buf));
    wcsbuffree(buf);
    return r;
}

char *strutf8enc(const wchar_t *wcs) {
    iconv_t cd = iconv_open("UTF-8", "WCHAR_T");

    if (cd == (iconv_t) -1)
        return NULL;

    struct strbuf *buf = strbufnew(256);
    char s[256];
    char *cs = s;
    size_t outleft = 256;
    size_t inleft = wcslen(wcs) * sizeof(wchar_t);

    while (inleft != 0) {
        size_t nconv = iconv(cd, (char **)&wcs, &inleft, &cs, &outleft);
        if (nconv == (size_t) -1) {
            if (errno == E2BIG) {
                strbufextn(buf, s, 256 - outleft);
                outleft = 256;
                cs = s;
            } else {
                iconv_close(cd);
                strbuffree(buf);
                return NULL;
            }
        }
    }
    strbufextn(buf, s, 256 - outleft);
    iconv_close(cd);
    char *r = strdup(strbufstr(buf));
    strbuffree(buf);
    return r;
}
