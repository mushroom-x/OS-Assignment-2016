#include "FCB.h"


/**
* 设置FCB 的时间
* @param fcb [description]
*/
void setFcbTime(FCB *fcb){
	time_t rawTime = time(NULL);
    struct tm *time = localtime(&rawTime);
    /*root->time 是unsigned short int类型的数据, 32位编译器下长16位,64位编译器下长32位
    * 这里考虑32位编译器, 所以完整的表示一个时间的秒数是不够长的,所以,保存秒数的一半
    * 这样小时占5位,分钟占6位,秒占5位
    */
    unsigned short currentTime = time->tm_hour * 2048 + time->tm_min*32 + time->tm_sec/2;

    /**
     * 年份我们保存的是实际值-2000, time->tm_mon要+1是因为,这个结构设计的时候0代表1月
     * 同理,年份占7位,月份占4位,日期占5位
     */
    unsigned short currentDate = (time->tm_year-2000)*512 + (time->tm_mon+1)*32 + (time->tm_mday);

    fcb->createTime = currentTime;
    fcb->createDate = currentDate;
}

/**
 * 初始化FCB
 * @param fcbPtr [description]
 */
void initFcb(FCB *fcbPtr){

	memset(fcbPtr,0,sizeof(FCB));

	 fcbPtr->length = 0;
 	strcpy(fcbPtr->fileName,"");
 	strcpy(fcbPtr->fileNameExten,"");
    fcbPtr->isUse = FALSE;
}

void initFcb(FCB * fcbPtr,char *fname,char *ename,unsigned char metadata){

    initFcb(fcbPtr);

    strcpy(fcbPtr->fileName,fname);
    strcpy(fcbPtr->fileNameExten,ename);
    setFcbTime(fcbPtr);
    fcbPtr->isUse = TRUE;
    fcbPtr->metadata = metadata;
}
/**
 * 判断文件名是否匹配
 * @return [description]
 */
int fileNameMatch(FCB * fcbPtr,char *fname,char *ename){

    if(strcmp(fcbPtr->fileName,fname) == 0 && strcmp(fcbPtr->fileNameExten,ename)==0){
        return TRUE;
    }else{
        return FALSE;
    }
}


void printDataFile(FCB *fcbPtr){

    // length - 2 是因为末尾有/n和/0两个字符
    unsigned int length = fcbPtr->length;
        if(length != 0)
            length -= 2;
            printf("%s.%s\t%dB\t%d/%d/%d\t%02d:%02d:%02d\n",
            fcbPtr->fileName,
            fcbPtr->fileNameExten,
            length,
            (fcbPtr->createDate>>9)+2000,
            (fcbPtr->createDate>>5)&0x000f,
            (fcbPtr->createDate)&0x001f,
            (fcbPtr->createTime>>11),
            (fcbPtr->createTime>>5)&0x003f,
            (fcbPtr->createTime)&0x001f * 2);
}

void printDirFile(FCB *fcbPtr){
     printf("%s\\\t<DIR>\t%d/%d/%d\t%d:%d:%d\n",
                       fcbPtr->fileName,
                       (fcbPtr->createDate>>9)+2000,
                       (fcbPtr->createDate>>5)&0x000f,
                       (fcbPtr->createDate)&0x001f,
                       (fcbPtr->createTime>>11),
                       (fcbPtr->createTime>>5)&0x003f,
                       (fcbPtr->createTime)&0x001f * 2);
}


int initDirectoryFile(unsigned char *VhdPtr,int blockNo,int parentBlockNo){

    int fcbNum = int(VHD_BLOCK_SIZE / sizeof(FCB));
    //void initFcb(FCB * fcbPtr,char *fname,char *ename,unsigned char metadata)
    FCB *fcbPtr = (FCB *)getBlockPtrByBlockNo(VhdPtr,blockNo);
    FCB *fcbPtrParent = (FCB *)getBlockPtrByBlockNo(VhdPtr,parentBlockNo);

    // Special File Directory Entry "."
    initFcb(fcbPtr,".","di",MD_DIR_FILE);
    fcbPtr->blockNo = blockNo;
    fcbPtr->length = 2 * sizeof(FCB);

    // Special File Directory Entry ".."
    fcbPtr++;
    initFcb(fcbPtr,"..","di",MD_DIR_FILE);
    fcbPtr->blockNo = parentBlockNo;
    //fcbPtr->length not ...

    return SUCCESS;
    /*
    for(int i = 2; i < fcbNum; i++,fcbPtr++){
        initFcb(fcbPtr);
    }
    */

}
/* Find a available directory */
int getNextFcb(FCB *fcbPtr){

    int fcbNum = int(VHD_BLOCK_SIZE/sizeof(FCB));

    for(int i = 2;i < fcbNum; i++,fcbPtr++){
        if(fcbPtr->isUse == FALSE){
            return i;
        }
    }

    return OBJECT_NOT_FOUND;
}

int initDataFileFcb(FCB *fcbPtr,char * fileName,int blockNo){

    char * fname = strtok(fileName,".");
    char * ename = strtok(NULL,".");

    if(!ename){
        cout << "Data File : file name extension could not be null " << endl;
        return FAILURE;
    }

    initFcb(fcbPtr,fname,ename,MD_DATA_FILE);
    fcbPtr->blockNo = blockNo;

    return TRUE;
}
