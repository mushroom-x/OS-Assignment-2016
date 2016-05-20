#ifdef _COMMON_H_
#define _COMMON_H_
	
#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <string.h>
#include <time.h>


using namespace std;

//BLOCK INDEX
#define BLOCK_INDEX_BOOT_BLOCK 0
#define BLOCK_INDEX_FAT1 1
#define BLOCK_INDEX_FAT2 3
#define BOLCK_INDEX_ROOT_DIR 5

#define BLOCK_NUM_BOOT_BLOCK 1
#define BLOCK_NUM_FAT 2
#define BLOCK_NUM_ROOT_DIR 2

//COMMON
#define FALSE 0
#define TRUE 1

#define EXIT_FAILURE -1



/* VHD: Virtual Hard Disk */
#define VHD_SIZE 1024000
#define VHD_BLOCK_SIZE	1024

#define VHD_BLOCK_FREE 0
#define VHD_BLOCK_FILE_END 65535

//File System
static const char * FileSysMagicNum = "10101010";   //file system magic number
static const char * FileSysName = "FileSys.txt";	//file system file name

//FAT


#define MAX_BUF_SIZE 10000
//File FCB
#define FILE_NAME_LEN 8
#define FILE_NAME_EXTEN_LEN 4 
#define FILE_META_LEN 100

// MD: METADATA
#define MD_DIR_FILE 0
#define MD_DATA_FILE 1


//DIR
#define DIR_NAME_LEN 80



//MAGIC NUMBER
#define MAGIC_NUM_LEN 8


//Open File Table
#define OFT_NUM 10



#endif