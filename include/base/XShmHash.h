/*
 * @file XShmHash.h
 *
 *  Created on: 2022年3月15日
 *      Author: DELL
 */

#ifndef INCLUDE_BASE_XSHMHASH_H_
#define INCLUDE_BASE_XSHMHASH_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "XShmLib.h"

/**
 * @brief 存储数据头
 */
typedef struct _HashNode {
	XChar rw;				/**< 读写控制位  @see eXShmRw */
	XChar _field[7];		//填充位
	XULong key;				/**< key */
	XLLong   compare;        /**< 以compare作为链表 */
	XLong cur;				/**< 当前位置 */
	XLong pre;				/**< 前移位置 */
	XLong next;				/**< 后一位置 */
	XLong _field2[2];          /**< 填充位 */
	XChar value[];			/**< 数据区 */
} HashNodeT, *pHashNodeT;

#define XSHM_HASHNODE_SIZE                sizeof(HashNodeT)
#define XSHM_HASHROW_SIZE                 sizeof(ULong)

/**
 * @brief HashTable
 */
typedef struct _HashTable {
	XShmHeaderT header;
	XUInt cols;							/**< 可存储的最大数量 */
	XUInt blockSize;					/**< 数据区块大小 */
	XChar _field[4];
	XLong usedCount;					/**< 当前已占用 */
	XLong lastNodeIndex;				/**< 最新存储位置 */
	XULong* modTable;					/**< 素数数组 */
	XULong* modTotal;					/**< 素数累计值数组 */
	XVoid* datas;						/**< 数据区 */
} XHashTableT, *pXHashTableT;

#define XSHM_HASHTABLE_SIZE                   sizeof(XHashTableT)

/**
 * @brief 初始化内存
 * @param pXShmHeader
 * @return 返回内存地址
 */
extern XHashTableT* XShmHashInit(XShmHeaderT* pXShmHeader);

/**
 * @brief 挂接内存
 * @param pXShmHeader
 * @return 返回内存地址
 */
extern XHashTableT* XShmHashAttach(XShmHeaderT* pXShmHeader);

/**
 * @brief  通过大块内存获取数据使用块
 * @note   
 * @param  pShared: 大块内存首地址
 * @param  offset: 使用偏移位置
 * @param  pXShmHeader: 数据使用信息
 * @retval 
 */
extern XHashTableT* XShmHashAttachByOffset(void* pShared, XULong offset, XShmHeaderT* pXShmHeader);

/**
 * @brief 根据hahs查找数据是否存在
 * @param pHashTable
 * @param _key
 * @return 0:存在，-1：不存在
 */
extern XInt XShmHashSearch(XHashTableT* pHashTable, XULong _key, XLLong compare);

/**
 * @brief 根据hash查找数据
 * @param pHashTable
 * @param _key
 * @return 查找到的数据
 */
extern XVoid* XShmHashFind(XHashTableT* pHashTable, XULong _key, XLLong compare);

/**
 * @brief 插入hash数据
 * @param pHashTable
 * @param _key
 * @param _value
 * @return 0：更新成功，-1：更新失败
 */
extern XInt XShmHashInsert(XHashTableT* pHashTable, XULong _key, XLLong compare, XVoid* _value);

/**
 * @brief 更新数据
 * @param pHashTable
 * @param _key
 * @param _value
 * @return 0：更新成功，-1：更新失败
 */
extern XInt XShmHashUpdate(XHashTableT* pHashTable, XULong _key, XLLong compare, XVoid* _value);

/**
 *	@brief 删除Hash内容
 * @param pHashTable
 * @param hashkey
 * @return 0:删除成功，-1：删除失败
 */
extern XInt XShmHashRemove(XHashTableT* pHashTable,  XULong hashkey, XLLong compare);

/**
 *	@brief 获取当前Hash容量
 * @param pHashTable
 * @return 0:无数据; > 0:实际数据量
 */
extern XLong XShmHashGetCount(XHashTableT* pHashTable);

#ifdef __cplusplus
};
#endif

#endif /* INCLUDE_BASE_XSHMHASH_H_ */
