#include "FAT.h"


void deleteFileAtFat(unsigned char *VhdPtr,int blockNo){

    FAT * fat1 = (FAT *)getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_FAT1);

    int current = blockNo;
    int next;
    while(TRUE){
        next = fat1[current].id;
        fat1[current].id = VHD_BLOCK_FREE;
        if(next != VHD_BLOCK_FILE_END){
            current = next;
        }else{
            fat1[next].id = VHD_BLOCK_FREE;
            break;
        }
    }
}
