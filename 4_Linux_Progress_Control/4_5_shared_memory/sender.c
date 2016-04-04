#include "lib/common/common.h"

int main(int argc, char const *argv[])
{
	//key
	key_t key;
	
	//shared memory
	int shmid;
	char * shmptr;
	char input[SHM_SIZE];

	//semaphore
	int semid;

	//init key
	key = get_key();
	// init shared memory
	shmid  = get_shmid(key);
	// attach segement to vitural ...?
	shmptr = shmat(shmid,NULL,0);
	memset(shmptr,0,SHM_SIZE);
	//init semaphore ([MUTEX,FULL])
	semid = get_semid(key,SEM_NUM);


	//input message from shell 
	scanf("%s",input);

	//save  message into SHM
	P(semid,MUTEX);
	strcpy(shmptr,input);
	V(semid,MUTEX);

	V(semid,FULL);

	printf("Sender:  Process End\n");
	return 0;
}