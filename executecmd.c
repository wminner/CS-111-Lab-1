#include <unistd.h>   // close, dup2, execvp, fork, getopt_long
#include <sys/wait.h> // waitpid
#include <stdio.h> 	  // printf
#include <errno.h>    // errno
#include <string.h>   // strerror

int executecmd(const char *file, int streams[], char *const argv[], int wait_flag)
{
	pid_t pid;
	int status;
	
	// Fork a child to do execvp work
	if ((pid = fork()) < 0) // Error forking
	{
		fprintf (stderr, "Error: %s\n", strerror(errno));
		return -1;
	}
	else if (pid == 0) // Child
	{
		if (dup2(streams[0], 0) == -1)
		{
			fprintf (stderr, "Error: failed to redirect stdin");
			return -1;
		}
		if (dup2(streams[1], 1) == -1)
		{
			fprintf (stderr, "Error: failed to redirect stdout");
			return -1;
		}
		if (dup2(streams[2], 2) == -1)
		{
			// Don't try to write to stderr because could cause infinite loop
			return -1;
		}
		// Execute command
		// If return value < 0, error occurred
		if (execvp(file, argv) < 0)
		{
			fprintf (stderr, "Error: %s\n", strerror(errno));
			return -1;
		}
	}
	else // Parent
	{
		// Wait for child to return with status if wait flag set
		if (wait_flag)
		{
			if (waitpid(pid, &status, 0) < 0)  // If error occurred
			{
				fprintf (stderr, "Error: %s\n", strerror(errno));
				return -1;
			}
			else
			{
				if (!(wait_flag >> 1))	// Check for test_flag = 2.  If found, don't print.
				{
					printf ("%d ", WEXITSTATUS(status));
					
					int i;
					for (i = 0; argv[i]; i++)
						printf ("%s ", argv[i]);
					printf ("\n");
				}	
				return WEXITSTATUS(status); // Mask LSB (8 bits) for status
			}
		}
		else	// Otherwise just return
			return 0;
	}
	return -1;	// Should never get here
}

