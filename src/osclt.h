#ifndef __OSCLT_OSCLT_H__
#define __OSCLT_OSCLT_H__

#include <uv.h>

struct osclt {
    uv_loop_t       *loop;
    struct config   *config;
};

#endif
