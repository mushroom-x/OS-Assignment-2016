#ifndef _FAT_H_
#define _FAT_H_


#include "Common.h"
/**
* FAT : File Allocation Table
*/
typedef struct FAT
{
	unsigned short id;
} FAT;



FAT * getFAT1(unsigned char *VHDPtr);

FAT * getFAT2(unsigned char *VHDPtr);

unsigned short int getFreeVHDBlock(FAT * FATPtr);

#endif
