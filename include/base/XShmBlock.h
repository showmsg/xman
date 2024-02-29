/**
 * @file XShmBlock.h
 * @since 2022/3/28
 */

#ifndef INCLUDE_BASE_XSHMBLOCK_H_
#define INCLUDE_BASE_XSHMBLOCK_H_

#include "XShmLib.h"

#define XARRAY_EXTEND_NUM            (4)             /** 最多支持4个扩展区 */
typedef struct _BlockHead {
	XInt extend[XARRAY_EXTEND_NUM]; /*扩展区使用情况,记录扩展区的位置，指示扩展区的对应位置*/
	XLong used; /* 同类数据存储个数 */
	XULong lastDelIdx; /* 最后数据删除位置 */
	XULong hash; /* 主键 */
} XBlockHeadT, *pBlockHeadT;
typedef struct _BlockData {
	XChar rw;				//读写控制位
	XChar _field[3];		//扩展区
	XChar data[];			//数据
} XBlockDataT, *pXBlockDataT;

/**
 *             |--head--|------ block    ----------|---------- block ---------|-------- extend1  ------|------- extend2 --------|
 *  head           指示扩展区使用起始位置,扩展区的数量由XARRAY_EXTEND_NUM决定，每个数据的扩展区个数由extendNum决定
 *  block  基础好数据区，每个数据由读写位+数据组成
 *  extennd 扩展区，每个数据由读写位+数据组成，区别为blockNum不同
 *
 * |ArrayHeader|DataHead|data|data|data|......|data|data|data|data|......|data|data2|data2|......|data2|data2|data2|......|data2|
 */

typedef struct _XShmBlockT {
	XShmHeaderT header;
	XUInt dataSize;						//数据区大小

	XUInt dataNum;						//数据区容量，每个block中对应的存放数据的个数
	XUInt blockNum;						//基础block个数
	XLong usedBlock;					//已经使用的基础区数量

	XUInt extendDataNum;				//扩展区容量,扩展区每个block存放的数据的个数
	XUInt extendBlockNum;              //扩展区的block个数
	XUInt usedExtend;					//扩展区使用情况

	XLong usedCount;					//当前已占用,使用的数据的数量

	void* datas;						//数据区,数据head + data
} XShmBlockT, *pXShmBlockT;

#endif /* INCLUDE_BASE_XSHMBLOCK_H_ */
