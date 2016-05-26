#include "SysStructure.h"

/**
 * IO Operation
 */

/* 保存整个虚拟磁盘文件 */
void saveVhdFile(unsigned char *VhdPtr,unsigned char *vhdBuf){

    memcpy(VhdPtr,vhdBuf,VHD_SIZE);
}

/* 保存某个盘块 */
void saveVhdFile(unsigned char *VhdPtr,int blockNo,unsigned char *blockBuf){

    unsigned char * blockPtr = getBlockPtrByBlockNo(VhdPtr,blockNo);
    memcpy(blockPtr,blockBuf,VHD_BLOCK_SIZE);
}

/* 保存某个盘块的对应偏移量offset的FCB*/
void saveVhdFile(unsigned char *VhdPtr,int blockNo,int offset,unsigned char *fcbBuf){

    unsigned char * blockPtr = getBlockPtrByBlockNo(VhdPtr,blockNo);
    unsigned char * fcbPtr = blockPtr + offset * sizeof(FCB);

    memcpy(fcbPtr,fcbBuf,sizeof(FCB));
}

/* 读取整个虚拟磁盘文件到Vhd Buffer*/
void readVhdFile(unsigned char *VhdPtr,unsigned char *vhdBuf){

    memcpy(vhdBuf,VhdPtr,VHD_SIZE);
}

/* 读取整个盘块到对应的Block Buffer */
void readVhdFile(unsigned char *VhdPtr,int blockNo,unsigned char *blockBuf){

    unsigned char * blockPtr = getBlockPtrByBlockNo(VhdPtr,blockNo);

    memcpy(blockBuf,blockPtr,VHD_BLOCK_SIZE);
}

/* 读取FCB到对应的fcbBuf*/
void readVhdFile(unsigned char *VhdPtr,int blockNo,int offset,unsigned char *fcbBuf){
     unsigned char * blockPtr = getBlockPtrByBlockNo(VhdPtr,blockNo);
     unsigned char * fcbPtr = blockPtr + offset * sizeof(FCB);

     memcpy(fcbBuf,blockPtr,sizeof(FCB));
}



/**
 * Boot Block
 */
/* 初始化引导块 */
void initBootBlock(unsigned char *VhdPtr){

    BootBlock * bootBlock = (BootBlock *)getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_BOOT_BLOCK);;

    strcpy(bootBlock->magicNum,FileSysMagicNum);
    strcpy(bootBlock->sysInfo,sysInfo);
    bootBlock->dataBlockPtr = getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_DATA_AREA);
    bootBlock->rootBlockNo = BLOCK_INDEX_ROOT_DIR;
}



/**
 * FAT
 * File Allocation Table
 */

/* 初始化FAT1 */
void initFat1(unsigned char *VhdPtr){
    FAT * fat1 = getFat1Ptr(VhdPtr);

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

    /**
	 * 将ROOT_DIR 的表项设为END
	 * |-----|-----|------------|-------------------|
 	 * |  5  | 5   | ROOT_DIR	| 根目录			|
 	 * |-----|-----|------------|-------------------|
 	 */
 	 fat1[BLOCK_INDEX_ROOT_DIR].id = VHD_BLOCK_FILE_END;
}

/* 同步FAT2 */
void updateFat2(unsigned char *VhdPtr){

    FAT * fat1 = (FAT *)getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_FAT1);
    FAT * fat2 = (FAT *)getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_FAT2);

    memcpy(fat1,fat2,BLOCK_NUM_FAT * VHD_BLOCK_SIZE);
}


/* 获取FAT1的FAT指针*/
FAT * getFat1Ptr(unsigned char *VhdPtr){
    FAT * fat1 = (FAT *)getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_FAT1);
    return fat1;
}

/* 根据盘块号获取盘块号对应的FAT的指针 */
FAT * getFatPtr(unsigned char *VhdPtr,int blockNo){
    FAT * fat1 = getFat1Ptr(VhdPtr);
    return fat1 + blockNo;
}

/* 根据盘块号和偏移量获取FCB的指针 : ? offset > fcbNum*/
FCB * getFcbPtr(unsigned char *VhdPtr,int blockNo,int offset){

    FCB * fcbPtr = (FCB *)(getBlockPtrByBlockNo(VhdPtr,blockNo) + offset * sizeof(FCB));
    return fcbPtr;
}


