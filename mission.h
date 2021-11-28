#ifndef __MISSION_H
#define __MISSION_H

struct mission_s
{
    char name[128];

    struct
    {
        int recv;
        int send;
    } sock;
};

#endif
