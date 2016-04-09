#include "common.h"

int main()
{
	msgbuf  msg;
	key_t key;
	int msgid;
	int i;
	char str_in[50];
	// Create Key
	if( (key = ftok(MSG_FILE,66)) == -1 )
	{
		fprintf(stderr, "Create Key Error : %s \n",strerror(errno) );
		exit(EXIT_FAILURE);
	}
	// Create Message Queue
	if((msgid = msgget(key,PERM|IPC_CREAT)) == -1)
	{
		fprintf(stderr, "Create Message Queue Error %s\n",strerror(errno) );
		exit(EXIT_FAILURE);
	}	

	//init msg
	msg.mtype = 1;

	while(1)
	{
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
		printf("Receiver received: %s\n",msg.mtext);
	}

	exit(EXIT_SUCCESS);
}