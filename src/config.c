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

void config_parse(struct config *config, wchar_t buf[], size_t buflen) {
    /* parse config, wbuf has wcslen wchar_t character */
    enum cp_state state = CP_START;
    int line_num = 0, col_num = 0, n =0;
    wchar_t c;
    while (c = *(buf + n++), n < 10) {
        printf("%lc\n", c);
    }
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

    config_parse(config, wbuf, wcslen);
    free(wbuf);
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
