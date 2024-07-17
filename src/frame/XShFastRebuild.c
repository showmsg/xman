#include "XBus.h"
#include "XLog.h"
#include "XTimes.h"
#include "XUtils.h"
#include "XShFastRebuild.h"

static XMonitorMdT *l_pMonitor = NULL;

#define __DEBUG_CODE__ "603739"
//#define __DEBUG_INFO__

// 委托订单
static XVoid AddFixedBidOrder(XTickOrderT *tickOrder, XRSnapshotT *pSnapshot) {
	XTickOrderT *pTickOrder = NULL;
	XMoney ordMoney = 0;
	XBool bBigOrder = false;

	ordMoney = (XMoney) tickOrder->ordPx * tickOrder->ordQty;
	if (tickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY) {
		bBigOrder = true;
		pSnapshot->bigBuyOrdAmt += ordMoney;
		pSnapshot->bigBuyOrdQty += tickOrder->ordQty;
		pSnapshot->bigBuyOrdCnt++;
	}

	//行情更新
	pSnapshot->execType = eXExecBOrd;
	pSnapshot->execMoney = ordMoney;
	pSnapshot->execPrice = tickOrder->ordPx;

	//计算1秒内买单的频次及涨停价超大单买的次数
	if (tickOrder->updateTime / 1000 == pSnapshot->updateTime / 1000) {

		pSnapshot->lsecBuyTimes = pSnapshot->secBuyTimes;
		pSnapshot->secUpBigBuyCnt = 0;
		pSnapshot->secUpBuyTimes = 0;
		pSnapshot->secBuyTimes = 0;
		pSnapshot->netBigTrdMoney = 0;
	}

	pSnapshot->secBuyTimes++;
	if (tickOrder->ordPx == pSnapshot->upperPx) {
		pSnapshot->secUpBuyTimes++;
		if (tickOrder->ordQty > SUPERORDER_VOLUME && (tickOrder->ordPx - pSnapshot->tradePx) > 0.015 * pSnapshot->tradePx) {
			pSnapshot->secUpBigBuyCnt++;
		}
	}
	if (tickOrder->ordQty >= MIDORDER_VOLUME && tickOrder->ordPx > pSnapshot->tradePx) {
		pSnapshot->netBigTrdMoney += ordMoney;
	}

	pSnapshot->totalBuyOrdQty += tickOrder->ordQty;
	pSnapshot->totalBuyOrdCnt++;
	pSnapshot->totalBuyOrdAmt += ordMoney;

	// 是不是涨跌停价委托
	// 涨停价买
	if (tickOrder->ordPx == pSnapshot->upperPx) {
		// 能否找到原始订单
		pTickOrder = XFndVTickOrder(tickOrder->market, tickOrder->securityId,
				tickOrder->channel, tickOrder->seqno);
		// 成交先来的乱序单未处理
		if (NULL == pTickOrder) {
			// 添加订单
			tickOrder->_priceIdx = UPPER_BID_ID;

			tickOrder->_leaveQty = tickOrder->ordQty;
			XUpdateVTickOrderByKey(tickOrder);
			//价格不为涨停价的时候要重置该档位
			//    if(pSnapshot->tradePx == pSnapshot->upperPx)
			{
				pSnapshot->bid[0] = pSnapshot->upperPx;
				pSnapshot->bidqty[0] += tickOrder->ordQty;
			}
			if (pSnapshot->_lastUpperTime != 0) {
				pSnapshot->_catchUpBidQty += tickOrder->ordQty;
				if (bBigOrder) {
					pSnapshot->_catchUpBidCBuyCnt++;
				}
			}

			//统计委托
			pSnapshot->upperBidOrdQty += tickOrder->ordQty;
			pSnapshot->upperBidOrdCnt++;
		}
	} else if (tickOrder->ordPx == pSnapshot->lowerPx) {
		// 能否找到原始订单
		pTickOrder = XFndVTickOrder(tickOrder->market, tickOrder->securityId,
				tickOrder->channel, tickOrder->seqno);
		// 成交先来的乱序单未处理
		if (NULL == pTickOrder) {
			// 添加订单
			tickOrder->_priceIdx = LOWER_BID_ID;

			tickOrder->_leaveQty = tickOrder->ordQty;
			XUpdateVTickOrderByKey(tickOrder);
			//价格不为跌停价的时候要重置
			//     if(pSnapshot->tradePx == pSnapshot->lowerPx)
			{
				//      pSnapshot->bid[0] = pSnapshot->lowerPx;
				//       pSnapshot->bidqty[0] += tickOrder->ordQty;
			}

			//统计委托
			pSnapshot->lowerBidOrdQty += tickOrder->ordQty;
			pSnapshot->lowerBidOrdCnt++;
		}
	}

	GenOrdKLine(pSnapshot, tickOrder);
}

