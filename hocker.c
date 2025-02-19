#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void hocker(char *path, char **argv)
{
	execve(path, argv, NULL);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fputs("no program provided. exited", stderr);
		exit(EXIT_FAILURE);
	}

	pid_t pid = fork();
	switch (pid)
	{
	case -1:
		perror("fork");
		exit(EXIT_FAILURE);
	case 0:
		hocker(argv[1], &argv[1]);
		break;
	default:
		int status;
		wait(&status);
		exit(EXIT_SUCCESS);
	}
}
