#include <stdio.h>
#include "option.h"

void register_option(const char *name, int key, const char *arg,
                     int flags, const char *doc, int group) {
    printf("%s\n", name);
}
