#include "lib/common/common.h"


//key
key_t key;

//shared memory
int shmid;
char * shmptr;
char result[SHM_SIZE];

//semaphore
int semid;
	
/*
* INIT 
*/	
void Init()
{
	key = get_key();				//init key
	shmid  = get_shmid(key);		// init shared memory
	shmptr = shmat(shmid,NULL,0);	// attach segement to vitural ...?
	semid = get_semid(key,SEM_NUM);	//init semaphore ([MUTEX,FULL])

}

/*
* Save Message in shared memory
*/
void GetMessage()
{
	P(semid,FULL);
	//save  message into SHM
	P(semid,MUTEX);
	strcpy(result,shmptr);
	V(semid,MUTEX);
}


int main(int argc, char const *argv[])
{


	Init();
	
	GetMessage();

	printf("Receiver : %s \n",result);

	//TODO:   remove shared memory ... //unlink shared memory				
	rm_sem(semid,SEM_NUM);	 //remove semaphore
	
	printf("Receiver :  Process End \n");
	return 0;
}
