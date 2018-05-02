#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#include "../src/osclt.h"
#include "../src/config.h"
#include "../src/log.h"

void log_writer() {
    log_debug("test debug log %d", 2);
}

int main() {
    struct osclt *oc = malloc(sizeof(struct osclt));

    char *error;
    struct config *config = config_load("./test_log.conf", &error);
    oc->config = config;

    uv_loop_t *loop = malloc(sizeof(uv_loop_t));
    uv_loop_init(loop);
    oc->loop = loop;

    log_init(oc);

    uv_timer_t timer_req;
    uv_timer_init(oc->loop, &timer_req);
    uv_timer_start(&timer_req, log_writer, 1000, 1000);

    uv_run(oc->loop, UV_RUN_DEFAULT);
    
    uv_loop_close(oc->loop);
    free(loop);
}
