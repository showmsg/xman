/**
 * @file XShmRing.h
 * @since 2022/3/15
 */

#ifndef INCLUDE_BASE_XSHMRING_H_
#define INCLUDE_BASE_XSHMRING_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include "XShmLib.h"

/**
 * @brief RingBuffer数据区
 * 
 */
typedef struct _RingData {
	XChar rw;				/**<  读写控制位  @see eXShmRw */
	XChar _field[7];		//填充位
	XLong _field2[7];       /**< 填充位 */
	XChar data[];
} XRingDataT, *pXRingDataT;

#define XSHM_RINGDATA_SIZE                    sizeof(XRingDataT)

#define MAX_XSHM_RINGBUF_READERS   16
/**
 * @brief 环形缓冲队列
 */
typedef struct _XShmRingBuff {
	XShmHeaderT header;
	XUInt blockSize;    						/**<  数据区块大小 */
	spinlock_t  spin;							//spin lock
	XLLong slowReadPos;							/**< 最慢的读 */
	XLLong maxReadablePos;						/**< 最大的读位置 */
	XLLong writePos;   							/**<  写索引位置 */
	XLong mask;
	XVoid* datas;      							/**<  数据区 */
	XLong __field2;
	XLong __field3;
	XLLong readPos[MAX_XSHM_RINGBUF_READERS];	/**< 存储每个读取者的位置 */
} XShmRingBuffT, *pXShmRingBuffT;

#define XSHM_RINGBUFF_SIZE                      sizeof(XShmRingBuffT)

/**
 * @brief 初始化内存
 * @param pXShmHeader
 * @return
 */
extern XShmRingBuffT* XShmRingInit(XShmHeaderT* pXShmHeader);

/**
 * @brief 挂接内存
 * @param pXShmHeader
 * @return
 */
extern XShmRingBuffT* XShmRingAttach(XShmHeaderT* pXShmHeader);

/**
 * @brief  通过大块内存获取数据使用块
 * @note   
 * @param  pShared: 大块内存首地址
 * @param  offset: 使用偏移位置
 * @param  pXShmHeader: 数据使用信息
 * @retval 
 */
extern XShmRingBuffT* XShmRingAttachByOffset(void* pShared, XULong offset, XShmHeaderT* pXShmHeader);

/**
 *
 * @param pRingBuff
 * @return
 */
//extern XLong XShmRingGetNextWrt(XShmRingBuffT* pRingBuff);

/**
 * @brief 往队列中压入数据
 * @param pRingBuff
 * @param _value
 * @return 0:成功，-1：失败
 */
extern XInt XShmRingPush(XShmRingBuffT* pRingBuff, XVoid* _value);

/**
 * @brief 获取下一个可读的序列
 * @param pRingBuff
 * @param beginFirst 0:从最新位置读取，1从开始读取
 * @return
 */
extern XInt XShmRingGetNextRead(XShmRingBuffT* pRingBuff);

/**
 * @brief 从队列中推出数据
 * @param pRingBuff
 * @return 数据内存地址
 */
extern XVoid* XShmRingPop(XShmRingBuffT* pRingBuff, XInt readId);

//获取数据时直接拷贝出数据
#define XShmRingPopC(pRingBuff, readId, T, data)						\
{																		\
	XVoid* memData = XShmRingPop(pRingBuff, readId);					\
	memcpy(data, memData, sizeof(T));									\
	asm volatile ("":::"memory");										\
}

/**
 * @brief 从队列中推出数据
 * @param pRingBuff
 * @param readId
 * @param curPos
 * @return 数据内存地址
 */
extern XVoid* XShmRingPopByPos(XShmRingBuffT* pRingBuff, XInt readId, XLLong curPos);

/**
 * @brief 删除读会话
 * @param pRingBuff
 * @param readId
 * @return
 */
extern XVoid XShmRingDelRead(XShmRingBuffT* pRingBuff, XInt readId);

#ifdef __cplusplus
};
#endif

#endif /* INCLUDE_BASE_XSHMRING_H_ */
