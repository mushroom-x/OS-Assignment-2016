#include "Common.h"

#include "FAT.h"
#include "FCB.h"
#include "FDT.h"
#include "OFT.h"
#include "SysStructure.h"
#include "main.h"

/**
*   @Author Scorpion
*/
/*VHD: Virtual Hard Disk */
unsigned char *VhdPtr;
unsigned char VhdBuffer[VHD_SIZE];
unsigned char *VhdDataBlockPtr;				//the start position  of data block for VHD

/*OFT:Open File Table*/
OFT OftList[OFT_NUM];
int currOftIndex = 0;



int main(){
    cout << "File System |¡¡2016/05/23" << endl;
    my_start_sys();
    cout << "Start System" <<endl;
    cout << "-help to get more info ..." << endl;

    char cmdLine[CMD_LEN];

   // cout <<"Cmd Index : "<<getIndexOfCmd("mkdir")<<endl;


    while(TRUE){
        printf("%s > ",OftList[currOftIndex].fdt.dirName);
        gets(cmdLine);

        excuteCmd(cmdLine);
    }

    return 0;
}

/**
*  初始化
*/
void my_format(){

	initBootBlock(VhdPtr);
	initFAT1(VhdPtr);
	initFAT2(VhdPtr);
	initRootDirBlock(VhdPtr);
	saveVhdFile(VhdPtr);
}

/**
 * 开启文件系统
 */
void my_start_sys(){

	VhdPtr = (unsigned char *)malloc(VHD_SIZE);

	readVhdFile(VhdPtr);

	initRootDirOft(VhdPtr,OftList);

    // Get Boot Block
	BootBlock * bootBlock = (BootBlock *)(getBlockPtrByBlockNo(VhdPtr,BLOCK_NUM_BOOT_BLOCK));
	//Read Data Block Ptr From Boot Block
	VhdDataBlockPtr = bootBlock->dataBlockPtr;

	//Set Current OFT is this OFT: OftList
	currOftIndex = 0;
}


/**
 * Exit System | 退出系统
 * @note
 *
 */
void my_exit_sys(){

	while(currOftIndex){
        // TODO
		my_close(currOftIndex);
	}

	saveVhdFile(VhdPtr);
}


int my_cd(char *dirname){
    if(OftList[currOftIndex].fcb.metadata == MD_DATA_FILE){
        cout << " Data file can not be cd" << endl;
        return FAILURE;
    }

    OFT *oftPtr = OftList + currOftIndex;
    char *buffer = (char *)malloc(FILE_BUF_SIZE);
    FCB *fcbPtr = (FCB *)buffer;

    int fcbNum = int(oftPtr->fcb.length/sizeof(FCB));

    // 遍历目录文件 查看该目录文件下 dirname匹配的目录文件是否存在
    int i;
    for(i=0;i < fcbNum;i++,fcbPtr++){
        if(fileNameMatch(fcbPtr,dirname,(char *)"di")==TRUE && fcbPtr->metadata == MD_DIR_FILE){
            break;
        }
    }

    if(i == fcbNum){
        cout << "Error : No Such Directory Here ..."<< endl;
        return FAILURE;
    }

    if(strcmp(fcbPtr->fileName,".") == 0){
        // 特殊目录项 "."
        return SUCCESS;
    }
    else if(strcmp(fcbPtr->fileName,"..") == 0){     // 特殊目录项 ".."
        if(currOftIndex == 0){          //如果是根目录文件
            return SUCCESS;
        }else{
            currOftIndex = my_close(currOftIndex);
        }
    }
    else{
        //普通文件
        int iOft = getNextOft(OftList);

        if(iOft == OBJECT_NOT_FOUND){
            cout << "Error : No enough OFT " << endl;
            return FAILURE;
        }
        OFT * oftPtrChild = OftList + iOft;
        // FCB Part
        loadFcb2Oft(oftPtrChild,fcbPtr);
        // FDT Part
        oftPtrChild->fdt.filePtr = 0;
        oftPtrChild->fdt.isUpate = FALSE;

        //c_str() 函数的作用就是将string类型转换成char *
        strcpy(oftPtrChild->fdt.dirName,(string(oftPtr->fdt.dirName) + string(dirname) + string("\\")).c_str());
        oftPtrChild->isUse = TRUE;
        oftPtrChild->fdt.pdfBlockNo = oftPtr->fcb.blockNo;
        oftPtrChild->fdt.fdEntryNo = i;
        currOftIndex = iOft;
    }
    return TRUE;
}

