/**
 * @file XBond.h
 * @version
 * 			- 初始版本
 * @since 2024/1/5
 */

#ifndef INCLUDE_XBOND_H_
#define INCLUDE_XBOND_H_

#include "XCom.h"
#include "XBus.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct BondTradeParam {
	XInt market; 			/**< 市场 */
	XSecurityId securityId; /**< 证券代码 */
	XInt  lmtBuyMinQty;     /**< 最小买数量 */
	XPrice price; 			/**< 证券价格 < 多少时买入 */
	XPrice buyGapPrice; 	/**< 盘口价差>控制量时买入 */
	XPrice sellGapPrice; 	/**< 盘口价差 >控制量时卖出 */
	XInt buySoir; 			/**< 买SOIR */
	XInt sellSoir; 			/**< 卖SOIR */
	XInt openZdf; 			/**< 开盘涨跌幅，买入控制 */
	XInt zdf; 				/**< 当前涨跌幅要大于设置买入 */
	XInt zfRatio; 			/**< 振幅波动与开盘涨幅比,卖出控制 */
	XInt buyQtyRatio; 		/**< 买盘口数量与买卖盘口数量总量比，买控制 */
	XInt sellQtyRatio; 		/**< 卖盘口数量与买卖盘口数量总量比,卖控制 */
	XSumQty buyQtyLimit; 	/**< 买一盘口数量大于控制量,买控制 */
	XInt latencyShTradeTime; /**< 延迟成交时间 */
	XInt latencySzTradeTime; /**< 延迟成交时间 */
} XBondTradeParamT;

extern XBool BondTrade(XRSnapshotT *snapshot, XStrategyT* pStrategy, XBondTradeParamT* param);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_XBOND_H_ */
