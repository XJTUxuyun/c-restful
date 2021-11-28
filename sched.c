#include "sched.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "http_parser.h"
#include "restful.h"

// method在http_parser和r3中的定义不一致
// 使用转换表做桥梁转换
int http_parser_method_to_r3_method_table[100] = {
    [0 ... 99] = -1,
    [HTTP_GET] = METHOD_GET,
    [HTTP_POST] = METHOD_POST,
    [HTTP_PUT] = METHOD_PUT,
    [HTTP_DELETE] = METHOD_DELETE,
    [HTTP_PATCH] = METHOD_PATCH,
    [HTTP_HEAD] = METHOD_HEAD,
    [HTTP_OPTIONS] = METHOD_OPTIONS,
};

static inline void restful_cache_clear(struct sched_s *sched)
{
    memset(&sched->restful_cache, 0x00, sizeof(sched->restful_cache));
}

static int restful_on_url(http_parser *parser, const char *at, size_t length)
{
    if (!parser || !at || !length)
    {
        return -1;
    }

    struct sched_s *sched = (struct sched_s *)parser->data;

    if (sched->restful_cache.url_len + length >= 65535)
    {
        length = 65535 - sched->restful_cache.url_len;
    }

    memcpy(sched->restful_cache.url + sched->restful_cache.url_len, at, length);

    sched->restful_cache.url_len += length;

    return 0;
}

int restful_on_body(http_parser *parser, const char *at, size_t length)
{
    if (!parser || !at || !length)
    {
        return -1;
    }

    struct sched_s *sched = (struct sched_s *)parser->data;

    if (sched->restful_cache.body_len + length >= 65535)
    {
        length = 65535 - sched->restful_cache.body_len;
    }

    memcpy(sched->restful_cache.body + sched->restful_cache.body_len, at, length);

    sched->restful_cache.body_len += length;
    
    return 0;
}

int restful_on_message_begin(http_parser *parser)
{
    if (!parser)
    {
        return -1;
    }

    return 0;
}

int restful_on_message_complete(http_parser *parser)
{
    if (!parser)
    {
        return -1;
    }

    return 0;
}

int restful_on_chunk_header(http_parser *parser)
{
    if (!parser)
    {
        return -1;
    }

    return 0;
}


int restful_on_chunk_complete(http_parser *parser)
{
    if (!parser)
    {
        return -1;
    }

    return 0;
}

int restful_on_header_complete(http_parser *parser)
{
    if (!parser)
    {
        return -1;
    }

    return 0;
}

int restful_on_headers_complete(http_parser *parser)
{
    if (!parser)
    {
        return -1;
    }

    return 0;
}

int restful_on_header_field(http_parser *parser, const char *at, size_t length)
{
    if (!parser || !at || !length)
    {
        return -1;
    }

    return 0;
}

int restful_on_header_value(http_parser *parser, const char *at, size_t length)
{
    if (!parser || !at || !length)
    {
        return -1;
    }

    return 0;
}

int restful_on_status(http_parser *parser, const char *at, size_t length)
{
    if (!parser || !at || !length)
    {
        return -1;
    }

    return 0;
}