int my_ls(){

    OFT *oftPtr = OftList + currOftIndex;
    if(oftPtr->fcb.metadata == MD_DATA_FILE){
        cout << "current file is data file, can not be ls" << endl;
        return FALSE;
    }

    char buffer[FILE_BUF_SIZE];
    oftPtr->fdt.filePtr = 0;
    do_read(oftPtr,oftPtr->fcb.length,buffer);

    FCB * fcbPtr = (FCB *)buffer;
    int i;
    int fcbNum = int(oftPtr->fcb.length/sizeof(FCB));

    for(i=0;i<fcbNum;i++,fcbPtr++){
        if(fcbPtr->isUse == TRUE){
            if(fcbPtr->metadata == MD_DIR_FILE){
                printDirFile(fcbPtr);
            }
            else if(fcbPtr->metadata == MD_DATA_FILE){
                printDataFile(fcbPtr);
            }
        }
    }
    return SUCCESS;
}

int my_mkdir(char *dirname){

    char *fname = strtok(dirname,".");
    char *ename = strtok(NULL,".");

    int blockNo;
    FAT* fat1 = (FAT *)getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_FAT1);

    if(ename){
        cout << "Error: directory file could not have file name extension."<< endl;
        return FAILURE;
    }


    char buffer[FILE_BUF_SIZE];

    OFT * oftPtr = OftList+ currOftIndex;

    oftPtr->fdt.filePtr = 0;
    // load data to buffer
    do_read(oftPtr,oftPtr->fcb.length,buffer);

    int fcbNum = int(oftPtr->fcb.length/sizeof(FCB));
    FCB * fcbPtr = (FCB *)buffer;
    for(int i=0;i < fcbNum;i++,fcbPtr++){
        if(fileNameMatch(fcbPtr,fname,ename) && fcbPtr->metadata == MD_DIR_FILE){
            cout << "Error:  Directory Already Exist " << endl;
            return FAILURE;
        }
    }

    // 申请OFT
    int iOft = getNextOft(OftList);
    if(iOft == OBJECT_NOT_FOUND){
        cout << "Error:No enough OFT Exist" << endl;
        return FAILURE;
    }
    // 申请存储的盘块
    blockNo = getNextVhdBlock(fat1);
    if(blockNo == VHD_BLOCK_FILE_END){
        cout << "Error: No Free Block Exist " << endl;
        //若失败 释放申请的OFT
        OftList[iOft].isUse = FALSE;
        return FAILURE;
    }

    fat1[blockNo].id  = VHD_BLOCK_FILE_END;
    initFAT2(VhdPtr);

    // 在父目录下寻找空闲的FCB
    int i;
    for(i=0;i<fcbNum;i++,fcbPtr++){
        if(fcbPtr->isUse == FALSE){
            break;
        }
    }

    oftPtr->fdt.filePtr = i*sizeof(FCB);
    oftPtr->fdt.isUpate = TRUE;
    // #
    // ÐÞ¸ÄÐÂ½¨µÄFCBÖÐµÄÄÚÈÝ
    FCB * fcbTmp  = new FCB;
    initFcb(fcbTmp,dirname,(char *)"di",MD_DIR_FILE);
    fcbTmp->blockNo = blockNo;
    fcbTmp->length = 2*sizeof(FCB);\

    //do_write Ç¿fcbTmpÐ´Èëµ½Ä¿Â¼ÎÄ¼þÀïÃæ
    do_write(oftPtr,(char*)fcbTmp,sizeof(FCB),WS_OVERWRITE); /*TODO Bug!!*/


    //ÉèÖÃ´ò¿ª±íÏî-Dot
    OFT * oftPtrChild = OftList+iOft;
    loadFcb2Oft(oftPtrChild,fcbTmp);
    strcpy(fcbTmp->fileName,".");
    oftPtrChild ->fdt.filePtr = 0;
    do_write(oftPtrChild,(char *)fcbTmp,sizeof(FCB),WS_OVERWRITE);

    // fcbTmp -> Dot Dot
    loadOft2Fcb(oftPtr,fcbTmp);
    strcpy(fcbTmp->fileName,"..");
    oftPtrChild ->fdt.filePtr = 1*sizeof(FCB);
    do_write(oftPtrChild,(char *)fcbTmp,sizeof(FCB),WS_OVERWRITE);


    my_close(iOft);



    // ¸üÐÂcurrent OFT (¸¸)Ä¿Â¼ÎÄ¼þµÄFCB
    fcbPtr = (FCB *)buffer;
    fcbPtr->length = oftPtr->fcb.length;
    oftPtr->fdt.filePtr = 0;
    do_write(oftPtr,(char *)fcbPtr,sizeof(FCB),WS_OVERWRITE);
    oftPtr->fdt.isUpate = TRUE;

    delete fcbTmp;

    return SUCCESS;
}

