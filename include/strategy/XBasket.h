/**
 * @file XCommon.h
 * @version 0.10.0 2022/4/29
 * 			- 初始版本
 * @since 2022/4/29
 */

#ifndef INCLUDE_XBASKET_H_
#define INCLUDE_XBASKET_H_

#include "XCom.h"
#include "XBus.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern void BasketBuy (XStrategyT *pStrategy, XRSnapshotT *snapshot, XStockT *pStock);


extern void BasketBuy2 (XStrategyT *pStrategy, XRSnapshotT *snapshot, XStockT *pStock);

extern void
BasketBuy3 (XStrategyT *pStrategy, XRSnapshotT *snapshot, XStockT *pStock);

extern void
BasketSell2 (XStrategyT *pStrategy, XSnapshotT *snapshot, XStockT *pStock);

extern void
BasketSell8 (XStrategyT *pStrategy, XSnapshotT *snapshot, XStockT *pStock);

extern XBool BasketTrade(XRSnapshotT *snapshot, XStrategyT* pStrategy, XStockT* pStock);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_XBASKET_H_ */