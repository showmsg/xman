/**
 * @file XLog.h
 *
 *  Created on: 2022年8月8日
 *      Author: DELL
 */

#ifndef INCLUDE_BASE_XLOG_H_
#define INCLUDE_BASE_XLOG_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include "XTypes.h"

#define XLOG_NONE_LEVEL        0                    /**< NONE */
#define XLOG_DEBUG_LEVEL       1                    /**< DEBUG */
#define XLOG_INFO_LEVEL        2                    /**< INFO */
#define XLOG_WARN_LEVEL        3                    /**< WARN */
#define XLOG_ERROR_LEVEL       4                    /**< ERROR */
#define XLOG_FATAL_EVEL        5                    /**< FATAL */

#define XLOG_FILE_LEN         (128)

typedef struct _XLogConfig
{
	void *logRing;                  /**< 内存句柄 */
	XInt isSync;					/**< 默认写入缓存 */
	XChar _field[4];
	XChar sdbName[112];			    /**< sdb文件名 */
	XChar logName[XLOG_FILE_LEN];	/**< 日志文件名 */
}XLogConfigT;

#define MAX_LOG_MSG_LENG             (1024)

typedef struct _XLogMsg
{
	XChar logName[XLOG_FILE_LEN];			/**< 日志文件名 */
	pid_t pid;								/**< 日志进程编号 */
	XInt isScreen;							/**< 是否打印屏幕 */
	XInt level;								/**< 日志级别 */
	XInt line;								/**< 源码行数 */
	XLong _field1;
	XLong _field2;
	XChar codename[96];						/**< 源码位置 */
	XChar msg[MAX_LOG_MSG_LENG];			/**< 日志内存 */
}XLogMsgT;

typedef enum _XLogScreen
{
	eXDefault = 0,							/**< 根据日志级别决定是否打印屏幕 */
	eXNoScreen = 1,							/**< 不打印屏幕 */
	eXSreen = 2								/**< 打印屏幕 */
}eXLogScreen;

#define slog_none(iscreen,...) \
	xslog(iscreen, XLOG_NONE_LEVEL, __FILE__, __LINE__, __VA_ARGS__);

#define slog_debug(iscreen,...) \
	xslog(iscreen,XLOG_DEBUG_LEVEL, __FILE__, __LINE__, __VA_ARGS__);

#define slog_info(iscreen,...) \
		xslog(iscreen,XLOG_INFO_LEVEL, __FILE__, __LINE__, __VA_ARGS__);

#define slog_warn(iscreen,...) \
		xslog(iscreen,XLOG_WARN_LEVEL, __FILE__, __LINE__, __VA_ARGS__);

#define slog_error(iscreen,...) \
		xslog(iscreen,XLOG_ERROR_LEVEL, __FILE__, __LINE__, __VA_ARGS__);

#define slog_fatal(iscreen,...) \
		xslog(iscreen,XLOG_FATAL_EVEL, __FILE__, __LINE__, __VA_ARGS__);


/**
 * @brief 检查系统有没有启动
 * @param sdbconf  -- 日志内存配置
 * @return   1：启动，0：未启动
 */
extern XBool xslog_check_init(const XChar* sdbconf);

/**
 * @brief 初始化异步日志
 * @param sdbconf  -- 日志内存配置
 * @param logFile  -- 日志文件名
 * @return
 */
extern void xslog_init(const XChar* sdbconf, const XChar *logFile);

/**
 * @brief 切换同步异步落地方式, TODO
 * @param dircLog  -- 同步异步
 * @return
 */
extern void xslog_config_type(XInt dircLog);

/**
 * @brief 落地日志到缓冲区
 * @param iscreen  -- 是否打印屏幕
 * @param level    -- 日志级别
 * @param codefile -- 源文件
 * @param line     -- 行数
 * @param pMsg     -- 错误信息
 * @return
 */
extern void xslog(int iscreen, int level, const char* codefile, int line, const char*pMsg, ...);

/////////////////////////////////////////////////////////////////////////////

//日志落地-异步线程
/**
 * @brief 创建异步线程，落地日志
 * @param sdbconf  -- 日志内存配置
 * @return
 */
extern XInt xslog_thread_main(const XChar *sdbconf);


//日志落地-同步进程
/**
 * @brief 初始化日志落地信息
 * @param sdbconf  -- 日志内存配置
 * @return
 */
extern void xslog_init_cache(const XChar* sdbconf);

/**
 * @brief 落地日志文件
 * @return
 */
extern void xslog_to_file();

/**
 * @brief 释放日志
 * @param sdbconf  -- 释放内存
 * @return
 */
extern void xslog_release_cache(const XChar* sdbconf);

#ifdef __cplusplus
};
#endif

#endif /* INCLUDE_BASE_XLOG_H_ */
