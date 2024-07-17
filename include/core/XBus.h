/**
 * @file XBus.h
 *
 *  Created on: 2022年8月22日
 *      Author: DELL
 */

#ifndef INCLUDE_CORE_XBUS_H_
#define INCLUDE_CORE_XBUS_H_
#include "XDataStruct.h"
#include "XStrategy.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief 获取可买的数量
 * 
 * @param pStrategy 
 * @param buyUnit 
 * @return XQty 
 */
extern XQty XGetCanBuyQty(XStrategyT *pStrategy, XInt buyUnit);

/**
 * @brief 获取可卖的数量
 * 
 * @param pStrategy 
 * @param buyUnit 
 * @return XQty 
 */
extern XQty XGetCanSellQty(XStrategyT *pStrategy, XInt buyUnit);

/**
 * @brief 获取策略已经发出但未成交的委托
 * 
 * @param pStrategy 
 * @param buyUnit 
 * @return XQty 
 */
extern XQty XGetBuyedQty(XStrategyT *pStrategy, XInt buyUnit);

/**
 * @brief 获取买委托插入位置
 * 
 * @param pStrategy 
 * @return XNum 
 */
extern XNum XGetBuyStorePos(XStrategyT *pStrategy);

/**
 * @brief 获取卖委托插入位置
 * 
 * @param pStrategy 
 * @return XNum 
 */
extern XNum XGetSellStorePos(XStrategyT *pStrategy);


/**
 * @brief 撤销订单
 * 
 * @param pStrategy 
 * @param localid 
 * @return XInt 
 */
extern XBool XCancelByLocalId(XStrategyT *pStrategy, XLocalId localid);

/**
 * @brief 撤销买单
 * 
 * @param pStrategy    策略
 * @param signalCtrl   停止策略
 */
extern XBool XCancelBuy(XStrategyT *pStrategy, XInt signalCtrl, XInt line);

/**
 * @brief 撤销卖单
 * 
 * @param pStrategy      策略
 * @param signalCtrl     停止策略
 */
extern XBool XCancelSell(XStrategyT *pStrategy, XInt signalCtrl);

/**
 * @brief 拷贝策略信息到委托订单
 * 
 * @param pStrategy 
 * @param pOrder 
 */
extern void CPStrategy2Ord(XStrategyT *pStrategy, XOrderReqT *pOrder);

/**
 * @brief 获取策略已经发送的委托
 * 
 * @param pStrategy     策略信息
 * @return XQty        发送数量
 */
extern XQty XGetSendedQty(XStrategyT *pStrategy);

/**
 * @brief 获取策略已经发送的委托
 *
 * @param pStrategy     策略信息
 * @param loclid        本地编号
 * @return XQty        发送数量
 */
extern XBool XGetCtrlEnableByLoc(XStrategyT *pStrategy, XLocalId localid);


/**
 * @brief 获取该订单能否撤单
 *
 * @param pOrgOrder     订单信息
 * @return XBool        true -- 能撤单
 */
extern XBool XGetCtrlEnableByOrder (XOrderT *pOrgOrder);

/**
 * @brief 获取买策略撤单状态
 *
 * @param pStrategy     策略信息
 * @return XBool        true -- 已经撤单
 */
extern XBool XGetBuyCtrlStatus (XStrategyT *pStrategy);

/**
 * @brief 获取买策略撤单状态
 *
 * @param pStrategy     策略信息
 * @return XBool        true -- 已经撤单
 */
extern XBool XGetSellCtrlStatus (XStrategyT *pStrategy);

/**
 * @brief 该策略是否存在已经发送的买单
 *
 * @param pStrategy     策略信息
 * @return XBool        true -- 未发送买单
 */
extern XBool CanBuySend(XStrategyT *pStrategy);

/**
 * @brief 该策略是否存在已经发送的买单
 *
 * @param pStrategy     策略信息
 * @return XBool        true -- 未发送买单
 */
extern XBool CanSellSend(XStrategyT *pStrategy);

/**
 * @brief 根据当前价获取滑点价格，现以2%的区间进行滑点设置
 *
 * @param tradePx       最新价
 * @param slip          滑点价格
 * @param lowerPrice    跌停价格
 * @return XPrice       委托价格
 */
extern XPrice GetSlipSellPx (XPrice tradePx, XInt slip, XPrice lowerPrice, XPrice priceTick);


#define XST_BUY_LOG(pStrategy, order) \
		slog_debug(0,																			\
						"[%s.%s-%lld]:<<<<<< 时间[%d],委托本地编号[%d],买卖[%d],委托价格[%.3f],委托数量[%d]",	\
						pStrategy->setting.securityId, pStrategy->setting.market == eXMarketSha ? "SH":"SZ",							\
						pStrategy->plotid, order._lastTime,  order.localId, order.bsType,	\
						order.ordPrice * 0.0001, order.ordQty);

#define XST_SELL_LOG(pStrategy, order) \
		slog_debug(0,																			\
						"[%s.%s-%lld]:>>>>>> 时间[%d],委托本地编号[%d],买卖[%d],委托价格[%.3f],委托数量[%d(%lld)]",	\
						pStrategy->setting.securityId, pStrategy->setting.market == eXMarketSha ? "SH":"SZ",							\
								pStrategy->plotid, order._lastTime,  order.localId, order.bsType,	\
						order.ordPrice * 0.0001, order.ordQty,0);

#define XST_CTRL_LOG(pStrategy, pOrgOrder) 									  \
slog_debug(0, "[%s.%s-%lld]:XXXXXX 撤单编号[%d] 委托本地编号[%d],柜台编号[%lld],策略时间间隔[%d]",  \
			pStrategy->setting.securityId, pStrategy->setting.market == eXMarketSha ? "SH":"SZ", 			\
			pStrategy->plotid, pOrgOrder->request.clocalId, pOrgOrder->request.orgLocalId, pOrgOrder->request.orgOrdId, pStrategy->plot.ctrGapTime);


#ifdef __cplusplus
}
#endif


#endif /* INCLUDE_CORE_XBUS_H_ */
