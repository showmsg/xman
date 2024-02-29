/*
 * @file ReBuild2.h
 *
 *  Created on: 2022年9月8日
 *      Author: DELL
 */

#ifndef INCLUDE_XMAN_OES_FAST_REBUILD_H_
#define INCLUDE_XMAN_OES_FAST_REBUILD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "XCom.h"

#define UPPER_BID_ID                (1) /**< 涨停价买的ID */
#define LOWER_OFFER_ID              (2) /**< 跌停价卖的ID */
#define UPPER_OFFER_ID              (3) /**< 涨停价卖的ID */
#define LOWER_BID_ID                (4) /**< 跌停价买的ID */

/**
 * @brief 处理交易所原始行情L1/L2
 * @param snapbase  --原始行情数据
 * @return
 */
extern XVoid OnOrgSnapshot(XSnapshotBaseT *snapbase);

/**
 * @brief 生成K线委托数据
 * @param pSnapshot  --重构的行情
 * @param tickOrder  --逐笔委托
 * @return
 */
extern XVoid GenOrdKLine(XRSnapshotT *pSnapshot, XTickOrderT* tickOrder);

/**
 * @brief 生成K线成交数据
 * @param pSnapshot  --重构的行情
 * @param tradePx  --成交价格
 * @param tradeQty --成交数量
 * @param tradeMoney --成交金额
 * @param tradeTime --成交时间
 * @return
 */
extern XVoid GenTrdKLine(XRSnapshotT *pSnapshot, XPrice tradePx, XQty tradeQty, XMoney tradeMoney, XShortTime tradeTime);

/**
 * @brief 生成重构后行情
 * @param snapshot  --重构后行情
 * @return
 */
extern XVoid OnReSnapshot(XRSnapshotT *snapshot);


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_XMAN_OES_REBUILD2_H_ */
