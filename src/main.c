#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <ev.h>
#include "config.h"
#include "schedule.h"

static char *config_file = NULL;

const char *argp_program_version = "osclt v1.0";
const char *argp_program_bug_address = "<imliuda@163.com>";
static char argp_doc[] = "An os information collecting tool";
static char argp_args[] = "";
static struct argp_option options[] = {
    {"config", 'c', "config file", 0, "Configureation file of osclt"},
    {0}
};

static error_t parse_option(int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'c':
            config_file = arg;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_option, argp_args, argp_doc};

int main(int argc, char **argv) {
    argp_parse(&argp, argc, argv, 0, 0, 0);
    if (!config_file) {
        printf("You must provide a config file with -c option\n");
        exit(EXIT_FAILURE);
    }

    config_init(config_file);

    task_list = malloc(sizeof(clt_task));
    task_list->cmd = "/tmp/test.sh";
    task_list->interval = 5;
    task_list->result = NULL;
    task_list->reslen = 0;
    task_list->next = NULL;
    struct ev_loop *loop = ev_default_loop(0);
    schedule(loop, task_list);
    ev_run(loop, 0);
    return 0;
}
