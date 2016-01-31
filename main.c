#define _GNU_SOURCE	// REG_RIP
#include <stdlib.h> // exit
#include <stdio.h> 	// printf
#include <string.h> // strlen, strerror

#include <unistd.h> // close, dup2, execvp, fork, getopt_long
#include <fcntl.h> 	// open
#include <signal.h> // signal
#include <getopt.h> // struct option (longopts)

#include <sys/wait.h> // waitpid
#include <errno.h>    // errno

#include <sys/time.h> 		// getrusage
#include <sys/resource.h>	// getrusage

#define OPT_START 0
#define OPT_END 1
#define CHI_SUM 2
#define PAR_SUM 3
#define USAGE_TOTAL 4
#define PIPEFLAGS 0

// Prototypes defined in other c files
int openfile(const char *pathname, int flags);	// openfile.c
// Prototypes defined in main.c
int findoflags(int open_flag);
void clearoflags();
void catch_signal(int sig);
void ignore_signal(int sig);
int executecmd(const char *file, int streams[], char *const argv[]);
void profile_print(int start, int end, struct rusage *usage, char *optname);

// General Option Flags
static int verbose_flag = 0;
static int wait_flag = 0;		// Flag to wait for child processes or not
static int test_flag = 0;		// Activates test mode, serializing all commands for verification testing
static int profile_flag = 0;	// Used to activate getrusage	

// File Oflags
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
static int direct_flag = 0;

// File Descriptor Tables
static int maxfd = 20;		// Capacity of logicalfd table
static int fdInd = 0;       // Index of fd table
static int *logicalfd;

// Struct for process arguments
static int maxChild = 10;	// Capacity of child table
static int childInd = 0;	// Index of child table
struct child_proc {			// Structure used to pair pid with arguments
	pid_t pid;
	char *args;
} *child;

// Profiling variables
static struct rusage usage[5];	// Stores data from getrusage

