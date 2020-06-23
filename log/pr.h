#ifndef __PR_H__
#define __PR_H__

#include <stdio.h>
#include <sstream>
#include <thread>
#include <string>

using namespace std;

#define PR_LEVEL_ERROR 0
#define PR_LEVEL_WARN 1
#define PR_LEVEL_INFO 2
#define PR_LEVEL_DEBUG 3

extern int pr_level;

#define PR(level, val, fmt, ...)  \
    do {                                                    \
            if( level <= pr_level )                         \
                printf("[%-5s]" "[%s:%d] " fmt, val,         \
                    __FUNCTION__, __LINE__, ##__VA_ARGS__);   \
    } while(0)

#define PR_DEBUG(fmt, ...)  \
    PR(PR_LEVEL_DEBUG, "debug", fmt, ##__VA_ARGS__)

#define PR_INFO(fmt, ...)  \
    PR(PR_LEVEL_INFO, "info", fmt, ##__VA_ARGS__)

#define PR_WARN(fmt, ...)  \
    PR(PR_LEVEL_WARN, "warn", fmt, ##__VA_ARGS__)

#define PR_ERROR(fmt, ...)  \
    PR(PR_LEVEL_ERROR, "error", fmt, ##__VA_ARGS__)

int pr_set_level(int level);

string tid_to_string(const thread::id& tid);

long long tid_to_ll(const thread::id& tid);

#endif          