#include "string.h"

char *strltrim(char *str, const char *ws) {
    char *p, *s = str;
    const char *w;
    for (p = str; *p != '\0'; p++) {
        for (w = ws; *w != '\0'; w++) {
            if (*p == *w) {
                break;
            }
        }
        // *p is the fist non empty char
        if (*w == '\0') {
            while (*p != '\0') {
                *s++ = *p++;
            }
            *s = '\0';
            break;
        }
    }
    return str;
}

char *strrtrim(char *str, const char *ws) {
    char *p = str;
    const char *w;
    while (*p != '\0') p++;
    p--;
    while (p >= str) {
        for (w = ws; *w != '\0'; w++) {
            if (*p == *w) {
                break;
            }
        }
        if (*w == '\0') {
            *(p + 1) = '\0';
            break;
        }
        p--;
    };
    return str;
}

char *strtrim(char *str, const char *ws) {
    strltrim(str, ws);
    strrtrim(str, ws);
    return str;
}
