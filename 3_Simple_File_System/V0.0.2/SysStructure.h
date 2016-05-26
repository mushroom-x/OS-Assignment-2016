#ifndef _SYS_STRUCTURE_H_
#define _SYS_STRUCTURE_H_

#include "Common.h"



/**
 * FAT File System Structure
 * FAT 文件系统结构
 *
 * |Start| End |	Name	|	Note			|
 * |-----|-----|------------|-------------------|
 * |  0  |  0  | Boot Block | 引导区			|
 * |-----|-----|------------|-------------------|
 * |  1  |  2  | 	FAT1	| 文件分配表 		|
 * |-----|-----|------------|-------------------|
 * |  3  |  4  | 	FAT2	| 文件分配表-备份 	|
 * |-----|-----|------------|-------------------|
 * |  5  |1000 |  DATA BLOCK| 数据区			|
 * |-----|-----|------------|-------------------|
 * |  5  |  5  | ROOT_DIR	| 根目录文件		|
 *
 */

/**
* Boot Block | 引导块
*
* @note
* 	引导块占用第0号物理块，不属于文件系统管辖，
* 	如果系统中有多个文件系统，只有根文件系统才
* 	有引导程序放在引导块中，其余文件系统都不
* 	使用引导块
*
*/
typedef struct BootBlock
{
	/**
	 * Magic Number |　魔数
	 *
	 * @value
	 * 	10101010
	 */
	char magicNum[MAGIC_NUM_LEN];

	/**
	 * System Information | 系统信息
	 */
	char sysInfo[SYS_INFO_LEN];

	/**
	 * Root Directory Block Number |根目录文件的盘块号
	 */
	unsigned short rootBlockNo;

	/**
	 * Data Area VHD Block Pointer | 指向数据区首盘块的指针
	 *
	 */
	unsigned char *dataBlockPtr;

}BootBlock;

/**
* FAT : File Allocation Table
*/
typedef struct FAT
{
	unsigned short id;
} FAT;

/**
 * @name
 * File Control Block |　文件控制块
 * @note
 * 用户存储文件的描述和控制信息的数据结构
 * 常用的有FCB和iNode,在FAT文件系统中使用的是FCB
 * 文件与文件控制块一一对应关系。
 *-------------------------------------------------
 * @name
 * File Directoty | 目录文件
 * @note
 * 文件控制块的有序集合,可以理解成好多个FCB的数组
 *
 *
 * -------------------------------------------------
 * @name
 * File Directoty Entry| 文件的目录项
 * @note
 * 	一个FCB就是一个文件目录项
 *  文件目录项分类：
 *  	1. 卷标目录项
 *  	2. "." 目录项
 *  	3. ".."目录项
 *  	4. 短文件名目录项*
 *  	5. 尝文件名目录项
 *
 * 	短文件名目录项是最重要的数据结构,其中存放着有关
 * 	子目录或文件的短文件名、属性、起始簇号、时间值
 * 	以及内容大小等信息。
 * 	@link
 * 	http://blog.csdn.net/eastonwoo/article/details/8450060
 * -------------------------------------------------
 */
