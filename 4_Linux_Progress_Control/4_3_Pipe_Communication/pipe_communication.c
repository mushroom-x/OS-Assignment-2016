#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXSIZE 100
#define READ_NUM 50
#define WRITE_NUM 70

char * makeString(int len)
{
	int i;
	char text[MAXSIZE];

	for(i = 0; i<len-1; i++){
		strcat(text,"*");
	}
	strcat(text,"\0");
	return text;
}

void PipeWrite(int fd[2],int len)
{
	int nWirted;
	char text[MAXSIZE];

	if(len <  MAXSIZE)	//safety check
	{
		strcpy(text,getStringFlow);
	}
	else
	{
		printf("PipeWrite Error : len is  too long \n");
		exit(1);
	}

	printf("Write Process :   Wanna input %d characters \n",len);

	close(fd[0]);
	nWirted = write(fd[1],text,len);
	printf("Write Process : Wrote in %d characters ...\n",nWirted);
}

void PipeRead(int fd[2],int len)
{
	char buf[MAXSIZE] = "\0";	//init text
	int nRead = 0;

	printf("Read Process : Wanna read %d characters.\n",len );
	close(fd[1]);
	nRead = read(fd[0],buf,len);
	printf("Read Process : Read %d characters\n", nRead);
}

void WriteProcess(int fd[2])
{

	PipeWrite(fd,WRITE_NUM);

	printf("Write Process: Sleep - 10 Seconds ...\n");
	sleep(10);
	printf("Write Process: Wake up  then Why not rewrite 70 characters  ? \n");

	PipeWrite(fd,WRITE_NUM);
	printf("Write Process : Done\n");

	return;
}

void ReadProcess(int fd[2])
{
	printf("Read Process :  Time 1\n");
	PipeRead(fd,READ_NUM);

	printf("Read Process :  Time 2\n");
	PipeRead(fd,READ_NUM);

	printf("Read Process :  Time 3\n");
	PipeRead(fd,READ_NUM + 20);

	return;
}


int main()
{
	int fd[2];
	pid_t pid;

	if (pipe(fd)  < 0)		// check pipe
	{
		fprintf(stderr, "Create Pipe Error %s\n", strerror(errno));	
		exit(EXIT_FAILURE);
	}
	
	if( (pid = fork()) < 0)	// check fork 
	{
		fprintf(stderr, " Fork Error : %s \n",strerror(errno) );
		exit(EXIT_FAILURE);
	}
				
	if(pid == 0)		//child interprocess 1  ==> Write
	{
		WriteProcess(int fd[2]);
	}
	else			//father interprocess 
	{
				//create a new child interprocess: read 50
		if((pid = fork()) < 0) 
		{
			fprintf(stderr, " Fork Error : %s \n",strerror(errno) );
			exit(EXIT_FAILURE);
		}
		if(pid == 0)	// child interprocess 2 => Read
		{
			ReadProcess(fd);
		}
		else		//father interprocess
		{
			wait(0);
			wait(0);
			printf("-------------End of Program-----------\n");
			return 0;
		}

	}

}