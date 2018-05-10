#ifndef __OSCLT_OSCLT_H__
#define __OSCLT_OSCLT_H__

#include <ev.h>

struct osclt {
    struct ev_loop  *loop;
    struct config   *config;
};

#endif
