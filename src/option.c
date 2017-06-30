#include <stdio.h>
#include "option.h"

typedef enum value_type {
    STRING, INTEGER
};

typedef struct _osclt_option osclt_option;
struct _osclt_option {
    argp_option     option;
    value_type      vtype;
    void            *value;
    osclt_option    *next;
};

static osclt_option *option = NULL;

void register_option(const char *name, char key, const char *doc, value_type vtype, void *value) {
    osclt_option *opt = malloc(sizeof(osclt_option));
    opt->option.name = strdup(name);
    opt->option.key = key;
    opt->option.doc = strdup(arg);
    opt->vtype = type;
    opt->value = value;

    opt->next = option;
    option = opt;
}

void option_init(int argc, char **argv) {
    
}
