#include "OFT.h"


OFT * openFile(OFT * OftList,FCB *fcbPtr){

	int iOft = getNextOft(OftList);
	OFT *oftPtr;

	if(iOft == OBJECT_NOT_FOUND){
		cout << "Error: Open File Table List is full.." << endl;
		return NULL;
	}


	oftPtr = OftList + iOft;

	initOftWithFcb(OftList,iOft,fcbPtr);
}



OFT * initOftWithFcb(OFT * OftList,int iOft,FCB *fcbPtr){

	OFT * oftPtr = OftList + iOft;
	int pdfOftIndex; 					// parent directory file in openfile list
	// 1.Init
	initOft(oftPtr);
	loadFcb2Oft(oftPtr,fcbPtr);


	/*
	// 2.Apply for a free OFT in OftList

	pdfOftIndex = findPdfOft(OftList,iOft);
	if(pdfOftIndex == OBJECT_NOT_FOUND){
		cout << "Error: OFT could not find parent directory in OftList" <<endl;
		return NULL;
	}

	// 3.
	OFT * pdfOftPtr = OftList + pdfOftIndex;
	oftPtr->fdt->pdfBlockNo = pdfOftPtr->fcb->blockNo;
	oftPtr->fdt->fdEntryNo = //TODO ?
	*/
}


/**
 * 初始化OFT
 * @param  oft [description]
 * @return     [description]
 */
OFT * initOft(OFT * oftPtr){

	initFcb(&(oftPtr->fcb));
	initFdt(&(oftPtr->fdt));

	oftPtr->isUse = FALSE;

	return oftPtr;
}



/**
 * Load FCB To OFT |加载FCB中的信息到OFT中来
 * @param oftPtr [description]
 * @param fcbPtr [description]
 */
void loadFcb2Oft(OFT *oftPtr,FCB *fcbPtr){
	// copy memory : fcb to OFT->fcb
	memcpy(&(oftPtr->fcb),fcbPtr,sizeof(FCB));
}

/* 加载 Oft中的信息到Fcb中来 */
void loadOft2Fcb(OFT *oftPtr,FCB *fcbPtr){
    memcpy(fcbPtr,&(oftPtr->fcb),sizeof(FCB));
}

/**
 * 获取OFTList中空闲的OFT
 * @param  OFTList [description]
 * @param  OFTNum  [description]
 * @return         [description]
 */
int getNextOft(OFT * OftList){
	int i;

	for(i = 0; i < OFT_NUM; i++){
		if(OftList[i].isUse == FALSE){
			OftList[i].isUse == TRUE;
			return i;
		}
	}

	return OBJECT_NOT_FOUND;
}

