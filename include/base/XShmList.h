/**
 * @file XShmList.h
 * @since 2022/3/24
 */

#ifndef INCLUDE_XSHMLIST_H_
#define INCLUDE_XSHMLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "XShmLib.h"

/**
 * 链表数据内容
 */
typedef struct _XListNode {
	XULong pre; 			/**< 前一个 */
	XULong next; 			/**< 后一个 */
	XULong cur; 			/**< 当前 */
	XULong del;				/**< 删除位置，往前查找 */
	XChar rw;				/**< 读写情况 */
	XChar _filed[7];
	XVoid* data; 			/**< 节点数据 */
} XListNodeT, *pXListNodeT;

#define XSHM_LISTNODE_SIZE sizeof(XListNodeT)
/**
 * 链表信息
 */
typedef struct _XShmList {
	XShmHeaderT header;
	XUInt blockSize;        /**< 数据区块大小 */
	XChar _field[4];		/**< 填充位 */
	XULong usedCount;	    /**<  当前已占用 */
	XULong _lastDelIdx;		/**<  最后删除位置 */
	XULong _headIdx;		/**< 头结点位置 */
	XVoid* datas;            /**<  数据区 */
} XShmListT, *pXShmListT;

#define XSHM_SHMLIST_SIZE sizeof(XShmListT)

/**
 * 内存初始化
 * @param pXShmHeader
 * @return 内存地址
 */
extern XShmListT* XShmListInit(XShmHeaderT* pXShmHeader);

/**
 * 内存挂接
 * @param pXShmHeader
 * @return
 */
extern XShmListT* XShmListAttach(XShmHeaderT* pXShmHeader);

/**
 * @brief  通过大块内存获取数据使用块
 * @note   
 * @param  pShared: 大块内存首地址
 * @param  offset: 使用偏移位置
 * @param  pXShmHeader: 数据使用信息
 * @retval 
 */
extern XShmListT* XShmListAttachByOffset(void* pShared, XULong offset, XShmHeaderT* pXShmHeader);

/**
 * 找到当前位置的数据
 * @param pShmList
 * @param index
 * @return node数据
 */
extern XListNodeT* XShmListFind(XShmListT* pShmList, XULong index);

/**
 * 打印当前链表
 * @param pList
 * @return 
 */
extern XVoid XShmListPrint(XShmListT* pList);

/**
 * 在当前位置之前插入数据
 * @param pShmList
 * @param index  当前位置
 * @param _data
 * @return 插入位置
 */
extern XULong XShmListInsertBefore(XShmListT* pShmList, XULong index, XVoid* _data, XInt size);

/**
 * 在当前位置之后插入数据
 * @param pShmList
 * @param index
 * @param _data
 * @return 插入位置
 */
extern XULong XShmListInsertAfter(XShmListT* pShmList, XULong index, XVoid* _data, XInt size);

/**
 * 更新数据
 * @param pShmList
 * @param index
 * @param _data
 * @return 0:成功，-1：失败
 */
extern XInt XShmListReset(XShmListT* pShmList, XULong index, XVoid* _data, XInt size);

/**
 * 删除数据
 * @param pShmList
 * @param index 删除位置
 * @return 0：成功，-1：失败
 */
extern XInt XShmListRemove(XShmListT* pShmList, XULong index);

#ifdef __cplusplus
};
#endif

#endif /* INCLUDE_XSHMLIST_H_ */
