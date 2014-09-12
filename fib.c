#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

const int MAX = 13;

static void doFib(int n, int doPrint);


/*
 * unix_error - unix-style error routine.
 */
inline static void 
unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}


int main(int argc, char **argv)
{
  int arg;
  int print;

  if(argc != 2){
    fprintf(stderr, "Usage: fib <num>\n");
    exit(-1);
  }

  if(argc >= 3){
    print = 1;
  }

  arg = atoi(argv[1]);
  if(arg < 0 || arg > MAX){
    fprintf(stderr, "number must be between 0 and %d\n", MAX);
    exit(-1);
  }

  doFib(arg, 1);

  return 0;
}

/* 
 * Recursively compute the specified number. If print is
 * true, print it. Otherwise, provide it to my parent process.
 *
 * NOTE: The solution must be recursive and it must fork
 * a new child for each call. Each process should call
 * doFib() exactly once.
 */
static void 
doFib(int n, int doPrint)
{
int status;
int status2;

	if (n <= 1){
		if(doPrint){printf("%d\n", 0);}		
		exit(0);}

	else if (n < 3){
		if (doPrint){printf("%d\n", 1);}
		exit(1);}
	else{
		//create two children, decrement n by two in child1 and one in child2
		pid_t pid_child = fork();
		if (pid_child == 0){n = n - 1;
			doPrint = 0;
		}
		
		pid_t pid_child2 = fork();
		if (pid_child2 == 0){n = n - 1;}
		
		//parent code	
		if (pid_child2 != 0){
      		
			wait(&status);
			status = WEXITSTATUS(status);
		    
			wait(&status2);
			status2 = WEXITSTATUS(status2);
			if (doPrint == 1){
			printf("%d\n", status + status2);
			}
			exit(status + status2);
		}
		//child code6
		else{doFib(n, 0);}
	
	}
}


