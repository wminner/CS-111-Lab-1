#include <stdlib.h> // exit
#include <stdio.h> 	// printf
#include <string.h> // strlen

#include <unistd.h> // close, dup2, execvp, fork, getopt_long
#include <fcntl.h> 	// open
#include <signal.h> // sigaction
#include <getopt.h> // struct option (longopts)

// Prototypes
int openfile(const char *pathname, int flags);
int executecmd(const char *file, int streams[], char *const argv[], int wait_flag);

static int verbose_flag = 0;
static int wait_flag = 0;		// Flag to wait for child processes or not

int main(int argc, char **argv)
{
	int ret; 				// What getopt_long returns
	extern char *optarg;	// Gives the option strings
	extern int optind;		// Gives the current option out of argc options
	int currOptInd = 0;		// Current option index
	int index = 0;			// Index into optarg

	int maxfd = 100;		// Size of logicalfd table
	int *logicalfd = (int*) malloc (maxfd*sizeof(int));	// fd table. Key is logical fd. Value is real fd.
	int fdInd = 0;			// Index/capacity of fd table
	int exit_status = 0;	// Keeps track of how the program should exit
	int exit_sum = 0;		// Sum of child process exit statuses to use when wait_flag set
	int args_found = 0;		// Used to verify --option num of arguments requirement
	int option_index = 0;	// Used with getopt_long
	
	static struct option long_options[] =
		{
			/* Flag setting options */
			{"verbose", no_argument, &verbose_flag, 1},
			{"brief",   no_argument, &verbose_flag, 0},
			{"wait", 	no_argument, &wait_flag, 1},

			/* Non flag setting options */
			{"rdonly",  required_argument, 0, 'r'},
			{"wronly",  required_argument, 0, 'w'},
			{"command", required_argument, 0, 'c'},
			{0, 0, 0, 0}
		};
	
	if (!logicalfd)		// Make sure logicalfd was allocated correctly
	{	
		fprintf (stderr, "Error: failed to allocate memory\n");
		exit(1);
	}

	if (argc <= 1) // No arguments
	{
		printf("usage: %s --<command> ...\n", argv[0]);
	}
	else if (argc > 1) // At least one argument
	{
		for (int i = 0; i < argc; i++)	// Pre-scan looking for "--wait"
		{
			if (strcmp(argv[i], "--wait") == 0)	// Found "--wait"
			{
				wait_flag = 1;
				break;
			}
		}
		
		while (1) 	// Loop until getop_long doesn't find anything (then break)
		{
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
					else	// Else, found another argument
						args_found++;
					if (verbose_flag)
						printf ("%s ", optarg+index);
					while (optarg[index] != '\0')
						index++;
					index++;
					currOptInd++;
				}
				if (verbose_flag)
					printf ("\n");
				
				if (args_found != 1)	// Error if num of args not 1
				{
					fprintf (stderr, "Error: \"--rdonly\" accepts one argument.  You supplied %d arguments.\n", args_found);
					exit_status = -1;
					goto end_r_case;
				}
				
				// Copy argument string into a new allocated string
				char *rdpath = (char*) malloc (strlen(optarg)+1);
				if (rdpath)
					strcpy(rdpath, optarg);
				else
				{
					fprintf(stderr, "Error: failed to allocate memory\n");
					exit(1);
				}
				
				// Open file and save its file descriptor
				if (fdInd >= maxfd)	// If not enough space in fd table...
				{
					maxfd *= 2;
					logicalfd = (int*) realloc (logicalfd, maxfd*sizeof(int)); // Double the array size
				}
				logicalfd[fdInd] = openfile(rdpath, O_RDONLY);
				if (logicalfd[fdInd] < 0)	// If something went wrong with open file operation...
					exit_status = -1;
				fdInd++;
				free (rdpath);	// Free argument string when done
				
				end_r_case:
				args_found = 0;		// Reset args found for next option
				break;
			case 'w':	// wronly
				if (verbose_flag)
					printf ("--wronly ");
				
				// Process options while optind <= argc or encounter '--' 
				while (currOptInd <= argc)
				{
					if (optarg[index] == '-' && optarg[index+1] == '-') // Check for '--'
						break;
					else	// Else, found another argument
						args_found++;
					if (verbose_flag)
						printf ("%s ", optarg+index);
					while (optarg[index] != '\0')
						index++;
					index++;
					currOptInd++;
				}
				if (verbose_flag)
					printf ("\n");
				
				if (args_found != 1)	// Error if num of args not 1
				{
					fprintf (stderr, "Error: \"--wronly\" accepts one argument.  You supplied %d arguments.\n", args_found);
					exit_status = -1;
					goto end_w_case;
				}
				
				// Copy argument string into a new allocated string
				char *wrpath = (char*) malloc (strlen(optarg)+1);
				if (wrpath)
					strcpy(wrpath, optarg);
				else
				{
					fprintf(stderr, "Error: failed to allocate memory\n");
					exit(1);
				}
				
				// Open file and save its file descriptor
				if (fdInd >= maxfd)	// If not enough space in fd table...
				{
					maxfd *= 2;
					logicalfd = (int*) realloc (logicalfd, maxfd*sizeof(int)); // Double the array size
				}
				logicalfd[fdInd] = openfile(wrpath, O_WRONLY);
				if (logicalfd[fdInd] < 0)	// If something went wrong with open file operation...
					exit_status = -1;
				
				fdInd++;
				free (wrpath);	// Free argument string when done
				
				end_w_case:
				args_found = 0;		// Reset args found for next option
				break;
			case 'c':	// command
			{
				if (verbose_flag)
					printf ("--command ");
				
				int streams[3];		// Holds three fd streams to use with dup2
				char *execArgv[argc - optind + 1];	// Size is conservatively set to number of arguments remaining to be parsed
				char *delim;		// Points to null byte at end of each option/arg
				int execArgc = 0;	// Number of args to use with --command
				
				// Process options while optind <= argc or encounter '--' 
				while (currOptInd <= argc)
				{
					if (optarg[index] == '-' && optarg[index+1] == '-') // Check for '--'
						break;
					else	// Else, found another argument
						args_found++;
					if (verbose_flag)
						printf ("%s ", optarg+index);
					if ((currOptInd-optind) < 3) // If in the first 3 arguments (streams)...
						streams[currOptInd-optind] = logicalfd[atoi(optarg+index)];
					else  // If in arguments after streams
						execArgv[execArgc++] = optarg+index;
					
					delim = strchr(optarg+index, '\0');	// Look for null byte
					index = delim - optarg + 1;	// Find index into optarg using delim
					currOptInd++;
				}
				if (verbose_flag)
					printf ("\n");
				execArgv[execArgc] = NULL;	// execvp requires NULL at end of arg string array
				
				if (args_found < 4)	// Error if num of args less than 4
				{
					fprintf (stderr, "Error: \"--command\" requires at least four arguments.  You supplied %d arguments.\n", args_found);
					exit_status = -1;
					goto end_c_case;
				}
				
				// Test arguments
				// int streams[] = {logicalfd[0],logicalfd[1],logicalfd[2]};
				// char *execArgv[] = {"ls", "-l", NULL};
				// End test arguments
				
				int exec_ret = executecmd(execArgv[0], streams, execArgv, wait_flag);
				if (exec_ret < 0) 	// Error occurred with executecmd
					exit_status = -1;
				else				// Sum process's exit status
					exit_sum += exec_ret;
				
				end_c_case:
				args_found = 0;
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
	free (logicalfd);
	
	if (wait_flag)
		exit(exit_sum);
	else
		exit(exit_status);
}