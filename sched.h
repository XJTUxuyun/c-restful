#ifndef __SCHED_H
#define __SCHED_H

#include "r3/r3.h"

struct sched_s
{
    int sock_fd;
    int epoll_fd;

    int run_flag;

    R3Node *n;

    void *udata;

    struct
    {
        char url[65536];
        size_t url_len;
        char body[65536];
        size_t body_len;
        char res[65536];
        size_t res_len;
        int res_status;
    } restful_cache;

    char *response_buf;
};

int sched_init(struct sched_s *sched);

int sched_free(struct sched_s *sched);

int sched_dispatch(struct sched_s *sched);

#endif
