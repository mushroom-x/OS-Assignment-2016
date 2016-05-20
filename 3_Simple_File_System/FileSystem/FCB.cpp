#include "FCB.h"


 /*root->time 是unsigned short int类型的数据, 32位编译器下长16位,64位编译器下长32位
  * 这里考虑32位编译器, 所以完整的表示一个时间的秒数是不够长的,所以,保存秒数的一半
  * 这样小时占5位,分钟占6位,秒占5位
  * 年份我们保存的是实际值-2000, time->tm_mon要+1是因为,这个结构设计的时候0代表1月
  * 同理,年份占7位,月份占4位,日期占5位
  */

void setFCBTime(FCB *fcb){
	time_t rawTIme = time(NULL);
    struct tm *time = localtime(&rawTime);

    unsigned short currentTime = time->tm_hour * 2048 + time->tm_min*32 + time->tm_sec/2;
    unsigned short currentDate = (time->tm_year-100)*512 + (time->tm_mon+1)*32 + (time->tm_mday);
    
    fcb->createTime = currentTime;
    fcb->currentDate = currentDate;
}


void initFCB(FCB *fcb){

	//strcpy(fcb->fileName,fileName);
	//strcpy(fcb->fileNameExten,fileNameExten);
   	setFCBTime(fcb);
    //fcb->isDirEntryFree = TRUE;/*TODO True or False?*/
}


FCB * getRootDir(unsigned char *VHDPtr){
	FCB * root = (FCB *)(VHDPtr + VHD_BLOCK_SIZE*BOLCK_INDEX_ROOT_DIR)

	return root;
}


FCB * initFCBRootDir(unsigned char *VHDPtr){
	FCB * rootDir = getRootDir(VHDPtr);

	int fcbNum = int(VHD_BLOCK_SIZE/sizeof(FCB));
	
	//Part1:  .
	FCB * fcbThis = rootDir;
	initFCB(fcbThis);
	strcpy(fcbThis->fileName,".");
	strcpy(fcbThis->fileNameExten,"dir");
	fcbThis->metadata = MD_DIR_FILE;/* TODO metadata type meaning*/
	fcbThis->length = 2 * sizeof(FCB);//TODO
	fcbThis->isDirEntryFree = FALSE; 

	//Part2: .. (ROOT's Parent is Root self)
	FCB * fcbParent = fcbThis + 1;
	memcpy(fcbParent,fcbThis,sizeof(FCB));
	strcpy(fcbParent->fileName,"..");

	//ELSE
	fcbPtr = fcbParent;
	for(int i = 2;i<fcbNum;i++){
		fcbPtr++;
		strcpy(fcbPtr->fileName,"");
		fcbPtr->isDirEntryFree = TRUE;
	}

}