int main(int argc, char **argv)
{
    int profile_succeed = 1;// Becomes 0 if getrusage fails parent profiling
	int exit_status = 0;	// Keeps track of how the program should exit	
	
	int ret; 				// What getopt_long returns
    extern char *optarg;	// Gives the option strings
    extern int optind;		// Gives the current option out of argc options
    int currOptInd = 0;		// Current option index
    int index = 0;			// Index into optarg
    int flags;				// Flags to open file with
	int signum;				// Used to keep signal number
    int status;             // Used with waitpid in --wait and --waitcmd
	char *delim;			// Used in --wait and --waitcmd to find end of arg string				

    int exit_max = 0;		// Sum of child process exit statuses to use when wait_flag set
    int args_found = 0;		// Used to verify --option num of arguments requirement
    int option_index = 0;	// Used with getopt_long
    
    extern int opterr;		// Declared in getopt_long
    opterr = 0;				// Turns off automatic error message from getopt_long
    
    logicalfd = (int*) malloc (maxfd*sizeof(int)); // fd table. Key is logical fd. Value is real fd. 
	child = (struct child_proc*) malloc (maxChild*sizeof(struct child_proc));  // keep track of child and the position of its arguments
    
    static struct option long_options[] =
    {
        /* Flag setting options */
        {"verbose",   no_argument, &verbose_flag,   1},
        {"brief",     no_argument, &verbose_flag,   0},
        {"test",      no_argument, &test_flag,      1},
		{"profile",   no_argument, &profile_flag,   1},
        
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
		{"direct",    no_argument, &direct_flag,    -1},
        
        /* Non flag setting options */
        {"rdonly",  required_argument, 0, 'r' },
        {"wronly",  required_argument, 0, 'w' },
        {"rdwr",    required_argument, 0, 'b' },
        {"pipe",    no_argument,       0, 'z' },
        {"command", required_argument, 0, 'c' },
        {"pause",   no_argument,       0, 'p' },
        {"abort",   no_argument,       0, 'a' },
        {"catch",   required_argument, 0, 's' },
        {"ignore",  required_argument, 0, 'i' },
        {"default", required_argument, 0, 'd' },
        {"close",   required_argument, 0, 'x' },
		{"wait", 	no_argument,       0, 't' },
        {"waitcmd", required_argument, 0, 'm' },
        {0, 0, 0, 0}
    };
    
    if (!logicalfd || !child)		// Make sure logicalfd was allocated correctly
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
                test_flag = 1;
        }
        
        while (1) 	// Loop until getop_long doesn't find anything (then break)
        {			
			// BEGIN profiling
			if (profile_flag)
			{
				if (getrusage(RUSAGE_SELF, &usage[OPT_START]) < 0)	// Start profiling (parent)
				{
					profile_succeed = 0;
					fprintf (stderr, "Error: --profile failed. %s\n", strerror(errno));
					exit_status = 1;
				}	
			}	
			
			ret = getopt_long(argc, argv, "", long_options, &option_index);
            currOptInd = optind;
            index = 0;
            
            if (ret == -1) // No more options found, then break out of while loop
                break;
            
            switch (ret)
            {
                case 0: // Occurs for options that set flags
                    if (verbose_flag)
                    {
                        const char *option_name = long_options[option_index].name;
                        // If not option is not --verbose or --test, then print it out
                        if (strcmp(option_name, "verbose") != 0 && strcmp(option_name, "test") != 0)
                        {
							printf ("--%s ", option_name);
                            if (currOptInd == argc || strcmp(option_name, "profile") == 0)	// If last argument or --profile, add a newline
                                printf ("\n");
                        }
                    }
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
                    
                    // Open file and save its file descriptor
                    if (fdInd >= maxfd)	// If not enough space in fd table...
                    {
                        maxfd *= 2;
                        logicalfd = (int*) realloc (logicalfd, maxfd*sizeof(int)); // Double the array size
                    }
                    
                    if (args_found != 1)	// Error if num of args not 1
                    {
                        fprintf (stderr, "Error: \"--rdonly\" requires one argument.  You supplied %d arguments.\n", args_found);
                        exit_status = 1;
						if (args_found == 0)	// If no args found, decrement optind so we don't skip the next option
							optind--;
                        args_found = 0;
                        logicalfd[fdInd] = -1;
                        fdInd++;
                        break;
                    }
                    
                    // Figure out flags
                    flags = findoflags(O_RDONLY);
                    
                    logicalfd[fdInd] = openfile(optarg, flags);	// Open file with appropriate flags
                    if (logicalfd[fdInd] < 0)	// If something went wrong with open file operation...
                        exit_status = 1;
						
                    clearoflags();	// Clear oflag
                    fdInd++;
                    args_found = 0;		// Reset args found for next option
					
					// END profiling
					if (profile_flag)
					{
						if (getrusage(RUSAGE_SELF, &usage[OPT_END]) < 0)	// Stop profiling (parent)
						{
							profile_succeed = 0;
							fprintf (stderr, "Error: --profile failed. %s\n", strerror(errno));
							exit_status = 1;
						}
						if (profile_succeed)		// Only print usage if getrusage succeeded
							profile_print(OPT_START, OPT_END, usage, "rdonly");
					}
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
                    
                    // Open file and save its file descriptor
                    if (fdInd >= maxfd)	// If not enough space in fd table...
                    {
                        maxfd *= 2;
                        logicalfd = (int*) realloc (logicalfd, maxfd*sizeof(int)); // Double the array size
                    }
                    
                    if (args_found != 1)	// Error if num of args not 1
                    {
                        fprintf (stderr, "Error: \"--wronly\" requires one argument.  You supplied %d arguments.\n", args_found);
                        exit_status = 1;
                        if (args_found == 0)	// If no args found, decrement optind so we don't skip the next option
							optind--;
						args_found = 0;
                        logicalfd[fdInd] = -1;
                        fdInd++;
                        break;
                    }
                    
                    // Figure out flags
                    flags = findoflags(O_WRONLY);
                    
                    logicalfd[fdInd] = openfile(optarg, flags);	// Open file with appropriate flags
                    if (logicalfd[fdInd] < 0)	// If something went wrong with open file operation...
                        exit_status = 1;
						
                    clearoflags();	// Clear oflag
                    fdInd++;
                    args_found = 0;		// Reset args found for next option
					
					// END profiling
					if (profile_flag)
					{
						if (getrusage(RUSAGE_SELF, &usage[OPT_END]) < 0)	// Stop profiling (parent)
						{
							profile_succeed = 0;
							fprintf (stderr, "Error: --profile failed. %s\n", strerror(errno));
							exit_status = 1;
						}
						if (profile_succeed)		// Only print usage if getrusage succeeded
							profile_print(OPT_START, OPT_END, usage, "wronly");
					}
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
                    
                    // Open file and save its file descriptor
                    if (fdInd >= maxfd)	// If not enough space in fd table...
                    {
                        maxfd *= 2;
                        logicalfd = (int*) realloc (logicalfd, maxfd*sizeof(int)); // Double the array size
                    }
                    
                    if (args_found != 1)	// Error if num of args not 1
                    {
                        fprintf (stderr, "Error: \"--rdwr\" requires one argument.  You supplied %d arguments.\n", args_found);
                        exit_status = 1;
                        if (args_found == 0)	// If no args found, decrement optind so we don't skip the next option
							optind--;
						args_found = 0;
                        logicalfd[fdInd] = -1;
                        fdInd++;
                        break;
                    }

                    // Figure out flags
                    flags = findoflags(O_RDWR);
                    
                    logicalfd[fdInd] = openfile(optarg, flags);	// Open file with appropriate flags
                    if (logicalfd[fdInd] < 0)	// If something went wrong with open file operation...
                        exit_status = 1;
						
                    clearoflags();	// Clear oflag
                    fdInd++;
                    args_found = 0;		// Reset args found for next option
					
					// END profiling
					if (profile_flag)
					{
						if (getrusage(RUSAGE_SELF, &usage[OPT_END]) < 0)	// Stop profiling (parent)
						{
							profile_succeed = 0;
							fprintf (stderr, "Error: --profile failed. %s\n", strerror(errno));
							exit_status = 1;
						}
						if (profile_succeed)		// Only print usage if getrusage succeeded
							profile_print(OPT_START, OPT_END, usage, "rdwr");
					}
                    break;
                case 'x':   // close
                    if (verbose_flag)
                        printf ("--close ");
										
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
                        fprintf (stderr, "Error: \"--close\" requires one argument.  You supplied %d arguments.\n", args_found);
                        exit_status = 1;
						if (args_found == 0)	// If no args found, decrement optind so we don't skip the next option
							optind--;
                        args_found = 0;
                        break;
                    }
					
					int fd_to_close = atoi(optarg);
                    // Check if valid fd
					if (fd_to_close >= 0 && logicalfd[fd_to_close] >= 0)
					{
						close(logicalfd[fd_to_close]);
						logicalfd[fd_to_close]= -1;
					}
					else
						fprintf (stderr, "Error: file descriptor %d to close is not open or not valid.\n", fd_to_close);
					
					args_found = 0;		// Reset args found for next option
					
					// END profiling
					if (profile_flag)
					{
						if (getrusage(RUSAGE_SELF, &usage[OPT_END]) < 0)	// Stop profiling (parent)
						{
							profile_succeed = 0;
							fprintf (stderr, "Error: --profile failed. %s\n", strerror(errno));
							exit_status = 1;
						}
						if (profile_succeed)		// Only print usage if getrusage succeeded
							profile_print(OPT_START, OPT_END, usage, "close");
					}
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
					int childArgsSet = 0;	// Blocks writing of child args after written once
					
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
						{
							if (childArgsSet == 0)	// Only set child args one time (needed for --wait output later)
							{
								child[childInd].args = optarg + index;
								childArgsSet = 1;
							}
							execArgv[execArgc++] = optarg+index;
						}
                        
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
					
                    int exec_ret = executecmd(execArgv[0], streams, execArgv);
                    
                    if (exec_ret < 0) 	// Error occurred with executecmd
                        exit_status = 1;
                    else				// Find max exit status among processes
                        if (exec_ret > exit_max)
                            exit_max = exec_ret;
					
					args_found = 0;		// Reset args found for next option
					
					// END profiling
					if (profile_flag)
					{
						if (getrusage(RUSAGE_SELF, &usage[OPT_END]) < 0)	// Stop profiling (parent)
						{
							profile_succeed = 0;
							fprintf (stderr, "Error: --profile failed. %s\n", strerror(errno));
							exit_status = 1;
						}
						if (profile_succeed)		// Only print usage if getrusage succeeded
							profile_print(OPT_START, OPT_END, usage, child[childInd-1].args);
					}
                    break;
                }
                case 'p':	// pause
                    pause();	// Wait for a signal to arrive
                    break;
                    
                case 'z': // pipe
                {
					if(verbose_flag)
                        printf("--pipe\n");
                    
                    // filedescriptors for pipe
                    int pipefds[2];
                    //int error = pipe(pipefds);
					
					flags = findoflags(PIPEFLAGS);
					int error = pipe2(pipefds, flags);
					clearoflags();	// Clear oflag

                    if ((fdInd + 1) >= maxfd)	// If not enough space in fd table...
                    {
                        maxfd *= 2;
                        logicalfd = (int*) realloc (logicalfd, maxfd*sizeof(int)); // Double the array size
                    }

                    if (error == -1) {
                        fprintf(stderr, "Error: \"--pipe\". Error creating pipe");
                        // read end
                        logicalfd[fdInd]   = -1;
                        // write end
                        logicalfd[++fdInd] = -1;
                        fdInd++;
						exit_status = 1;
                        break;
                    }
                    // else, transfer pipe file descriptors to logicalfd[]
                    logicalfd[fdInd] = pipefds[0];
                    logicalfd[++fdInd] = pipefds[1];
                    fdInd++;

					// END profiling
					if (profile_flag)
					{
						if (getrusage(RUSAGE_SELF, &usage[OPT_END]) < 0)	// Stop profiling (parent)
						{
							profile_succeed = 0;
							fprintf (stderr, "Error: --profile failed. %s\n", strerror(errno));
							exit_status = 1;
						}
						if (profile_succeed)		// Only print usage if getrusage succeeded
							profile_print(OPT_START, OPT_END, usage, "pipe");
					}
                    break;
                }
                case 'a':	// abort
                {
					if (verbose_flag)
						printf("--abort\n");
					
					if (raise(SIGSEGV) != 0)	// Cause segmentation fault
					{
						fprintf (stderr, "Error: unable to abort\n");
						exit_status = 1;
					}
					
					// END profiling
					if (profile_flag)
					{
						if (getrusage(RUSAGE_SELF, &usage[OPT_END]) < 0)	// Stop profiling (parent)
						{
							profile_succeed = 0;
							fprintf (stderr, "Error: --profile failed. %s\n", strerror(errno));
							exit_status = 1;
						}
						if (profile_succeed)		// Only print usage if getrusage succeeded
							profile_print(OPT_START, OPT_END, usage, "abort");
					}
                    break;
                }
                case 's':	// catch (signal)
					if (verbose_flag)
                        printf ("--catch ");
                    
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
                        fprintf (stderr, "Error: \"--catch\" requires one argument.  You supplied %d arguments.\n", args_found);
                        exit_status = 1;
						if (args_found == 0)	// If no args found, decrement optind so we don't skip the next option
							optind--;
                        args_found = 0;
                        break;
                    }
					
                    signum = atoi(optarg);
					if (signum < 1 || signum > 64)
					{
						fprintf(stderr, "Error: invalid signal number %d\n", signum);
						exit_status = 1;
					}
					else
					{
						if (signal(signum, &catch_signal) == SIG_ERR)	// Catch signal
						{
							fprintf (stderr, "Error: --catch failed. %s\n", strerror(errno));
							exit_status = 1;
						}
					}
                    
                    args_found = 0;		// Reset args found for next option
					
					// END profiling
					if (profile_flag)
					{
						if (getrusage(RUSAGE_SELF, &usage[OPT_END]) < 0)	// Stop profiling (parent)
						{
							profile_succeed = 0;
							fprintf (stderr, "Error: --profile failed. %s\n", strerror(errno));
							exit_status = 1;
						}
						if (profile_succeed)		// Only print usage if getrusage succeeded
							profile_print(OPT_START, OPT_END, usage, "catch");
					}
                    break;
                case 'i':	// ignore (signal)
					if (verbose_flag)
                        printf ("--ignore ");
                    
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
                        fprintf (stderr, "Error: \"--ignore\" requires one argument.  You supplied %d arguments.\n", args_found);
                        exit_status = 1;
						if (args_found == 0)	// If no args found, decrement optind so we don't skip the next option
							optind--;
                        args_found = 0;
                        break;
                    }
                    
                    signum = atoi(optarg);
					if (signum < 1 || signum > 64)
					{
						fprintf(stderr, "Error: invalid signal number %d\n", signum);
						exit_status = 1;
					}
					else
					{
						// Reassign signal handler to ignore handler
						if (signal(signum, &ignore_signal) == SIG_ERR)
						{
							fprintf (stderr, "Error: --ignore failed. %s\n", strerror(errno));
							exit_status = 1;
						}
					}
                    
                    args_found = 0;		// Reset args found for next option
					
					// END profiling
					if (profile_flag)
					{
						if (getrusage(RUSAGE_SELF, &usage[OPT_END]) < 0)	// Stop profiling (parent)
						{
							profile_succeed = 0;
							fprintf (stderr, "Error: --profile failed. %s\n", strerror(errno));
							exit_status = 1;
						}
						if (profile_succeed)		// Only print usage if getrusage succeeded
							profile_print(OPT_START, OPT_END, usage, "ignore");
					}
                    break;
                case 'd':	// default (signal)
					if (verbose_flag)
                        printf ("--default ");
                    
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
                        fprintf (stderr, "Error: \"--default\" requires one argument.  You supplied %d arguments.\n", args_found);
                        exit_status = 1;
						if (args_found == 0)	// If no args found, decrement optind so we don't skip the next option
							optind--;
                        args_found = 0;
                        break;
                    }
                    
					signum = atoi(optarg);
					if (signum < 1 || signum > 64)
					{
						fprintf(stderr, "Error: invalid signal number %d\n", signum);
						exit_status = 1;
					}
					else
					{
						if (signal(signum, SIG_DFL) == SIG_ERR)	// Set to default signal handler
						{
							fprintf (stderr, "Error: --default failed. %s\n", strerror(errno));
							exit_status = 1;
						}
					}
                    
                    args_found = 0;		// Reset args found for next option
					
					// END profiling
					if (profile_flag)
					{
						if (getrusage(RUSAGE_SELF, &usage[OPT_END]) < 0)	// Stop profiling (parent)
						{
							profile_succeed = 0;
							fprintf (stderr, "Error: --profile failed. %s\n", strerror(errno));
							exit_status = 1;
						}
						if (profile_succeed)		// Only print usage if getrusage succeeded
							profile_print(OPT_START, OPT_END, usage, "default");
					}
                    break;
				case 'm':   // waitcmd
                {
                    if (verbose_flag)
                        printf ("--waitcmd ");
                    
                    int status;
                    int childnum;
                    
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
                        fprintf (stderr, "Error: \"--waitcmd\" requires one argument.  You supplied %d arguments.\n", args_found);
                        exit_status = 1;
						if (args_found == 0)	// If no args found, decrement optind so we don't skip the next option
							optind--;
                        args_found = 0;
                        break;
                    }
                    
                    childnum = atoi(optarg);   // Get requested argument
                    if (childnum < childInd)
					{
						waitpid(child[childnum].pid, &status, 0);
						if (WIFEXITED(status))
						{
							printf ("%d ", WEXITSTATUS(status));
							if (WEXITSTATUS(status) > exit_max) // Mask LSB (8 bits) for status
								exit_max = WEXITSTATUS(status);
							wait_flag = 1;	// Only set wait_flag if we have successfully gathered the child's exit status
						}
						else
							exit_status = 1;
						
						index = 0;
						while (!(child[childnum].args[index] == '-' && child[childnum].args[index+1] == '-'))
						{
							printf ("%s ", child[childnum].args+index);
							delim = strchr(child[childnum].args+index, '\0');	// Look for null byte
							index = delim - child[childnum].args + 1;
						}
						printf ("\n");
					}
					else
					{
						fprintf(stderr, "Error: --waitcmd invalid command number \'%d\'.\n", childnum);
						exit_status = 1;
					}
					
                    args_found = 0;
                    break;
                }
                case 't': 	// wait
				{
					if (verbose_flag)
                        printf ("--wait\n");
					
					pid_t waited_pid;		// Stores pid of child reaped
					int i; 
					index = 0;
					
					while (1)	// Keeping reaping until no more children
					{
						waited_pid = waitpid(-1, &status, 0);
						if (waited_pid < 0)	  // No more children to wait on
							break;
						
						if (WIFEXITED(status))	// Check if child exited normally
						{
							printf ("%d ", WEXITSTATUS(status));
							if (WEXITSTATUS(status) > exit_max) // Mask LSB (8 bits) for status
								exit_max = WEXITSTATUS(status);
							wait_flag = 1;	// Only set wait_flag if we have successfully gathered the child's exit status
						}
						else
							exit_status = 1;
						
						// Loop through all children and find matching pid
						for (i = 0; i < childInd; i++)
						{
							if (child[i].pid == waited_pid)
							{
								// Print out all the args of that command
								while (!(child[i].args[index] == '-' && child[i].args[index+1] == '-'))
								{
									printf ("%s ", child[i].args+index);
									delim = strchr(child[i].args+index, '\0');	// Look for null byte
									index = delim - child[i].args + 1;
								}
								index = 0;
								printf ("\n");
								break;
							}
						}
					}
					
					if (profile_flag)
					{
						if (getrusage(RUSAGE_CHILDREN, &usage[CHI_SUM]) == 0)	// Get all children usage data
						{
							// Print data for children sum
							profile_print(-1, CHI_SUM, usage, "total CHILD usage");
						}
						else
						{
							fprintf (stderr, "Error: --profile failed. %s\n", strerror(errno));
							exit_status = 1;
						}
					}
					break;
				}
                case '?':
                    fprintf (stderr, "Error: unrecognized option \"%s\"\n", argv[optind]);
                    exit_status = 1;
                    break;
                default:
                    fprintf (stderr, "Error: unrecognized option \"%s\"\n", argv[optind]);
                    exit_status = 1;
            }
        }
    }
	if (profile_flag)
	{
		profile_print(-1, PAR_SUM, usage, "total PARENT usage");	// Print data for parent sum
		profile_print(-1, USAGE_TOTAL, usage, "total PARENT + CHILD usage");	// Print sum of parent and child
	}
	
    free (logicalfd);
	free (child);
	
    if (wait_flag || test_flag)
        exit(exit_max);
    else
        exit(exit_status);
}

