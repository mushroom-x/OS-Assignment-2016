#include "lib/common/common.h"

int main(int argc, char const *argv[])
{
	key_t key;
	int semid;			//semaphore id
	int shmid;			//shared memory id
	extern union semun arg;	// semaphore union

	/* Create key*/
	key = get_key();

	//create the shared memory segment with this key
	shmid =  get_shmid(key);
	
	/* Create a semaphore set with 2 emaphore : */
	semid = get_semid(key,SEM_NUM);

	/*
	* 	SEM_Name  	INIT_VALUE
	*	MUTEX 		1
	*	FULL 		0
	*/
	/* initialize semaphore*/

	set_sem(semid,MUTEX,1);
	set_sem(semid,FULL,0);
	printf("End of initialize\n");
	return 0;
}