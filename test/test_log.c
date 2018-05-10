#include <stdio.h>
#include <stdlib.h>
#include <ev.h>

#include "../src/osclt.h"
#include "../src/config.h"
#include "../src/log.h"

static void log_writer(EV_P_ ev_timer *w, int revents) {
    log_debug("exec error: %s, %d", "sss", 234);
    log_info("url error: %s, %d", "sss", 234);
    log_warn("connect db error: %s, %d", "sss", 234);
    log_error("log error: %s, %d", "sss", 234);
    log_fatal("mem error: %s, %d", "sss", 234);
}

int main() {
    struct osclt *oc = malloc(sizeof(struct osclt));

    char *error;
    struct config *config = config_load("./test_log.conf", &error);
    oc->config = config;

    struct ev_loop *loop = ev_loop_new(0);
    oc->loop = loop;

    log_init(oc);

    ev_timer timer_watcher;
    ev_timer_init(&timer_watcher, log_writer, 0, 1);
    ev_timer_start(oc->loop, &timer_watcher);

    ev_run(oc->loop, 0);
}