int my_rmdir(char *dirname){

    char * fname = strtok(dirname,".");
    char * ename = strtok(NULL,".");

    if(strcmp(dirname,".") == 0 && strcmp(dirname,"..") == 0){
        cout << "ÌØÊâÄ¿Â¼Ïî ²»¿É±»É¾³ý" << endl;
        return FAILURE;
    }

    if(ename){
        cout << "·Ç·¨Ä¿Â¼ÎÄ¼þÃû³Æ" << endl;
        return FAILURE;
    }

    char buffer[FILE_BUF_SIZE];
    OFT * oftPtr = OftList + currOftIndex;
    oftPtr->fdt.filePtr = 0;

    do_read(oftPtr,oftPtr->fcb.length,buffer);

    int i;
    FCB *fcbPtr = (FCB *)buffer;

    int fcbNum = int(oftPtr->fcb.length /sizeof(FCB));
    //Ñ°ÕÒÕâ¸öÃû×ÖµÄÄ¿Â¼Ïî
    for(i = 0; i < fcbNum; i++,fcbPtr++){
        if(fileNameMatch(fcbPtr,fname,(char *)"di")){
            break;
        }
    }

    if(i == fcbNum){
        cout << "Ã»ÓÐÕâ¸öÄ¿Â¼ÎÄ¼þ" << endl;
        return FAILURE;
    }

    // ÅÐ¶Ïµ±Ç°ÎÄ¼þ¼ÐÊÇ·ñÎª¿Õ
    if(fcbPtr->length > 2*sizeof(FCB)){
        cout << "Ä¿Â¼ÎÄ¼þ²»Îª¿Õ£¬ÇëÏÈÉ¾³ýÎÄ¼þ" << endl;
        return FAILURE;
    }

    int blockNo = fcbPtr->blockNo;
    // Çå¿ÕÕâ¸öÄ¿Â¼ÏîÕ¼¾ÝµÄFAT
    deleteFileAtFat(VhdPtr,blockNo);
    initFAT2(VhdPtr);
    // Çå¿Õµ±Ç°µÄFCB
    initFcb(fcbPtr);

    oftPtr->fdt.filePtr = i*sizeof(FCB);
    do_write(oftPtr,(char *)fcbPtr,sizeof(FCB),WS_OVERWRITE);
    oftPtr->fcb.length -= sizeof(FCB);

    // ¸üÐÂÄ¿Â¼Ïî"."ÖÐµÄÄÚÈÝ
    fcbPtr = (FCB *)buffer;
    fcbPtr->length = oftPtr->fcb.length;
    oftPtr->fdt.filePtr = 0;
    do_write(oftPtr,(char *)fcbPtr,sizeof(FCB),WS_OVERWRITE);

    oftPtr->fdt.isUpate = FALSE;

    return SUCCESS;
}


int my_create(char *filename){

    int i;
    char buffer[FILE_BUF_SIZE];

    char *fname = strtok(filename,".");
    char *ename = strtok(NULL,".");

    FAT * fat1 = (FAT *)getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_FAT1);

    // ÅÐ¶ÏÎÄ¼þÃûÊÇ·ñºÏ·¨
    if(strcmp(fname,"") == 0){
        cout << "Please Input File Name" <<endl;
        return FAILURE;
    }
    if(strcmp(ename,"") == 0){
        cout << "Please Input File Name Extension" << endl;
        return FAILURE;
    }

    // ¼ÓÔØµ±Ç°µÄÄ¿Â¼ÎÄ¼þ
    OFT * oftPtr = OftList + currOftIndex;
    oftPtr->fdt.filePtr = 0;
    do_read(oftPtr,oftPtr->fcb.length,buffer);
    FCB * fcbPtr = (FCB *)buffer;

    // ¼ì²éÊÇ·ñÓÐÎÄ¼þÖØÃû
    int fcbNum = int(oftPtr->fcb.length / sizeof(FCB));
    for(i=0; i < fcbNum;i++, fcbPtr++){
        if(fileNameMatch(fcbPtr,fname,ename)){
            cout << "File Already Exist " <<endl;
            return FAILURE;
        }
    }

    // Ñ°ÕÒ¿ÕÏÐµÄFCB¿é
    fcbPtr = (FCB *)buffer;
    for(i = 0;i < fcbNum;i++,fcbPtr ++){

        if(fcbPtr->isUse == FALSE){
            break;
        }
    }
        // TODO check if i == fcbNum
    //ÉêÇë¿ÕÏÐµÄÅÌ¿é
    int blockNo = getNextVhdBlock(fat1);
    if(blockNo == OBJECT_NOT_FOUND){
        return FAILURE;
    }
    // ¸üÐÂFATÐÅÏ¢²¢Í¬²½FAT2
    fat1[blockNo].id = VHD_BLOCK_FILE_END;
    initFAT2(VhdPtr);

    // ÌîÈëfcbµÄÐÅÏ¢
    initFcb(fcbPtr,fname,ename,MD_DATA_FILE);
    oftPtr->fdt.filePtr = i * sizeof(FCB);
    // Ð´Èëµ±Ç°Ä¿Â¼ÏîµÄÐÅÏ¢
    do_write(oftPtr,(char *)fcbPtr,sizeof(FCB),WS_OVERWRITE);

    //ÐÞ¸Äµ±Ç° "." Ä¿Â¼Ïî
    fcbPtr = (FCB *)buffer;
    fcbPtr->length = oftPtr->fcb.length;
    oftPtr->fdt.filePtr = 0;
    do_write(oftPtr,(char *)fcbPtr,sizeof(FCB),WS_OVERWRITE);

    oftPtr->fdt.isUpate = TRUE;
    return SUCCESS;
}


