#include <stdio.h>
#include <stdlib.h>
#include "options.h"

int main(int argc, char **argv) {
    struct options *options = malloc(sizeof(struct options));
    parse_options(argc, argv, options);
    printf("config: %s, daemon: %d\n", options->config, options->daemon);
    return 0;
}