int findoflags(int open_flag)	// open_flag will be 0 to signify pipe
{
	int total;
    if (open_flag != 0)	// if not for pipe
		total = (append_flag & O_APPEND)     | (cloexec_flag & O_CLOEXEC)     |
				(creat_flag & O_CREAT)       | (directory_flag & O_DIRECTORY) |
				(dsync_flag & O_DSYNC)       | (excl_flag & O_EXCL)           |
				(nofollow_flag & O_NOFOLLOW) | (nonblock_flag & O_NONBLOCK)   |
				(rsync_flag & O_RSYNC)       | (sync_flag & O_SYNC)           |
				(trunc_flag & O_TRUNC)       | (open_flag);
	else	// if for pipe
		total = (cloexec_flag & O_CLOEXEC)   | (direct_flag & O_DIRECT) | 
				(nonblock_flag & O_NONBLOCK);
    
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
    direct_flag = 0;
}

void catch_signal(int sig)
{
	char catch_str[20];
	if (sig > 0 && sig < 10)
	{
		catch_str[0] = (char) (sig + '0');
		catch_str[1] = '\0';
	}
	else if (sig >= 10 && sig <= 64)
	{
		catch_str[0] = (char) ((sig / 10) + '0');
		catch_str[1] = (char) ((sig % 10) + '0');
		catch_str[2] = '\0';
	}
	strcat(catch_str, " caught\n");
	write(2, catch_str, strlen(catch_str));
    //fprintf (stderr, "%d caught\n", sig);		// Prof says we cannot use printf in sighandlers
    exit(sig);
}

