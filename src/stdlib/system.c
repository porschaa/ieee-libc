#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int system(const char *cmd)
{
	int state;
	pid_t pid;

	if (cmd == NULL) {
		return (1);
	}
	if ((pid = fork()) == 0) {
		execl("/bin/sh", "sh", "-c", cmd, (char *)0);
		_exit(127);
	}
	if (pid == -1) {
		state = -1;
	}
	else {
		while (waitpid(pid, &state, 0) == -1) {
			if (errno != EINTR) {
				state = -1;
				break;
			}
		}
	}
	return (state);
}
