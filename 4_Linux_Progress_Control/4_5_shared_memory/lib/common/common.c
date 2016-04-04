#include "common.h"

union semun {
           int              val;    /* Value for SETVAL */
           struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
           unsigned short  *array;  /* Array for GETALL, SETALL */
           struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                       (Linux-specific) */
}arg;

/* Create key*/
key_t get_key()
{
	key_t key;
	if( (key = ftok("./tmp",'J')) == -1)
	{
		perror("Error : Ftok");
		exit (EXIT_FAILURE);
	}
	return key;
}

/*
*  create the shared memory segment with this key
*/
int get_shmid(key_t key){
	int shmid = shmget(key,SHM_SIZE,IPC_CREAT|0666);
	
	if(shmid < 0)
	{
		perror("failure in shmget");
		exit(-1);
	}

	return shmid;
}
/*
* Create a semaphore set with sem_num  emaphore
*/
int get_semid(key_t key,int sem_num)
{
	int semid;

	if( (semid = semget(key,sem_num,0666 | IPC_CREAT))  == -1)
	{
		perror("Error : semget");
		exit(EXIT_FAILURE);
	}
	return semid;
}

void set_sem(int semid,int sem_name,int value)
{
	arg.val = value;

	if (semctl(semid,sem_name,SETVAL,arg) == -1)
	{
		perror("semctl");
		exit(EXIT_FAILURE);
	}
}

void rm_sem(int semid,int sem_num)
{
	/* remove it: */
	if( semctl(semid,sem_num,IPC_RMID,arg) == -1)
	{
		perror("semctl");
		exit(1);
	}
}

void   P(int semid,int sem_name)
{
	set_sem(semid,sem_name,-1);
}

void V(int semid,int sem_name)
{
	set_sem(semid,sem_name,1);
}
