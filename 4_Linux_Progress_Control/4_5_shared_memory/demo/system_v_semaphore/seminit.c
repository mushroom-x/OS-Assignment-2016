#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int main(int argc, char const *argv[])
{
	key_t key;
	int semid;		//semaphore id
	union semun {
		int 	val;
		//...
	}arg;	// semaphore union

	/* Create key*/
	if( (key = ftok("semdemo.c",'J')) == -1)
	{
		perror("Error : Ftok");
		exit (EXIT_FAILURE);
	}

	/* Create a semaphore set with 1 semaphore : */
	if( (semid = semget(key,1,0666 | IPC_CREAT))  == -1)
	{
		perror("Error : semget");
		exit(EXIT_FAILURE);
	}

	/* initialize semaphore #0 to 1: */
	arg.val  = 1;
	if (semctl(semid,0,SETVAL,arg) == -1)	//semphore contrl function : SETVAL
	{
		perror("semctl");
		exit(EXIT_FAILURE);
	}

	return 0;
}