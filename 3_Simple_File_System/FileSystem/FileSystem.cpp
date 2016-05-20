#include "Common.h"
#include "FileSystem.h"
#include "OFT.h"

/*VHD: Virtual Hard Disk */
unsigned char *VHDPtr;
unsigned char VHDBuffer[VHD_SIZE];		
unsigned char *VHDDataBlockPtr;				//the start position  of data block for VHD

/*OFT:Open File Table*/
OFT OFTList[OFT_NUM];
int currOFTIndex = 0;

int main()
{
	
}





int do_read(OFT* oftPtr,int len,char *text){

	int lenTmp = len;
	//TODO VHD_BLOCK_SIZE = 1024
	unsigned char * buf = (unsigned char *)malloc(VHD_BLOCK_SIZE);

	// TODO Wiki for offset
	int offset = oftPtr->filePtr;
	int blockNo = oftPtr->fcb.blockNo;

	FAT * fatPtr = getFATPtrByBlockNo(VHDPtr,blockNo);
	unsigned char * blockPtr;

	if(buf == NULL){
		cout << "Error: do_read - malloc error" << endl;
		return EXIT_FAILURE;
	}

	// TODO  Why there is offset >= VHD_BLOCK_SIZE
	while(offset >= VHD_BLOCK_SIZE){
		offset -= VHD_BLOCK_SIZE;
		blockNo = fatPtr->id;			//index to 
	
		if(blockNo == VHD_BLOCK_FILE_END){
			cout << "Error: do_read no such block..." << endl;

			return EXIT_FAILURE;
		}

		fatPtr = getFAT1(VHDPtr) + blockNo;
	}

	blockPtr = getBlockPtrByBlockNo(VHDPtr,blockNo);

	// Read File To Buffer
	memcpy(buf,blockPtr,VHD_BLOCK_SIZE);


	char * textPtr = text;
	//FCB * debug = (FCB *)text;

	while(len > 0){

		// One Disk Could Save all 
		if( VHD_BLOCK_SIZE - offset > len){

			memcpy(textPtr,buf + offset,len);
			textPtr += len;
			offset += len;		//? TODO why offset need to add to len
			oftPtr->filePtr += len;
			len = 0;
		}
		else{

			memcpy(textPtr,buf + offset,VHD_BLOCK_SIZE - offset);
			textPtr += (VHD_BLOCK_SIZE - offset);
			offset = 0;
			len -= (VHD_BLOCK_SIZE - offset);

			// find next block
			blockNo = fatPtr->id;

			if(blockNo == VHD_BLOCK_FILE_END){
				cout << "Error: do_read too long "<< endl;
				break;
			}

			fatPtr = getFATPtrByBlockNo(VHDPtr,blockNo);
			blockPtr = getBlockPtrByBlockNo(VHDPtr,blockNo);

			// TODO
			memcpy(buf,blockPtr,VHD_BLOCK_SIZE);
		}

	}
	free(buf);

	return lenTmp - len;
}


/*
#define WS_REWRITE 0
#define WS_OVERWRITE 1
#define WS_APPEND 2 
*/

int do_write(OFT * oftPtr,char *text,int len,char wstyle){

	int blockNo = oftPtr.blockNo;

	FAT1 * fat1 = getFAT1(VHDPtr);
	FAT * fatPtr = getFATPtrByBlockNo(VHDPtr,blockNo);

	preProcessWrite(OFTList,iOFT,wstyle);

	int offset = oftPtr->filePtr;

	while(offset >= VHD_BLOCK_SIZE){
		blockNo = fatPtr->id;

		if(blockNo == VHD_BLOCK_FILE_END){
			blockNum = getFreeBlock();
		}
	}

}

void preProcessWrite(OFT*  oftPtr,char wstyle){
		// Preprocess
	switch(wstyle){
		/*
		* REWRITE: Write the file from the begining of the FCB
		*/
		case WS_REWRITE:
			oftPtr->filePtr = 0;
			oftPtr->fcb.length = 0;
			break;
		/*
		* Overwrite  ? is that necessary
		*/
		case WS_OVERWRITE:
			/* delete the end of the file's \0  */
			if(OoftPtr->fcb.metadata == MD_DATA_FILE && oftPtr->fcb.length != 0){
				// ?TODO not right
				oftPtr->filePtr -= 1;
			}
			break;

		/*
		*  Append : appending data to the File
		*/
		case WS_APPEND:
			
			if(oftPtr->fcb.metadata == MD_DIR_FILE){
				oftPtr->filePtr = oftPtr->fcb.length;
			}
			else if(oftPtr->fcb.metadata == MD_DATA_FILE && oftPtr->fcb.length != 0){
				/* delete the end of the file's \0  */
				oftPtr->filePtr = oftPtr->fcb.length - 1;
			}
			break;
	}
}