/*　获取下一个空闲的盘块 并更新FAT表　*/
int getNextBlock(unsigned char *VhdPtr,int blockNo){

    FAT * fat1 = getFat1Ptr(VhdPtr);
    FAT * fatPtr = fat1;
    int fatNum = (BLOCK_NUM_FAT * VHD_BLOCK_SIZE )/(sizeof(FAT));

    for(int i = 0; i<fatNum; i++,fatPtr++){
        if(fatPtr->id == VHD_BLOCK_FREE){
            return i;
        }
    }
    return OBJECT_NOT_FOUND;
}

/*  释放占据的盘块*/
void freeBlock(unsigned char *VhdPtr,int blockNo){

    FAT * fat1 = getFat1Ptr(VhdPtr);
    FAT * fatPtr = fat1 + blockNo;

    while(fatPtr->id != VHD_BLOCK_FILE_END){
        fatPtr->id = VHD_BLOCK_FREE;
        fatPtr++;
    }

    fatPtr->id = VHD_BLOCK_FREE;

}


/**
 * Root Block
 */

/* 初始化根目录的磁盘块 */
void initRootBlock(unsigned char *VhdPtr){

     initDirectoryBlock(VhdPtr,BLOCK_INDEX_ROOT_DIR,BLOCK_INDEX_ROOT_DIR);
}


/**
 * FCB
 * File Control Block
 */

/* 初始化FCB的时间和日期 */
void setFcbDateTime(FCB *fcbPtr){
    time_t rawTime = time(NULL);
    struct tm *time = localtime(&rawTime);
    /*root->time 是unsigned short int类型的数据, 32位编译器下长16位,64位编译器下长32位
    * 这里考虑32位编译器, 所以完整的表示一个时间的秒数是不够长的,所以,保存秒数的一半
    * 这样小时占5位,分钟占6位,秒占5位
    */
    unsigned short currentTime = time->tm_hour * 2048 + time->tm_min*32 + time->tm_sec/2;

     /**
     * 年份我们保存的是实际值-2000, time->tm_mon要+1是因为,这个结构设计的时候0代表1月
     * 同理,年份占7位,月份占4位,日期占5位 TODO -100
     */
    unsigned short currentDate = (time->tm_year-100)*512 + (time->tm_mon+1)*32 + (time->tm_mday);

    fcbPtr->createDate = currentDate;
    fcbPtr->createTime = currentTime;
}

/* 初始化空闲的FCB */
void initFcb(FCB *fcbPtr){
    memset(fcbPtr,0,sizeof(FCB));

    fcbPtr->length = 0;
    fcbPtr->isUse = FALSE;
    strcpy(fcbPtr->fileName,"");
 	strcpy(fcbPtr->fileNameExten,"");
}

/* 根据文件名和类型初始化FCB */
void initFcb(FCB *fcbPtr,char *fname,char *ename,unsigned char metadata){

    initFcb(fcbPtr);
    fcbPtr->metadata = metadata;
    strcmp(fcbPtr->fileName,fname);
    strcmp(fcbPtr->fileNameExten,ename);

    fcbPtr->isUse = TRUE;
}

void initDirectoryBlock(unsigned char *VhdPtr,int curBlockNo,int parentBlockNo){

    FCB * fcbPtr = getFcbPtr(VhdPtr,curBlockNo,0);

    FCB * dotPtr = fcbPtr;
    FCB * dotDotPtr = fcbPtr + 1;

    initFcb(dotPtr,".","di",MD_DIR_FILE);
    dotPtr->blockNo = curBlockNo;
    dotPtr->length = 2 * sizeof(FCB);

    FCB * parentFcbPtr = getFcbPtr(VhdPtr,parentBlockNo,0);
    memcpy(dotDotPtr,parentFcbPtr,sizeof(FCB));
    strcpy(dotDotPtr->fileName,"..");
}

/* 匹配文件名和文件类型是否匹配 */
int fileIsEqual(FCB * fcbPtr,char *fname,char *ename,unsigned char metadata){

    if(strcmp(fcbPtr->fileName,fname) == 0 && strcmp(fcbPtr->fileNameExten,ename) == 0 && fcbPtr->metadata == metadata){

        return TRUE;
    }

    return FALSE;
}

