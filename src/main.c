#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include <ev.h>
#include "config.h"
#include "schedule.h"

const char *argp_program_version = "osclt v1.0";
const char *argp_program_bug_address = "<imliuda@163.com>";
static char argp_doc[] = "An os information collecting tool";
static char argp_args[] = "";
static struct argp_option options[] = {
    {"config", 'c', "configuration plugin", 0, "Configureation plugin an parameter of osclt"},
    {0}
};

static error_t parse_option(int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'c':
            *((char **)state->input) = arg;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_option, argp_args, argp_doc};

int main(int argc, char **argv) {
    char *cname, *cparam;

    argp_parse(&argp, argc, argv, 0, 0, &cparam);

    cname = strsep(&cparam, ":");
    config_load(cname, cparam);

    clt_task *task = malloc(sizeof(clt_task));
    task->cmd = "/tmp/test.sh";
    task->interval = 5;
    task->timeout = 1;
    task->result = NULL;
    task->reslen = 0;
    task->next = NULL;
    struct ev_loop *loop = ev_default_loop(0);
    schedule(loop, task);
    ev_run(loop, 0);
    return 0;
}
