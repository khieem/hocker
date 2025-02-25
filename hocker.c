#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mount.h>

void unshare_namespace()
{
	const int FLAGS = CLONE_NEWUTS|CLONE_NEWPID|CLONE_NEWNS;

	if (unshare(FLAGS) == -1)	
	{
		perror("unshare");
		exit(EXIT_FAILURE);
	}
}

void start_container(char *path, char **argv, char** envp)
{
	char hostname[] = "container";
	sethostname(hostname, sizeof(hostname)/sizeof(*hostname));

	mkdir("/proc", 0755);
	if (mount("proc", "/proc", "proc", 0, NULL) != 0)
	{
		perror("mount procfs");
		exit(EXIT_FAILURE);
	}

	execve(path, argv, envp);
}

void hocker(char *path, char **argv)
{
	unshare_namespace();
	mount("./chroot", "./chroot", NULL, MS_BIND | MS_PRIVATE | MS_REC, NULL);
	chroot("./chroot");
	chdir("/");

	pid_t pid = fork();
	switch (pid)
	{
	case -1:
		perror("fork");
		exit(EXIT_FAILURE);
	case 0:	// init
		pid = fork();
		switch (pid)
		{
		case -1:
			perror("fork init");
			exit(EXIT_FAILURE);
			break;
		case 0:	// first user process. TODO drop privileges
			start_container(path, argv, NULL);
		default:
			wait(NULL);
			break;
		}
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
		fputs("no program provided\n", stderr);
		exit(EXIT_FAILURE);
	}

	if (geteuid() != 0)
	{
		fputs("not run as root\n", stderr);
		exit(EXIT_FAILURE);
	}

	if (system("cd chroot 2>/dev/null") != 0)
	{
		mkdir("chroot", 0755);
		puts("no images found. downloading...");
		system("wget -O - https://dl-cdn.alpinelinux.org/alpine/v3.21/releases/x86_64/alpine-minirootfs-3.21.3-x86_64.tar.gz | tar -C chroot -xzpf -");
		puts("image extracted");
	}
	
	hocker(argv[1], &argv[1]);
	exit(EXIT_SUCCESS);
}
