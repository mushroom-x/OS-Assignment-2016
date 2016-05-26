#include "Common.h"
#include "SysStructure.h"

unsigned char Vhd[VHD_SIZE];
unsigned char VhdBuffer[VHD_SIZE];
unsigned char *VhdPtr = Vhd;
unsigned char *VhdDataBlockPtr;

int currOftIndex = 0;
OFT OftList[OFT_NUM];

void my_sys_start();               // 进入文件系统
void my_sys_exit();                // 退出文件系统
void my_format();                  // 磁盘格式化函数
void my_mkdir(char *dirname);      // 创建子目录
void my_rmdir(char *dirname);      // 删除子目录
void my_ls();                      // 显示目录中的内容
void my_cd(char *dirname);         // 用于更改当前目录
int  my_create(char *filename);    // 创建文件
void my_rm(char *filename);        // 删除文件
int  my_open(char *filename);      // 打开文件
int  my_close(int fd);             // 关闭文件
int  my_write(int fd);             // 写文件
int  my_read(int fd);              // 读文件

int  do_write(int fd, char *text, int len, char wstyle);
int  do_read(int fd, int len, char *text);

void saveHdFile();
void readHdFile();
void excuteCmd(char *cmdLine);
int getIndexOfCmd(char *cmdStr);
void help();
void initOftList();




int main(){
    //my_format();
    my_sys_start();
    char cmdLine[CMD_LEN];

    while(TRUE){
        printf("%s > ",OftList[currOftIndex].fdt.dirName);
        gets(cmdLine);
        excuteCmd(cmdLine);
    }

    return 0;
}


void my_sys_start(){

    readHdFile();

    initOftList();

    BootBlock * bootBlock = (BootBlock *)getBlockPtrByBlockNo(VhdPtr,BLOCK_INDEX_BOOT_BLOCK);

    VhdDataBlockPtr = bootBlock->dataBlockPtr;

    currOftIndex = 0;

    cout << bootBlock->sysInfo << endl;

}


void my_sys_exit(){

    while(currOftIndex){
        my_close(currOftIndex);
    }

    saveHdFile();
}


void my_format(){
    initBootBlock(VhdPtr);
    initFat1(VhdPtr);
    updateFat2(VhdPtr);
    initRootBlock(VhdPtr);

    saveHdFile();
}


void my_mkdir(char *dirname){

}

void my_rmdir(char *dirname){

}

void my_ls(){

    if(OftList[currOftIndex].fcb.metadata == MD_DATA_FILE){
        cout << "Data File Could Not be ls" << endl;
        return;
    }

    unsigned char blockBuf[VHD_BLOCK_SIZE];
    //void readVhdFile(unsigned char *VhdPtr,int blockNo,unsigned char *blockBuf){
    readVhdFile(VhdPtr,OftList[currOftIndex].fcb.blockNo,blockBuf);

    FCB * fcbPtr = (FCB *)blockBuf;

    int fcbNum = VHD_BLOCK_SIZE / sizeof(FCB);
    for(int i=0; i< fcbNum; i++,fcbPtr++){

        if(fcbPtr->isUse == TRUE){
            printFcbInfo(fcbPtr);
        }
    }
}

void my_cd(char *dirname){

}

int  my_create(char *filename){
    return 0;
}

void my_rm(char *filename){

}

int  my_open(char *filename){
    return 0;
}

int  my_close(int fd){

    if(fd >= OFT_NUM || fd < 0){
        cout << "Error : Open File Not Exist " << endl;
        return FAILURE;
    }

    int parentOftIndex = getParentOft(OftList,currOftIndex);
    if(parentOftIndex == OBJECT_NOT_FOUND){
        cout << "Error : Parent File Not Exist" << endl;
        return FAILURE;
    }

    if(OftList[fd].fdt.isUpate == TRUE){
        //TODO
    }
    return 0;
}

int  my_write(int fd){
    return 0;
}

int  my_read(int fd){
    return 0;
}

int  do_write(int fd, char *text, int len, char wstyle){
    return 0;
}

int  do_read(int fd, int len, char *text){
    return 0;
}


/* 保存整个虚拟磁盘文件 */
/* Save Hard Disk File */
void saveHdFile(){
    FILE *filePtr = fopen(FileSysName,"w");
	fwrite(VhdPtr,VHD_SIZE,1,filePtr);
	fclose(filePtr);
}

/* Read Hard Disk File*/
void readHdFile(){

    FILE * sysFile = fopen(FileSysName,"r");

	if(sysFile != NULL){
		// Write VhdBuffer from sysFile
		fread(VhdBuffer,VHD_SIZE,1,sysFile);
		fclose(sysFile);
		// FileSysMagicNum: "10101010"
		if(memcmp(VhdBuffer,FileSysMagicNum,MAGIC_NUM_LEN) == 0){

			saveVhdFile(VhdPtr,VhdBuffer);
			cout << "Success: FileSys.txt Read" <<endl;
			cout << "Update VHD from VhdBuffer" <<endl;
		}
		else{
			cout << "Error: Magic Number Not Match" <<endl;
			my_format();
            saveVhdFile(VhdBuffer,VhdPtr);
		}
	}
	else{
		cout << "Error: FileSys.txt is not exist!" <<endl;
		cout << "Now FileSys.txt will be create" << endl;
		my_format();
		saveVhdFile(VhdBuffer,VhdPtr);
	}
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
                my_open(arg2);
            }
            break;
        case 7:
            // close
            if(OftList[currOftIndex].fcb.metadata == MD_DATA_FILE){
                my_close(currOftIndex);
            }
            else{
                cout <<  "Current file is directory ,could not be close" << endl;
            }
            break;
        case 8:
            // write
            if(OftList[currOftIndex].fcb.metadata == MD_DATA_FILE){
                my_write(currOftIndex);
            }
            else{
                cout <<  "Current file is directory ,could not be write" << endl;
            }
            break;
        case 9:
            //read
            if(OftList[currOftIndex].fcb.metadata == MD_DATA_FILE){
                my_read(currOftIndex);
            }
            else{
                cout <<  "Current file is directory ,could not be read" << endl;
            }
            break;
        case 10:
            // exit
            my_sys_exit();
            cout << "Exist File System " <<endl;
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
            cout << "No such Command"<< endl;
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

/* During my_sys_start*/
 void initOftList(){
   int i;

   initRootOft(VhdPtr,OftList);

   for (i = 1; i < OFT_NUM; i++){
        initOft(OftList + i);
   }
 }
