#include <fcntl.h> 	// open
#include <errno.h>  // errno
#include <string.h> // strerror
#include <stdio.h>  // printf

int openfile(const char *pathname, int flags)
{
	int fd = open(pathname, flags);
	if (fd < 0)
		printf("Error with \"%s\": %s\n", pathname, strerror(errno));
	return fd;
}