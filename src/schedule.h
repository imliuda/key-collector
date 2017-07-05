#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <ev.h>

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

#endif
