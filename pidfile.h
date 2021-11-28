#ifndef __SRC_PIDFILE_H
#define __SRC_PIDFILE_H

#define PIDFILENAME "dp.pid"

int pidfile_write(int pid);

void pidfile_remove();

int is_running();

#endif
