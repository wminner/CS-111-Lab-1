#include <fcntl.h> // open
// #include <errno.h>

int openfile(const char *pathname, int flags)
{
	int fd = open(pathname, flags);
	// if (fd == -1)
		// printf("%s\n", strerror(errno));
	return fd;
}