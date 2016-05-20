#include "FAT.h"

FAT * initFAT1AndFAT2(unsigned char *VHDPtr){
	FAT fat1 = getFAT1(VHDPtr);
	FAT fat2 = getFAT2(VHDPtr);

	initFAT(fat1);
	// ROOT DIRECTORY
	fat1[BOLCK_INDEX_ROOT_DIR].id = VHD_BLOCK_FILE_END;

	synchronizeFAT2(VHDPtr);
}

FAT * initFAT(FAT *fat){
	int i;

	/*TODO 5? 1000?*/
	for(int i=0;i<5;i++){
		fat[i].id = VHD_BLOCK_FILE_END;
	}

	for(int i=5;i<1000;i++){
		/* TODO  ? Why*/
		fat[i].id = VHD_BLOCK_FREE;
	}

	return fat;
}


FAT * getFAT1(unsigned char *VHDPtr){
	FAT * FAT1 = (FAT*)(VHDPtr + VHD_BLOCK_SIZE * BLOCK_INDEX_FAT1);
	return FAT1;
}


FAT * getFAT2(unsigned char *VHDPtr){
	FAT * FAT2 = (FAT*)(VHDPtr + VHD_BLOCK_SIZE * BLOCK_INDEX_FAT2);
	return FAT2;
}

void synchronizeFAT2(unsigned char *VHDPtr){
	FAT1 * fat1 = getFAT1(VHDPtr);
	FAT * fat2 = getFAT2(VHDPtr);
	memcpy(fat2,fat1,VHD_BLOCK_SIZE);
}


unsigned short int getFreeVHDBlock(FAT * FATPtr){

	int i;
	int blockNum = (int)(VHD_SIZE/VHD_BLOCK_SIZE)
	
	for(i = 0; i < blockNum; i++){
		if(FATPtr[i].id == VHD_BLOCK_FREE){
			return i;
		}
	}

	return VHD_BLOCK_FILE_END;
}

FAT* getFATPtrByBlockNo(unsigned char *VHDPtr, int blockNo){

	return getFAT1(VHDPtr) + blockNo;
}

unsigned char * getBlockPtrByBlockNo(unsigned char *VHDPtr, int blockNo){
	return VHDPtr + blockNo * VHD_BLOCK_SIZE;
}