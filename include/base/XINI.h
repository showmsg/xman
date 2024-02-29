/**
 * @file XINI.h
 * @since 2022/5/26
 */

#ifndef SRC_INI_H_
#define SRC_INI_H_

#ifdef __cplusplus
extern "C" {
#endif

#define XINI_VERSION "0.1.1"

typedef struct _XINI XIniT;

/**
 * @brief 加载配置文件
 * @param filename
 * @return
 */
XIniT* XINILoad(const char *filename);

/**
 * @brief 关闭配置文件
 * @param ini
 */
void XINIFree(XIniT *ini);

/**
 * @brief 根据块、主键获取值
 * @param ini
 * @param section
 * @param key
 * @return
 */
const char* XINIGet(XIniT *ini, const char *section, const char *key);

/**
 * @brief 
 * @param pfile
 * @param section
 * @param key
 * @return
 */
int XIniGetIntBySecKey(const char *pfile, const char *section, const char *key);

/**
 * @brief 设置块、主键的值
 * @param ini
 * @param section
 * @param key
 * @param scanfmt
 * @param dst
 * @return
 */
int XINISet(XIniT *ini, const char *section, const char *key,
		const char *scanfmt, void *dst);

#ifdef __cplusplus
};
#endif

#endif /* SRC_INI_H_ */
