#ifndef _FCB_H_
#define _FCB_H_

#include "Common.h"

typedef struct FCB
{
	char fileName[FILE_NAME_LEN];				//file name
	char fileNameExten[FILE_NAME_EXTEN_LEN];	//filename extension
	
	unsigned char metadata;						//file attribute			
	
	unsigned long length;						//file length
	unsigned short createTime;					//create time
  	unsigned short createDate;					//create date
	unsigned short blockNo;						//this file's the first Block Number	
	char isDirEntryFree;						//is Directory Entry is free ->FREE:TRUE	
};
	


#endif