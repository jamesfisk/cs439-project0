#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "util.h"


/*
 * First, print out the process ID of this process.
 *
 * Then, set up the signal handler so that ^C causes
 * the program to print "Nice try.\n" and continue looping.
 *
 * Finally, loop forever, printing "Still here\n" once every
 * second.
 */
void signal_callback_handler1(){
		ssize_t bytes;
		const int STDOUT = 1;
		bytes = write(STDOUT, "Nice try.\n", 10);
		if(bytes != 10)
			exit(-999);
	}

void signal_callback_handler2(){
		ssize_t bytes;
		const int STDOUT = 1;
		bytes = write(STDOUT, "exiting\n", 10);
		if(bytes != 10)
			exit(1);
}

int main(int argc, char **argv)
{
	Signal(SIGINT, signal_callback_handler1);
	Signal(SIGUSR1, signal_callback_handler2);

	printf("My pid is %ld\n", (long)getpid());


	struct timespec sleepValue;
	sleepValue.tv_nsec = 0;
	sleepValue.tv_sec = 1;

  struct timespec remaining;
	remaining.tv_nsec = 0;
	remaining.tv_sec = 0;

	//print "still here" once every second
	while(1){
		if(nanosleep(&sleepValue, &remaining) == 0){
			printf("Still here\n");
		}
		//if one second interval was interrupted, sleep for remaining interval
		else{
			nanosleep(&remaining, &remaining);
			printf("Still here\n");
			
		}
	}
  return 0;
}


