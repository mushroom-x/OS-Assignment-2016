#include "common.h"

int main()
{
	msgbuf  msg;
	key_t key;
	int i;
	int msgid;
	char input[50];
	
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
	
	while (1)
	{
		// Read Message from shell
		scanf("%s",input);
		/*
		*  TODO there is a bug.
		*  I don't know how to input a whole line
		*  
		* scanf("%[^\n]",input); 
		*/

		if(strcmp(input,"exit") == 0)
		{
			strncpy(msg.mtext,"end",BUF_SIZE);
			msgsnd(msgid,&msg,sizeof(msgbuf),0);
			break;
		}
		strncpy(msg.mtext,input,BUF_SIZE);
		msgsnd(msgid,&msg,sizeof(msgbuf),0);
		printf("Sender Sent: %s\n",msg.mtext );
	}
	
	
	// Clear Node
	memset(&msg,'\0',sizeof(msgbuf));
	
	// Block ,waiting for msg with type = 2
	msgrcv(msgid,&msg,sizeof(msgbuf),2,0);
	printf("Sender  sent:%s\n",msg.mtext );
	
	//Remove Message Queue
	if( msgctl (msgid,IPC_RMID,0) == -1)
	{
		fprintf(stderr, "Remove Message Queue Error%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