static XVoid AddFixedOfferOrder(XTickOrderT *tickOrder, XRSnapshotT *pSnapshot) {
	XTickOrderT *pTickOrder = NULL;
	XMoney ordMoney = 0;
	// XBool bBigOrder = false;

	ordMoney = (XMoney) tickOrder->ordPx * tickOrder->ordQty;
	if (tickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY) {
		//   bBigOrder = true;
		pSnapshot->bigSellOrdAmt += ordMoney;
		pSnapshot->bigSellOrdCnt++;
		pSnapshot->bigSellOrdQty += tickOrder->ordQty;
	}
	pSnapshot->totalSellOrdQty += tickOrder->ordQty;
	pSnapshot->totalSellOrdCnt++;
	pSnapshot->totalSellOrdAmt += ordMoney;

	//行情更新
	pSnapshot->execType = eXExecSOrd;
	pSnapshot->execMoney = ordMoney;
	pSnapshot->execPrice = tickOrder->ordPx;

	//计算大单净买入额
	if (tickOrder->updateTime / 1000 == pSnapshot->updateTime / 1000) {

		pSnapshot->netBigTrdMoney = 0;
	}

	if (tickOrder->ordQty >= MIDORDER_VOLUME && tickOrder->ordPx < pSnapshot->tradePx) {
		pSnapshot->netBigTrdMoney -= ordMoney;
	}

	// 是不是涨跌停价委托
	// 涨停价买
	if (tickOrder->ordPx == pSnapshot->upperPx) {
		// 能否找到原始订单
		pTickOrder = XFndVTickOrder(tickOrder->market, tickOrder->securityId,
				tickOrder->channel, tickOrder->seqno);
		// 成交先来的乱序单未处理
		if (NULL == pTickOrder) {
			// 添加订单
			tickOrder->_priceIdx = UPPER_OFFER_ID;

			tickOrder->_leaveQty = tickOrder->ordQty;
			XUpdateVTickOrderByKey(tickOrder);
			//价格不为涨停价的时候要重置该档位
			//     if(pSnapshot->tradePx == pSnapshot->upperPx)
			{
				pSnapshot->ask[0] = pSnapshot->upperPx;
				pSnapshot->askqty[0] += tickOrder->ordQty;
			}

			//统计
			pSnapshot->upperOfferOrdQty += tickOrder->ordQty;
			pSnapshot->upperOfferOrdCnt++;

#ifdef __DEBUG_INFO__
		if(strncmp(tickOrder->securityId, __DEBUG_CODE__, 6) == 0)
		{
			slog_debug(0, "[%d-%s-%lld] 涨停价卖出添加 [%d], [%lld]", tickOrder->market, tickOrder->securityId,
					tickOrder->seqno,
					tickOrder->_leaveQty, pSnapshot->upperOfferOrdQty);
		}
#endif
		}
	} else if (tickOrder->ordPx == pSnapshot->lowerPx) {
		// 能否找到原始订单
		pTickOrder = XFndVTickOrder(tickOrder->market, tickOrder->securityId,
				tickOrder->channel, tickOrder->seqno);
		// 成交先来的乱序单未处理
		if (NULL == pTickOrder) {
			// 添加订单
			tickOrder->_priceIdx = LOWER_OFFER_ID;

			tickOrder->_leaveQty = tickOrder->ordQty;
			XUpdateVTickOrderByKey(tickOrder);
			//价格不为跌停价的时候要重置
			//     if(pSnapshot->tradePx == pSnapshot->lowerPx)
			{
				//      pSnapshot->ask[0] = pSnapshot->lowerPx;
				//      pSnapshot->askqty[0] += tickOrder->ordQty;
			}

			//统计
			pSnapshot->lowerOfferOrdQty += tickOrder->ordQty;
			pSnapshot->lowerOfferOrdCnt++;
		}
	}
	GenOrdKLine(pSnapshot, tickOrder);
}