/* 在当前block中 申请下一个空闲的FCB | 当前的OFT 默认为目录文件 */
int getNextFcb(unsigned char * VhdPtr,int blockNo){

    FCB * fcbPtr = getFcbPtr(VhdPtr,blockNo,0);

    int i;
    int fcbNum = VHD_BLOCK_SIZE / sizeof(FCB);

    for(i = 0; i < fcbNum; i++,fcbPtr++){
        if(fcbPtr->isUse == FALSE){
            return i;
        }
    }

    return OBJECT_NOT_FOUND;
}

/* 打印文件信息 | 根据文件类型选择不同的打印方式  */
void printFcbInfo(FCB *fcbPtr){

    unsigned int length = fcbPtr->length;

    if(length == 0){
        return;
    }

    if(fcbPtr->metadata == MD_DATA_FILE){


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
    else if(fcbPtr->metadata == MD_DIR_FILE){

        printf("%s\\\t<DIR>\t%d/%d/%d\t%d:%d:%d\n",
            fcbPtr->fileName,
            (fcbPtr->createDate>>9)+2000,
            (fcbPtr->createDate>>5)&0x000f,
            (fcbPtr->createDate)&0x001f,
            (fcbPtr->createTime>>11),
            (fcbPtr->createTime>>5)&0x003f,
            (fcbPtr->createTime)&0x001f * 2);
    }
}



/**
 * FDT
 * Fild Descriptor Table
 */

void initFdt(FDT *fdtPtr){
    fdtPtr->isUpate = FALSE;
    fdtPtr->filePtr = 0;
}



/**
 * OFT
 * Open File Table
 */



 void initOft(OFT *oftPtr){

    initFcb(&(oftPtr->fcb));
    initFdt(&(oftPtr->fdt));

    oftPtr->isUse = FALSE;
 }


void initOft(OFT * OftList,int curOftIndex,int parOftIndex,int fdEntryNo,FCB *fcbPtr){

    OFT * curOftPtr = OftList + curOftIndex;
    OFT * parOftPtr = OftList + parOftIndex;

    initOft(curOftPtr);

    cloneFcb2Oft(curOftPtr,fcbPtr);

    if(fcbPtr->metadata == MD_DATA_FILE){
        strcpy(curOftPtr->fdt.dirName,(string(parOftPtr->fdt.dirName)+string(curOftPtr->fcb.fileName)).c_str());
    }
    else if(fcbPtr->metadata == MD_DIR_FILE){
        strcpy(curOftPtr->fdt.dirName,(string(parOftPtr->fdt.dirName)+string(curOftPtr->fcb.fileName)+string("\\")).c_str());
    }

    curOftPtr->fdt.fdEntryNo = fdEntryNo;
    curOftPtr->fdt.filePtr = 0;
    curOftPtr->fdt.isUpate = FALSE;
    curOftPtr->fdt.pdfBlockNo = parOftPtr->fcb.blockNo;

    curOftPtr->isUse = TRUE;
}


 /* 初始化根目录文件的打开文件 */
void initRootOft(unsigned char* VhdPtr,OFT *OftList){

    OFT *oftPtr = OftList;

    FCB *fcbPtr = getFcbPtr(VhdPtr,BLOCK_INDEX_ROOT_DIR,0);

    initOft(OftList,0,0,0,fcbPtr);

    strcpy(oftPtr->fdt.dirName,"\\root\\");

}






/* 获取父打开文件在OftList的Index*/
int getParentOft(OFT *OftList,int curOft){

    int i;

    for( i=0; i < OFT_NUM; i++){
        if(OftList[i].isUse == FALSE){
            continue;
        }

        if(OftList[curOft].fdt.pdfBlockNo == OftList[i].fcb.blockNo){
            return i;
        }
    }
    return OBJECT_NOT_FOUND;
}


/* 将OFT中的信息拷贝到FCB中*/
void cloneFcb2Oft(OFT *oftPtr,FCB *fcbPtr){
    memcpy(&(oftPtr->fcb),fcbPtr,sizeof(FCB));
}

/* 将Oft中的FCB信息保存到FCB中*/
void cloneOft2Fcb(OFT *oftPtr,FCB *fcbPtr){
    memcpy(fcbPtr,&(oftPtr->fcb),sizeof(FCB));
}


unsigned char * getBlockPtrByBlockNo(unsigned char *VHDPtr,int blockNo){
    unsigned char * blockPtr = VHDPtr + blockNo * VHD_BLOCK_SIZE;
    return blockPtr;
}
