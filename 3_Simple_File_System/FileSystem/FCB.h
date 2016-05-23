#ifndef _FCB_H_
#define _FCB_H_

#include "Common.h"


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

//Directory Entry ，
									//
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

void setFcbTime(FCB *fcb);
void initFcb(FCB *fcbPtr);
void initFcb(FCB * fcbPtr,char *fname,char *ename,unsigned char metadata);
int fileNameMatch(FCB * fcbPtr,char *fname,char *ename);
void printDirFile(FCB *fcbPtr);
void printDataFile(FCB *fcbPtr);


#endif
