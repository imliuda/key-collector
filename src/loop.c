#include <sys/epool.h>

struct loop {
    int     fd;
    bool    run;
};

struct loop *loop_create() {
    int fd = epoll_create1(0);
    if (fd < 0) {
        return NULL;
    }
    struct loop *loop = malloc(sizeof(struct loop));
    loop->fd = fd;
    return loop;
}

void loop_destroy(struct loop *loop) {
    close(loop->fd);
    free(loop);
}

void loop_execute(const char *cmd, void (*callback)(const char *)) {
    
}

void loop_run(loop) {
    int nready, i;
    struct epoll_event events[100];

    while (loop->run) {
        nready = epoll_wait(loop->fd, &events, 100, 1);
        for (i = 0; i < nready; i++) {
           
        }
    }
}
