#ifndef _FDT_H_
#define _FDT_H_

#include "Common.h"

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


FDT * initFdt(FDT* fdtPtr);
#endif
