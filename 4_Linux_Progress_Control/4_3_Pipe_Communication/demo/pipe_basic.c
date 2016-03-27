#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#define MAXCHARS 0

int main()
{
	int fd[2];	
	/**
	*  fd[0] : Write
	*  fd[1] : Read
	*/
	int size;
	pid_t pid;
	char str[] = "hello world! \n";
	char buf[MAXCHARS];

	if(pipe(fd) < 0)
	{
		printf(" pipe error \n");
		exit(1);
	}

	if((pid=fork()) < 0 )
	{
		printf("fork error \n");
		exit(1);
	}

	//child progress 
	if(pid == 0)
	{
		close(fd[1]);	//close the read
		printf("write the string : %s \n", buf);
		write(fd[0],str,strlen(str));
		exit(0);
	}
	else{
		close(fd[0]);
		size = read(fd[1],buf,sizeof(buf));
		printf("read the string : %s \n", buf);
		exit(0);
	}
}