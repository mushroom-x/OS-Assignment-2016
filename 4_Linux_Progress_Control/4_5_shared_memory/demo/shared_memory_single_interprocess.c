#include <sys/shm.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 1024

extern int errno;

int main()
{
	int shmid;	//shared memory id
	char *shmptr;  //shared memory pointer

	//create shared memory
	if((shmid = shmget(IPC_PRIVATE,SIZE,0600))<0)
	{
		printf("shmget error: %s \n",strerror(errno) );
		return -1;
	}

	// connect shared memory to the usable address
	if((shmptr = (char *)shmat(shmid,0,0) ) == (void *)-1)
	{
		printf("shmat error: %s\n",strerror(errno) );
		return -1;
	}

	memcpy(shmptr,"hello world",sizeof("hello word"));
	printf("shared memory form %lx to %lx,content: %s\n",
			(unsigned long)shmptr,
			(unsigned long)(shmptr + SIZE),shmptr);

	//delete the shared memory
	if (shmctl(shmid,IPC_RMID,0) < 0)
	{
		printf("shmctl error : %s\n", strerror(errno));
		return -1;
	}
}