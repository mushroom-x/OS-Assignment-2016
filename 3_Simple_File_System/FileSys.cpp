#include "FileSys.h"


/***** Global Vriable *****/
unsigned char *VirtualDiskPtr;
unsigned char buffer[VIRTUAL_DISK_SIZE];

UOFT UoftList[MAX_UOFT_NUM];
int UoftPtr;						// current UOFT

char * fileName = "myfsys.txt";

char cmdCollection[MAX_CMD_NUM][MAX_CMD_LEN] = {"mkdir", "rmdir", "ls", "cd", "create",
  	"rm", "open", "close", "write", 
  	"read", "exit", "help","bigfile"};

/**
* ======== MAIN =======
*/


/**
* ======== DIY Function =======
*/
// 获取一个空闲的磁盘块
unsigned short getFreeDiskBlock(){

}     

// get Free User Open File Table List
int getFreeOFTList(){

}          

// 寻找一个打开文件的父目录打开文件   
// TODO 
int getParentDir(int fd){

}


int executeCmd(String cmdStr){
	//getCmdIndex 
	// -1 Invalid
	// 0-N ...
}


int getCmdIndex(String cmdStr){

}


/*
* ======== SYS  OPERATION =======
*/ 
/*
* Start File System
*/
void my_startsys(){

}

/**
* Exit File System
*/
void my_exitsys();{

}

/**
* Initialize the Virtual Disk
*/
void my_format(){

}



/**
* ======== DIR  OPERATION=======
*/

void my_cd(char *dirname){

}

void my_mkdir(char *dirname){

}

void my_rmdir(char *dirname){

}

void my_ls(){

}


/**
* ======== FILE  OPERATION=======
*/

int  my_create(char *filename){

}

void my_rm(char *filename){

}

int  my_open(char *filename){

}

int  my_close(int fd){

}

int  my_write(int fd){

}

int  do_write(int fd, char *text, int len, char wstyle){

}

int  my_read(int fd){

}

int  do_read(int fd, int len, char *text){

}

int my_help(){
	// print all avaliable command
}



