/**
 * @file XShmRaw.h
 * @since 2022/3/15
 */

#ifndef INCLUDE_BASE_XSHMRAW_H_
#define INCLUDE_BASE_XSHMRAW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "XShmLib.h"

/**
 * @brief 数组类消息头
 */
typedef struct _XShmRawT {
	XShmHeaderT header;					/**< 内存创建信息 */
	XUInt blockSize;					/**< 数据区块大小 */
	XChar _field[4];
	XLLong usedCount;					/**< 当前已占用 */
	XVoid* datas;						/**< 数据区,数据head + data */
} XShmRawT, *pXShmRawT;

#define XSHM_RAW_SIZE                       sizeof(XShmRawT)

/**
 * @brief 数组类共享内存数据创建
 * @param pXShmHeader
 * @return XShmRawT* 内存地址
 */
extern XShmRawT* XShmRawInit(XShmHeaderT* pXShmHeader);

/**
 * @brief 数组类共享内存挂接
 * @param pXShmHeader
 * @return XShmRawT* 内存地址
 */
extern XShmRawT* XShmRawAttach(XShmHeaderT* pXShmHeader);

/**
 * @brief  通过大块内存获取数据使用块
 * @note   
 * @param  pShared: 大块内存首地址
 * @param  offset: 使用偏移位置
 * @param  pXShmHeader: 数据使用信息
 * @retval 
 */
extern XShmRawT* XShmRawAttachByOffset(void* pShared, XULong offset, XShmHeaderT* pXShmHeader);

/**
 * @brief 数组数据插入
 * @param array
 * @param data
 * @return 0：插入正确，-1插入失败
 */
extern XInt XShmRawInsert(XShmRawT* array, XLLong index, void* data);

/**
 * @brief 数组数据插入
 * @param array
 * @param data
 * @return 0：插入正确，-1插入失败
 */
extern XInt XShmRawInsertBySize(XShmRawT* array, XLLong index, void* data, XInt size);

/**
 * @brief 获取当前可以插入位置
 * @param array
 * @param index -- 插入索引
 * @return 插入内存地址
 */
extern XVoid* XShmRawGet(XShmRawT *array, XLLong index);

/**
 * @brief 根据插入地址，找到插入数据
 * @param array
 * @param index
 * @return 数据地址
 */
extern XVoid* XShmRawFind(XShmRawT* array, XLLong index);

/**
 * @brief 根据地址更新插入数据
 * @param array
 * @param index
 * @param data
 * @return 0:更新成功，-1：更新失败
 */
extern XInt XShmRawUpdate(XShmRawT* array, XLLong index, XVoid* data);

/**
 * @brief 更新数据
 * 
 * @param array 
 * @param index 
 * @param data 
 * @param size 
 * @return XInt 
 */
extern XInt XShmRawUpdateBySize(XShmRawT* array, XLLong index, XVoid* data, XInt size);

/**
 * @brief 根据内存索引删除数据
 * @param array
 * @param index
 * @return 0:删除成功，-1：删除失败
 */
extern XInt XShmRawDelete(XShmRawT* array, XLLong index);

#ifdef __cplusplus
};
#endif

#endif /* INCLUDE_BASE_XShmRaw_H_ */