static XVoid RevokeBidOrder(XTickOrderT *tickOrder, XRSnapshotT *pSnapshot) {
	XTickOrderT *pTickOrder = NULL;
	XMoney ordMoney = 0;
	XBool bBigOrder = false;

	pTickOrder = XFndVTickOrder(tickOrder->market, tickOrder->securityId,
			tickOrder->channel, tickOrder->seqno);
	if (NULL == pTickOrder) {
		return;
	}
	ordMoney = (XMoney) pTickOrder->ordPx * pTickOrder->ordQty;
	if (pTickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY) {
		bBigOrder = true;
		pSnapshot->bigBuyOrdAmt -= (XMoney) tickOrder->ordPx
				* tickOrder->ordQty;
		pSnapshot->bigBuyOrdQty -= tickOrder->ordQty;
		pSnapshot->bigBuyOrdCnt--;
	}

	//行情更新
	pSnapshot->execType = eXExecBCtrl;
	pSnapshot->execMoney = ordMoney;
	pSnapshot->execPrice = pTickOrder->ordPx;

	pSnapshot->bidqty[0] -= tickOrder->ordQty;
	if (pSnapshot->bidqty[0] <= 0) {
		pSnapshot->bid[0] = 0;
		pSnapshot->bidqty[0] = 0;
	}

	//更新委托
	if (pTickOrder->_priceIdx == UPPER_BID_ID) {
		pSnapshot->upperBidOrdQty -= tickOrder->ordQty;
		pSnapshot->upperBidOrdCnt--;
		if (pSnapshot->_lastUpperTime != 0) {
			pSnapshot->_catchUpBidCQty += tickOrder->ordQty;
			if (bBigOrder) {
				pSnapshot->_catchUpBidCBuyCnt++;
			}
		}
	} else if (pTickOrder->_priceIdx == LOWER_BID_ID) {
		pSnapshot->lowerBidOrdQty -= tickOrder->ordQty;
		pSnapshot->lowerBidOrdCnt--;
	}

	//根据priceIdx进行判断撤单是涨停还是跌停买单
	XRmvVTickOrderByKey(pTickOrder->market, pTickOrder->securityId,
			pTickOrder->channel, pTickOrder->seqno);
}

