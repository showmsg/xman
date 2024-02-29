/*
 * @file Store.h
 *
 *  Created on: 2022年9月8日
 *      Author: DELL
 */

#ifndef INCLUDE_XMAN_OES_STORE_H_
#define INCLUDE_XMAN_OES_STORE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "XCom.h"

/**
 * @brief 交易所原始数据保存
 * 
 * @param params 
 * @return XVoid 
 */
extern XVoid XStoreTick(XVoid* params);

/**
 * 保存快照数据到文件
 * @param params   cpu绑定信息
 * @return
 */
extern XVoid XStoreSnap(XVoid* params);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_XMAN_OES_STORE_H_ */
