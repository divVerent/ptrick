#include <stdint.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <sys/ptrace.h>

#include <err.h>

int main(int argc, char **argv)
{
	if(argc < 2)
		errx(__LINE__, "Usage: %s program args...\n", argv[0]);
	struct stat st;
	if(stat(argv[1], &st))
		err(__LINE__, "stat");
	fprintf(stderr, "Pre-exec size: %ju\n", (uintmax_t) st.st_size);
	pid_t pid = fork();
	if(pid < 0)
		err(__LINE__, "fork");
	if(pid == 0)
	{
		if(ptrace(PTRACE_TRACEME, 0, 0, 0))
			err(__LINE__, "ptrace");
		raise(SIGSTOP);
		execv(argv[1], argv + 1);
		err(__LINE__, "execv");
	}
	int state = 0;
	int status;
	while(waitpid(pid, &status, 0) == pid)
	{
		if(state == 0 && WSTOPSIG(status) == SIGSTOP)
		{
			int data = PTRACE_O_TRACEEXEC;
			if(ptrace(PTRACE_SETOPTIONS, pid, 0, (void*)data))
				err(__LINE__, "ptrace");
			if(kill(pid, SIGCONT))
				err(__LINE__, "kill");
			if(ptrace(PTRACE_CONT, pid, 0, 0))
				err(__LINE__, "ptrace");
			state = 1;
		}
		else if(state == 1 && (status >> 8) == (SIGTRAP | (PTRACE_EVENT_EXEC << 8)))
		{
			char buf[64];
			snprintf(buf, sizeof(buf), "/proc/%ju/exe", (uintmax_t) pid);
			buf[sizeof(buf) - 1] = 0;
			if(stat(buf, &st))
				err(__LINE__, "stat");
			fprintf(stderr, "Real size: %ju\n", (uintmax_t) st.st_size);
			if(ptrace(PTRACE_DETACH, pid, 0, 0))
				err(__LINE__, "ptrace");
			state = 2;
		}
		else if(WIFEXITED(status))
			break;
		else if(state < 2)
			if(ptrace(PTRACE_CONT, pid, 0, 0))
				err(__LINE__, "ptrace");
	}
	fprintf(stderr, "Program exited (status %d).\n", WEXITSTATUS(status));
	if(state != 2)
		fprintf(stderr, "Tracing was NOT active yet!\n");
	return WEXITSTATUS(status);
}
