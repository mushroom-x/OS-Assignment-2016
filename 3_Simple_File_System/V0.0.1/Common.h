#ifndef  COMMON_H
#define  COMMON_H


#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <string.h>
#include <time.h>


using namespace std;


/**
 * CONST DEFINE |　定义常量
 *
 * @Author Scorpion
 *
 */


/**
 * COMMON ｜　公用
 *
 * FALSE　	:假
 * TRUE		:真
 * OBJECT_NOT_FOUND : 对象没有找到
 */
#define FALSE 0
#define TRUE 1
#define SUCCESS 1
#define FAILURE -1
#define OBJECT_NOT_FOUND -1


/**
 * VHD: Virtual Hard Disk 虚拟磁盘
 *
 * VHD_SIZE 			：虚拟磁盘的大小
 * VHD_BLOCK_SIZE 		: VHD中单个盘块的大小
 * VHD_BLOCK_FREE		：FAT表中盘块空闲的标志
 * VHD_BLOCK_FILE_END 	：FAT 文件结束标志
 * VHD_BLOCK_NUM		: 盘块的总数
 *
 */
#define VHD_SIZE 1024000
#define VHD_BLOCK_SIZE	1024
#define VHD_BLOCK_FREE 0
#define VHD_BLOCK_FILE_END 65535
#define VHD_BLOCK_NUM 1000

/**
 * File System Structure | 文件系统结构
 *
 * BLOCK_INDEX_BOOT_BLOCK 	：引导块的首盘块号
 * BLOCK_INDEX_FAT1			: FAT1 的首盘块号
 * BLOCK_INDEX_FAT2			: FAT2 的首盘块号
 * BOLCK_INDEX_ROOT_DIR 	: 根目录文件的盘块号
 *
 */

/**
 * FAT File System Structure
 * FAT 文件系统结构
 *
 * |Start| End |	Name	|	Note			|
 * |-----|-----|------------|-------------------|
 * |  0  |  0  | Boot Block | 引导区			|
 * |-----|-----|------------|-------------------|
 * |  1  |  2  | 	FAT1	| 文件分配表 		|
 * |-----|-----|------------|-------------------|
 * |  3  |  4  | 	FAT2	| 文件分配表-备份 	|
 * |-----|-----|------------|-------------------|
 * |  5  |  5  |  ROOT DIR	| 根目录区			|
 * |-----|-----|------------|-------------------|
 * |  5  | 1000| DATA Area	| 数据区			|
 *
 */

#define BLOCK_INDEX_BOOT_BLOCK 0
#define BLOCK_INDEX_FAT1 1
#define BLOCK_INDEX_FAT2 3
#define BLOCK_INDEX_ROOT_DIR 5
#define BLOCK_INDEX_DATA_AREA 5

#define BLOCK_NUM_BOOT_BLOCK 1
#define BLOCK_NUM_FAT 2
#define BLOCK_NUM_ROOT_DIR 1


/**
 *  File Sytem + Boot Block | 文件系统
 */

#define MAGIC_NUM_LEN 8
#define SYS_INFO_LEN  200
static const char * FileSysMagicNum = "10101010";


/**
 * File Buffer ：文件缓冲
 */
#define FILE_BUF_SIZE 10000


/**
 * FCB
 * FILE_NAME_LEN 		: 文件名的长度    [Filename Extension Length]
 * FILE_NAME_EXTEN_LEN	: 文件拓展名的长度[Filename Extension Length]
 * [metadata]
 * 		MD_DIR_FILE 	：目录文件
 * 		MD_DATA_FILE	：用户数据文件
 */
#define FILE_NAME_LEN 8
#define FILE_NAME_EXTEN_LEN 3
#define MD_DIR_FILE 0
#define MD_DATA_FILE 1



//#define FILE_META_LEN 100


/**
 * File Discriptor Table | 文件描述符表
 *
 * DIR_NAME_LEN ： 文件目录名的长度
 */
#define DIR_NAME_LEN 80



/**
 * Open File Table |用户打开文件表
 *
 * OFT_NUM : OftList的长度
 */
#define OFT_NUM 10



/**
 * Data Store File  | 数据存储文件TXT
 *
 * MAGIC_NUM_LEN 	：魔数的长度 [Magic Number Length]
 * FileSysMagicNum	: 文件系统DATA储存文件的文件魔数
 * FileSysName 		：文件系统DATA储存文件的名称
 *
 */
static const char * FileSysName = "FileSys.txt";


/**
 * File Read Style  | 文件写入的几种方式
 *
 * WS_REWRITE 		：
 * WS_OVERWRITE		：
 * WS_APPEND		： 追加写
 */
#define WS_REWRITE 0
#define WS_OVERWRITE 1
#define WS_APPEND 2


/**
 * Console Command | 终端命令
 * CMD_NUM ：命令的个数
 * CMD_LEN : 命令的长度
 * CmdGroup: 指令集
 */
#define CMD_NUM 15
#define CMD_LEN 10
static const char CmdGroup[CMD_NUM][CMD_LEN] = {
	"mkdir", "rmdir", "ls", "cd", "create",
  	"rm", "open", "close", "write",
  	"read", "exit", "help","bigfile"
};



unsigned char * getBlockPtrByBlockNo(unsigned char *VHDPtr,int blockNo);

#endif