void ignore_signal(int sig)
{
	// Do nothing (necessary to unpause when --pause is used)
	if (sig)
	{;}		// temp to avoid warning of unused parameter from compiler
	
    //fprintf (stderr, "%d ignored\n", sig);
}

int executecmd(const char *file, int streams[], char *const argv[])
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
            fprintf (stderr, "Error: failed to redirect stdin\n");
            return -1;
        }
        if (dup2(streams[1], 1) == -1)
        {
            fprintf (stderr, "Error: failed to redirect stdout\n");
            return -1;
        }
        if (dup2(streams[2], 2) == -1)
        {
            // Don't try to write to stderr because could cause infinite loop
            return -1;
        }
		
		// Close all fds in case there are later commands dependent on pipes
		int i;
		for (i = 0; i < fdInd; i++)
			close(logicalfd[i]);
		
        // Execute command
        if (execvp(file, argv) < 0)	// If return value < 0, error occurred
        {
            fprintf (stderr, "Error: %s\n", strerror(errno));
            return -1;
        }
    }
    else // Parent
    {
        child[childInd].pid = pid;
		childInd++;
		
		if (childInd >= maxChild)	// If not enough space in child array...
		{
			maxChild *= 2;
			child = (struct child_proc*) realloc (child, maxChild*sizeof(struct child_proc));  // Double the array size
		}
		
		// Serialize commands by waiting for each individually (--test mode)
        if (test_flag)
        {
            if (waitpid(pid, &status, 0) < 0)  // If error occurred...
            {
                fprintf (stderr, "Error: %s\n", strerror(errno));
                return -1;
            }
            else
            {
				if (WIFEXITED(status))	// Check if child exited normally
					return WEXITSTATUS(status); // Mask LSB (8 bits) for status
				else
					return -1;
            }
        }
        else    // Otherwise just return
            return 0;
    }
    return -1;  // Should never get here
}

