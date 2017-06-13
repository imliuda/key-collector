#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <stdbool.h>

struct options {
    char    *config;
    bool    daemon;
};

void parse_options(int argc, char **argv, struct options *options);

#endif
