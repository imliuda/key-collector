#include <ev.h>

void schedule() {
    ev_loop *loop = ev_default_loop(EVFLAG_AUTO);

    ev_run(loop, 0);

    
}
