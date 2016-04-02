#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>


#define MSG_FILE "./tmp"

#define BUF_SIZE 255
#define PERM S_IRUSR|S_IWUSR

struct msgbuf
{
	long mtype;
	char mtext[BUF_SIZE + 1];
};

typedef struct msgbuf msgbuf;
