#ifndef _SYS_STRUCTURE_H_
#define _SYS_STRUCTURE_H_

#include "Common.h"
#include "FAT.h"
#include "FCB.h"
#include "FDT.h"
#include "OFT.h"


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
 * |  5  |1000 |  DATA BLOCK| 数据区			|
 * |-----|-----|------------|-------------------|
 * |  5  |  5  | ROOT_DIR	| 根目录文件		|
 *
 */

/*
* Boot Block | 引导块
*
* @note
* 	引导块占用第0号物理块，不属于文件系统管辖，
* 	如果系统中有多个文件系统，只有根文件系统才
* 	有引导程序放在引导块中，其余文件系统都不
* 	使用引导块
*
*/
typedef struct BootBlock
{
	/**
	 * Magic Number |　魔数
	 *
	 * @value
	 * 	10101010
	 */
	char magicNum[MAGIC_NUM_LEN];

	/**
	 * System Information | 系统信息
	 */
	char sysInfo[SYS_INFO_LEN];

	/**
	 * Root Directory Block Number |根目录文件的盘块号
	 */
	unsigned short rootBlockNo;

	/**
	 * Data Area VHD Block Pointer | 指向数据区首盘块的指针
	 *
	 */
	unsigned char *dataBlockPtr;

}BootBlock;



/**
 * Get Next VHD Block | 寻找下一个空闲的盘块
 * @note
 * 	从数据区的第一个盘块开始查找
 * @param  fatPtr [description]
 * @return        [description]
 */
unsigned short int getNextVhdBlock(FAT * fatPtr);
/**
 * 获取OFTList中空闲的OFT
 * @param  OFTList [description]
 * @param  OFTNum  [description]
 * @return         [description]
 */
//int getNextOft(OFT * OftList);

/**
 * 根据盘块号获取在FAT1表中的指针
 * @param  VHDPtr  [description]
 * @param  blockNo [description]
 * @return         [description]
 */
FAT* getFatPtrByBlockNo(unsigned char *VHDPtr, int blockNo);

/**
* Find Parent Directory File Open File‘s index in OFTList
* @note
* 	寻找当前打开文件的父目录文件的OFT在OftList中的序号
* @param  OftList [description]
* @param  oftNum  [description]
* @param  iOft    [description]
* @return         [description]
*/
int findPdfOft(OFT *OftList,int iOft);

/**
 * 初始化引导块
 * @param  VhdPtr [description]
 * @return        [description]
 */
BootBlock * initBootBlock(unsigned char *VhdPtr);

/**
 * Init FAT1 | 初始化FAT1
 */
FAT * initFAT1(unsigned char *VhdPtr);


/**
 * Init FAT2 | 初始化FAT2
 * @note
 *
 * @param VhdPtr [description]
 */
void initFAT2(unsigned char *VhdPtr);

/**
 * 初始化根目录[Root Directory]的FCB
 *
 * 初始化的根目录 只有两个FCB
 * 1. fcbDot
 * 	"."  当前目录
 *
 * 		length ： 2 * fcb
 * 2. fcbDotDot
 * 	".." 上一级目录 比较特殊在这里指向自己
 * 3. 其他空白PCB
 * @param  VhdPtr [description]
 * @return        [description]
 */
void initRootDirBlock(unsigned char *VhdPtr);

/**
 * 初始化根目录的OFT
 * @param  VhdPtr  [description]
 * @param  OftList [description]
 * @return         [description]
 */
OFT * initRootDirOft(unsigned char* VhdPtr,OFT * OftList);


#endif