int sched_init(struct sched_s *sched)
{
    memset(sched, 0x00, sizeof(struct sched_s));

    sched->response_buf = (char *)malloc(65536);
    if (!sched->response_buf)
    {
        goto err;
    }

    // r3
    sched->n = r3_tree_create(64);
    if (!sched->n)
    {
        goto err;
    }

#define RESTFUL_GEN(pattern, method, func)                              \
    do                                                                  \
    {                                                                   \
        r3_tree_insert_route(sched->n, METHOD_##method, #pattern, func);\
        printf("r3 add pattern->%s method->%s\n", #pattern, #method);   \
    } while(0);

RESTFUL_API_MAP(RESTFUL_GEN)

#undef RESTFUL_GEN

    char *errstr = NULL;
    if (r3_tree_compile(sched->n, &errstr))
    {
        free(errstr);
        goto err;
    }

    sched->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sched->sock_fd <= 0)
    {
        goto err;
    }

    if (setsockopt(sched->sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1)
    {
        goto err;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr("0.0.0.0"),
        .sin_port = htons(8888)
    };

    if (bind(sched->sock_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        goto err;
    }

    if (listen(sched->sock_fd, 64) == -1)
    {
        goto err;
    }

    sched->epoll_fd = epoll_create(64);
    if (sched->epoll_fd == -1)
    {
        goto err;
    }

    struct epoll_event ev = {
        .events = EPOLLIN,
        .data.fd = sched->sock_fd
    };

    if (epoll_ctl(sched->epoll_fd, EPOLL_CTL_ADD, sched->sock_fd, &ev) == -1)
    {
        return -1;
    }

    sched->run_flag = 1;

    return 0;

err:
    if (sched->sock_fd > 0)
    {
        close(sched->sock_fd);
    }

    if (sched->epoll_fd > 0)
    {
        close(sched->epoll_fd);
    }

    if (sched->n)
    {
        r3_tree_free(sched->n);
    }

    if (sched->response_buf)
    {
        free(sched->response_buf);
    }

    memset(sched, 0x00, sizeof(struct sched_s));

    return 0;
}

int sched_free(struct sched_s *sched)
{
    if (sched->sock_fd > 0)
    {
        close(sched->sock_fd);
    }

    if (sched->epoll_fd > 0)
    {
        close(sched->epoll_fd);
    }

    if (sched->n)
    {
        r3_tree_free(sched->n);
    }

    if (sched->response_buf)
    {
        free(sched->response_buf);
    }


    memset(sched, 0x00, sizeof(struct sched_s));

    return 0;
}

int sched_dispatch(struct sched_s *sched)
{
    while (sched->run_flag)
    {
        struct epoll_event evs[8];
        int n = epoll_wait(sched->epoll_fd, evs, 8, 1000);

        if (n == -1)
        {
            if (errno == EINTR)
            {
                // interupt by sigint
                break;
            }
        }

        if (n == 0)
        {
            printf("epoll timeout\n");
            continue;
        }

        int i;
        for (i=0; i<n; i++)
        {
            if (sched->sock_fd == evs[i].data.fd)
            {
                struct sockaddr_in addr;
                int fd = accept(sched->sock_fd, (struct sockaddr *)&addr, &(int){sizeof(int)});
                if (fd == -1)
                {
                    continue;
                }

                printf("accept fd->%d\n", fd);

                struct epoll_event ev = {
                    .events = EPOLLIN,
                    .data.fd = fd
                };

                if (epoll_ctl(sched->epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
                {
                    continue;
                }

                continue;
            }

            // remote sock
            char buf[65536] = {0};

            int nbyte = read(evs[i].data.fd, buf, 65535);
            if (nbyte == 65535)
            {
                // data is too long
                sched->restful_cache.res_status = HTTP_STATUS_INTERNAL_SERVER_ERROR;
                strcpy(sched->restful_cache.res, "data too long to parser...");
                goto RESPONSE;
            }

            if (nbyte == 0 || nbyte == -1)
            {
                if (errno == EINTR)
                {
                    continue;
                }

                // disconnect from remote
                if (epoll_ctl(sched->epoll_fd, EPOLL_CTL_DEL, evs[i].data.fd, NULL) == -1)
                {
                    // fd del from epoll failed...
                }

                printf("epoll del fd->%d\n", evs[i].data.fd);
                close(evs[i].data.fd);
                continue;
            }

            restful_cache_clear(sched);

            // call http parser
            http_parser parser;
            http_parser_init(&parser, HTTP_REQUEST);
            parser.data = sched;
                
            http_parser_settings settings = {
                .on_url = restful_on_url,
                .on_body = restful_on_body,
                .on_message_begin = restful_on_message_begin,
                .on_headers_complete = restful_on_headers_complete,
                .on_status = restful_on_status,
                .on_header_field = restful_on_header_field,
                .on_header_value = restful_on_header_value,
                .on_chunk_header = restful_on_chunk_header,
                .on_chunk_complete = restful_on_chunk_complete
            };

            int nparsed = http_parser_execute(&parser, &settings, buf, nbyte);
            if (nparsed != nbyte)
            {
                // http parser handle error
                sched->restful_cache.res_status = HTTP_STATUS_INTERNAL_SERVER_ERROR;
                strcpy(sched->restful_cache.res, "parser data error...");
                goto RESPONSE;
            }

            int url_len = sched->restful_cache.url_len;
            int j;
            for (j=0; j<url_len; j++)
            {
                if (sched->restful_cache.url[j] == '?')
                {
                    url_len = j;
                }
            }

            if (url_len > 1)
            {
                if (sched->restful_cache.url[url_len-1] == '/')
                {
                    url_len--;
                }
            }
            printf("parse url->%s %d\n", sched->restful_cache.url, url_len);

            // r3
            match_entry *entry = match_entry_createl(sched->restful_cache.url, url_len);
            entry->request_method = http_parser_method_to_r3_method_table[parser.method];

            if (entry->request_method == -1)
            {
                // unsupport http request method
                match_entry_free(entry);

                sched->restful_cache.res_status = HTTP_STATUS_NOT_IMPLEMENTED;
                strcpy(sched->restful_cache.res, "request method not support...");
                goto RESPONSE;
            }

            R3Route *matched_route = r3_tree_match_route(sched->n, entry);

            if (matched_route)
            {
                if (entry->vars.tokens.size == entry->vars.slugs.size)
                {
                    // do something
                    int (*func)(match_entry*, char *, void *) = matched_route->data;
                    func(entry, sched->restful_cache.res, sched->udata);
                    match_entry_free(entry);
                }
                else
                {
                    // slugs and tokens size not equal
                    sched->restful_cache.res_status = HTTP_STATUS_INTERNAL_SERVER_ERROR;
                    strcpy(sched->restful_cache.res, "internal error...");
                    match_entry_free(entry);
                    goto RESPONSE;
                }
            }
            else
            {
                // not found
                sched->restful_cache.res_status = HTTP_STATUS_NOT_FOUND;
                strcpy(sched->restful_cache.res, "request not found...");
                match_entry_free(entry);
                goto RESPONSE;
            }

RESPONSE:
            memset(sched->response_buf, 0x00, 65536);
            snprintf(sched->response_buf, 65535,\
                "HTTP/1.1 %d %s\r\n"\
                "Content-Length: %d\r\n"\
                "\r\n"\
                "%s"\
                , sched->restful_cache.res_status, http_status_str(sched->restful_cache.res_status)\
                , sched->restful_cache.res_len == 0 ? strnlen(sched->restful_cache.res, 65535) : sched->restful_cache.res_len\
                , sched->restful_cache.res
            );

            write(evs[i].data.fd, sched->response_buf, strnlen(sched->response_buf, 655356));
        }
    }

    return 0;
}

