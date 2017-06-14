#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "config.h"

int main(int argc, char **argv) {
    config_init("/tmp/test.config");
    printf("AAA: %ld\n", config_get_long("AAA", 123));
    printf("BBB: %d\n", config_get_bool("BBB", false));
    printf("CCC: %s\n", config_get_string("CCC", "waeqrweffwaeafdsaf"));
    return 0;
}
