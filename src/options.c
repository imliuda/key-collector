#include <argp.h>

struct options {

};

const char *argp_program_version = "sa33";
static char doc[] = "sfpoiawpifp asdifjpawefaf";
static char args_doc[] = "arg1 arg2";

/*
 * long, short, name, flags, doc, group
 */
static struct argp_option options[] = {
    {"config", 'c', "config file", 0, "configuration file of keys"},
    {"daemon", 'd', "daemon mode", 0, "configuration file of keys"},
    {0}
};

static error_t option_parser(int key, char *arg, struct argp_state *state) {
    printf("%c, %s\n", key, arg);
    return 0;
}

static struct argp argp = {options, option_parser, args_doc, doc};

void parse_options(int argc, char **argv, struct options options) {
    argp_parse(&argp);
}
