/**
 * @file XShmArray.h
 * @since 2022/3/15
 */

#ifndef INCLUDE_BASE_XSHMARRAY_H_
#define INCLUDE_BASE_XSHMARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "XShmLib.h"

/**
|写位置0:第一位,1:第二位;意思为当此位为0时,当前可写,1位置为可读;当此位置为1时,0位置可读，1位置可写;
正在读的时候,更新为写的位置，读要加锁
多读的情况不加锁,存在可能写的情况
*/

/**
 * @brief 数据内容
 */
typedef struct _ArrayNode {
	XChar rw;				/**< 读写控制位 */
//	XChar w;
	XChar _field[7];
	XLong version;
	XLong _field2[6];       /**< 填充位 */
	XChar data[];			/**< 数据 */
} ArrayNodeT, *pArrayNodeT;

#define XSHM_ARRAYNODE_SIZE                   sizeof(ArrayNodeT)

/**
 * @brief 数组类消息头
 \dot
 digraph structs {
    node [shape=plaintext]
    shminfo [label=<
<TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" BGCOLOR="bisque">
  <TR><TD>mode</TD><TD>type</TD><TD>dataSize</TD><TD>name</TD><TD>path</TD><TD>rows</TD><TD>cols</TD><TD>key</TD><TD>shmid</TD><TD port="info">iTotalSize</TD></TR>
</TABLE>>];

    header [label=<
<TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" BGCOLOR="green">
  <TR><TD port="h1">header</TD><TD>blockSize</TD><TD>blockNum</TD><TD>usedCount</TD><TD port="datas">datas</TD></TR>
</TABLE>>];
	data [label=<
	<TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" BGCOLOR="yellow">
  	  <TR><TD port="d1">rw</TD><TD>data</TD><TD>rw</TD><TD>data</TD><TD COLSPAN="2">......</TD><TD>rw</TD><TD>data</TD></TR>
	</TABLE>>];
	shminfo:info -> header:h1;
    header:datas -> data:d1;
}
 \enddot
 */
typedef struct _XShmArrayT {
	XShmHeaderT header;					/**< 内存信息头 */
	XUInt blockSize;					/**< 数据区块大小 */
	XChar _field[4];
	XLLong usedCount;					/**< 当前已占用 */
//	XLLong _lastDelIdx;					/**< 最后删除位置 */
	XVoid* datas;						/**< 数据区,数据head + data */
} XShmArrayT, *pXShmArrayT;

#define XSHM_ARRAY_SIZE                       sizeof(XShmArrayT)

/**
 * @brief 数组类共享内存数据创建
 * @param pXShmHeader
 * @return XShmArrayT* 内存地址
 */
extern XShmArrayT* XShmArrayInit(XShmHeaderT* pXShmHeader);

/**
 * @brief 数组类共享内存挂接
 * @param pXShmHeader
 * @return XShmArrayT* 内存地址
 */
extern XShmArrayT* XShmArrayAttach(XShmHeaderT* pXShmHeader);

/**
 * @brief  通过大块内存获取数据使用块
 * @note   
 * @param  pShared: 大块内存首地址
 * @param  offset: 使用偏移位置
 * @param  pXShmHeader: 数据使用信息
 * @retval 
 */
extern XShmArrayT* XShmArrayAttachByOffset(void* pShared, XULong offset, XShmHeaderT* pXShmHeader);

/**
 * @brief 数组数据插入
 * @param array
 * @param data
 * @return 0：插入正确，-1插入失败
 */
extern XInt XShmArrayInsert(XShmArrayT* array, XLLong index, void* data);

/**
 * @brief 数组数据插入
 * @param array
 * @param data
 * @return 
 */
extern XVoid* XShmArrayVInsert(XShmArrayT* array, XLLong index, void* data);

/**
 * @brief 数组数据插入
 * @param array
 * @param data
 * @return 0：插入正确，-1插入失败
 */
extern XInt XShmArrayInsertBySize(XShmArrayT* array, XLLong index, void* data, XInt size);

/**
 * @brief 数组数据插入
 * @param array
 * @param data
 * @return 0：插入正确，-1插入失败
 */
extern XVoid* XShmArrayInsertVBySize(XShmArrayT* array, XLLong index, void* data, XInt size);

/**
 * @brief 获取当前可以插入位置
 * @param array
 * @param index -- 插入索引
 * @return 插入内存地址
 */
extern XVoid* XShmArrayGet(XShmArrayT *array, XLLong index);

extern XVoid* XShmArrayFind2(XShmArrayT* array, XLLong index, XVoid* out);

/**
 * @brief 根据插入地址，找到插入数据
 * @param array
 * @param index
 * @return 数据地址
 */
extern XVoid* XShmArrayFind(XShmArrayT* array, XLLong index);

/**
 * @brief 根据地址更新插入数据
 * @param array
 * @param index
 * @param data
 * @return 0:更新成功，-1：更新失败
 */
extern XInt XShmArrayUpdate(XShmArrayT* array, XLLong index, XVoid* data);


/**
 * @brief 根据地址更新插入数据
 * @param array
 * @param index
 * @param data
 * @return 
 */
extern XVoid* XShmArrayUpdateV(XShmArrayT* array, XLLong index, XVoid* data);

/**
 * @brief 更新数据
 * 
 * @param array 
 * @param index 
 * @param data 
 * @param size 
 * @return XInt 
 */
extern XInt XShmArrayUpdateBySize(XShmArrayT* array, XLLong index, XVoid* data, XInt size);

/**
 * @brief 更新数据
 * 
 * @param array 
 * @param index 
 * @param data 
 * @param size 
 * @return  
 */
extern XVoid* XShmArrayUpdateVBySize(XShmArrayT* array, XLLong index, XVoid* data, XInt size);

/**
 * @brief 根据内存索引删除数据
 * @param array
 * @param index
 * @return 0:删除成功，-1：删除失败
 */
extern XInt XShmArrayDelete(XShmArrayT* array, XLLong index);

#ifdef __cplusplus
};
#endif

#endif /* INCLUDE_BASE_XSHMARRAY_H_ */
