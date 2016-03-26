#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFNUM 60

int main(void)
{
	int n;
	int fd[2];
	pid_t pid;
	char buf[BUFNUM];

	if(pipe(fd)  < 0)
	{
		fprintf(stderr, "Create Pipe Error : %s\n",strerror(errno) );
		exit(EXIT_FAILURE);
	}

	if((pid = fork()) < 0)
	{
		fprintf(stderr, " Fork Error : %s\n",strerror(errno) );
		exit(EXIT_FAILURE);
	}

	if(pid  > 0)	/* parent process*/
	{
		close(fd[0]);
		write(fd[1],"I 'm your father.\n",17);
		write(fd[1],"I'm waiting for your termination.\n",35);
		wait(NULL);
	}
	else		/* child  process */
	{
		close(fd[1]);
		n = read(fd[0],buf,BUFNUM);
		printf("%d btyes read : %s\n", n,buf);
	}

	return 0;
}