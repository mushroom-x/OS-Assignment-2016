#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>

#define IPCKEY 0x11

int main(void)
{
	int i = 0;
	for(i = 1;i<256;++i)
	{
		printf("key = %x\n",ftok("/tmp",i));
	}
	return 0;
}