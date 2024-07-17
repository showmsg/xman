/**
 * @file XUtils.h
 * @since 2020/6/11
 */

#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "XTypes.h"
#include <pthread.h>



#define getbit(Number, pos) ((Number) >> (pos)&1) /** 用宏得到某数Number的某位的值 */
#define setbit(Number, pos) (Number)|=(1<<(pos)) /** 将Number的第pos位置1 */
#define clrbit(Number, pos) (Number)&=~(1<<(pos)) /** 将Number的第pos位清0 */

/**
 * @brief 去除字符串空格
 * @param str
 * @return
 */
extern XChar* XStrTrim(XChar *str);

/** 
 * @brief 去除字符串前空格
 * @param str
 * @return
 */
extern XChar* XStrTrimHead(XChar *str);

/**
 * @brief 去除字符串尾空格
 * @param str
 * @return
 */
extern XChar* XStrTrimTail(XChar *str);

/**
 * @brief 
 * @param str
 * @param head
 * @param tail
 * @return
 */
extern XChar* XStrUnchar(XChar *str, XChar head, XChar tail);

/**
 * @brief 转换为大写
 * @param str
 * @return
 */
extern XChar* XStrUpper(XChar *str);

/**
 * @brief 转换为小写
 * @param str
 * @return
 */
extern XChar* XStrLower(XChar *str);

/**
 * @brief 
 * @param src
 * @param separator
 * @param maxlen
 * @param dest
 * @param num
 * @return
 */
extern XVoid XSplit(XChar *src, XCChar *separator, XInt maxlen, XChar **dest,
		XInt *num);

/**
 * @brief
 * @param pStr
 * @param c1
 * @param c2
 * @param n
 * @return
 */
extern XInt XStrReplace(XChar *pStr, XCChar c1, XCChar c2, XInt n);

/**
 * @brief
 * @param str
 * @param delimiters
 * @param retstop
 * @param offset
 * @return
 */
extern XChar* XStrTok(XChar *str, XCChar *delimiters, XChar *retstop,
		XInt *offset);

/**
 * @brief
 * @param mode
 * @param srcstr
 * @param tokstr
 * @param word
 * @return
 */
extern XChar* XStrReplace2(XCChar *mode, XChar *srcstr, XCChar *tokstr,
		XCChar *word);

/**
 * @brief 绑定cpu
 * @param cpuid
 * @return 0：成功，-1：失败
 */
extern XInt XBindCPU(XInt cpuid);

/**
 * 绑定cpu
 * @param pthid
 * @param cpuid
 * @return 0：成功，-1：失败
 */
extern XInt XBindThrCPU(pthread_t pthid, XInt cpuid);

/**
 * @brief 检查是不是目录
 * @param path
 * @return 0:是目录,其它:不是
 */
extern XInt XIsDir(XChar* path);

/**
 * @brief 检查是不是文件
 * @param path
 * @return 0:文件,其它:不是
 */
extern XInt XIsFile(XChar* path);

/**
 * @brief 检查目录是否存咋
 * @param path
 * @return 0:文件,其它:不是
 */
extern XInt XPathExist(XChar* path);

/**
 * @brief 获取文件对应目录
 * @param filepath
 * @return
 */
extern XChar* XDirName(XChar* filepath);

/**
 * @brief 将十六进制的字符串转换成整数
 * 
 * @param s 
 * @return XInt 
 */
extern XInt XHex2I(const char s[]); 

#ifdef __cplusplus
}
#endif

#endif /* SRC_UTILS_H_ */