typedef struct FCB
{
	/**
	 * File Name | 文件名称
	 * @length 8B
	 */
	char fileName[FILE_NAME_LEN];				//file name

	/**
	 * Filename Extension | 文件拓展名
	 * @length  3B
	 */
	char fileNameExten[FILE_NAME_EXTEN_LEN];	//

	/**
	 * Metadata | 元数据
	 * @note 元数据[metadata] 等同于 文件属性[attribute]
	 * 每一位代表文件是具备该属性(具备：1 不具备：0)
	 * 目前实现的文件系统里面只用到了其中一位用于表示是数据文件还是目录文件
	 * @length 1B
	 * @value
	 * 	MD_DATA_FILE	: 数据文件
	 * 	MD_DIR_FILE  	: 目录文件
	 *
	 */
	unsigned char metadata;

	/**
	 * Create Time | 文件创建时间
	 *
	 * @note
	 * 	root->time 是unsigned short int类型的数据, 32位编译器下长16位,64位编译器下长32位
	 * 	这里考虑32位编译器, 所以完整的表示一个时间的秒数是不够长的,所以,保存秒数的一半
	 * 	这样小时占5位,分钟占6位,秒占5位
	 * @value
	 * @length
	 *
	 */
	unsigned short createTime;

	/**
	 * Create Date | 创建日期
	 * @note
	 *  年份我们保存的是实际值-2000, time->tm_mon要+1是因为,这个结构设计的时候0代表1月
	 *  同理,年份占7位,月份占4位,日期占5位
	 * @value
	 *
	 */
  	unsigned short createDate;

  	/**
  	 * Block Number | 盘块号
  	 * @note FCB的首盘块号
  	 * @length 2B
  	 */
	unsigned short blockNo;						//this file's the first Block Number


	/**
	 * File Length |　文件大小/长度
	 * @length 4B
	 */
	unsigned long length;


	/**
	 * Is Free | 是否被占用
	 * @value
	 * 	TRUE  : 正在被使用
	 * 	FALSE : FCB空闲
	 * @length 1B
	 */
	char isUse;
}FCB;


/**
 * File Discriptor Table | 文件描述符表
 * @note
 *
 * @wiki
 * 文件描述符、文件描述符表、打开文件表、目录项、索引节点之间的联系
 * http://www.cnblogs.com/yanenquan/p/4614805.html
 *
 */
typedef struct FDT{

	/**
	 * File Pointer | 文件读写指针 / 游标
	 * @note
	 * 	用于记录读取的位置
	 */
	int  filePtr;

	/**
	 * Is Update | 是否更新
	 * @note
	 * 	FCB是否更新过
	 * @value
	 * 	TRUE	: 跟新过
	 * 	FALSE	: 未更新
	 */
	char isUpate;				//to Record the Change of FCB-> Change:TRUE

	/**
	 * Parent Directory File's Block Number
	 * @note
	 * 	当前文件的父目录文件所在的盘块号
	 *
	 */
	int  pdfBlockNo;

	/**
	 *
	 * File Directory Entry Number
	 * @note
	 * 	 当前文件的父目录文件中的目录项的序号
	 * 	 即该文件在父目录文件中对应的FCB序号
	 * @name
	 * File Directory Entry 目录项
	 * 参见FCB.h中的介绍
	 */
	int  fdEntryNo;				// Parent Dirrectory Entry Number in parentDirBlockNo

	/**
	 * Directory Name | 路径名称
	 * @demo
	 * 	\root\scropion\
	 * @length
	 * 	80
	 */
	char dirName[DIR_NAME_LEN];
}FDT;

/*
*  Open File Table | 打开文件表
*  @note
*  	打开文件表包含两部分 ： 用户文件描述表 和 内存FCB表
*
*/
typedef struct OFT
{
	/**
	 * File Control Block | 文件控制块
	 */
	FCB fcb;

	/**
	 * File Discriptor Table | 文件描述符表
	 */
	FDT fdt;

	/*
	* Is Use |是否被占用
	* @note
	* 	当前OFT是否被占用
	* @value
	* 	TRUE	: 正在被使用
	* 	FALSE	: OFT空闲
	*/
	char isUse;
}OFT;



/**
 * IO Operation
 */

/* 保存整个虚拟磁盘文件 */
/* Save Hard Disk File */
//void saveHdFile(unsigned char *VhdPtr)；

/* Read Hard Disk File*/
//void readHdFile(unsigned char *VhdPtr)；
/* 保存整个虚拟磁盘文件 */
void saveVhdFile(unsigned char *VhdPtr,unsigned char *vhdBuf);

/* 保存某个盘块 */
void saveVhdFile(unsigned char *VhdPtr,int blockNo,unsigned char *blockBuf);