void profile_print(int start, int end, struct rusage *usage, char *optname)
{
	double user_time;		// Holds user CPU time spent
	double kernel_time;		// Holds kernel CPU time spent
	long max_res_set_size;	// Maximum resident set size
	long page_reclaims;		// Page reclaims (soft page faults)
	long page_faults;		// Page faults (hard page faults)
	long block_in_ops;		// Block input operations
	long block_out_ops;		// Block output operations
	long vol_context_switch;	// Voluntary context switches
	long invol_context_switch;	// Involuntary context switches

	if (start != -1)	// Parent usage
	{
		// Add seconds
		user_time = usage[end].ru_utime.tv_sec - usage[start].ru_utime.tv_sec;
		kernel_time = usage[end].ru_stime.tv_sec - usage[start].ru_stime.tv_sec;
		// Add microseconds
		user_time += (usage[end].ru_utime.tv_usec - usage[start].ru_utime.tv_usec)/1000000.0;
		kernel_time += (usage[end].ru_stime.tv_usec - usage[start].ru_stime.tv_usec)/1000000.0;
		// Other resources
		max_res_set_size = usage[end].ru_maxrss - usage[start].ru_maxrss;
		page_reclaims = usage[end].ru_minflt - usage[start].ru_minflt;
		page_faults = usage[end].ru_majflt - usage[start].ru_majflt;
		block_in_ops = usage[end].ru_inblock - usage[start].ru_inblock;
		block_out_ops = usage[end].ru_oublock - usage[start].ru_oublock;
		vol_context_switch = usage[end].ru_nvcsw - usage[start].ru_nvcsw;
		invol_context_switch = usage[end].ru_nivcsw - usage[start].ru_nivcsw;
		
		// Keep track of total parent usage
		usage[PAR_SUM].ru_utime.tv_sec += usage[end].ru_utime.tv_sec - usage[start].ru_utime.tv_sec;
		usage[PAR_SUM].ru_stime.tv_sec += usage[end].ru_stime.tv_sec - usage[start].ru_stime.tv_sec;
		usage[PAR_SUM].ru_utime.tv_usec += usage[end].ru_utime.tv_usec - usage[start].ru_utime.tv_usec;
		usage[PAR_SUM].ru_stime.tv_usec += usage[end].ru_stime.tv_usec - usage[start].ru_stime.tv_usec;
		if (max_res_set_size > usage[PAR_SUM].ru_maxrss)	// Check if new maximum set size
			usage[PAR_SUM].ru_maxrss = max_res_set_size;
		usage[PAR_SUM].ru_minflt += page_reclaims;
		usage[PAR_SUM].ru_majflt += page_faults;
		usage[PAR_SUM].ru_inblock += block_in_ops;
		usage[PAR_SUM].ru_oublock += block_out_ops;
		usage[PAR_SUM].ru_nvcsw += vol_context_switch;
		usage[PAR_SUM].ru_nivcsw += invol_context_switch;
	} 
	else	// Children or Parent summed usage
	{
		// Add seconds
		user_time = usage[end].ru_utime.tv_sec;
		kernel_time = usage[end].ru_stime.tv_sec;
		// Add microseconds
		user_time += usage[end].ru_utime.tv_usec/1000000.0;
		kernel_time += usage[end].ru_stime.tv_usec/1000000.0;
		// Other resources
		max_res_set_size = usage[end].ru_maxrss;
		page_reclaims = usage[end].ru_minflt;
		page_faults = usage[end].ru_majflt;
		block_in_ops = usage[end].ru_inblock;
		block_out_ops = usage[end].ru_oublock;
		vol_context_switch = usage[end].ru_nvcsw;
		invol_context_switch = usage[end].ru_nivcsw;
		
		if (end != USAGE_TOTAL)	// Don't add USAGE_TOTAL to USAGE_TOTAL
		{
			// Keep track of total parent + child usage
			usage[USAGE_TOTAL].ru_utime.tv_sec += usage[end].ru_utime.tv_sec;
			usage[USAGE_TOTAL].ru_stime.tv_sec += usage[end].ru_stime.tv_sec;
			usage[USAGE_TOTAL].ru_utime.tv_usec += usage[end].ru_utime.tv_usec;
			usage[USAGE_TOTAL].ru_stime.tv_usec += usage[end].ru_stime.tv_usec;
			if (max_res_set_size > usage[USAGE_TOTAL].ru_maxrss)	// Check if new maximum set size
				usage[USAGE_TOTAL].ru_maxrss = max_res_set_size;
			usage[USAGE_TOTAL].ru_minflt += page_reclaims;
			usage[USAGE_TOTAL].ru_majflt += page_faults;
			usage[USAGE_TOTAL].ru_inblock += block_in_ops;
			usage[USAGE_TOTAL].ru_oublock += block_out_ops;
			usage[USAGE_TOTAL].ru_nvcsw += vol_context_switch;
			usage[USAGE_TOTAL].ru_nivcsw += invol_context_switch;
		}
	}
	
	printf ("Profiling %s...\n", optname);
	printf ("  User CPU Time:                %.6fs\n", user_time);
	printf ("  System CPU Time:              %.6fs\n", kernel_time);
	printf ("  Max resident set size:        %ldkB\n", max_res_set_size);
	printf ("  Page reclaims:                %ld\n", page_reclaims);
	printf ("  Page faults:                  %ld\n", page_faults);
	printf ("  Block input Ops:              %ld\n", block_in_ops);
	printf ("  Block output Ops:             %ld\n", block_out_ops);
	printf ("  Voluntary context switches:   %ld\n", vol_context_switch);
	printf ("  Involuntary context switches: %ld\n", invol_context_switch);
}