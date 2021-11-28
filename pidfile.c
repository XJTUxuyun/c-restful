#include "pidfile.h"

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>

void pidfile_remove()
{
	unlink(PIDFILENAME);
}

int pidfile_write(int pid)
{
	FILE *fp = fopen(PIDFILENAME, "w");
	if (!fp)
	{
		perror("fopen failed");
		return -1;
	}

	fprintf(fp, "%ld\n", pid);
	fclose(fp);
	return 0;
}

int is_running()
{
	FILE *fp = fopen(PIDFILENAME, "r");
	if (!fp)
	{
		perror("fopen failed");
		return 0;
	}

	pid_t pid;
	fscanf(fp, "%d", &pid);
	fclose(fp);

	if (kill(pid, 0))
	{
		pidfile_remove();
		return 0;
	}

	return 1;
}
