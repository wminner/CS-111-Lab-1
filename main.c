#include <stdlib.h> // exit
#include <stdio.h> // printf

#include <unistd.h>  // close, dup2, execvp, fork, getopt_long
#include <sys/types.h> // open
#include <signal.h> // sigaction
#include <getopt.h> // struct option (longopts)

static int verbose_flag;

int main(int argc, char **argv)
{
  int ret; // what getopt_long returns
  
  if (argc <= 1) // No arguments
  {
    printf ("usage: %s --<command> ...\n", argv[0]);
  } else if (argc > 1) // At least one argument
  {
    while (1)
      {
	static struct option long_options[] =
	  {
	    /* Flag setting options */
	    {"verbose", no_argument, &verbose_flag, 1},
	    {"brief",   no_argument, &verbose_flag, 0},

	    /* Non flag setting options */
	    {"rdonly",  required_argument, 0, 'r'},
	    {"wronly",  required_argument, 0, 'w'},
	    {"command", required_argument, 0, 'c'},
	    {0, 0, 0, 0}
	  };

	int option_index = 0;

	ret = getopt_long (argc, argv, "", long_options, &option_index);

	if (ret == -1) // No more options found...
	  break;

	switch (ret)
	  {
	  case 0:
	    printf ("found 0\n");
	    break;
	  case 'r':
	    printf ("found rdonly\n");
	    break;
	  case 'w':
	    printf ("found wronly\n");
	    break;
	  case 'c':
	    printf ("found command\n");
	    break;
	  default:
	    printf ("default case\n");
	  }	
      }
      if (verbose_flag)
	printf ("verbose flag set\n");
      else
	printf ("verbose flag unset\n");
      //printf ("options for argument %s is %s\n", argv[1], );
  }
  exit(0);  
}
