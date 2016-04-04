#include "shm_commu.h"

char  *sem_name  = "empty";

int main()
{
	int shmid;
	key_t key;
	char *shm;
	char input[SHM_SIZE];
	sem_t  *mutex;
	key = KEY_NUM;	// name the shared memory segment

	//create & initialize semophore
				// 0644 ?  rw-r--r--
	mutex = sem_open(sem_name,O_CREAT,0644,1);
	if( mutex == SEM_FAILED)
	{
		perror("unable to create semophore");
		sem_unlink(sem_name);
		exit(-1);
	}

	//create the shared memory segment with this key
	shmid = shmget(key,SHM_SIZE,IPC_CREAT|0666);
	if(shmid < 0)
	{
		perror("failure in shmget");
		exit(-1);
	}

	shm  = shmat(shmid,NULL,0);

	//input  message from shell
	scanf("%s",input);

	//send message
	sem_wait(mutex);
	strcpy(shm,input);
	sem_post(mutex);

	//end
	sem_close(mutex);
	sem_unlink(sem_name);
	shmctl(shmid,IPC_CREAT,0);
	exit(0);
}