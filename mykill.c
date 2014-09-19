#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>


int main(int argc, char **argv)
{
	if (argc < 2){
		printf("Too few arguments");}
	else{
		int killpid = atoi(argv[1]);
		kill(killpid, SIGUSR1);
	}	
  return 0;
}
