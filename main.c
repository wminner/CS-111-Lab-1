#include <stdlib.h> // exit
#include <stdio.h> 	// printf
#include <string.h> // strlen

#include <unistd.h> // close, dup2, execvp, fork, getopt_long
#include <fcntl.h> 	// open
#include <sys/stat.h> // fchmod
#include <signal.h> // sigaction
#include <getopt.h> // struct option (longopts)

// Prototypes
int openfile(const char *pathname, int flags);
int executecmd(const char *file, int streams[], char *const argv[], int wait_flag);
int findoflags(int open_flag);
void clearoflags();

static int verbose_flag = 0;
static int wait_flag = 0;		// Flag to wait for child processes or not
static int test_flag = 0;		// Activates test mode, serializing all commands for verification testing

// Oflags
static int append_flag = 0;
static int cloexec_flag = 0;
static int creat_flag = 0;
static int directory_flag = 0;
static int dsync_flag = 0;
static int excl_flag = 0;
static int nofollow_flag = 0;
static int nonblock_flag = 0;
static int rsync_flag = 0;
static int sync_flag = 0;
static int trunc_flag = 0;

int main(int argc, char **argv)
{
	int ret; 				// What getopt_long returns
	extern char *optarg;	// Gives the option strings
	extern int optind;		// Gives the current option out of argc options
	int currOptInd = 0;		// Current option index
	int index = 0;			// Index into optarg
	int flags;
	int ever_waited = 0;

	int maxfd = 100;		// Size of logicalfd table
	int *logicalfd = (int*) malloc (maxfd*sizeof(int));	// fd table. Key is logical fd. Value is real fd.
	int fdInd = 0;			// Index/capacity of fd table
	int exit_status = 0;	// Keeps track of how the program should exit
	int exit_max = 0;		// Sum of child process exit statuses to use when wait_flag set
	int args_found = 0;		// Used to verify --option num of arguments requirement
	int option_index = 0;	// Used with getopt_long
	
	extern int opterr;		// Declared in getopt_long
	opterr = 0;				// Turns off automatic error message from getopt_long
	
	static struct option long_options[] =
		{
			/* Flag setting options */
			{"verbose",   no_argument, &verbose_flag,   1},
			{"brief",     no_argument, &verbose_flag,   0},
			{"wait", 	  no_argument, &wait_flag,      0},	// Pre-scan turns wait on. Turn off after you pass it.
			{"test",      no_argument, &test_flag,      1},
			
			/* Oflags go to -1 so we can AND them with actual oflags */
			{"append",    no_argument, &append_flag,    -1},
			{"cloexec",   no_argument, &cloexec_flag,   -1},
			{"creat",     no_argument, &creat_flag,     -1},
			{"directory", no_argument, &directory_flag, -1},
			{"dsync",     no_argument, &dsync_flag,     -1},
			{"excl",      no_argument, &excl_flag,      -1},
			{"nofollow",  no_argument, &nofollow_flag,  -1},
			{"nonblock",  no_argument, &nonblock_flag,  -1},
			{"rsync",     no_argument, &rsync_flag,     -1},
			{"sync",      no_argument, &sync_flag,      -1},
			{"trunc",     no_argument, &trunc_flag,     -1},

			/* Non flag setting options */
			{"rdonly",  required_argument, 0, 'r'},
			{"wronly",  required_argument, 0, 'w'},
			{"rdwr",    required_argument, 0, 'b'},
			{"command", required_argument, 0, 'c'},
			{"pause",   no_argument,       0, 'p'},
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
		for (int i = 0; i < argc; i++)	// Pre-scan looking for "--test"
		{
			if (strcmp(argv[i], "--test") == 0)	// Found "--test"
			{
				test_flag = 1;
				break;
			}
		}
		
		for (int i = 0; i < argc; i++)	// Pre-scan looking for "--wait"
		{
			if (strcmp(argv[i], "--wait") == 0)	// Found "--wait"
			{
				wait_flag = 1;
				ever_waited = 1;
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
						exit_status = 1;
						args_found = 0;
						break;
					}
										
					// Open file and save its file descriptor
					if (fdInd >= maxfd)	// If not enough space in fd table...
					{
						maxfd *= 2;
						logicalfd = (int*) realloc (logicalfd, maxfd*sizeof(int)); // Double the array size
					}
					
					// Figure out flags
					flags = findoflags(O_RDONLY);
					
					logicalfd[fdInd] = openfile(optarg, flags);	// Open file with appropriate flags					
					if (logicalfd[fdInd] < 0)	// If something went wrong with open file operation...
						exit_status = 1;
					clearoflags();	// Clear oflag
						
					fdInd++;
					
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
						exit_status = 1;
						args_found = 0;
						break;
					}
					
					// Open file and save its file descriptor
					if (fdInd >= maxfd)	// If not enough space in fd table...
					{
						maxfd *= 2;
						logicalfd = (int*) realloc (logicalfd, maxfd*sizeof(int)); // Double the array size
					}
					
					// Figure out flags
					flags = findoflags(O_WRONLY);
					
					logicalfd[fdInd] = openfile(optarg, flags);	// Open file with appropriate flags
					if (logicalfd[fdInd] < 0)	// If something went wrong with open file operation...
						exit_status = 1;
					clearoflags();	// Clear oflag
					
					fdInd++;
					
					args_found = 0;		// Reset args found for next option
					break;
				case 'b': 	// read and write (both)
					if (verbose_flag)
						printf ("--rdwr ");
					
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
						fprintf (stderr, "Error: \"--rdwr\" accepts one argument.  You supplied %d arguments.\n", args_found);
						exit_status = 1;
						args_found = 0;
						break;
					}
										
					// Open file and save its file descriptor
					if (fdInd >= maxfd)	// If not enough space in fd table...
					{
						maxfd *= 2;
						logicalfd = (int*) realloc (logicalfd, maxfd*sizeof(int)); // Double the array size
					}
					
					// Figure out flags
					flags = findoflags(O_RDWR);
					
					logicalfd[fdInd] = openfile(optarg, flags);	// Open file with appropriate flags
					if (logicalfd[fdInd] < 0)	// If something went wrong with open file operation...
						exit_status = 1;
					clearoflags();	// Clear oflag
					
					fdInd++;
					
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
					int optstart = optind;	// Copy optind as it may change
					
					// Process options while optind <= argc or encounter '--' 
					while (currOptInd <= argc)
					{
						if (optarg[index] == '-' && optarg[index+1] == '-') // Check for '--'
							break;
						else	// Else, found another argument
						{
							optind++;
							args_found++;
						}
						if (verbose_flag)
							printf ("%s ", optarg+index);
						if ((currOptInd-optstart) < 3) // If in the first 3 arguments (streams)...
							streams[currOptInd-optstart] = logicalfd[atoi(optarg+index)];
						else  // If in arguments after streams
							execArgv[execArgc++] = optarg+index;
						
						delim = strchr(optarg+index, '\0');	// Look for null byte
						index = delim - optarg + 1;	// Find index into optarg using delim
						currOptInd++;
					}
					optind--;	// Needed or optind off by one otherwise
					
					if (verbose_flag)
						printf ("\n");
					execArgv[execArgc] = NULL;	// execvp requires NULL at end of arg string array
					
					if (args_found < 4)	// Error if num of args less than 4
					{
						fprintf (stderr, "Error: \"--command\" requires at least four arguments.  You supplied %d arguments.\n", args_found);
						exit_status = 1;
						args_found = 0;
						break;
					}
										
					// Encode test_flag at binary position 2.  Executecmd prints will be supressed.
					int exec_ret = executecmd(execArgv[0], streams, execArgv, wait_flag | (test_flag << 1));
					if (exec_ret < 0) 	// Error occurred with executecmd
						exit_status = 1;
					else				// Sum process's exit status
						if (exec_ret > exit_max)
							exit_max = exec_ret;
					
					args_found = 0;
					break;
				}
				case 'p':	// pause
					pause();	// Wait for a signal to arrive
					break;
				case '?':
					fprintf (stderr, "Error: unrecognized option\n");
					exit_status = 1;
					break;
				default:
					fprintf (stderr, "Error: unrecognized option\n");
					exit_status = 1;
			}
		}
	}
	free (logicalfd);
	
	if (ever_waited || test_flag)
		exit(exit_max);
	else
		exit(exit_status);
}

int findoflags(int open_flag)
{
	int total = (append_flag & O_APPEND)     | (cloexec_flag & O_CLOEXEC)     |
				(creat_flag & O_CREAT)       | (directory_flag & O_DIRECTORY) |
				(dsync_flag & O_DSYNC)       | (excl_flag & O_EXCL)           |
				(nofollow_flag & O_NOFOLLOW) | (nonblock_flag & O_NONBLOCK)   |
				(rsync_flag & O_RSYNC)       | (sync_flag & O_SYNC)           |
				(trunc_flag & O_TRUNC)       | (open_flag);
	
	return total;
}

void clearoflags()
{
	append_flag = 0;
	cloexec_flag = 0;
	creat_flag = 0;
	directory_flag = 0;
	dsync_flag = 0;
	excl_flag = 0;
	nofollow_flag = 0;
	nonblock_flag = 0;
	rsync_flag = 0;
	sync_flag = 0;
	trunc_flag = 0;
}