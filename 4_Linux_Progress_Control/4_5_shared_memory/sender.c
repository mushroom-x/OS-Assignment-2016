#include "lib/common/common.h"


//key
key_t key;

//shared memory
int shmid;
char * shmptr;
char input[SHM_SIZE];

int semid;							//semaphore


void Init()
{
	key = get_key();					//init key
	shmid  = get_shmid(key);			// init shared memory
	shmptr = shmat(shmid,NULL,0);		// attach segement to vitural ...?
	// TODO initialize the shared memory : ?
	semid = get_semid(key,SEM_NUM);		//init semaphore ([MUTEX,FULL])
}

void SaveMessage()
{

	P(semid,MUTEX);						
	strcpy(shmptr,input);
	V(semid,MUTEX);

	V(semid,FULL);
}

int main(int argc, char const *argv[])
{
	
	
	Init();
	
	/*waiting for user to input message*/
	scanf("%s",input);					//input message from shell 
										// TODO input a whole line

	SaveMessage();
	
	printf("Sender:  Process End\n");
	return 0;
}