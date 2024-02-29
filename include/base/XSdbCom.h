/**
 * @file XSdbCom.h
 * 内存管理
 * @since 2022/3/29
 */

#ifndef INCLUDE_CORE_XSDBCOM_H_
#define INCLUDE_CORE_XSDBCOM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "XShm.h"
#include "XINI.h"

/**< 系统默认日志缓存 */
#define XSHM_SYSTEM_LOG_SECTION "sys.logs"

/**< 模式:SHM|MMAP */
#define XSHM_KEY_MODE   "mode"

/**< 创建的共享内存类型 */
#define XSHM_KEY_TYPE   "type"

/**< 创建的共享内存名称 */
#define XSHM_KEY_NAME   "name"

/**< 创建的共享内存key,SHM模式需要 */
#define XSHM_KEY_KEY    "key"

/**< 创建的共享内存总大小 */
#define XSHM_KEY_SIZE   "size"

/**< 共享内存容纳的数量 */
#define XSHM_KEY_SCALAR "scalar"

/**< Hash模式创建的共享内存行数 */
#define XSHM_KEY_ROW    "rows"

/**< Hash模式创建的共享内存列数 */
#define XSHM_KEY_COL    "cols"

/**< 创建共享内存数据的大小 */
#define XSHM_KEY_DATASIZE "datasize"

/**< 共享内存的模式:SHM */
#define XSHM_VMODE_SHM   "SHM"

/**< 共享内存的模式:MMAP */
#define XSHM_VMODE_MMAP  "MMAP"

/**< 共享内存类型:Array */
#define XSHM_VTYPE_ARRAY  "ARRAY"

/**< 共享内存类型:Hash */
#define XSHM_VTYPE_HASH   "HASH"

/**< 共享内存类型:RingBuff */
#define XSHM_VTYPE_RING   "RING"

/**< 共享内存类型: Block */
#define XSHM_VTYPE_BLOCK  "BLOCK"

/**< 共享内存类型: list */
#define XSHM_VTYPE_LIST  "LIST"

/**< 原始数据格式,区别于数组，只包含头信息和原始数据 */
#define XSHM_VTYPE_RAW   "RAW"
/**
 * @brief 共享内存创建集合信息
 */
typedef struct _XShmInfo {
	XChar name[XSHM_NAME_LENGTH];  	/**< 内存名 */
	XLong size;						/**< 数据大小 */
} XShmInfoT;

/**
 * @brief 检查内存信息对应的块是否存在
 * @param config [in]
 * @param section [in]
 * @return 成功or失败
 */
extern XInt CheckSection(XIniT *config, const char *section);

/**
 * @brief 读取sdb文件，获取对应的内存信息
 * @param config [in]
 * @param section [in]
 * @param xshm [out]
 * @return 获取成功or失败
 */
extern XInt XShmGetSdbBySection(XIniT *config, const char *section,
		XShmHeaderT *xshm);

/**
 * @brief 通过块初始化共享内存
 * @param config
 * @param section
 * @param datasize
 * @return 申请使用的内存
 */
extern XLong XShmInitBySec(const char* sdbconf, const char *section,
		XLong datasize);

/**
 * @brief 通过块初始化共享内存
 * @param sdbconf
 * @param section
 * @param datasize
 * @return 申请使用的内存
 */
extern XVoid* XShmStartBySec(const char* sdbconf, const char *section, XLong datasize);

/**
 * @brief 通过块初始化共享内存
 * @param config
 * @param section
 * @param datasize
 * @return 申请使用的内存
 */
extern XVoid* XShmLoadBySec(const char* sdbconf, const char *section, XLong datasize);

/**
 * @brief 通过块删除共享内存
 * @param config
 * @param section
 * @return
 */
extern XVoid XShmDelBySec(const char* sdbconf, const char *section, XLong datasize);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_CORE_XSDBCOM_H_ */
