#include "SysStructure.h"


/**
 * Get Next VHD Block | 寻找下一个空闲的盘块
 * @note
 * 	从数据区的第一个盘块开始查找
 * @param  fatPtr [description]
 * @return        [description]
 */
unsigned short int getNextVhdBlock(FAT * fatPtr){

	int i;

	// Search In Data Area
	for(i =BLOCK_INDEX_DATA_AREA; i < VHD_BLOCK_NUM; i++){
		if(fatPtr[i].id == VHD_BLOCK_FREE){
			return i;
		}
	}

	return VHD_BLOCK_FILE_END;
}


/**
 * 根据盘块号获取在FAT1表中的指针
 * @param  VHDPtr  [description]
 * @param  blockNo [description]
 * @return         [description]
 */
FAT* getFatPtrByBlockNo(unsigned char *VHDPtr, int blockNo){

	FAT  * fat1 = (FAT *)getBlockPtrByBlockNo(VHDPtr,BLOCK_INDEX_FAT1);
	return  fat1 + blockNo;
}


/**
* Find Parent Directory File Open File‘s index in OFTList
* @note
* 	寻找当前打开文件的父目录文件的OFT在OftList中的序号
* @param  OftList [description]
* @param  oftNum  [description]
* @param  iOft    [description]
* @return         [description]
*/
int findPdfOft(OFT *OftList,int iOft){

	int i;
	for(i = 0; i < OFT_NUM; i++){
		if(OftList[i].fcb.blockNo == OftList[iOft].fdt.pdfBlockNo){
			return i;
		}
	}
	return OBJECT_NOT_FOUND;
}



/**
 * 初始化引导块
 * @param  VhdPtr [description]
 * @return        [description]
 */
BootBlock * initBootBlock(unsigned char *VhdPtr){

	char * sysInfoStr = "文件系统,外存分配方式:FAT12,\n磁盘空间管理:结合于FAT的位示图,\n目录结构:单用户多级目录结构.";
	//get Boot Block
	unsigned char * bootBlockPtr = getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_BOOT_BLOCK);
	BootBlock * bootBlock = (BootBlock *)bootBlockPtr;

	//assign the value of Boot Block
	strcpy(bootBlock->magicNum,FileSysMagicNum);
	strcpy(bootBlock->sysInfo,sysInfoStr);
	bootBlock->rootBlockNo = BLOCK_INDEX_ROOT_DIR;
	bootBlock->dataBlockPtr = getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_DATA_AREA);

	return bootBlock;
}

/**
 * Init FAT1 | 初始化FAT1
 */
FAT * initFAT1(unsigned char *VhdPtr){

	FAT *fat1 = (FAT *)getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_FAT1);

	int i;

	/**
	 * 下列盘块对应的FAT表中的值设置为 文件结束标志
	 * |Start| End |	Name	|	Note			|
	 * |-----|-----|------------|-------------------|
	 * |  0  |  0  | Boot Block | 引导区			|
	 * |-----|-----|------------|-------------------|
	 * |  1  |  2  | 	FAT1	| 文件分配表 		|
	 * |-----|-----|------------|-------------------|
	 * |  3  |  4  | 	FAT2	| 文件分配表-备份 	|
	 * |-----|-----|------------|-------------------|
	 */
	for(i = BLOCK_INDEX_BOOT_BLOCK;i < BLOCK_INDEX_DATA_AREA; i++){
		fat1[i].id = VHD_BLOCK_FILE_END;
	}

	/**
	 * 下列数据区的盘块对应的FAT表中的值设置为 FREE
	 * |-----|-----|------------|-------------------|
 	 * |  5  | 1000| DATA Area	| 数据区			|
 	 * |-----|-----|------------|-------------------|
	 */
	for(i = BLOCK_INDEX_DATA_AREA;i < VHD_BLOCK_NUM; i++){

		fat1[i].id = VHD_BLOCK_FREE;
	}

	fat1[BLOCK_INDEX_ROOT_DIR].id = VHD_BLOCK_FILE_END;

	return fat1;
}

/**
 * Init FAT2 | 初始化FAT2
 * @note
 *
 * @param VhdPtr [description]
 */
void initFAT2(unsigned char *VhdPtr){
	FAT * fat1 = (FAT *)getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_FAT1);
	FAT * fat2 = (FAT *)getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_FAT2);
	memcpy(fat2,fat1,VHD_BLOCK_SIZE *2);
}

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
 void initRootDirBlock(unsigned char *VhdPtr){
	//FCB * rootPtr = (FCB *)getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_ROOT_DIR);

    initDirectoryFile(VhdPtr,BLOCK_INDEX_ROOT_DIR,BLOCK_INDEX_ROOT_DIR);

}


/**
 * 初始化根目录的OFT
 * @param  VhdPtr  [description]
 * @param  OftList [description]
 * @return         [description]
 */
OFT * initRootDirOft(unsigned char* VhdPtr,OFT * OftList){

	//OftList[0] use to Store Root FCB
	OFT * oftPtr = OftList;

	FCB * fcbPtr =(FCB *)getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_ROOT_DIR);

	initOft(oftPtr);
	loadFcb2Oft(oftPtr,fcbPtr);
    strcpy(oftPtr->fdt.dirName,"\\root\\");

	oftPtr->fdt.pdfBlockNo = BLOCK_INDEX_ROOT_DIR;  //5
	oftPtr->fdt.fdEntryNo = 0;					    //Root Dir 的父目录文件就是自己 目录项为首个FCB .

	return oftPtr;
}




