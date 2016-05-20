#ifndef _BOOT_BLOCK_H_
#define _BOOT_BLOCK_H_

#include "Common.h"

/*
* Boot Block 
*/
typedef struct BootBlock
{
	char magicNum[8];				// Magic Number in File System
	char info[200];					// 
	unsigned short rootBlockNo;		// Root File 's Block Number
	unsigned char *dataBlockPtr;	// Start Block Position in Virtual Disk
									// TODO in this file system root directory is the first block in Data Area 
									// So dataBlockPtr = rootBlockPtr;
}BootBlock;



#endif
