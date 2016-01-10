#include <stdlib.h> // exit
#include <stdio.h> // printf

#include <unistd.h>  // close, dup2, execvp, fork, getopt_long
#include <fcntl.h> // open
#include <signal.h> // sigaction
#include <getopt.h> // struct option (longopts)

static int verbose_flag;

int main(int argc, char **argv)
{
	int ret; 				// What getopt_long returns
	extern char *optarg;	// Gives the option strings
	extern int optind;		// Gives the current option out of argc options
	int tempind;
	int index;
	int fdTable[];			// File descriptor table. Key is logical fd. Value is real fd.

	if (argc <= 1) // No arguments
	{
		printf("usage: %s --<command> ...\n", argv[0]);
	}
	else if (argc > 1) // At least one argument
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

			ret = getopt_long(argc, argv, "", long_options, &option_index);
			tempind = optind;
			index = 0;
			
			if (ret == -1) // No more options found, then break out of while loop
				break;

			switch (ret)
			{
			case 0: // Occurs for options that set flags (verbose or brief)
				break;
			case 'r':	// rdonly
				printf ("found \"rdonly\" with arguments ");
				// Process options while optind <= argc or encounter '--' 
				while (tempind <= argc)
				{
					if (optarg[index] == '-') 
						if (optarg[index+1] == '-') // Check for '--'
							break;
					printf ("\'%s\' ", optarg+index);
					while (optarg[index] != '\0')
						index++;
					index++;
					tempind++;
				}
				printf ("\n");
				
				// TODO: Do open file operation and create logical file descriptor
				// openfile(...)
				break;
			case 'w':	// wronly
				printf ("found \"wronly\" with arguments ");
				// Process options while optind <= argc or encounter '--' 
				while (tempind <= argc)
				{
					if (optarg[index] == '-') 
						if (optarg[index+1] == '-') // Check for '--'
							break;
					printf ("\'%s\' ", optarg+index);
					while (optarg[index] != '\0')
						index++;
					index++;
					tempind++;
				}
				printf ("\n");
				
				// TODO: Do open file operation and create logical file descriptor
				// openfile(...)
				break;
			case 'c':	// command
				printf ("found \"command\" with arguments ");
				// Process options while optind <= argc or encounter '--' 
				while (tempind <= argc)
				{
					if (optarg[index] == '-') 
						if (optarg[index+1] == '-') // Check for '--'
							break;
					printf ("\'%s\' ", optarg+index);
					while (optarg[index] != '\0')
						index++;
					index++;
					tempind++;
				}
				printf ("\n");
				
				// TODO: execute command with options in a child fork
				// executecmd(...)
				break;
			case '?':
				// printf ("unrecognized option\n"); // exit gracefully?
				break;
			default:
				printf ("default case\n");
			}
		}
		if (verbose_flag)
			printf ("verbose flag set\n");
		else
			printf ("verbose flag not set\n");
	}
	exit(0);
}
