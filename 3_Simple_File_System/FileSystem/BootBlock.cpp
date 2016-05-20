#include "BootBlock.h"


BootBlock * getBootBlock(unsigned char *VHDPtr){
	BootBlock * bootBlock = (BootBLock*)VHDPtr;
	return bootBlock;
}

BootBlock * initBootBlock(unsigned char *VHDPtr){

	BootBlock* bootBlock = getBootBlock(VHDPtr);
	strcpy(bootBlock->magicNum,FileSysMagicNum);
	strcpy(bootBlock->info,"文件系统,外存分配方式:FAT,"+
			"磁盘空间管理:结合于FAT的位示图,目录结构:单用户多级目录结构.");
	bootBlock->rootBlockNo = BOLCK_INDEX_ROOT_DIR;
	/* The True Meaning for Data Block Pointer*/
	bootBlock->dataBlockPtr = VHDPtr + VHD_BLOCK_SIZE * BOLCK_INDEX_ROOT_DIR;

	return bootBlock;
}