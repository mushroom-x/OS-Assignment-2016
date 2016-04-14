#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define TRUE 1


#define BUF_SIZE 255
#define PERM S_IRUSR|S_IWUSR
#define KEY_NUM 1000


typedef struct msgbuf msgbuf;

struct msgbuf
{
	long mtype;
	char mtext[BUF_SIZE + 1];
};


//semaphore and mutex
sem_t full;
sem_t empty;
sem_t mutex;

//pthread
pthread_t write_pid;
pthread_t read_pid;


key_t key;
//message
int msgid;

struct msgbuf msg;


void Init()
{
	/// init semaphore
	sem_init(&full,0,0);
	sem_init(&empty,0,1);
	sem_init(&mutex,0,1);

	key = KEY_NUM;
	
	// Create Message Queue
	if((msgid = msgget(key,PERM|IPC_CREAT)) == -1)
	{
		fprintf(stderr, "Create Message Queue Error %s\n",strerror(errno) );
		exit(EXIT_FAILURE);
	}	

	

}


void * ReadProcess(void *arg)
{
	msgbuf msg;
	//init msg
	msg.mtype = 1;
	while(TRUE)
	{

		sem_wait(&full);
		sem_wait(&mutex);

		//receive message from  message queue
		msgrcv(msgid,&msg,sizeof(msgbuf),1,0);

		//detect for end
		if(strcmp(msg.mtext,"end") == 0)
		{
			msg.mtype = 2;
			strncpy(msg.mtext,"over",BUF_SIZE);
			msgsnd(msgid,&msg,sizeof(msgbuf),0);
			break;
		}

		//print message
		printf("RP: [message received] %s\n\n",msg.mtext);
	
		sem_post(&empty);
		sem_post(&mutex);
	}

	exit(EXIT_SUCCESS);
}

void * WriteProcess(void *arg)
{
	
	char input[50];
	msgbuf msg;
	msg.mtype = 1;

	while (TRUE)
	{

		
		//semaphore
		sem_wait(&empty);
		sem_wait(&mutex);

		printf("WP: Please Input the message you want to send.\n");
		scanf("%s",input);

		if(strcmp(input,"exit") == 0)
		{
			strncpy(msg.mtext,"end",BUF_SIZE);
			msgsnd(msgid,&msg,sizeof(msgbuf),0);
			break;
		}
		strncpy(msg.mtext,input,BUF_SIZE);
		msgsnd(msgid,&msg,sizeof(msgbuf),0);
		
		printf("WP: [message sent] %s\n",msg.mtext );

		//semaphore
		sem_post(&full);
		sem_post(&mutex);
	}
	
	
	
	// Clear Node
	memset(&msg,'\0',sizeof(msgbuf));
	// Block ,waiting for msg with type = 2
	msgrcv(msgid,&msg,sizeof(msgbuf),2,0);
	printf("WP: [message sent]%s\n",msg.mtext );
	
	//Remove Message Queue
	if( msgctl (msgid,IPC_RMID,0) == -1)
	{
		fprintf(stderr, "Remove Message Queue Error%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}

int main()
{
	

	Init();

	pthread_create(&write_pid,NULL,WriteProcess,NULL);
	pthread_create(&read_pid,NULL,ReadProcess,NULL);

	//waiting for the thread end
	pthread_join(write_pid,NULL);
	pthread_join(read_pid,NULL);


	printf("Main Function End...\n");

	return 0;
}