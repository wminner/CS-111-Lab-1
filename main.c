#include <stdlib.h> // exit
#include <stdio.h> // printf
#include <string.h> // strlen

#include <unistd.h>  // close, dup2, execvp, fork, getopt_long

#include <signal.h> // sigaction
#include <getopt.h> // struct option (longopts)

#include "openfile.c"
#include "executecmd.c"

static int verbose_flag;

int main(int argc, char **argv)
{
	int ret; 				// What getopt_long returns
	extern char *optarg;	// Gives the option strings
	extern int optind;		// Gives the current option out of argc options
	int tempind;
	int index;
	int logicalfd[100];		// File descriptor table. Key is logical fd. Value is real fd.
	int fdInd = 0;			// Index for logical fd

	if (argc <= 1) // No arguments
	{
		printf("usage: %s --<command> ...\n", argv[0]);
	}
	else if (argc > 1) // At least one argument
	{
		while (1) // Loop until getop_long doesn't find anything (then break)
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
				if (verbose_flag)
					printf ("verbose flag set\n");
				else
					printf ("brief flag set\n");
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
				
				// Copy argument string into a new allocated string
				char *rdpath = (char*) malloc (strlen(optarg)+1);
				strcpy(rdpath, optarg);
				
				// Open file and save its file descriptor
				logicalfd[fdInd] = openfile(rdpath, O_RDONLY);
				printf("rdonly logicalfd[%d] is %d\n", fdInd, logicalfd[fdInd]);
				
				// TODO: don't go over max logicalfd size (100) or dynamicaly allocate more space
				// Temporary: wrap fdInd so you can't go past end of array
				fdInd = (fdInd+1) % (sizeof(logicalfd)/sizeof(int)); // Will be '% 100'
				
				// Free argument string when done
				free ((char*)rdpath);
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
				
				// Copy argument string into a new allocated string
				char *wrpath = (char*) malloc (strlen(optarg)+1);
				strcpy(wrpath, optarg);
				
				// Open file and save its file descriptor
				logicalfd[fdInd] = openfile(wrpath, O_WRONLY);
				printf("wronly logicalfd[%d] is %d\n", fdInd, logicalfd[fdInd]);
				
				// TODO: don't go over max logicalfd size (100) or dynamicaly allocate more space
				// Temporary: wrap fdInd so you can't go past end of array
				fdInd = (fdInd+1) % (sizeof(logicalfd)/sizeof(int)); // Will be '% 100'
				
				// Free argument string when done
				free ((char*)wrpath);
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
	}
	exit(0);
}

// /* Wrapper for open */
// int openfile(const char *pathname, int flags)
// {
	
// }

// /* Wrapper for execvp */
// int executecmd(const char *file, char *const argv[])
// {
	
// }