#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <ev.h>

typedef struct _clt_task clt_task;
struct _clt_task {
    ev_timer    w_prd;
    ev_io       w_io;
    ev_child    w_chld;
    ev_timer    w_to;
    int         interval;
    int         timeout;
    char        *key;
    char        *cmd;
    char        **argv;
    char        *result;
    size_t      reslen;
    clt_task    *next;
};

void schedule(struct ev_loop *loop, clt_task *task_list);

#endif
