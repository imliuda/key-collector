#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <jansson.h>
#include <ev.h>
#include "schedule.h"
#include "config.h"

osclt_task *task_list = NULL;
/*
*/
void schedule_load(osclt_task **task_list) {
    json_t *tasks = json_object_get(config, "schedule");
    if (tasks == NULL) {
        printf("there are no scheduled tasks\n");
    }
    void *iter = json_object_iter(tasks);
    const char *key;
    json_t *value, *prop;

    while (iter != NULL) {
        key = json_object_iter_key(iter);
        value = json_object_iter_value(iter);

        osclt_task *task = malloc(sizeof(osclt_task));

        if ((prop = json_object_get(value, "exec")) != NULL) {
            task->exec = strdup(json_string_value(prop));
        }
        if ((prop = json_object_get(value, "period")) != NULL) {
            task->interval = json_integer_value(prop);
        }
        if ((prop = json_object_get(value, "timeout")) != NULL) {
            task->interval = json_integer_value(prop);
        }
        if ((prop = json_object_get(value, "log")) != NULL) {
            task->log = strdup(json_string_value(prop));
        }
        task->result = NULL;
        task->reslen = 0;
        task->next = *task_list;
        *task_list = task;

        printf("%s\n", key);
        iter = json_object_iter_next(tasks, iter);
    }
}

void schedule_io_cb(struct ev_loop *loop, ev_io *iow, int revents) {
    osclt_task *task = (osclt_task *)((char *)iow - offsetof(osclt_task, w_io));
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
    osclt_task *task = (osclt_task *)((char *)cw - offsetof(osclt_task, w_chld));

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
    osclt_task *task = (osclt_task *)((char *)tw - offsetof(osclt_task, w_to));
    printf("child process timeout when executing %s, send SIGKILL to it\n", task->exec);

    ev_io_stop(loop, &task->w_io);
    ev_child_stop(loop, &task->w_chld);
    ev_timer_stop(loop, &task->w_to);
    close(task->w_io.fd);
    free(task->result);
    task->result = NULL;
    task->reslen = 0;

    kill(task->w_chld.pid, SIGTERM);
}

void schedule_exec(struct ev_loop *loop, osclt_task *task) {
    pid_t pid;
    int fd[2];

    pipe(fd);
    pid = fork();
    if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        execlp(task->exec, NULL);
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
        close(fd[0]);
        close(fd[1]);
        printf("failed to fork when executing task: %s\n", task->key);
    }
}

void schedule_period_cb(struct ev_loop *loop, ev_timer *tw, int revents) {
    osclt_task *task = (osclt_task *)((char *)tw - offsetof(osclt_task, w_prd));

    if (ev_is_active(&task->w_chld)) {
        printf("previous proccess is still running...\n");
        return;
    } else {
        schedule_exec(loop, task);
    }

}

void schedule_once(struct ev_loop *loop, osclt_task *task) {
    ev_init(&task->w_io, schedule_io_cb);
    ev_init(&task->w_chld, schedule_child_cb);
    ev_init(&task->w_to, schedule_timeout_cb);
    schedule_exec(loop, task);
}

void schedule_periodic(struct ev_loop *loop, osclt_task *task) {
    ev_init(&task->w_io, schedule_io_cb);
    ev_init(&task->w_chld, schedule_child_cb);
    ev_init(&task->w_to, schedule_timeout_cb);
    ev_init(&task->w_prd, schedule_period_cb);

    ev_timer_set(&task->w_prd, 0, task->interval);
    ev_timer_start(loop, &task->w_prd);
}

void schedule_start(struct ev_loop *loop, osclt_task *tlist) {
    task_list = tlist;
    osclt_task *h = tlist;
    while (h != NULL) {
        if (strcmp(h->log, "oncse") == 0) {
            printf("once\n");
            schedule_once(loop, h);
        } else {
            schedule_periodic(loop, h);
        }
        h = h->next;
    }
}

void schedule_reload(struct ev_loop *loop, osclt_task *tlist) {

}
