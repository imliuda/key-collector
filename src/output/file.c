#include <jansson.h>
#include <ev.h>

#include "../config.h"

typedef struct _osclt_task osclt_task;
struct _osclt_task {
    ev_timer    w_prd;
    ev_io       w_io;
    ev_child    w_chld;
    ev_timer    w_to;
    int         interval;
    int         timeout;
    char        *key;
    char        *exec;
    char        *log;
    char        **argv;
    char        *result;
    size_t      reslen;
    osclt_task  *next;
};

extern osclt_task *task_list;
void schedule_load(osclt_task **task_list);
void schedule_start(struct ev_loop *loop, osclt_task *task_list);


static void config_load_filesystem(const char *param) {
    json_error_t error;

    config = json_load_file(param, 0, &error);
    if (!config) {
        printf("error when opening config file: %s\n", param);
        return;
    }
}

static void __attribute__((constructor)) config_register_filesystem() {
    config_register("file", config_load_filesystem);
}
