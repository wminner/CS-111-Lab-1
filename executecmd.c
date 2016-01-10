#include <unistd.h>   // close, dup2, execvp, fork, getopt_long
#include <sys/wait.h> // waitpid
#include <stdio.h> 	  // printf
#include <errno.h>    // errno
#include <string.h>   // strerror
#include <stdio.h>    // printf

int executecmd(const char *file, int streams[], char *const argv[])
{
	pid_t pid;
	int status;
	
	// Fork a child to do execvp work
	if ((pid = fork()) < 0) // Error forking
	{
		printf ("Error: %s\n", strerror(errno));
		return -1;
	}
	else if (pid == 0) // Child
	{
		dup2(streams[0], 0);
		dup2(streams[1], 1);
		dup2(streams[2], 2);
		// Execute command
		// If return value < 0, error occurred
		if (execvp(file, argv) < 0)
		{
			printf ("Error: %s\n", strerror(errno));
			return -1;
		}
	}
	else // Parent
	{
		// Wait for child to return with status?
		if (waitpid(pid, &status, 0) < 0)  // If error occurred
		{
			printf ("Error: %s\n", strerror(errno));
			return -1;
		}
		else
			return WEXITSTATUS(status); // Mask LSB (8 bits) for status
	}
	return -1;
}