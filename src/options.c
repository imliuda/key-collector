#include <argp.h>
#include <stdbool.h>
#include "options.h"

const char *argp_program_version = "osclt v1.0";
const char *argp_program_bug_address = "<imliuda@163.com>";
static char doc[] = "osclt [-c config] [-d]";
static char args_doc[] = "arg1 arg2";

/*
 * long, short, name, flags, doc, group
 */
static struct argp_option base[] = {
    {"config", 'c', "config file", 0, "configuration file of keys"},
    {"daemon", 'd', "daemon mode", OPTION_ARG_OPTIONAL, "configuration file of keys"},
    {0}
};

// add sub options
void add_options(struct argp *argp, const char *header) {
    
}

static error_t base_parser(int key, char *arg, struct argp_state *state) {
    struct options *options = state->input;

    switch (key) {
        case 'c':
            options->config = arg;
            break;
        case 'd':
            options->daemon = true;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

void parse_options(int argc, char **argv, struct options *options) {
    struct argp argp = {base, base_parser, args_doc, doc};
    argp_parse(&argp, argc, argv, 0, 0 , options);
}
