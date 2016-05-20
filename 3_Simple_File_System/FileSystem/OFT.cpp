#include "OFT.h"



int getFreeOFT(OFT * OFTList,int OFTNum)
{
	int i;

	for(i = 0; i < OFTNum; i++){
		if(OFTList[i].isOFTFree == TRUE){
			OFTListp[i].isOFTFree == FALSE;
			return i;
		}
	}

	return -1;
}


OFT * initOFT(OFT* oft){


	oft->isFCBChange = FALSE;
	oft->filePtr = 0;
	oft->isOFTFree = FALSE;

}

/* TODO : Not Complete*/
OFT * recycleOFT(OFT* oft){
	oft->fcb = new FCB;
	oft->isOFTFree = TRUE;
	// memset(&openfilelist[fd], 0, sizeof(USEROPEN));
}


void addFcbToOft(OFT * oft,FCB * fcb){

	strcpy(oft->fcb.fileName,fcb->fileName);
	strcpy(oft->fcb.fileNameExten,fcb->fileNameExten);

	oft->fcb.metadata = fcb->metadata;
	oft->fcb.length = fcb->length;
	oft->fcb.createTime	= fcb->createTime;
	oft->fcb.creaetDate = fcb->creaetDate;
	oft->fcb.blockNo = fcb->blockNo;
	oft->fcb.isDirEntryFree = fcb->isDirEntryFree;

}


OFT * initRootOFT(unsigned char* VHDPtr,OFT * OFTList){

	//OFTList[0] use to Store Root FCB 
	OFT * rootOftPtr = OFTList;

	FCB * rootFcbPtr = getRootDir(VHDPtr);

	initOFT(rootOftPtr);
	
	addFcbToOft(rootOftPtr,rootFcbPtr);

	rootOftPtr->isOFTFree = FALSE;
	rootOftPtr->dirBlockNo = BOLCK_INDEX_ROOT_DIR; //5
	rootOftPtr->dirEntryNo = 0;						/*TODO What is O standard for ?*/
	
	strcpy(rootOftPtr->dirName,"\\root\\");

	return rootOftPtr;
}



int findParentDir(OFT *OFTList,int indexOfOft){

	int i;

	for(i=0;i<OFT_NUM;i++){
		if(OFTList[i].fcb.blockNo == OFTList[indexOfOft].dirBlockNo){
			return i;
		}
	}

	return -1;
}