static XVoid RevokeOfferOrder(XTickOrderT *tickOrder, XRSnapshotT *pSnapshot) {
	XTickOrderT *pTickOrder = NULL;
	XMoney ordMoney = 0;

	pTickOrder = XFndVTickOrder(tickOrder->market, tickOrder->securityId,
			tickOrder->channel, tickOrder->seqno);
	if (NULL == pTickOrder) {
		return;
	}
	ordMoney = (XMoney) pTickOrder->ordPx * pTickOrder->ordQty;
	if (pTickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY) {
//		bBigOrder = true;
		pSnapshot->bigSellOrdAmt -= (XMoney) tickOrder->ordPx
				* tickOrder->ordQty;
		pSnapshot->bigSellOrdCnt--;
		pSnapshot->bigSellOrdQty -= tickOrder->ordQty;
	}

	//行情更新
	pSnapshot->execType = eXExecSCtrl;
	pSnapshot->execMoney = ordMoney;
	pSnapshot->execPrice = pTickOrder->ordPx;

	pSnapshot->askqty[0] -= tickOrder->ordQty;
	if (pSnapshot->askqty[0] <= 0) {
		pSnapshot->ask[0] = 0;
		pSnapshot->askqty[0] = 0;
	}

	//更新委托
	if (pTickOrder->_priceIdx == UPPER_OFFER_ID) {
		pSnapshot->upperOfferOrdQty -= tickOrder->ordQty;
		pSnapshot->upperOfferOrdCnt--;

#ifdef __DEBUG_INFO__
	if(strncmp(tickOrder->securityId, __DEBUG_CODE__, 6) == 0)
	{
		slog_debug(0, "[%d-%s-%lld]涨停价卖出,撤单 [%d], [%lld]", tickOrder->market, tickOrder->securityId,
				tickOrder->seqno,
				tickOrder->ordQty, pSnapshot->upperOfferOrdQty);
	}
#endif
//		slog_debug(0, "[%d-%s]涨停卖撤单", pTickOrder->market, pTickOrder->securityId);

	} else if (pTickOrder->_priceIdx == LOWER_OFFER_ID) {
		pSnapshot->lowerOfferOrdQty -= tickOrder->ordQty;
		pSnapshot->lowerOfferOrdCnt--;
//		slog_debug(0, "[%d-%s]跌停卖撤单", pTickOrder->market, pTickOrder->securityId);
	}

	//根据priceIdx进行判断撤单是涨停还是跌停买单
	XRmvVTickOrderByKey(pTickOrder->market, pTickOrder->securityId,
			pTickOrder->channel, pTickOrder->seqno);
}

static XVoid OnOrder(XTickOrderT *tickOrder) {
	XRSnapshotT *pSnapshot, snapshot = { 0 };
	XIdx idx = -1;
	XStockT *pStock = NULL;

#ifdef __BTEST__
  // IsSSEOrdChnlLoss(tickOrder->channel, tickOrder->ordSeq);
#endif

	idx = XFndOrderBook(tickOrder->market, tickOrder->securityId);
	if (idx < 1) {
		idx = XPutOrderBookHash(tickOrder->market, tickOrder->securityId);
	}

	// 存储涨跌停价
	pSnapshot = XFndVRSnapshotById(idx);
	if (NULL != pSnapshot) {
		memcpy(&snapshot, pSnapshot, XRSNAPSHOT_SIZE);
	} else {
		snapshot.idx = idx;
		snapshot.market = tickOrder->market;
		memcpy(snapshot.securityId, tickOrder->securityId, SECURITYID_LEN);

		snapshot.secStatus = eXSecTrading;
		pStock = XFndVStockByKey(tickOrder->market, tickOrder->securityId);
		if (NULL != pStock) {
			snapshot.upperPx = pStock->upperPrice;
			snapshot.lowerPx = pStock->lowerPrice;
			snapshot.preClosePx = pStock->preClose;
			snapshot.secStatus = pStock->secStatus;
		}
	}
	snapshot.updateTime = tickOrder->updateTime;

#ifdef __DEBUG_INFO__
	if(strncmp(tickOrder->securityId, __DEBUG_CODE__, 6) == 0 && tickOrder->ordPx == snapshot.upperPx && tickOrder->bsType == eXSell)
	{
		slog_debug(0, "============================== OnSzseOrder ================================");
		slog_debug(0, "OnOrder [%d-%s],买卖[%d],ordPx[%d],ordQty[%d],ordType[%d],委托时间[%d],序号[%lld]", tickOrder->market, tickOrder->securityId,
		tickOrder->bsType, tickOrder->ordPx, tickOrder->ordQty, tickOrder->ordType, tickOrder->updateTime, tickOrder->bizIndex);
	}
#endif

	if (tickOrder->bsType == eXBuy) {
		if (tickOrder->isCancel) {
			RevokeBidOrder(tickOrder, &snapshot);
		}
		// 存在乱序情况
		else {
			AddFixedBidOrder(tickOrder, &snapshot);
		}
	} else {
		if (tickOrder->isCancel) {
			RevokeOfferOrder(tickOrder, &snapshot);
		} else {
			AddFixedOfferOrder(tickOrder, &snapshot);
		}
	}

	OnReSnapshot(&snapshot);
}

