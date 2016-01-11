#include <stdlib.h> // exit
#include <stdio.h> 	// printf
#include <string.h> // strlen

#include <unistd.h> // close, dup2, execvp, fork, getopt_long
#include <fcntl.h> 	// open
#include <signal.h> // sigaction
#include <getopt.h> // struct option (longopts)

// Prototypes
int openfile(const char *pathname, int flags);
int executecmd(const char *file, int streams[], char *const argv[]);

static int verbose_flag;

int main(int argc, char **argv)
{
	int ret; 				// What getopt_long returns
	extern char *optarg;	// Gives the option strings
	extern int optind;		// Gives the current option out of argc options
	int currOptInd;
	int index;
	int logicalfd[100];		// File descriptor table. Key is logical fd. Value is real fd.
	int fdInd = 0;			// Index for logical fd
	int success = 1;

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
			currOptInd = optind;
			index = 0;
			
			if (ret == -1) // No more options found, then break out of while loop
				break;

			switch (ret)
			{
			case 0: // Occurs for options that set flags (verbose or brief)
				if (verbose_flag)
					//printf ("verbose flag set\n");
					continue;	// Do verbose stuff
				else
					//printf ("brief flag set\n");
					continue;	// Do brief stuff
				break;
			case 'r':	// rdonly
				if (verbose_flag)
					printf ("--rdonly ");
				
				// Process options while optind <= argc or encounter '--' 
				while (currOptInd <= argc)
				{
					if (optarg[index] == '-' && optarg[index+1] == '-') // Check for '--'
						break;
					if (verbose_flag)
						printf ("%s ", optarg+index);
					while (optarg[index] != '\0')
						index++;
					index++;
					currOptInd++;
				}
				if (verbose_flag)
					printf ("\n");
				
				// Copy argument string into a new allocated string
				char *rdpath = (char*) malloc (strlen(optarg)+1);
				strcpy(rdpath, optarg);
				
				// Open file and save its file descriptor
				logicalfd[fdInd] = openfile(rdpath, O_RDONLY);
				if (logicalfd[fdInd] < 0)
					success = 0;
				
				// TODO: don't go over max logicalfd size (100) or dynamicaly allocate more space
				// Temporary: wrap fdInd so you can't go past end of array
				fdInd = (fdInd+1) % (sizeof(logicalfd)/sizeof(int)); // Will be '% 100'
				
				// Free argument string when done
				free ((char*)rdpath);
				break;
			case 'w':	// wronly
				if (verbose_flag)
					printf ("--wronly ");
				
				// Process options while optind <= argc or encounter '--' 
				while (currOptInd <= argc)
				{
					if (optarg[index] == '-' && optarg[index+1] == '-') // Check for '--'
						break;
					if (verbose_flag)
						printf ("%s ", optarg+index);
					while (optarg[index] != '\0')
						index++;
					index++;
					currOptInd++;
				}
				if (verbose_flag)
					printf ("\n");
				
				// Copy argument string into a new allocated string
				char *wrpath = (char*) malloc (strlen(optarg)+1);
				strcpy(wrpath, optarg);
				
				// Open file and save its file descriptor
				logicalfd[fdInd] = openfile(wrpath, O_WRONLY);
				if (logicalfd[fdInd] < 0)
					success = 0;
				
				// TODO: don't go over max logicalfd size (100) or dynamicaly allocate more space
				// Temporary: wrap fdInd so you can't go past end of array
				fdInd = (fdInd+1) % (sizeof(logicalfd)/sizeof(int)); // Will be '% 100'
				
				// Free argument string when done
				free ((char*)wrpath);
				break;
			case 'c':	// command
			{
				if (verbose_flag)
					printf ("--command ");
				
				int streams[3];
				char *execArgv[sizeof(optarg)/sizeof(optarg[0])];
				char *delim;	
				int execArgc = 0;	
				
				// Process options while optind <= argc or encounter '--' 
				while (currOptInd <= argc)
				{
					if (optarg[index] == '-' && optarg[index+1] == '-') // Check for '--'
						break;
					if (verbose_flag)
						printf ("%s ", optarg+index);
					if ((currOptInd-optind) < 3) // If in the first 3 arguments (streams)...
					{
						streams[currOptInd-optind] = logicalfd[atoi(optarg+index)];
					}
					else  // If in arguments after streams
					{
						execArgv[execArgc++] = optarg+index;
					}
					
					delim = strchr(optarg+index, '\0');	// Look for null byte
					index = delim - optarg + 1;	// Find index into optarg using delim
					currOptInd++;
				}
				if (verbose_flag)
					printf ("\n");
				execArgv[execArgc] = NULL;
				
				// Test arguments
				// int streams[] = {logicalfd[0],logicalfd[1],logicalfd[2]};
				// char *execArgv[] = {"ls", "-l", NULL};
				// End test arguments
				
				if (executecmd(execArgv[0], streams, execArgv) < 0) // Error occurred
					success = 0;
				break;
			}
			case '?':
				fprintf (stderr, "Error: unrecognized option\n");
				break;
			default:
				//printf ("default case\n");
				fprintf (stderr, "Error: unrecognized option\n");
			}
		}
	}
	if (success)
		exit(0);
	else
		exit(1);
}