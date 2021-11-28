#ifndef __RESTFUL_H
#define __RESTFUL_H

#include "r3/r3.h"

#define RESTFUL_API_MAP(XX)                                             \
    XX(/v1/zxj/wc/missions, GET, get_v1_zxj_wc_missions)


// declare statement
#define RESTFUL_GEN(pattern, method, func)                              \
    int func(match_entry *entry, char *res, void *udata);

RESTFUL_API_MAP(RESTFUL_GEN)

#undef RESTFUL_GEN

#endif
