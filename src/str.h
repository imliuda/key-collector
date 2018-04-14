#ifndef __OSCLT_STR_H__
#define __OSCLT_STR_H__

#include <stddef.h>

struct strbuf {
    char *str;
    size_t slen;
    size_t blen;
    size_t blksz;
};

struct wcsbuf {
    wchar_t *wcs; /* gnu c wchar_t is 32 bits wides */
    size_t slen;
    size_t blen;
    size_t blksz;
};

void strltrim(char *str);
void strrtrim(char *str);
void strtrim(char *str);
wchar_t *wcsndup(wchar_t *s, size_t n);

struct strbuf *strbufnew(size_t blksz);
void strbuffree(struct strbuf *buf);
const char *strbufstr(struct strbuf *buf);
void strbufexts(struct strbuf *buf, const char *str);
void strbufextn(struct strbuf *buf, const char *str, size_t n);
void strbufextf(struct strbuf *buf, const char *fmt, ...);


struct wcsbuf *wcsbufnew(size_t blksz);
void wcsbuffree(struct wcsbuf *buf);
wchar_t *wcsbufwcs(struct wcsbuf *buf);
void wcsbufexts(struct wcsbuf *buf, wchar_t *str);
void wcsbufextn(struct wcsbuf *buf, wchar_t *str, size_t n);

wchar_t *strutf8dec(const char *s);
char *strutf8enc(const wchar_t *s);

#endif