static XVoid OnComTrade(XTickTradeT *tickTrade, XRSnapshotT *pSnapshot) {
	/** 记录成交的索引位置 */
	pSnapshot->traday = tickTrade->traday;
	pSnapshot->_channel = tickTrade->channel;
	pSnapshot->_bizIndex = tickTrade->bizIndex;
	pSnapshot->_recvTime = tickTrade->_recvTime;
	pSnapshot->updateTime = tickTrade->updateTime;
	pSnapshot->version++;


	//行情
	pSnapshot->execType = eXExecTrd;
	pSnapshot->execMoney = tickTrade->tradeMoney;
	pSnapshot->execPrice = tickTrade->tradePx;

	if (tickTrade->updateTime >= 93000000) {
		/** 主动成交价格,后来的是主动方 */
		if (tickTrade->askSeq < tickTrade->bidSeq) {
			pSnapshot->driveBidPx = tickTrade->tradePx;
			pSnapshot->outsideTrdAmt += tickTrade->tradeMoney;
			pSnapshot->side = eXBuy;
		} else {
			pSnapshot->driveAskPx = tickTrade->tradePx;
			pSnapshot->outsideTrdAmt += tickTrade->tradeMoney;
			pSnapshot->side = eXSell;
		}
	}
	//处理涨停价大单计算
	if (tickTrade->tradePx != pSnapshot->upperPx) {
		pSnapshot->_catchUpBidQty = 0;
		pSnapshot->_catchUpBidCQty = 0;
		pSnapshot->_lastUpperTime = 0; /**< 当前未涨停 */
		pSnapshot->_sealTime = 0;
		pSnapshot->_sealCursor = 0;
		pSnapshot->_catchUpTrdQty = 0; //触发涨停后的成交量
		pSnapshot->_catchUpBidBuyCnt = 0;
		pSnapshot->_catchUpBidCBuyCnt = 0;
	} else {
		/** 判断是否涨停 */
		if (pSnapshot->_lastUpperTime == 0) {
			pSnapshot->_upperTimes++;
		}
		pSnapshot->_lastUpperTime = tickTrade->updateTime;
		pSnapshot->_catchUpTrdQty += tickTrade->tradeQty;
	}

	if (tickTrade->tradePx) {
		//开盘价
		if (0 == pSnapshot->openPx) {
			pSnapshot->openPx = tickTrade->tradePx;
		}
		//最高价
		if (tickTrade->tradePx > pSnapshot->highPx) {
			pSnapshot->highPx = tickTrade->tradePx;
		}
		//最低价
		if (0 == pSnapshot->lowPx) {
			pSnapshot->lowPx = tickTrade->tradePx;
		} else if (tickTrade->tradePx < pSnapshot->lowPx) {
			pSnapshot->lowPx = tickTrade->tradePx;
		}
	}

	/** 最新价 */
	pSnapshot->tradePx = tickTrade->tradePx;
	pSnapshot->numTrades++;
	pSnapshot->volumeTrade += tickTrade->tradeQty;
	pSnapshot->amountTrade += tickTrade->tradeMoney;

	//封板，记录封板时间
	if (pSnapshot->_sealTime == 0 && pSnapshot->_lastUpperTime != 0
			&& pSnapshot->bidqty - pSnapshot->askqty >= 0) {
		pSnapshot->_sealTime = tickTrade->updateTime;
		pSnapshot->_sealCursor = pSnapshot->kcursor1;
	}

}

