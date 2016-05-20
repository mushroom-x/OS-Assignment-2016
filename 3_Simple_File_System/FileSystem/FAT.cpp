#include "FAT.h"


FAT * initFAT(FAT *fat){
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
	 * |  5  |  6  |  ROOT DIR	| 根目录区			|
	 * |-----|-----|------------|-------------------|
	 */
	for(int i = BLOCK_INDEX_BOOT_BLOCK;i < BLOCK_INDEX_DATA_AREA; i++){
		fat[i].id = VHD_BLOCK_FILE_END;
	}

	/**
	 * 下列盘块对应的FAT表中的值设置为 FREE
	 * |-----|-----|------------|-------------------|
 	 * |  7  |1000 | DATA Area	| 数据区			|
	 */
	for(int i = BLOCK_INDEX_DATA_AREA;i < VHD_BLOCK_NUM; i++){

		fat[i].id = VHD_BLOCK_FREE;
	}

	return fat;
}

/**
 * 根据盘块号获取在FAT1表中的指针
 * @param  VHDPtr  [description]
 * @param  blockNo [description]
 * @return         [description]
 */
FAT* getFatPtrByBlockNo(unsigned char *VHDPtr, int blockNo){

	unsigned char * fat1Ptr = getBlockPtrByBlockNo(VHDPtr,BLOCK_INDEX_FAT1);
	return  fat1Ptr + blockNo;
}
