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

void config_load(struct config *config, const char *path) {
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

    /* convert buf to wchar_t */
    size_t wbuflen = len * sizeof(wchar_t);
    size_t wlen = wbuflen;
    wchar_t *wbuf = malloc(wbuflen); /* need optimise */
    char *inptr = buf;
    char *wrptr = (char *)wbuf;
    int nconv;
    iconv_t cd = iconv_open("WCHAR_T", "UTF-8");
    if (cd == (iconv_t) -1) {
        fprintf(stderr, "iconv_open failed: %s\n", strerror(errno));
        exit(1);
    }
    while (len > 0) {
        nconv = iconv(cd, &inptr, &len, &wrptr, &wlen);
        if (nconv == -1) {
            fprintf(stderr, "converting config file encoding error: %s.\n", strerror);
            exit(1);
        } 
    }
    wbuflen -= wlen;
    wbuflen = wbuflen / 4;
    if (iconv_close(cd) != 0) {
        fprintf(stderr, "iconv_close error: %s", strerror(errno));
        exit(1);
    }
    free(buf);

    setlocale(LC_CTYPE, "C");

    /* parse config */
    enum cp_state state = CP_START;
    int line_num = 0, col_num = 0, n =0;
    wchar_t c;
    while (c = *(wbuf + n++), n < 10) {
        printf("%x\n", c);
    }
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