static XVoid OnBidTrade(XTickTradeT *tickTrade, XRSnapshotT *pSnapshot) {
	XTickOrderT *pTickOrder = NULL;
	XBool bStatics = false;
	XMoney ordMoney = 0;

	pTickOrder = XFndVTickOrder(tickTrade->market, tickTrade->securityId,
			tickTrade->channel, tickTrade->bidSeq);
	if (NULL == pTickOrder) {
		return;
	}

	//统计大单成交
	if (tickTrade->tradeQty >= BIGORDER_VOLUME
			|| tickTrade->tradeMoney >= BIGORDER_MONEY) {
		pSnapshot->bigBuyTrdAmt += tickTrade->tradeMoney;
		pSnapshot->bigBuyOrdAmt -= tickTrade->tradeMoney;
		pSnapshot->bigBuyOrdQty -= tickTrade->tradeQty;
		bStatics = true;
	}

	//计算1秒内买单的频次及涨停价超大单买的次数
		if (tickTrade->updateTime / 1000 == pSnapshot->updateTime / 1000) {

			pSnapshot->lsecBuyTimes = pSnapshot->secBuyTimes;
			pSnapshot->secUpBigBuyCnt = 0;
			pSnapshot->secUpBuyTimes = 0;
			pSnapshot->secBuyTimes = 0;
		}

	if (tickTrade->tradePx == pSnapshot->upperPx) {
			pSnapshot->secUpBuyTimes++;
			if (tickTrade->tradeQty > SUPERORDER_VOLUME) {
				pSnapshot->secUpBigBuyCnt++;
			}
		}

	ordMoney = (XMoney) pTickOrder->ordQty * pTickOrder->ordPx;
	if (!bStatics
			&& (pTickOrder->ordQty >= BIGORDER_VOLUME
					|| ordMoney >= BIGORDER_MONEY)) {
		pSnapshot->bigBuyTrdAmt += tickTrade->tradeMoney;
		pSnapshot->bigBuyOrdAmt -= tickTrade->tradeMoney;
		pSnapshot->bigBuyOrdQty -= tickTrade->tradeQty;
	}

	if (pTickOrder->_priceIdx == UPPER_BID_ID)
	{
		pSnapshot->upperBidOrdQty -= tickTrade->tradeQty;
	}
	else if (pTickOrder->_priceIdx == LOWER_BID_ID)
	{
		pSnapshot->lowerBidOrdQty -= tickTrade->tradeQty;
	}

	pSnapshot->bidqty[0] -= tickTrade->tradeQty;
	if (pSnapshot->bidqty[0] <= 0) {
		pSnapshot->bid[0] = 0;
		pSnapshot->bidqty[0] = 0;
	}

	if (pSnapshot->_lastUpperTime != 0) {
		pSnapshot->_catchUpBidCQty += tickTrade->tradeQty;
	}

	pTickOrder->_leaveQty -= tickTrade->tradeQty;
	if (pTickOrder->_leaveQty == 0) {
		XRmvVTickOrderByKey(pTickOrder->market, pTickOrder->securityId,
				pTickOrder->channel, pTickOrder->seqno);
	}

	//判断priceIdx进行盘口数量的更新

}

