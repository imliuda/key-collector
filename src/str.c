#include <ctype.h>

#include "string.h"

char *strltrim(char *str) {
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
    return str;
}

char *strrtrim(char *str) {
    char *p = str;
    while (*p != '\0') p++;
    while (--p >= str) {
        if (!isspace(*p)) {
            *++p = '\0';
            break;
        }
    };
    return str;
}

char *strtrim(char *str) {
    str = strltrim(str);
    str = strrtrim(str);
    return str;
}
