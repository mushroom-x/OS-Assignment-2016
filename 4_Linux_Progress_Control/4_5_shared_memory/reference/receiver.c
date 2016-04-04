#include "shm_commu.h"

char  *sem_name  = "empty";

int main(int argc, char const *argv[])
{

	int shmid;
	key_t key;
	char *shm;
	char buf[SHM_SIZE];

	sem_t * mutex;
	
	// named the shared memory segment
	key = KEY_NUM;

	//crate & initialize existing semaphore 
	mutex = sem_open(sem_name,0,0644,0);
	if(mutex == SEM_FAILURE)
	{
		perror("Receiver : Unable to execute semaphore\n");
		sem_close(mutex);
		exit(-1);
	}

	//crate the shared memory segment with this key
	shimid = shmget(key,SHM_SIZE,0666);
	if(shmid < 0)
	{
		perror("Receiver: Shmget Error");
		exit(-1);
	}

	//attach this segment to virtual memory
	shm = shmat(shmid,NULL,0);

	//start reading

	sem_wait(mutex);
	strcpy(buf,shm);
	sem_post(mutex);

	printf("Receiver : %s\n",buf);
	return 0;
}