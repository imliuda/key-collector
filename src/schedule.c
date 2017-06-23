#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <ev.h>
#include "schedule.h"

clt_task *task_list = NULL;

void schedule_io_cb(struct ev_loop *loop, ev_io *iow, int revents) {
    clt_task *task = (clt_task *)((char *)iow - offsetof(clt_task, io));
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
    clt_task *task = (clt_task *)((char *)cw - offsetof(clt_task, child));

    ev_io_stop(loop, &task->io);
    ev_child_stop(loop, &task->child);
    close(task->io.fd);

    printf("result: %s\n", task->result);    

    free(task->result);
    task->result = NULL;
    task->reslen = 0;
}

void schedule_period_cb(struct ev_loop *loop, ev_periodic *pw, int revents) {
    clt_task *task = (clt_task *)((char *)pw - offsetof(clt_task, period));
    pid_t pid;
    int fd[2];

    if (ev_is_active(&task->child)) {
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
        ev_io_init(&task->io, schedule_io_cb, fd[0], EV_READ);
        ev_io_start(loop, &task->io);

        ev_child_set(&task->child, pid, 0);
        ev_child_start(EV_DEFAULT_ &task->child);
    } else {
        perror("fork");
    }
}

void schedule(struct ev_loop *loop, clt_task *tlist) {
    clt_task *h = tlist;
    while (h != NULL) {
        ev_init(&h->child, schedule_child_cb);
        ev_periodic_init(&h->period, schedule_period_cb, 0, h->interval, NULL);
        ev_periodic_start(loop, &h->period);
        h = h->next;
    }
}
