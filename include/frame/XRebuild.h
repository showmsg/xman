/*
 * @file ReBuild.h
 * 
 *
 *  @since 2022年5月24日 Author: Administrator
 */

#ifndef INCLUDE_XMAN_OES_REBUILD_H_
#define INCLUDE_XMAN_OES_REBUILD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "XCom.h"

/**
 * @brief 处理交易所原始行情L1/L2
 * @param snapbase  --原始行情数据
 * @return
 */
extern XVoid OnOrgSnapshot(XSnapshotBaseT *snapbase);

/**
 * @brief 生成重构后行情
 * @param snapshot  --重构后行情
 * @return
 */
extern XVoid UpdateSnapshot (XOrderBookT *pOrderBook, XInt isTrade);

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

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_XMAN_OES_REBUILD_H_ */
