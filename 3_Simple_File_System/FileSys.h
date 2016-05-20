#ifndef   _FILE_SYSTEM_H_
#define	  _FILE_SYSTEM_H_

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <iostream>
#include <fstream>

using namespace std;

/* Define the Const*/
#define DISK_BLOCK_SIZE			1024
#define VIRTUAL_DISK_SIZE 		1024000

//FAT
#define FAT_FILE_END_FLAG	65535
#define MAX_UOFT_NUM 		10
#define MAX_TEXT_SIZE		10000

#define DIR_NAME_LEN 80
//COMMON
#define FALSE	0
#define TRUE	1

#define MAX_CMD_NUM 15
#define MAX_CMD_LEN 10


/*
* FCB: File Control Block 
*/
typedef struct FCB{
	char fileName[8];
	char fileType[3];
	unsigned long fileLength;
	unsigned char fileAttribute;
	unsigned short createTime;
	unsigned short createDate;
	unsigned short firstDiskBlockNo;
	char isBusy;	
} FCB;

/**
* FAT : File Allocation Table
*/
typedef struct FAT
{
	unsigned short id;
} FAT;

/*
*  OFT : Open File Table
*/
typedef struct OFT
{
	char fileName[8];
	char fileType[3];
	unsigned long fileLen;
	unsigned char metadata;
	unsigned short createTime;
	unsigned short createDate;
	unsigned short firstBlockNo;
	char isBusy;	


	int  dirBlockNo;			// Parent Directory File Block Number
	int  dirEntryNo;			// Dirrectory Entry Number in parentDirBlockNo
	char dirName[DIR_NAME_LEN];
	int  filePtr;
	char isUpdate;
	char isBusy;
} OFT;


/*
* Boot Block 
*/
typedef struct BootBlock
{
	char magicNum[8];				// Magic Number in File System
	char info[200];					// 
	unsigned short rootBlockNo;		// Root File 's Block Number
	unsigned char *startBlockNo;	// Start Block Position in Virtual Disk
}BootBlock;




/***** 函数申明 *****/
void my_startsys();                	// 进入文件系统
void my_exitsys();                 	// 退出文件系统
void my_format();                  	// 磁盘格式化函数
void my_mkdir(char *dirname);      	// 创建子目录
void my_rmdir(char *dirname);      	// 删除子目录
void my_ls();                      	// 显示目录中的内容
void my_cd(char *dirname);         // 用于更改当前目录
int  my_create(char *filename);    // 创建文件
void my_rm(char *filename);        // 删除文件
int  my_open(char *filename);      // 打开文件
int  my_close(int fd);             // 关闭文件
int  my_write(int fd);             // 写文件
int  my_read(int fd);     // 读文件
int  do_write(int fd, char *text, int len, char wstyle);
int  do_read(int fd, int len, char *text);



unsigned short getFreeDiskBlock();      // 获取一个空闲的磁盘块
int getFreeOpenFileList();          // 获取一个空闲的文件打开表项
int findParentDir(int fd);			// 寻找一个打开文件的父目录打开文件   


#endif