int my_rm(char *filename){
    // TODO strtok
    char *fname = strtok(filename,".");
    char *ename = strtok(NULL,".");

    if(!ename){
        cout << "Error: Please Input " << endl;
        return FAILURE;
    }

    if(strcmp(ename,"di") == 0){
        cout << "Error: File Directory can not be rm" << endl;
        return FAILURE;
    }

    //¶ÁÈ¡µ±Ç°µÄÄ¿Â¼ÎÄ¼þµ½buffer
    OFT * oftPtr = OftList + currOftIndex;
    oftPtr->fdt.filePtr = 0;
    char buffer[FILE_BUF_SIZE];
    do_read(oftPtr,oftPtr->fcb.length,buffer);

    //Ñ°ÕÒÕâ¸öÃû×ÖµÄÄ¿Â¼Ïî
    int i;
    FCB * fcbPtr = (FCB *)buffer;
    int fcbNum = int(oftPtr->fcb.length / sizeof(FCB));
    for(i = 0; i < fcbNum; i++,fcbPtr++){
        if(strcmp(fcbPtr->fileName,fname)==0&&strcmp(fcbPtr->fileNameExten,ename)==0){
            break;
        }
    }
    if( i == fcbNum){
        cout << "No such file exist " << endl;
        return FAILURE;
    }

    // Çå¿ÕÄ¿Â¼ÏîÕ¼¾ÝµÄFAT
    deleteFileAtFat(VhdPtr,fcbPtr->blockNo);

    // Í¬²½FAT2
    initFAT2(VhdPtr);

    //½«µ±Ç°Ä¿Â¼ÎÄ¼þÏÂµÄfcbPtrÖÃ¿Õ
    initFcb(fcbPtr);

    // ½«filePtrÖÐµÄÄÚÈÝÐ´µ½OFTÉÏ
    oftPtr->fdt.filePtr = i * sizeof(FCB);
    oftPtr->fcb.length -= sizeof(FCB);
    do_write(oftPtr,(char *)fcbPtr,sizeof(FCB),WS_OVERWRITE);


    // ¸üÐÂÄ¿Â¼Ïî "." µÄ³¤¶È
    fcbPtr  = (FCB *)buffer;
    fcbPtr->length = oftPtr->fcb.length;
    oftPtr->fdt.filePtr = 0; // Point to the start of the directory file
    do_write(oftPtr,(char *)fcbPtr,sizeof(FCB),WS_OVERWRITE);

    oftPtr->fdt.isUpate = TRUE;

    return SUCCESS;
}

/**
 * Close File|¹Ø±ÕÒ»¸öÎÄ¼þ    my_close(OFT* oft);
 * @param  iOft [description]
 * @return      [description]
 */
int my_close(int iOft)
{
	//²é¿´OftµÄË÷ÒýºÅÊÇ·ñºÏ·¨
	if(iOft > OFT_NUM || iOft < 0){
		cout << "Error : No such file exist.." << endl;
		return EXIT_FAILURE;
	}
	else{
        // piOft prarent OFT index
		int piOft = findPdfOft(OftList,iOft);
		if(piOft == OBJECT_NOT_FOUND){
			cout << "Father Directory is not exist" <<endl;
			return -1;
		}

		// Èç¹ûPCB·¢ÉúÁË¸Ä¶¯
		// ´ÅÅÌ¶ÁÈ¡ ->´æÈëBuffer -> ÐÞ¸ÄBuffer -> ½«Buffer´æÈë´ÅÅÌ
		if(OftList[iOft].fdt.isUpate == TRUE){
			char buffer[FILE_BUF_SIZE];
			do_read(&(OftList[piOft]),OftList[piOft].fcb.length,buffer);


			//¸üÐÂPCBÄÚÈÝ
            FCB * fcbBuf = (FCB *)(buffer + sizeof(FCB) * OftList[iOft].fdt.fdEntryNo);
			//strncpy(fcbBuf->fileName,OftList[iOft].fcb.fileName,);

            loadOft2Fcb(OftList+iOft,fcbBuf);
            OftList[piOft].fdt.filePtr = OftList[iOft].fdt.fdEntryNo * sizeof(FCB);

            do_write(OftList+piOft,(char *)fcbBuf,sizeof(FCB),WS_OVERWRITE);

		}

		initOft(OftList+iOft);
		currOftIndex = piOft;
		return piOft;
	}
}



