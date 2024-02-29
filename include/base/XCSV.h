/**
 * @file XCSV.h
 */
#ifndef CSV_H_INCLUDED
#define CSV_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CSV_LINE_BUFLEN  (1024)
#define MAX_CSV_HEADER    (256)
#define MAX_FIELD_LENGTH  (128)

#define XCSV_DELIMETER         ","
#define XCSV_QUOTE             '"'
#define XCSV_ESCAPE            '\\'
#define XCSV_LINE_ENTER        '\n\r'
#define XCSV_END_DELIM         ',\n\r'

/**
 * @brief CSV操作信息
 */
typedef struct _XCsvHandle {
	FILE* file;							/**<  文件 */
	int colSize;						/**<  列数 */
	int rows;							/**<  读取的总数据行 */
	char *dataRow[MAX_CSV_HEADER];		/**<  每个字段最多100个长度 */
	char *headers[MAX_CSV_HEADER];
	char* (*GetField)(struct _XCsvHandle *csv, const char* field);
	char* (*GetFieldByCol)(struct _XCsvHandle *csv, int idx);
} XCsvHandleT;

/**
 * @brief 读取CSV文件
 * @param csv
 * @param filename
 * @return
 */
extern int XCsvOpen(XCsvHandleT* csv, const char* filename);

/**
 * @brief 关闭CSV句柄
 * @param csv
 * @return
 */
extern int XCsvClose(XCsvHandleT* csv);

/**
 * @brief 读取一行
 * @param csv
 * @return
 */
extern int XCsvReadLine(XCsvHandleT* csv);

typedef void (*XCsvRowUpdate)(XCsvHandleT* csv);

/**
 * @brief 回调方式读取内容
 * @param filename
 * @param callback
 */
extern void XCsvReadCallback(const char* filename, XCsvRowUpdate callback);

#ifdef __cplusplus
};
#endif

#endif
