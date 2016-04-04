#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>


int main(void)
{
	key_t key;
	int semid;
	struct sembuf sb = {0, -1, 0}; 	/*set to allocate resource */

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

	/* Lock the semaphore */
	printf("Press return to lock :  ");
	getchar();
	printf("Trying to lock ...\n");

	if(semop(semid,&sb,1) == -1)
	{
		perror("semop");
		exit(EXIT_FAILURE);
	}

	printf("Locked.\n");

	/* Unlock the semaphore */
	printf("Press return to unlock:" );
	getchar();

	sb.sem_op = 1;		/* free resource */
	if(semop(semid,&sb,1) == -1)
	{
		perror("semop");
		exit(EXIT_FAILURE);
	}
	printf("Unlocked\n");
	return 0;
}