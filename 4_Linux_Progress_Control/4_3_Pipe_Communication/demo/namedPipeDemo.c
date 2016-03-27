#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define FIFO_NAME "/tmp/myfifio"

int main(int argc, char const *argv[])
{
	int fd;
	char w_buf[50];
	int w_num;

	// if  myfifo already exist then use it .otherwise create it
	if(mkfifo(FIFO_NAME,0777)<0 && (errno != EEXIST))
	{
		printf("connot create fifo ...\n");
		exit(1);
	}

	// block read only ,open fifo 
	fd = open(FIFO_NAME,O_WRONLY,0);
	if(fd == -1 || errno == ENXIO)
	{
		printf("cannot open fifo for read \n");
		exit(1);
	}
	while(1)
	{
		// input string and write it into fifo
		printf("please input something : \n");
		scanf("%s",w_buf);
		w_num = write(fd,w_buf,strlen(w_buf));
		printf("real write num is %d \n", w_num);
		if(strcmp(w_buf,"exit") == 0) 
		{
			break;
		}
	}
	return 0;
}