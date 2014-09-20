/* 
 * msh - A mini shell program with job control
 * 
 * <Put your name and login ID here>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "util.h"
#include "jobs.h"


/* Global variables */
int verbose = 0;            /* if true, print additional output */

extern char **environ;      /* defined in libc */
static char prompt[] = "msh> ";    /* command line prompt (DO NOT CHANGE) */
static struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
void usage(void);
void sigquit_handler(int sig);



/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the comma/jobsnd line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline)
{
    char *argv[MAXARGS];        /* Argument list execve() */
    char buf[MAXLINE];          /* Holds modified command line */
    int bg;                     /* Should the job run in bg or fg? */
    pid_t pid;                  /* Process id */
    struct job_t *newJob;

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if (argv[0] == NULL) return;   /* Ignore empty lines */

    if (!builtin_cmd(argv))
    {	
				if ((pid = fork()) == 0)       /* Child runs user job */
     		{   
						setpgid(pid, pid);				//Establish process group ID
						if (execv(argv[0],argv) < 0)
      			{  	printf("%s: Command not found.\n", argv[0]);
            		exit(0);
      			}
     		}

		
     		if (!bg) // Foreground Process - Parent Waits
     		{		
		 			addjob(jobs, pid, 1, cmdline);							// Parent adds process to job
        	waitfg(pid);																// Parent waits for Child
					newJob = getjobpid(jobs,pid);								// newJob = Child
        
					if (newJob != NULL  && newJob->state != 3)  // if (Child !NULL and !Stopped)
					{
						kill(pid,SIGKILL);												// kill child
						deletejob(jobs,pid);											// delete child from joblist
					}
				}else // Background Process
				{
					addjob(jobs,pid,2,cmdline);
					newJob = getjobpid(jobs,pid);								// newJob = Child
					printf("[%d] (%d) %s", (*getjobpid(jobs, pid)).jid, pid, cmdline);					 
				}
     }

    return;
}




/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 * Return 1 if a builtin command was executed; return 0
 * if the argument passed in is *not* a builtin command.
 */
int builtin_cmd(char **argv)
{
		listjobs(jobs);
    if (!strcmp(argv[0], "quit"))		 // QUIT MSH
    {
		    exit(0);
		}
		if (!strcmp(argv[0], "jobs"))		// Call jobs - list current jobs
		{
				listjobs(jobs);
				return 1;
		}
		if (!strcmp(argv[0], "bg")){  //Determines bg fg
				printf("hit here\n");
				do_bgfg(argv);
				return 1;}
		if (!strcmp(argv[0], "fg")){
				printf("hit\n");
				do_bgfg(argv);
				return 1;
		}
    if (!strcmp(argv[0], "&"))			// This is a recognized command,
		{																//  but ignored as single command
				return 1;	
		}
return 0;		// Not a Built-in Command
}


/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) 
{
		if (!strcmp(argv[0], "bg")){
			
			char* id = argv[1];				// passed 	pid or jid
			printf("i mae it here\n");

			if(id[0] == "%"){					// if jid
				id += 1;
				printf("here's my value: %s\n", id);
				int jid = atoi(id);
				if ((*getjobjid(jobs, jid)).state == 3){
					(*getjobjid(jobs, jid)).state = 2;
					kill(SIGCONT, id);
				}}
			else{
				int pid = atoi(id);
				if ((*getjobpid(jobs, pid)).state == 3){
	  			(*getjobpid(jobs, pid)).state = 2;
		  		kill(SIGCONT, id);
				}}
		}
		/*			
			if (id > 16){
				if ((*getjobpid(jobs, id)).state == 3){
	  			(*getjobpid(jobs, id)).state = 2;
		  		kill(SIGCONT, id);
				}}
			else{
				if((*getjobjid(jobs, id)).state == 3){
				  (*getjobjid(jobs, id)).state = 2;
				  kill(SIGCONT, (*getjobjid(jobs, id)).pid);
				}}
			return; 
		}
		if (!strcmp(argv[0], "fg")){
			int id = atoi(argv[1]);				// passed 	pid or jid
			
			if (id > 16){
			if((*getjobpid(jobs, id)).state != 1){
				(*getjobpid(jobs, id)).state = 1;
				kill(SIGCONT, id);}}
			else{
				if((*getjobjid(jobs, id)).state != 1){
					(*getjobjid(jobs, id)).state = 1;
				kill(SIGCONT, (*getjobjid(jobs, id)).pid);}}
			return; 
		}*/
    if (!strcmp(argv[0], "&"))
        return;
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)
{	int status;
	if(waitpid(-1,&status,0)<0)
		unix_error("waitfg: waitpid error");	
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
      received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{

	pid_t pidkill = fgpid(jobs);

	char str[80];
	sprintf(str, "Job [%d] (%d) terminated by signal 2", (*getjobpid(jobs, pidkill)).jid, pidkill);
	puts(str);
	deletejob(jobs, pidkill);
	if (pidkill != 0){
		kill(getpgid(pidkill), SIGINT);
		}

	//need to kill all child processes
   return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
		pid_t pidstop = fgpid(jobs);
		
		char str[80];
		sprintf(str, "Job[%d] (%d) stopped by signal 20", (*getjobpid(jobs, pidstop)).jid, pidstop);
		puts(str);
		(*getjobpid(jobs, pidstop)).state = 3;
		if (pidstop != 0){
			kill(getpgid(pidstop), SIGTSTP);
		}	

		//need to stop all child processes.
    return;
}

/*********************
 * End signal handlers
 *********************/



/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    ssize_t bytes;
    const int STDOUT = 1;
    bytes = write(STDOUT, "Terminating after receipt of SIGQUIT signal\n", 45);
    if(bytes != 45)
       exit(-999);
    exit(1);
}