void my_format(){
	
	initBootBlock(VHDPtr);
	
	initFAT1AndFAT2(VHDPtr);

	initFCBRootDir(VHDPtr);

	saveVHDFile(VHDPtr);	
}


void my_start_sys(){

	VHDPtr = (unsigned char *)malloc(VHD_SIZE);

	readVHDFile();

	initRootOFT(VHDPtr);

	BootBlock * bootBlock = getBootBlock(VHDPtr);

	// set the pointer to the Data Area (=root block pointer)
	VHDDataBlockPtr = bootBlock->dataBlockNo;
	//Set Current OFT is this OFT
	currOFTIndex = 0;
}


/*
* 心累, 还是解释下, 当currfd=0的时候,也就是根节点, 它是不用更新的
* 因为我们之前在任何目录下mkdir或者create时,已经把length的变化写到目录文件下,名字叫.的目录项里了
* 即任何目录文件的'.'目录项都是实时更新的,但是他的父目录文件的没有被更新,所以需要一个个close
* 但是根目录没有父目录文件, 所以不需要close, 此处也许比较难理解, 大家随缘吧, 不过努力理解下,还是能懂的
*/
void my_exit_sys(){
	
	while(currOFTIndex){
		my_close(currOFTIndex);
	}

	saveVHDFile();
}



int my_close(int iOFT)
{
	//cheack if index of OFT is legal
	if(iOFT > OFT_NUM || iOFT < 0){
		count << "Error : No such file exist.." << endl;
		return EXIT_FAILURE;
	}
	else{
		
		int parentOFT = findParentDir(iOFT);
		if(parentOFT == -1){
			cout << "Father Directory is not exist" <<endl;
			return -1;
		}

		// if FCB Change 
		if(OFTList[iOFT].isFCBChange == TRUE){
			char FCBBuffer[MAX_BUF_SIZE];
			// P:674
			//Parent Directory:
			// 1. . Read Disk -> Buffer
			//do_read()

			//update pcb

			// 2.new FCB -> Buffer
			
			// 3.Buffer -> Disk
			// do_write()
		}

		recycleOFT(OFTList+iOFT);
		//update current OFTIndex
		currOFTIndex = parentOFT;
		return parentOFT;
	}
}



void readVHDFile(){
	FILE * sysFile;

	sysFile = fopen(FileSysName,"r");
	if(sysFile != NULL){
		// Write VHDBuffer from sysFile
		fread(VHDBuffer,VHD_SIZE,1,sysFile);
		fclose(sysFile);
		// FileSysMagicNum: "10101010"
		if(memcmp(VHDBuffer,FileSysMagicNum,MAGIC_NUM_LEN) == 0){
			// VHD  <- VHDBuffer
			memcpy(VHDPtr,VHDBuffer,VHD_SIZE);
			cout << "Success: FileSys.txt Read" <<endl;
			cour << "Update VHD from VHDBuffer" <<endl;
		}
		else{
			cout << "Error: Magic Number Not Match"<<endl;
			my_format();
			// VHDBuffer <- VHD
			memcpy(VHDBuffer,VHDPtr,VHD_SIZE);

		}
	}
	else{
		cout << "Error: FileSys.txt is not exist!"<<endl;
		cout << "Now FileSys.txt will be create" << endl;
		my_format();
		// VHDBuffer <- VHD
		memcpy(VHDBuffer,VHDPtr,VHD_SIZE);
	}
}


void saveVHDFile(unsigned char *VHDPtr){
	FILE *filePtr = fopen(FileSysName,"w");
	/* TODO what is 1 stand for? */
	fwrite(VHDPtr,VHD_SIZE,1,filePtr);
	fclose(filePtr);
}


unsigned char * getBlockPtrByBlockNo(unsigned char *VHDPtr,int blockNo)
{
	return VHDPtr + VHD_BLOCK_SIZE * blockNo;
}