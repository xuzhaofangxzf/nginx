#ifndef __NGX_AFFINITY_HPP__
#define __NGX_AFFINITY_HPP__

//#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>

int ngx_setaffinity(pid_t pid, int icpu);

#endif