#ifndef _OFT_H_
#define _OFT_H_

#include "Common.h"
#include "FCB.h"

/*
*  OFT : Open File Table
*/
typedef struct OFT
{	
	//FCB PART 1
	
	/*
	char fileName[FILE_NAME_LEN];				//file name
	char fileNameExten[FILE_NAME_EXTEN_LEN];	//filename extension
	
	unsigned char metadata;						//file attribute			
	
	unsigned long length;						//file length
	unsigned short createTime;					//create time
  	unsigned short createDate;					//create date
	unsigned short blockNo;						//this file's the first Block Number	
	char isDirEntryFree;						//is Directory Entry is free ->FREE:TRUE	
	*/

	FCB fcb;

	//FCB PART 2 
	int  filePtr;					//read write Pointer in file
	char isFCBChange;				//to Record the Change of FCB-> Change:TRUE

	//DIRECTORY
	int  dirBlockNo;				// Parent Directory File Block Number
	int  dirEntryNo;				// Parent Dirrectory Entry Number in parentDirBlockNo
									//Directory Entry 短文件名目录项是最重要的数据结构，
									//其中存放着有关子目录或文件的短文件名、属性、起始簇号、时间值以及内容大小等信息。
	char dirName[DIR_NAME_LEN];

	//OFT
	char isOFTFree;
}OFT;




void InitOFT(OFT * OFTPtr);

void GetFreeOFT(OFT * OFTList,int OFTNum);

int findParentDir(OFT *OFTList,int fd);

#endif