int my_read(int iOft){

    //²é¿´OftµÄË÷ÒýºÅÊÇ·ñºÏ·¨
	if(iOft > OFT_NUM || iOft < 0){
		cout << "Error : No such file exist.." << endl;
		return FAILURE;
	}

    OFT * oftPtr = OftList + iOft;

    OftList[iOft].fdt.filePtr = 0;
    char text[FILE_BUF_SIZE] = "\0";
    do_read(oftPtr,oftPtr->fcb.length,text);

    //OUTPUT
    cout << text;
    return SUCCESS;
}

int my_write(int iOft){

    //²é¿´OftµÄË÷ÒýºÅÊÇ·ñºÏ·¨
	if(iOft > OFT_NUM || iOft < 0){
		cout << "Error : No such file exist.." << endl;
		return FAILURE;
	}

	OFT * oftPtr = OftList + iOft;
    int wstyle;

	cout << "Please Choose Write Style " << endl;
    cout << "ÊäÈë 0 = ½Ø¶ÏÐ´£¬1=¸²¸ÇÐ´£¬2=×·¼ÓÐ´" << endl;
    cin >> wstyle;
    if(wstyle > 2 || wstyle < 0){
        cout << "Error: wrong wstyle " << endl;
        return FAILURE;
    }

	char text[FILE_BUF_SIZE] = "\0";
	char textTmp[100] = "\0";           // MAX 100 character per line

    cout << "Ã¿ÐÐ×î´ó³¤¶È£º100 »Ø³µ»»ÐÐ£¬ÊäÈë½áÊø£ºctrl+z" << endl;
	getchar();
	while(gets(textTmp)){
        strcat(text,textTmp);
	}
	//+1ÊÇÒòÎªÒª°Ñ½áÎ²µÄ\0Ò²Ð´½øÈ¥
	do_write(oftPtr,text,strlen(text)+1,wstyle);
	oftPtr->fdt.isUpate = TRUE;
	return SUCCESS;
}



