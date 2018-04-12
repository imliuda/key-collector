#ifndef __OSCLT_STR_H__
#define __OSCLT_STR_H__

struct strbuf {
    char *str;
    size_t slen;
    size_t blen;
    size_t blksz;
}

char *strltrim(char *str);
char *strrtrim(char *str);
char *strtrim(char *str);

struct strbuf *strbufnew(size_t blksz);
void strbuffree(struct strbuf *buf);
const char *strbufstr(struct strbuf *buf);
void strbufexts(struct strbuf *buf, const char *str);
void strbufextn(struct strbuf *buf, const char *str, size_t n);
void strbufextf(struct strbuf *buf, const char *fmt, ...);

#endif
