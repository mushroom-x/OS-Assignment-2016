#include "Common.h"

/**
 * 返回指向对应分区的首盘块的指针
 * @param  VHDPtr  [description]
 * @param  blockNo [盘块号]
 *          BOOT_BLOCK  0
 *          FAT1 		1
 *          FAT2		3
 *          ROOT_DIR  	5
 * @return         [description]
 */
unsigned char * getBlockPtrByBlockNo(unsigned char *VHDPtr,int blockNo)
{
	return VHDPtr + VHD_BLOCK_SIZE * blockNo;
}