/**
*
*/
int do_read(OFT* oftPtr,int len,char *text){

	int lenToRead = len;
	int offset = oftPtr->fdt.filePtr;	// offset £ºÆ«ÒÆÁ¿
	int blockNo = oftPtr->fcb.blockNo;	// ÅÌ¿éºÅ
	FAT * fatPtr = getFatPtrByBlockNo(VhdPtr,blockNo);
	unsigned char * blockPtr;
    /* file buffer : load the whole block then next...*/
	unsigned char * fileBuf = getBlockBuffer();

	// Èç¹ûÆ«ÒÆÁ¿µÄÖ¸Õë´óÓÚÅÌ¿éµÄ´óÐ¡£¬ËµÃ÷¿ªÊ¼¶ÁµÄÖ¸Õë²»ÔÚµ±Ç°ÅÌ¿é
	// fatPtrÐèÒªÖ¸ÏòÏÂÒ»¸öÅÌ¿é
	while(offset >= VHD_BLOCK_SIZE){
		offset -= VHD_BLOCK_SIZE;
		blockNo = fatPtr->id;			//index to

		if(blockNo == VHD_BLOCK_FILE_END){
			cout << "Error: do_read no such block..." << endl;

			return EXIT_FAILURE;
		}

		fatPtr = getFatPtrByBlockNo(VhdPtr,blockNo);
	}

	blockPtr = getBlockPtrByBlockNo(VhdPtr,blockNo);

	// Read File To Buffer |ÏÈ½«µ±Ç°ÅÌ¿é¼ÓÔØµ½BufferÖÐ
	memcpy(fileBuf,blockPtr,VHD_BLOCK_SIZE);


	char * textPtr = text;

	while(len > 0){

		//ÐèÒª¶ÁµÄÄÚÈÝ¶ÌÓÚµ±Ç°ÅÌ¿éÖÐÎ´¶ÁµÄÄÚÈÝ->¾ÍÊ£×îºóÒ»¸öÅÌ¿éÃ»¶ÁÁË
		if( VHD_BLOCK_SIZE - offset > len){

			memcpy(textPtr,fileBuf + offset,len);
			textPtr += len;
			offset += len;
			oftPtr->fdt.filePtr += len;
			len = 0;
		}
		else{
            // ºóÃæ»¹ÓÐÅÌ¿éÎ´¶Á£¬Ò²¾ÍÊÇËµµ±Ç°ÅÌ¿éÊ£ÓàÎ´¶ÁµÄ¶¼ÊÇÐèÒª¶ÁÈ¡µÄ
			memcpy(textPtr,fileBuf + offset,VHD_BLOCK_SIZE - offset);
			textPtr += (VHD_BLOCK_SIZE - offset);
			offset = 0;
			len -= (VHD_BLOCK_SIZE - offset);

			// ¸üÐÂÏÂÒ»¸öÅÌ¿éµÄÐÅÏ¢
			blockNo = fatPtr->id;
			if(blockNo == VHD_BLOCK_FILE_END){
				cout << "Error: do_read too long "<< endl;
				break;
			}
			fatPtr = getFatPtrByBlockNo(VhdPtr,blockNo);
			blockPtr = getBlockPtrByBlockNo(VhdPtr,blockNo);

			// ¼ÓÔØÏÂÒ»¸öÅÌ¿éµÄÄÚÈÝµ½Buffer
			memcpy(fileBuf,blockPtr,VHD_BLOCK_SIZE);
		}

	}

	free(fileBuf);
    /* Ô¤ÆÚ¶ÁµÄ³¤¶È ¼õÈ¥×îºóÃ»¶ÁµÄ³¤¶È ¾ÍÊÇÊµ¼Ê¶ÁµÄ³¤¶È */
	return lenToRead - len;
}




int do_write(OFT * oftPtr,char *text,int len,char wstyle){

    int blockNo = oftPtr->fcb.blockNo;

    FAT *fatPtr = (FAT *)getBlockPtrByBlockNo(VhdPtr,blockNo);
    FAT *fat1 = (FAT *)getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_FAT1);

    // ¸ù¾Ýwstyle½øÐÐÔ¤´¦Àí
    writePreProcess(oftPtr,wstyle);
    int offset = oftPtr->fdt.filePtr;

    // PART1 : ÕÒµ½ÐèÒª¶ÁµÄÅÌ¿é
    while(offset >= VHD_BLOCK_SIZE){
        // offset >= VHD_BLOCK_SIZE -> ¸ÃÅÌ¿é²»ÊÇ×îºóÒ»¸ö£¬Ñ°ÕÒÏÂÒ»¸öÅÌ¿é
        blockNo = fatPtr->id;

        if(blockNo == VHD_BLOCK_FILE_END){
            // Èç¹û²»´æÔÚ ÔòÐèÒªÌí¼ÓÐÂµÄÅÌ¿é½øÈ¥
            blockNo = getNextVhdBlock(fat1);
            // Ã»ÓÐ¿ÕÏÐÅÌ¿é ±¨´í
            if(blockNo == VHD_BLOCK_FILE_END){
                cout << "Error: Not Enough Block "<< endl;
                return -1;
            }
            else{
                // Ìí¼ÓÅÌ¿é ²¢ÇÒ¸üÐÂFAT±íÐÅÏ¢ (FAT1)
                // Note Ö®ºóÒª¸üÐÂFAT2
                fatPtr->id = blockNo;
                fatPtr = (FAT *)getBlockPtrByBlockNo(VhdPtr,blockNo);
                fatPtr->id = VHD_BLOCK_FILE_END;
            }
        }

        fatPtr = (FAT *)getBlockPtrByBlockNo(VhdPtr,blockNo);
        offset -= VHD_BLOCK_SIZE;
    }

    // PART 2
    int lenTmp = 0;
    char *textPtr = text;
    unsigned char * buffer = getBlockBuffer();
    unsigned char * blockPtr = getBlockPtrByBlockNo(VhdPtr,blockNo);


    while(len > lenTmp){
        // ½«ÅÌ¿éÄÚÈÝ¶ÁÈ¡µ½BufferÖÐ
        memcpy(buffer,blockPtr,VHD_BLOCK_SIZE);

        // °ÑTextÖÐµÄÄÚÈÝÐ´µ½BufferÖÐ

        for(;offset < VHD_BLOCK_SIZE;offset++){
            *(buffer + offset) = *(textPtr);
            textPtr++;
            lenTmp++;

            cout << "haha" << endl;
            if(len >= lenTmp){
                break;
            }
        }


        // ½«BufferÖÐµÄÄÚÈÝ±£´æµ½ÅÌ¿éÖÐ
        memcpy(blockPtr,buffer,VHD_BLOCK_SIZE);

        // Èç¹ûoffset Ð´µ½ÅÌ¿éÄ©Î²¶øÇÒÎªÐ´Íê -> ÉêÇëÐÂµÄÅÌ¿é
        if(offset == VHD_BLOCK_SIZE && len != lenTmp){
                // Èç¹û¶Áµ½BlockµÄÄ©Î² ²¢ÇÒÎ´Ð´Íê
                offset = 0;
                blockNo = fatPtr->id;

                if(blockNo == VHD_BLOCK_FILE_END){
                    blockNo = getNextVhdBlock(fat1);

                    if(blockNo == VHD_BLOCK_FILE_END){
                        cout << "Error: No available block " << endl;
                        return EXIT_FAILURE;
                    }

                    blockPtr = getBlockPtrByBlockNo(VhdPtr,blockNo);
                    fatPtr->id = blockNo;
                    fatPtr = (FAT *)getBlockPtrByBlockNo(VhdPtr,blockNo);
                    fatPtr->id = VHD_BLOCK_FILE_END;

                }
                else{
                    blockPtr = getBlockPtrByBlockNo(VhdPtr,blockNo);
                    fatPtr = getFatPtrByBlockNo(VhdPtr,blockNo);
                }
        }
    }


    //¸üÐÂ³¤¶ÈÐÅÏ¢ºÍÎÄ¼þÖ¸Õë
    oftPtr->fdt.filePtr += len;
    if((int)oftPtr->fcb.length  < (int)oftPtr->fdt.filePtr){
        oftPtr->fcb.length = oftPtr->fdt.filePtr;
    }

    free(buffer);
    // ÊÍ·ÅÖ®ºó¶àÓàµÄÅÌ¿é
    releaseBlock(blockNo);

    // Í¬²½FAT2
    initFAT2(VhdPtr);

    return len;
}


