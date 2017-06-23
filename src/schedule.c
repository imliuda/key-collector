#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <ev.h>
#include "schedule.h"

static clt_task *task_list = NULL;

void schedule_io_cb(struct ev_loop *loop, ev_io *iow, int revents) {
    clt_task *task = (clt_task *)((char *)iow - offsetof(clt_task, w_io));
    size_t nread;
    char buf[64];

    if (revents & EV_READ) {
        while (1) {
            nread = read(iow->fd, buf, 64);
            if (nread == -1) {
                break;
            } else if (nread == 0) {
                task->result = realloc(task->result, task->reslen + 1);
                task->result[task->reslen] = '\0';
                task->reslen += 1;
                break;
            } else if (nread > 0) {
                task->result = realloc(task->result, task->reslen + nread);
                memcpy(task->result + task->reslen, buf, nread);
                task->reslen += nread;
            }
        }
    } else {
        printf("other events");
    }
}

void schedule_child_cb(struct ev_loop *loop, ev_child *cw, int revents) {
    clt_task *task = (clt_task *)((char *)cw - offsetof(clt_task, w_chld));

    ev_io_stop(loop, &task->w_io);
    ev_child_stop(loop, &task->w_chld);
    ev_timer_stop(loop, &task->w_to);
    close(task->w_io.fd);

    printf("result: %s\n", task->result);    

    free(task->result);
    task->result = NULL;
    task->reslen = 0;
}

void schedule_timeout_cb(struct ev_loop *loop, ev_timer *tw, int revents) {
    clt_task *task = (clt_task *)((char *)tw - offsetof(clt_task, w_to));
    printf("execute script timeout: %s\n", task->cmd);

    ev_io_stop(loop, &task->w_io);
    ev_child_stop(loop, &task->w_chld);
    ev_timer_stop(loop, &task->w_to);
    close(task->w_io.fd);
    free(task->result);
    task->result = NULL;
    task->reslen = 0;

    kill(task->w_chld.pid, SIGTERM);
}

void schedule_period_cb(struct ev_loop *loop, ev_timer *tw, int revents) {
    clt_task *task = (clt_task *)((char *)tw - offsetof(clt_task, w_prd));
    pid_t pid;
    int fd[2];

    if (ev_is_active(&task->w_chld)) {
        printf("previous proccess is still running...\n");
        return;
    }

    pipe(fd);
    pid = fork();
    if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        execlp(task->cmd, NULL);
    } else if (pid > 0) {
        close(fd[1]);
        fcntl(fd[0], F_SETFL, O_NONBLOCK);

        ev_io_set(&task->w_io, fd[0], EV_READ);
        ev_io_start(loop, &task->w_io);

        ev_child_set(&task->w_chld, pid, 0);
        ev_child_start(EV_DEFAULT_ &task->w_chld);

        ev_timer_set(&task->w_to, task->timeout, 0);
        ev_timer_start(loop, &task->w_to);
    } else {
        perror("fork");
    }
}

void schedule(struct ev_loop *loop, clt_task *tlist) {
    task_list = tlist;
    clt_task *h = tlist;
    while (h != NULL) {
        ev_init(&h->w_io, schedule_io_cb);
        ev_init(&h->w_chld, schedule_child_cb);
        ev_init(&h->w_to, schedule_timeout_cb);
        ev_timer_init(&h->w_prd, schedule_period_cb, 0, h->interval);
        ev_timer_start(loop, &h->w_prd);
        h = h->next;
    }
}
