#ifndef _MAIN_H_
#define _MAIN_H_






// System

void my_format();

void my_start_sys();

void my_exit_sys();




// Directory
int my_cd(char *dirname);

int my_ls();

int my_mkdir(char *dirname);

int my_rmdir(char *dirname);

// File

int my_create(char *filename);

int my_close(int iOft);

int my_rm(char *filename);

int my_read(int iOft);

int my_write(int iOft);

int do_read(OFT* oftPtr,int len,char *text);

int do_write(OFT * oftPtr,char *text,int len,char wstyle);

void writePreProcess(OFT*  oftPtr,char wstyle);




// DIY
unsigned char * getBlockBuffer();

void releaseBlock(int blockNo);

int initOftByFileName(OFT *OftList,int piOft,char * filename,unsigned char metadata);

int getIndexOfFcb(OFT *oftPtr,char * fname,char *ename,unsigned char metadata);





void excuteCmd(char *cmdLine);

int getIndexOfCmd(char *cmdStr);


void help();

void readVhdFile(unsigned char * VhdPtr);

void saveVhdFile(unsigned char *VhdPtr);


#endif
