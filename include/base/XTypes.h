/**
 * @file XTypes.h
 * @since 2022/3/16
 */

#ifndef INCLUDE_BASE_XTYPES_H_
#define INCLUDE_BASE_XTYPES_H_
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef char XBool;

/**< false */
#define false 0

/**< true */
#define true 1

/**< char */
typedef char XChar;

/**< short */
typedef short XShort;

typedef const char XCChar;

/**< int */
typedef int XInt;

/**< unsigned int */
typedef unsigned int XUInt;

/**< long */
typedef long XLong;

/**< unsigned long */
typedef unsigned long XULong;

/**< long long */
typedef long long XLLong;

/**< float */
typedef float XFloat;

/**< double */
typedef double XDouble;

/**< void */
typedef void XVoid;

//双缓冲区第一位的可读写状态
typedef enum XBufType {
	XBufNone = '0',       /**< 初始化状态 */
	XBufRead = '1',      /**< 读状态 */
	XBufWrite = '2'       /**< 写状态 */
} XBufType;

#define XOffsetof(TYPE, MEMBER)                    \
            ( (int) &((TYPE *) 0)->MEMBER )

#define XContainerOf(PTR, TYPE, MEMBER)           \
            ( (TYPE *) ((char *) (PTR) - XOffsetof(TYPE, MEMBER)) )


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_BASE_XTYPES_H_ */
