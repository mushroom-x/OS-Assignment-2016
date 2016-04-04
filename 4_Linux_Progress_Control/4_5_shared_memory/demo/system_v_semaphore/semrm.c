#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>


int main(int argc, char const *argv[])
{
	key_t key;
	int semid;
	union semun {
		int 	val;
		//...
	}arg;	// semaphore union


	if( (key = ftok("semdemo.c",'J')) == -1)
	{
		perror("ftok");
		exit(EXIT_FAILURE);
	}

	/* grab the semaphore set created by seminit.c*/
	if ((semid = semget(key,1,0)) == -1)
	{
		perror("semget");
		exit(EXIT_FAILURE);
	}

	/* remove it: */
	if( semctl(semid,0,IPC_RMID,arg) == -1)
	{
		perror("semctl");
		exit(1);
	}

	return 0;
}