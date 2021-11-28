#include "restful.h"

#include "http_parser.h"

int get_v1_zxj_wc_missions(match_entry *entry, char *res, void *udata)
{
    strcpy(res, "hello world");
    return HTTP_STATUS_OK;
}
