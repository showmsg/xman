/*
 * OesCom.h
 *
 *  Created on: 2022年4月1日
 *      Author: kyle
 */

#ifndef INCLUDE_CORE_OESCOM_H_
#define INCLUDE_CORE_OESCOM_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "OesInit.h"
#include "OesMkt.h"
#include "OesTrd.h"

#define XTRADE_PREOPEN_TIME          (91457000)    //开盘前阶段

#define XTRADE_CALLAUCT_TIME         (92500000)    //集合竞价结束阶段
#define XTRADE_CONAUCT_TIME          (93000000)    //连续竞价开始
#define XTRADE_NOONBRK_TIME         (113000000)    //中午修饰开始
#define XTRADE_AFTERAUCT_TIME       (130000000)    //下午开盘
#define XTRADE_CLOSING_TIME         (150000000)    //下午收市，主要为股票和基金业务收市
#define XTRADE_CLOSED_TIME          (153000000)    //下午收盘，主要为债券及债券质押式回购收盘


/**
 * @brief 返回当前时间是否在对应交易阶段
 * @param time
 * @param period            @see eXMktStatus
 * @return
 * @retval  0  成功
 * @retval  -1 失败
 */
XInt is_trade_period(XShortTime time, XInt period);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_CORE_OESCOM_H_ */