/**
 * do_write Ö®Ç°¸ù¾Ýwrite style ½øÐÐÔ¤´¦Àí
 * @param oftPtr [description]
 * @param wstyle [description]
 */
void writePreProcess(OFT*  oftPtr,char wstyle){

	/**
	 * WS_REWRITE 0
	 * WS_OVERWRITE 1
	 * WS_APPEND 2
	 */
	switch(wstyle){
		/*
		* REWRITE: Write the file from the begining of the FCB
		*/
		case WS_REWRITE:
			oftPtr->fdt.filePtr = 0;
			oftPtr->fcb.length = 0;
			break;
		/*
		* Overwrite  ? is that necessary
		*/
		case WS_OVERWRITE:
			/* delete the end of the file's \0  */
			if(oftPtr->fcb.metadata == MD_DATA_FILE && oftPtr->fcb.length != 0){
				// ?TODO not right
				oftPtr->fdt.filePtr -= 1;
			}
			break;

		/*
		*  Append : appending data to the File
		*/
		case WS_APPEND:

			if(oftPtr->fcb.metadata == MD_DIR_FILE){
				oftPtr->fdt.filePtr = oftPtr->fcb.length;
			}
			else if(oftPtr->fcb.metadata == MD_DATA_FILE && oftPtr->fcb.length != 0){
				/* delete the end of the file's \0  */
				oftPtr->fdt.filePtr = oftPtr->fcb.length - 1;
			}
			break;
	}
}



unsigned char * getBlockBuffer(){
    unsigned char *buffer = (unsigned char *)malloc(VHD_BLOCK_SIZE*sizeof(unsigned char));
    if(buffer == NULL){
        cout << "¶¯Ì¬·ÖÅä¿Õ¼äÊ§°Ü" << endl;
        exit(EXIT_FAILURE);
    }
    return buffer;
}


void releaseBlock(int blockNo){

    FAT * fat1 = (FAT *)getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_FAT1);

    int i;
    int next;

    while(fat1[i].id != VHD_BLOCK_FILE_END){
        next = fat1[i].id;
        fat1[i].id = VHD_BLOCK_FREE;
        i = next;
    }

    fat1[blockNo].id = VHD_BLOCK_FILE_END;
}


/**
 * ´Ó´æ´¢ÎÄ¼þ[txt]ÖÐ¶ÁÈ¡ÐÅÏ¢µ½Buffer
 */