/* 保存某个盘块的对应偏移量offset的FCB*/
void saveVhdFile(unsigned char *VhdPtr,int blockNo,int offset,unsigned char *fcbBuf);

/* 读取整个虚拟磁盘文件到Vhd Buffer*/
void readVhdFile(unsigned char *VhdPtr,unsigned char *vhdBuf);

/* 读取整个盘块到对应的Block Buffer */
void readVhdFile(unsigned char *VhdPtr,int blockNo,unsigned char *blockBuf);

/* 读取FCB到对应的fcbBuf*/
void readVhdFile(unsigned char *VhdPtr,int blockNo,int offset,unsigned char *fcbBuf);



/**
 * Boot Block
 */
/* 初始化引导块 */
void initBootBlock(unsigned char *VhdPtr);


/**
 * Root Block
 */

/* 初始化根目录的磁盘块 */
void initRootBlock(unsigned char *VhdPtr);


/**
 * FAT
 * File Allocation Table
 */

/* 初始化FAT1 */
void initFat1(unsigned char *VhdPtr);

/* 同步FAT2 */
void updateFat2(unsigned char *VhdPtr);


/* 获取FAT1的FAT指针*/
FAT * getFat1Ptr(unsigned char *VhdPtr);

/* 根据盘块号获取盘块号对应的FAT的指针 */
FAT * getFatPtr(unsigned char *VhdPtr,int blockNo);

/* 根据盘块号和偏移量获取FCB的指针 : ? offset > fcbNum*/
FCB * getFcbPtr(unsigned char *VhdPtr,int blockNo,int offset);

/* 根据盘块号和偏移量获取FCB的指针 : ? offset > fcbNum */
//FCB * getNextFcbPtr(unsigned char *VhdPtr,int blockNo,int offset);

/*　获取下一个空闲的盘块 并更新FAT表　*/
int getNextBlock(unsigned char *VhdPtr,int blockNo);

/*  释放占据的盘块*/
void freeBlock(unsigned char *VhdPtr,int blockNo);


/**
 * FCB
 * File Control Block
 */

/* 初始化FCB的时间和日期 */
void setFcbDateTime(FCB *fcbPtr);

/* 初始化空闲的FCB */
void initFcb(FCB *fcbPtr);

/* 根据文件名和类型初始化FCB */
void initFcb(FCB *fcbPtr,char *fname,char *ename,unsigned char metadata);

/* 匹配文件名和文件类型是否匹配 */
int fileIsEqual(FCB * fcbPtr,char *fname,char *ename,unsigned char metadata);

/* 在当前block中 申请下一个空闲的FCB | 当前的OFT 默认为目录文件 */
int getNextFcb(unsigned char * VhdPtr,int blockNo);

/* 打印文件信息 | 根据文件类型选择不同的打印方式  */
void printFcbInfo(FCB *fcbPtr);


void initDirectoryBlock(unsigned char *VhdPtr,int curBlockNo,int parentBlockNo);

/**
 * FDT
 * Fild Descriptor Table
 */

void initFdt(FDT *fdtPtr);

/**
 * OFT
 * Open File Table
 */
void initOft(OFT *oftPtr);
 
void initOft(OFT * OftList,int curOftIndex,int parOftIndex,int fdEntryNo,FCB *fcbPtr);

void initRootOft(unsigned char* VhdPtr,OFT *OftList);

/* 获取父打开文件在OftList的Index*/
int getParentOft(OFT *OftList,int curOft);

/* 初始化根目录文件的打开文件 */
void initRootOft(unsigned char* VhdPtr,OFT *OftList);

/* 将OFT中的信息拷贝到FCB中*/
void cloneFcb2Oft(OFT *OftPtr,FCB *pcbPtr);

/* 将Oft中的FCB信息保存到FCB中*/
void cloneOft2Fcb(OFT *oftPtr,FCB *fcbPtr);

unsigned char * getBlockPtrByBlockNo(unsigned char *VHDPtr,int blockNo);


#endif
