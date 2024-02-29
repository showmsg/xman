/**
 * @file XShmLib.h
 * @since 2022/03/25
 *
*/

#ifndef INCLUDE_BASE_XSHMLIB_H_
#define INCLUDE_BASE_XSHMLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "XTypes.h"
#include <sys/shm.h>

#define XSHM_NAME_LENGTH      (24)
#define XSHM_PATH_LENGTH      (48)

/**
 * 共享内存模式
 */
typedef enum eXShmType {
	EXSHM_MODE_SHM = '1',    /**<  shmget方式 */
	EXSHM_MODE_MMAP = '2'    /**<  mmap方式 */
} eXShmMode;

/**
 * 读写控制
 */
typedef enum _eXShmRw {
	EXSHM_NONE = '0',     /**< 默认 */
	EXSHM_READ = '1',     /**<  可读 */
	EXSHM_WRITE = '2'     /**<  不可读(正在写) */
} eXShmRw;

/**
 * 共享内存类型
 */
typedef enum _eXShmType {
	EXSHM_TYPE_ARRAY = '1',    /**< 数组 */
	EXSHM_TYPE_HASH = '2',     /**< Hash */
	EXSHM_TYPE_RING = '3',	   /**< 缓冲队列 */
	EXSHM_TYPE_LIST = '4',     /**< 链表 */
	EXSHM_TYPE_BLOCK = '5',     /**< 块 */
	EXSHM_TYPE_RAW = '6'		/**< 原始数据格式 */
} eXShmType;

/**
 * @brief 共享内存消息头
 * 
 */
typedef struct _XShmHeader {
	XChar mode;							/**<  共享内存创建方式 @see eXShmMode */
	XChar type;							/**<  共享内存类型 @see eXShmType */
	XChar isEnable;						/**< 是否创建共享内存 */
	XChar _field[5];
	XULong blockNum;    				/**<  数据区容量 */
	XUInt dataSize;						/**<  数据大小 */
	XUInt rows;                         /**<  Hash使用 */
	key_t key;							/**<  SHM内存Key */
	XInt shmid;							/**<  内存地址 */
	XLong iTotalSize;					/**<  内存总大小 */
	XChar name[XSHM_NAME_LENGTH];          /**<  内存别名,唯一不重复 */
	XChar path[XSHM_PATH_LENGTH];			/**<  内存路径 */
	XLong __field2;
	XLong __field3;
} XShmHeaderT, *pXShmHeaderT;

#define XSHM_HEADER_SIZE                     (sizeof(XShmHeaderT))

/**
 * @brief 内存创建
 * @param        pXShmHeader          内存创建信息
 * @retval       void*                内存挂接地址
 *
 */
extern XVoid* XShmInit(XShmHeaderT *pXShmHeader);

/**
 * @brief 内存挂接
 * @param pXShmHeader
 * @retval      void*    内存挂接地址
 */
extern XVoid* XShmAttach(XShmHeaderT *pXShmHeader);

/**
 * @brief 内存获取
 * @param        shmid          内存信息
 * @retval       void*
 *
 */
extern XVoid* XShmAt(int shmid);

/**
 * @brief 内存退出
 * @param        pshared          内存地址
 * @retval       void*
 *
 */
extern XVoid XShmQuit(void *pshared);

/**
 * @brief 内存删除
 * @param        shmid          内存信息
 * @retval       void*
 *
 */
extern XVoid XShmDel(int shmid);

#ifdef __cplusplus
};
#endif

#endif /* INCLUDE_BASE_XSHMLIB_H_ */