static XVoid OnOfferTrade(XTickTradeT *tickTrade, XRSnapshotT *pSnapshot) {
	XTickOrderT *pTickOrder = NULL;
	XBool bStatics = false;
	XMoney ordMoney = 0;

	pTickOrder = XFndVTickOrder(tickTrade->market, tickTrade->securityId,
			tickTrade->channel, tickTrade->askSeq);
	if (NULL == pTickOrder) {
		return;
	}

	//扣除涨停卖或跌停卖的数量
	if (pTickOrder->_priceIdx == UPPER_OFFER_ID)
	{
		pSnapshot->upperOfferOrdQty -= tickTrade->tradeQty;
#ifdef __DEBUG_INFO__
		if(strncmp(tickTrade->securityId, __DEBUG_CODE__, 6) == 0)
		{
			slog_debug(0, "[%d-%s-%lld]涨停价卖出 ,成交 [%d], [%lld]", tickTrade->market, tickTrade->securityId,
					tickTrade->askSeq,
					tickTrade->tradeQty, pSnapshot->upperOfferOrdQty);
		}
#endif
	}
	else if (pTickOrder->_priceIdx == LOWER_OFFER_ID)
	{
		pSnapshot->lowerOfferOrdQty -= tickTrade->tradeQty;
	}

	//统计大单成交
	if (tickTrade->tradeQty >= BIGORDER_VOLUME
			|| tickTrade->tradeMoney >= BIGORDER_MONEY) {
		pSnapshot->bigSellTrdAmt += tickTrade->tradeMoney;
		pSnapshot->bigSellOrdAmt -= tickTrade->tradeMoney;
		bStatics = true;
	}

	ordMoney = (XMoney) pTickOrder->ordQty * pTickOrder->ordPx;
	if (!bStatics
			&& (pTickOrder->ordQty >= BIGORDER_VOLUME
					|| ordMoney >= BIGORDER_MONEY)) {
		pSnapshot->bigSellTrdAmt += tickTrade->tradeMoney;
		pSnapshot->bigSellOrdAmt -= tickTrade->tradeMoney;
		pSnapshot->bigSellOrdQty -= tickTrade->tradeQty;
	}
	pSnapshot->askqty[0] -= tickTrade->tradeQty;
	if (pSnapshot->askqty[0] <= 0) {
		pSnapshot->ask[0] = 0;
		pSnapshot->askqty[0] = 0;
	}

	pTickOrder->_leaveQty -= tickTrade->tradeQty;
	if (pTickOrder->_leaveQty == 0) {
		XRmvVTickOrderByKey(pTickOrder->market, pTickOrder->securityId,
				pTickOrder->channel, pTickOrder->seqno);
	}

	//判断priceIdx进行盘口数量的更新
}

static XVoid OnTrade(XTickTradeT *tickTrade) {
	XRSnapshotT *pSnapshot, snapshot = { 0 };
	XStockT *pStock;
	XIdx idx = -1;

	idx = XFndOrderBook(tickTrade->market, tickTrade->securityId);
	if (idx < 1) {
		idx = XPutOrderBookHash(tickTrade->market, tickTrade->securityId);
	}

	pSnapshot = XFndVRSnapshotById(idx);
	if (NULL != pSnapshot) {
		memcpy(&snapshot, pSnapshot, XRSNAPSHOT_SIZE);
	} else {
		snapshot.idx = idx;
		snapshot.market = tickTrade->market;
		memcpy(snapshot.securityId, tickTrade->securityId, SECURITYID_LEN);

		snapshot.secStatus = eXSecTrading;
		pStock = XFndVStockByKey(tickTrade->market, tickTrade->securityId);
		if (NULL != pStock) {
			snapshot.upperPx = pStock->upperPrice;
			snapshot.lowerPx = pStock->lowerPrice;
			snapshot.preClosePx = pStock->preClose;
			snapshot.secStatus = pStock->secStatus;
		}
	}

#ifdef __DEBUG_INFO__

	if(strncmp(tickTrade->securityId, __DEBUG_CODE__, 6) == 0 && tickTrade->tradePx == snapshot.upperPx)
	{
		slog_debug(0, "================================  OnSzseTrade  =======================");
		slog_debug(0, "OnTrade [%d-%s],成交价格[%d],成交数量[%d],是否撤单[%d],更新时间[%d],序号[%lld],买序号[%lld],卖序号[%lld]",
				tickTrade->market, tickTrade->securityId, tickTrade->tradePx, tickTrade->tradeQty,
				tickTrade->isCancel, tickTrade->updateTime, tickTrade->bizIndex, tickTrade->bidSeq, tickTrade->askSeq);
	}
#endif

	OnComTrade(tickTrade, &snapshot);
	//大单成交无意义，只能统计涨跌停大单成交
	OnBidTrade(tickTrade, &snapshot);
	OnOfferTrade(tickTrade, &snapshot);

	GenTrdKLine(&snapshot, tickTrade->tradePx, tickTrade->tradeQty,
			tickTrade->tradeMoney, tickTrade->updateTime);

#ifdef __LATENCY__
	snapshot._genTime = XGetClockTime();
#endif

	OnReSnapshot(&snapshot);
}