void readVhdFile(unsigned char * VhdPtr){
	FILE * sysFile;

	sysFile = fopen(FileSysName,"r");
	if(sysFile != NULL){
		// Write VhdBuffer from sysFile
		fread(VhdBuffer,VHD_SIZE,1,sysFile);
		fclose(sysFile);
		// FileSysMagicNum: "10101010"
		if(memcmp(VhdBuffer,FileSysMagicNum,MAGIC_NUM_LEN) == 0){
			// VHD  <- VhdBuffer
			memcpy(VhdPtr,VhdBuffer,VHD_SIZE);
			cout << "Success: FileSys.txt Read" <<endl;
			cout << "Update VHD from VhdBuffer" <<endl;
		}
		else{
			cout << "Error: Magic Number Not Match"<<endl;
			my_format();
			// VhdBuffer <- VHD
			memcpy(VhdBuffer,VhdPtr,VHD_SIZE);

		}
	}
	else{
		cout << "Error: FileSys.txt is not exist!"<<endl;
		cout << "Now FileSys.txt will be create" << endl;
		my_format();
		// VhdBuffer <- VHD
		memcpy(VhdBuffer,VhdPtr,VHD_SIZE);
	}
}

/**
 * ±£´æVhdBufferÖÐµÄÐÅÏ¢µ½´æ´¢ÎÄ¼þtxt
 * @param VhdPtr [description]
 */
void saveVhdFile(unsigned char *VhdPtr){
	FILE *filePtr = fopen(FileSysName,"w");
	fwrite(VhdPtr,VHD_SIZE,1,filePtr);
	fclose(filePtr);
}

void excuteCmd(char *cmdLine){

    char * arg1 = strtok(cmdLine," ");
    char * arg2 = strtok(NULL," ");

    int indexOfCmd = getIndexOfCmd(arg1);

    switch(indexOfCmd){
        case 0:
            //mkdir
            if(arg2 != NULL){
                cout << arg2 << endl;
                my_mkdir(arg2);
            }
            break;
        case 1:
            // rmdir
            if(arg2 != NULL){
                my_rmdir(arg2);
            }
            break;
        case 2:
            //ls
            my_ls();
            break;
        case 3:
            //cd
            if(arg2 != NULL){
                my_cd(arg2);
            }
            break;
        case 4:
            //create
            if(arg2 != NULL){
                my_create(arg2);
            }
            break;
        case 5:
            //rm
            if(arg2 != NULL){
                my_rm(arg2);
            }
            break;
        case 6:
            // open
            if(arg2 != NULL){
                my_create(arg2);
            }
            break;
        case 7:
            // close
            if(OftList[currOftIndex].fcb.metadata == MD_DATA_FILE){
                my_close(currOftIndex);
            }
            else{
                cout <<  "ÇëÏÈ´ò¿ªÎÄ¼þ£¬È»ºóÔÚ½øÐÐÎÄ¼þ²Ù×÷" << endl;
            }
            break;
        case 8:
            // write
            if(OftList[currOftIndex].fcb.metadata == MD_DATA_FILE){
                my_write(currOftIndex);
            }
            else{
                cout <<  "ÇëÏÈ´ò¿ªÎÄ¼þ£¬È»ºóÔÚ½øÐÐÎÄ¼þ²Ù×÷" << endl;
            }
            break;
        case 9:
            //read
            if(OftList[currOftIndex].fcb.metadata == MD_DATA_FILE){
                my_read(currOftIndex);
            }
            else{
                cout <<  "ÇëÏÈ´ò¿ªÎÄ¼þ£¬È»ºóÔÚ½øÐÐÎÄ¼þ²Ù×÷" << endl;
            }
            break;
        case 10:
            // exit
            my_exit_sys();
            cout << "ÍË³öÎÄ¼þÏµÍ³ " <<endl;
            exit(EXIT_SUCCESS);
            break;
        case 11:
            //help
            help();
            break;
        case 12:
            cout << "BigFile -TODO"<<endl;
            break;
        default:
            cout << "´ËÃüÁîÎÞÐ§"<< endl;
    }

}

int getIndexOfCmd(char *cmdStr){

    int i;
    if(strcmp(cmdStr,"") == 0){

        cout << "Command Can not be Null" << endl;
        return FAILURE;
    }

    for(i = 0; i < CMD_NUM; i++){
        if(strcmp(CmdGroup[i],cmdStr) == 0){
            return i;
        }
    }

    cout << "Error: Command Not Found !" << endl;
    return OBJECT_NOT_FOUND;
}

void help(){
    int i;

    cout << "------ Help ------" << endl;
    for(i = 0; i < CMD_NUM; i++){
        if(strcmp(CmdGroup[i],"") == 0){
            continue;
        }

        cout << "Index: "<< i << "; Command : "<<CmdGroup[i] <<endl;
    }
}
