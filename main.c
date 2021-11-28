#include <stdio.h>

#include "sched.h"

int main(int argc, char **argv)
{

    struct sched_s sched;
    if (sched_init(&sched) == -1)
    {
        return -1;
    }

    sched_dispatch(&sched);

    sched_free(&sched);

    return 0;
}
