#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>

void unshare_namespace()
{
	const int FLAGS = CLONE_NEWUTS|CLONE_NEWPID;

	if (unshare(FLAGS) == -1)	
	{
		perror("unshare");
		exit(EXIT_FAILURE);
	}

	// TODO
	// mount /proc in new namespace
}

// wrapper for exec()
// TODO initialize env
void start_container(char *path, char **argv, char** envp)
{
	char hostname[] = "container";
	sethostname(hostname, sizeof(hostname)/sizeof(*hostname));
	// fprintf(stderr, "%zd %zd", sizeof(hostname), sizeof(*hostname));

	execve(path, argv, envp);
}

void hocker(char *path, char **argv)
{
	unshare_namespace();

	pid_t pid = fork();
	switch (pid)
	{
	case -1:
		perror("fork");
		exit(EXIT_FAILURE);
	case 0:
		start_container(path, argv, NULL);
		break;
	default:
		int status;
		wait(&status);
		exit(EXIT_SUCCESS);
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fputs("no program provided. exited", stderr);
		exit(EXIT_FAILURE);
	}

	hocker(argv[1], &argv[1]);
	exit(EXIT_SUCCESS);
}