XVoid XShRebuild(XVoid *params) {
	XL2LT *l2l = NULL;
	XTickTradeT trade;
	XTickOrderT order;
	XSnapshotBaseT snapshot;
	XInt readId = -1;
	XBindParamT *pBind = NULL;
	XInt iret = 0;

	slog_info(0, "XShRebuild启动......");
	pBind = (XBindParamT*) params;
	if (NULL != pBind) {
		iret = XBindCPU(pBind->cpuid);
		if (iret) {
			slog_warn(0, "绑核失败[%d]", pBind->cpuid);
		}
	}

	l_pMonitor = XFndVMdMonitor(eXExchSec);

	if (NULL == l_pMonitor) {
		slog_error(0, "获取行情信息错误");
		return;
	}

	if (XIsNullCache(XSHMKEYCONECT(mktCache))) {
		slog_error(0, "获取行情缓存失败");
		return;
	}

	readId = XGetReadCache(XSHMKEYCONECT(mktCache));
	slog_info(3, "开始重构订单薄行情[%d]......", readId);

	for (;;) {

		l2l = (XL2LT*) XPopCache(XSHMKEYCONECT(mktCache), readId);
		if (NULL == l2l || l2l->head.market != eXMarketSha) {
			continue;
		}

		switch (l2l->head.type) {

		case eMTickOrder:

			order = l2l->order;

			if ((!order.seqno)) {
				break;
			}
			l_pMonitor->shBiz = order.bizIndex;
			l_pMonitor->shChannel = order.channel;
			OnOrder(&order);
#ifdef __BTEST__						

				__atomic_store_n(&l_pMonitor->updateTime, order.updateTime, __ATOMIC_RELAXED);
#endif
			l_pMonitor->totalShOrders++;
			break;

			// 只处理涨停价或跌停价的档位
		case eMTickTrade:

			trade = l2l->trade;

			l_pMonitor->shBiz = trade.bizIndex;
			l_pMonitor->shChannel = trade.channel;
			// 1、查看对应的原始委托是否存在，不存在报数据丢失;存放数据，等后续补发
			// 2、找到对应的委托然后找到价格档位，更新价格档位信息
			// 3、更新实时快照
			OnTrade(&trade);
#ifdef __BTEST__

				__atomic_store_n(&l_pMonitor->updateTime, trade.updateTime, __ATOMIC_RELAXED);

#endif
			l_pMonitor->totalShTrades++;
			break;
		case eMSnapshot:
			snapshot = l2l->snapshot;

			l_pMonitor->updateTime = snapshot.updateTime;

			if (l_pMonitor->traday == 0) {
				l_pMonitor->traday = snapshot.traday;
			}

			if (__atomic_load_n(&l_pMonitor->_locFirstTime, __ATOMIC_RELAXED)
					== 0) {
				__atomic_store_n(&l_pMonitor->_locFirstTime, XGetClockTime(),
						__ATOMIC_RELAXED);
				__atomic_store_n(&l_pMonitor->_mktFirstTime,
						snapshot.updateTime, __ATOMIC_RELAXED);
			}

			OnOrgSnapshot(&snapshot);

			break;
		default:
			break;
		}
	}

	XReleaseCache(XSHMKEYCONECT(mktCache), readId);

	exit(0);

}
