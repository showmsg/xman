#include "XTimes.h"
#include "XUtils.h"
#include "XLog.h"
#include "XBus.h"
#include "XSzFastRebuild.h"

static XMonitorMdT *l_pMonitor = NULL;

#ifdef __BTEST__
static FILE* fpbckfile = NULL;
#endif

#define __DEBUG_CODE__ "001696"
//#define __DEBUG_INFO__

static XVoid AddFixedBidOrder(XTickOrderT *tickOrder, XRSnapshotT *pSnapshot) {
	XMoney ordMoney = 0;
	XBool bBigOrder = false;

	ordMoney = (XMoney) tickOrder->ordPx * tickOrder->ordQty;
	pSnapshot->totalBuyOrdQty += tickOrder->ordQty;
	pSnapshot->totalBuyOrdCnt++;
	pSnapshot->totalBuyOrdAmt += ordMoney;


	//行情更新
	pSnapshot->execType = eXExecBOrd;
	pSnapshot->execMoney = ordMoney;
	pSnapshot->execPrice = tickOrder->ordPx;

	//计算大单委托
	if (tickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY) {
		bBigOrder = true;
		pSnapshot->bigBuyOrdCnt++;
		pSnapshot->bigBuyOrdAmt += ordMoney;
		pSnapshot->bigBuyOrdQty += tickOrder->ordQty;
	}

	//计算1秒内买单的频次及涨停价超大单买的次数
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
	if (tickOrder->ordQty >= MIDORDER_VOLUME && (tickOrder->ordType != eXOrdLimit || (tickOrder->ordType == eXOrdLimit && tickOrder->ordPx > pSnapshot->tradePx))) {
		pSnapshot->netBigTrdMoney += ordMoney;

//回测的时候打印主动大单记录,证券代买,时间,订单记录,
#ifdef __BTEST__
		fprintf(fpbckfile, "%d,%s,B,0, %d, %d,%d\n", tickOrder->market, tickOrder->securityId, tickOrder->updateTime, tickOrder->ordPx, tickOrder->ordQty);
#endif
	}

	//涨停价买
	if (tickOrder->ordPx == pSnapshot->upperPx
			|| (tickOrder->ordType != eXOrdLimit
					&& pSnapshot->tradePx == pSnapshot->upperPx)) {
		// 判断价格是买涨停
		tickOrder->_priceIdx = UPPER_BID_ID;
		tickOrder->ordPx = pSnapshot->upperPx;
		tickOrder->_leaveQty = tickOrder->ordQty;

//		if(pSnapshot->tradePx == pSnapshot->upperPx)
		{
			pSnapshot->bid[0] = pSnapshot->upperPx;
			pSnapshot->bidqty[0] += tickOrder->ordQty;
		}
		// 在达到涨停价后，记录以涨停价委托的情况
		if (pSnapshot->_lastUpperTime != 0) {
			pSnapshot->_catchUpBidQty += tickOrder->ordQty;
			//统计大单委托
			if (bBigOrder) {
				pSnapshot->_catchUpBidBuyCnt++;
			}
		}

		XUpdateVTickOrderByKey(tickOrder);

		//统计委托
		pSnapshot->upperBidOrdQty += tickOrder->ordQty;
		pSnapshot->upperBidOrdCnt++;
	}
	//跌停价买
	else if (tickOrder->ordPx == pSnapshot->lowerPx
			|| (tickOrder->ordType != eXOrdLimit
					&& pSnapshot->tradePx == pSnapshot->lowerPx)) {
		// 判断价格是买涨停
		tickOrder->_priceIdx = LOWER_BID_ID;
		tickOrder->ordPx = pSnapshot->lowerPx;
		tickOrder->_leaveQty = tickOrder->ordQty;
//		if(pSnapshot->tradePx == pSnapshot->lowerPx)
		{
//        pSnapshot->bid[0] = pSnapshot->lowerPx;
//        pSnapshot->bidqty[0] += tickOrder->ordQty;
		}
		XUpdateVTickOrderByKey(tickOrder);

		//统计委托
		pSnapshot->lowerBidOrdQty += tickOrder->ordQty;
		pSnapshot->lowerBidOrdCnt++;
	}

	GenOrdKLine(pSnapshot, tickOrder);
}

static XVoid AddFixedOfferOrder(XTickOrderT *tickOrder, XRSnapshotT *pSnapshot) {
	XMoney ordMoney = 0;
//    XBool bBigOrder = false;

	ordMoney = (XMoney) tickOrder->ordPx * tickOrder->ordQty;

	pSnapshot->totalSellOrdQty += tickOrder->ordQty;
	pSnapshot->totalSellOrdCnt++;
	pSnapshot->totalSellOrdAmt += ordMoney;


	//行情更新
	pSnapshot->execType = eXExecSOrd;
	pSnapshot->execMoney = ordMoney;
	pSnapshot->execPrice = tickOrder->ordPx;

	//计算大单委托
	if (tickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY) {
		//       bBigOrder = true;
		pSnapshot->bigSellOrdCnt++;
		pSnapshot->bigSellOrdAmt += ordMoney;
		pSnapshot->bigSellOrdQty += tickOrder->ordQty;
	}

	//计算大单净买入额
	if (tickOrder->updateTime / 1000 == pSnapshot->updateTime / 1000) {

		pSnapshot->netBigTrdMoney = 0;
	}

	if (tickOrder->ordQty >= MIDORDER_VOLUME && (tickOrder->ordType != eXOrdLimit || (tickOrder->ordType == eXOrdLimit && tickOrder->ordPx < pSnapshot->tradePx))) {
		pSnapshot->netBigTrdMoney -= ordMoney;

		//回测的时候打印主动大单记录,证券代买,时间,订单记录,
		#ifdef __BTEST__
				fprintf(fpbckfile, "%d,%s,S,0, %d, %d,%d\n", tickOrder->market, tickOrder->securityId, tickOrder->updateTime, tickOrder->ordPx, tickOrder->ordQty);
		#endif
	}

	//涨停价卖
	if (tickOrder->ordPx == pSnapshot->upperPx
			|| (tickOrder->ordType != eXOrdLimit
					&& pSnapshot->tradePx == pSnapshot->upperPx)) {

		tickOrder->_priceIdx = UPPER_OFFER_ID;
		tickOrder->ordPx = pSnapshot->upperPx;
		tickOrder->_leaveQty = tickOrder->ordQty;
		XUpdateVTickOrderByKey(tickOrder);

//		if(pSnapshot->tradePx == pSnapshot->upperPx)
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
			slog_debug(0, "[%d-%s] 涨停价卖出添加 [%d], [%lld]", tickOrder->market, tickOrder->securityId, tickOrder->_leaveQty, pSnapshot->upperOfferOrdQty);
		}
#endif
	}
	//跌停价卖
	else if (tickOrder->ordPx == pSnapshot->lowerPx
			|| (tickOrder->ordType != eXOrdLimit
					&& pSnapshot->tradePx == pSnapshot->lowerPx)) {
		tickOrder->_priceIdx = LOWER_OFFER_ID;
		tickOrder->ordPx = pSnapshot->lowerPx;
		tickOrder->_leaveQty = tickOrder->ordQty;
		XUpdateVTickOrderByKey(tickOrder);
//		if(pSnapshot->tradePx == pSnapshot->lowerPx)
		{
//        pSnapshot->ask[0] = pSnapshot->lowerPx;
//        pSnapshot->askqty[0] += tickOrder->ordQty;
		}
		//统计
		pSnapshot->lowerOfferOrdQty += tickOrder->ordQty;
		pSnapshot->lowerOfferOrdCnt++;
	}
	GenOrdKLine(pSnapshot, tickOrder);
}

static XVoid OnOrder(XTickOrderT *tickOrder) {
	XRSnapshotT *pSnapshot, snapshot = { 0 };
	XIdx idx = -1;
	XStockT *pStock;

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
		slog_debug(0, "[%d-%s],买卖[%d],ordPx[%d],ordQty[%d],ordType[%d],委托时间[%d],序号[%lld]", tickOrder->market, tickOrder->securityId,
		tickOrder->bsType, tickOrder->ordPx, tickOrder->ordQty, tickOrder->ordType, tickOrder->updateTime, tickOrder->bizIndex);
	}
#endif

	if (tickOrder->bsType == eXBuy) {
		AddFixedBidOrder(tickOrder, &snapshot);
	} else {
		AddFixedOfferOrder(tickOrder, &snapshot);
	}

	//封板，记录封板时间
	if (snapshot._sealTime == 0 && snapshot._lastUpperTime != 0
			&& snapshot.bidqty - snapshot.askqty >= 0) {
		snapshot._sealTime = tickOrder->updateTime;
		snapshot._sealCursor = snapshot.kcursor1;
	}

	/** 记录当前快照更新的索引位置 */
	snapshot.traday = tickOrder->traday;
	snapshot._channel = tickOrder->channel;
	snapshot._bizIndex = tickOrder->bizIndex;
	snapshot.updateTime = tickOrder->updateTime;
	snapshot.version++;
	snapshot._recvTime = tickOrder->_recvTime;
#ifdef __LATENCY__
	snapshot._genTime = XGetClockTime();
#endif

	OnReSnapshot(&snapshot);
}

//删除订单
XVoid RevokeBidOrder(XTickTradeT *trade, XRSnapshotT *pSnapshot) {
	XTickOrderT *pTickOrder = NULL;
	XBool bBigOrder = false;
	XMoney ordMoney = 0;

	//处理撤单,找到原始订单,删除
	pTickOrder = XFndVTickOrder(trade->market, trade->securityId,
			trade->channel, trade->bidSeq);

	if (NULL == pTickOrder) {
		return;
	}

	ordMoney = pTickOrder->ordPx * pTickOrder->ordQty;
	//如果是市价单
	if (pTickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY) {
		bBigOrder = true;
		pSnapshot->bigBuyOrdAmt -= trade->tradeMoney;
		pSnapshot->bigBuyOrdCnt--;
		pSnapshot->bigBuyOrdQty -= trade->tradeQty;
	}

	//行情更新
	pSnapshot->execType = eXExecBCtrl;
	pSnapshot->execMoney = ordMoney;
	pSnapshot->execPrice = pTickOrder->ordPx;

	pSnapshot->bidqty[0] -= trade->tradeQty;
	if (pSnapshot->bidqty[0] <= 0) {
		pSnapshot->bid[0] = 0;
		pSnapshot->bidqty[0] = 0;
	}
	//更新委托
	if (pTickOrder->_priceIdx == UPPER_BID_ID) {
		pSnapshot->upperBidOrdQty -= trade->tradeQty;
		pSnapshot->upperBidOrdCnt--;
//		slog_debug(0, "[%d-%s]涨停买撤单,时间[%d],撤单数量[%d],总量[%lld]", pTickOrder->market, pTickOrder->securityId, pTickOrder->updateTime, trade->tradeQty, pSnapshot->upperBidOrdQty);
	} else if (pTickOrder->_priceIdx == LOWER_BID_ID) {
		pSnapshot->lowerBidOrdQty -= trade->tradeQty;
		pSnapshot->lowerBidOrdCnt--;
//		slog_debug(0, "[%d-%s]跌停卖撤单,时间[%d],撤单数量[%d],总量[%lld]", pTickOrder->market, pTickOrder->securityId, pTickOrder->updateTime, trade->tradeQty, pSnapshot->lowerBidOrdQty);
	}
	if (pSnapshot->_lastUpperTime != 0) {
		pSnapshot->_catchUpBidCQty += trade->tradeQty;
		if (bBigOrder) {
			pSnapshot->_catchUpBidCBuyCnt++;
		}
	}
	//更新涨跌停的订单数量统计
	XRmvVTickOrderByKey(pTickOrder->market, pTickOrder->securityId,
			pTickOrder->channel, pTickOrder->seqno);

}

XVoid RevokeOfferOrder(XTickTradeT *trade, XRSnapshotT *pSnapshot) {
	XTickOrderT *pTickOrder = NULL;
//	XBool bBigOrder = false;
	XMoney ordMoney = 0;

	//处理撤单,找到原始订单,删除
	pTickOrder = XFndVTickOrder(trade->market, trade->securityId,
			trade->channel, trade->askSeq);

	if (NULL == pTickOrder) {
		return;
	}
	ordMoney = pTickOrder->ordPx * pTickOrder->ordQty;
	//如果是市价单
	if (pTickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY) {
//		bBigOrder = true;
		pSnapshot->bigSellOrdAmt -= trade->tradeMoney;
		pSnapshot->bigSellOrdCnt--;
		pSnapshot->bigSellOrdQty -= trade->tradeQty;
	}

	//行情更新
	pSnapshot->execType = eXExecSCtrl;
	pSnapshot->execMoney = ordMoney;
	pSnapshot->execPrice = pTickOrder->ordPx;

	pSnapshot->askqty[0] -= trade->tradeQty;
	if (pSnapshot->askqty[0] <= 0) {
		pSnapshot->ask[0] = 0;
		pSnapshot->askqty[0] = 0;
	}
	//更新委托
	if (pTickOrder->_priceIdx == UPPER_OFFER_ID) {
		pSnapshot->upperOfferOrdQty -= trade->tradeQty;
		pSnapshot->upperOfferOrdCnt--;

#ifdef __DEBUG_INFO__
	if(strncmp(trade->securityId, __DEBUG_CODE__, 6) == 0)
	{
		slog_debug(0, "[%d-%s]涨停价卖出,撤单 [%d], [%lld]", trade->market, trade->securityId, trade->tradeQty, pSnapshot->upperOfferOrdQty);
	}
#endif
//		slog_debug(0, "[%d-%s]涨停卖撤单,时间[%d],撤单数量[%d],总量[%lld]", pTickOrder->market, pTickOrder->securityId, pTickOrder->updateTime, trade->tradeQty, pSnapshot->upperOfferOrdQty);
	} else if (pTickOrder->_priceIdx == LOWER_OFFER_ID) {
		pSnapshot->lowerOfferOrdQty -= trade->tradeQty;
		pSnapshot->lowerOfferOrdCnt--;
//		slog_debug(0, "[%d-%s]跌停卖撤单,时间[%d],撤单数量[%d],总量[%lld]", pTickOrder->market, pTickOrder->securityId, pTickOrder->updateTime, trade->tradeQty, pSnapshot->lowerOfferOrdQty);
	}

	//更新涨跌停的数量统计
	XRmvVTickOrderByKey(pTickOrder->market, pTickOrder->securityId,
			pTickOrder->channel, pTickOrder->seqno);
}

//处理成交
XVoid OnBidTrade(XTickTradeT *trade, XRSnapshotT *pSnapshot) {
	XTickOrderT *pTickOrder = NULL;
	XMoney ordMoney = 0;

	pTickOrder = XFndVTickOrder(trade->market, trade->securityId,
			trade->channel, trade->bidSeq);
	//不是涨跌停的订单
	if (NULL == pTickOrder) {
		return;
	}
	ordMoney = pTickOrder->ordPx * pTickOrder->ordQty;
	pSnapshot->bidqty[0] -= trade->tradeQty;
	if (pSnapshot->bidqty[0] <= 0) {
		pSnapshot->bid[0] = 0;
		pSnapshot->bidqty[0] = 0;
	}

	if (pTickOrder->_priceIdx == UPPER_BID_ID)
	{
		pSnapshot->upperBidOrdQty -= trade->tradeQty;
	}
	else if (pTickOrder->_priceIdx == LOWER_BID_ID)
	{
		pSnapshot->lowerBidOrdQty -= trade->tradeQty;
	}

	//计算大单成交或者撤单
	if (pTickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY) {
		pSnapshot->bigBuyTrdAmt += trade->tradeMoney;
		pSnapshot->bigBuyOrdAmt -= trade->tradeMoney;
		pSnapshot->bigBuyOrdQty -= trade->tradeQty;
	}

	if (pSnapshot->_lastUpperTime != 0) {
		pSnapshot->_catchUpBidCQty += trade->tradeQty;

	}

	pTickOrder->_leaveQty -= trade->tradeQty;

	if (0 >= pTickOrder->_leaveQty) {
		XRmvVTickOrderByKey(pTickOrder->market, pTickOrder->securityId,
				pTickOrder->channel, pTickOrder->seqno);
	}

}

//处理成交
XVoid OnOfferTrade(XTickTradeT *trade, XRSnapshotT *pSnapshot) {
	XTickOrderT *pTickOrder = NULL;
	XMoney ordMoney = 0;

	pTickOrder = XFndVTickOrder(trade->market, trade->securityId,
			trade->channel, trade->askSeq);
	//不是涨跌停的订单
	if (NULL == pTickOrder) {
		return;
	}
	ordMoney = pTickOrder->ordPx * pTickOrder->ordQty;

	//扣除涨停卖或跌停卖的数量
	if (pTickOrder->_priceIdx == UPPER_OFFER_ID)
	{
		pSnapshot->upperOfferOrdQty -= trade->tradeQty;

#ifdef __DEBUG_INFO__
		if(strncmp(trade->securityId, __DEBUG_CODE__, 6) == 0)
		{
			slog_debug(0, "[%d-%s]涨停价卖出 ,成交 [%d], [%lld]", trade->market, trade->securityId, trade->tradeQty, pSnapshot->upperOfferOrdQty);
		}
#endif
	}
	else if (pTickOrder->_priceIdx == LOWER_OFFER_ID)
	{
		pSnapshot->lowerOfferOrdQty -= trade->tradeQty;
	}


	//成交上扣减
	pSnapshot->askqty[0] -= trade->tradeQty;
	if (pSnapshot->askqty[0] <= 0) {
		pSnapshot->ask[0] = 0;
		pSnapshot->askqty[0] = 0;
	}
	//计算大单成交或者撤单
	if (pTickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY) {
		pSnapshot->bigSellTrdAmt += trade->tradeMoney;
		pSnapshot->bigSellOrdAmt -= trade->tradeMoney;
		pSnapshot->bigSellOrdQty -= trade->tradeQty;
	}

	pTickOrder->_leaveQty -= trade->tradeQty;

	if (0 >= pTickOrder->_leaveQty) {
		XRmvVTickOrderByKey(pTickOrder->market, pTickOrder->securityId,
				pTickOrder->channel, pTickOrder->seqno);
	}
}

static XVoid OnComTrade(XTickTradeT *trade, XRSnapshotT *pSnapshot) {

	//行情
	pSnapshot->execType = eXExecTrd;
	pSnapshot->execMoney = trade->tradeMoney;
	pSnapshot->execPrice = trade->tradePx;

	/** 主动成交价格 */
	if (trade->updateTime >= 93000000) {
		if (trade->askSeq < trade->bidSeq) {
			pSnapshot->driveBidPx = trade->tradePx;
			//统计净流入
			pSnapshot->outsideTrdAmt += trade->tradeMoney;
			pSnapshot->side = eXBuy;
		} else {
			pSnapshot->driveAskPx = trade->tradePx;
			pSnapshot->insideTrdAmt += trade->tradeMoney;
			pSnapshot->side = eXSell;

		}
	}
	//处理涨停价大单计算
	if (trade->tradePx != pSnapshot->upperPx) {
		pSnapshot->_catchUpBidQty = 0;
		pSnapshot->_catchUpBidCQty = 0;
		pSnapshot->_lastUpperTime = 0; /**< 当前未涨停 */
		pSnapshot->_sealTime = 0;
		pSnapshot->_sealCursor = 0;
		pSnapshot->_catchUpTrdQty = 0;
		pSnapshot->_catchUpBidBuyCnt = 0;
		pSnapshot->_catchUpBidCBuyCnt = 0;
	} else {
		/** 判断是否涨停 */
		if (pSnapshot->_lastUpperTime == 0) {
			pSnapshot->_upperTimes++;
		}

		pSnapshot->_lastUpperTime = trade->updateTime;
		pSnapshot->_catchUpTrdQty += trade->tradeQty;
	}

	if (trade->tradePx) {
		//开盘价
		if (0 == pSnapshot->openPx) {
			pSnapshot->openPx = trade->tradePx;
		}
		//最高价
		if (trade->tradePx > pSnapshot->highPx) {
			pSnapshot->highPx = trade->tradePx;
		}
		//最低价
		if (0 == pSnapshot->lowPx) {
			pSnapshot->lowPx = trade->tradePx;
		} else if (trade->tradePx < pSnapshot->lowPx) {
			pSnapshot->lowPx = trade->tradePx;
		}
	}

	/** 最新价 */
	pSnapshot->tradePx = trade->tradePx;
	pSnapshot->numTrades++;
	pSnapshot->volumeTrade += trade->tradeQty;
	pSnapshot->amountTrade += trade->tradeMoney;

	/** 统计集合竞价成交量 */
	if (pSnapshot->updateTime < MARKET_MORN_BEGIN) {
		pSnapshot->auctionQty += trade->tradeQty;
	}
	//封板，记录封板时间
	if (pSnapshot->_sealTime == 0 && pSnapshot->_lastUpperTime != 0
			&& pSnapshot->bidqty - pSnapshot->askqty >= 0) {
		pSnapshot->_sealTime = trade->updateTime;
		pSnapshot->_sealCursor = pSnapshot->kcursor1;
	}
}

static XVoid OnTrade(XTickTradeT *trade) {
	XRSnapshotT *pSnapshot, snapshot = { 0 };
	XStockT *pStock;
	XIdx idx = -1;

	idx = XFndOrderBook(trade->market, trade->securityId);
	if (idx < 1) {
		idx = XPutOrderBookHash(trade->market, trade->securityId);
	}

	pSnapshot = XFndVRSnapshotById(idx);
	if (NULL != pSnapshot) {
		memcpy(&snapshot, pSnapshot, XRSNAPSHOT_SIZE);
	} else {
		snapshot.idx = idx;
		snapshot.market = trade->market;
		memcpy(snapshot.securityId, trade->securityId, SECURITYID_LEN);

		snapshot.secStatus = eXSecTrading;
		pStock = XFndVStockByKey(trade->market, trade->securityId);
		if (NULL != pStock) {
			snapshot.upperPx = pStock->upperPrice;
			snapshot.lowerPx = pStock->lowerPrice;
			snapshot.preClosePx = pStock->preClose;
			snapshot.secStatus = pStock->secStatus;
		}
	}

#ifdef __DEBUG_INFO__

	if(strncmp(trade->securityId, __DEBUG_CODE__, 6) == 0 && trade->tradePx == snapshot.upperPx)
	{
		slog_debug(0, "================================  OnSzseTrade  =======================");
		slog_debug(0, "[%d-%s],成交价格[%d],成交数量[%d],是否撤单[%d],更新时间[%d],序号[%lld],买序号[%lld],卖序号[%lld]",
		trade->market, trade->securityId, trade->tradePx, trade->tradeQty,
		trade->isCancel, trade->updateTime, trade->bizIndex, trade->bidSeq, trade->askSeq);
	}
#endif

	/** 记录成交的索引位置 */
	snapshot.traday = trade->traday;
	snapshot._channel = trade->channel;
	snapshot._bizIndex = trade->bizIndex;
	snapshot.updateTime = trade->updateTime;
	snapshot.version++;
	snapshot._recvTime = trade->_recvTime;

	if (trade->isCancel) {
		if (trade->bidSeq) {
			RevokeBidOrder(trade, &snapshot);
		}
		if (trade->askSeq) {
			RevokeOfferOrder(trade, &snapshot);
		}
	} else {

		OnBidTrade(trade, &snapshot);
		OnOfferTrade(trade, &snapshot);
		OnComTrade(trade, &snapshot);
		GenTrdKLine(&snapshot, trade->tradePx, trade->tradeQty,
				trade->tradeMoney, trade->updateTime);
	}

#ifdef __LATENCY__
        snapshot._genTime = XGetClockTime();
    #endif

	OnReSnapshot(&snapshot);
}

XVoid XSzRebuild(XVoid *params) {
	XL2LT *l2l = NULL;
	XTickTradeT trade;
	XTickOrderT order;
	XSnapshotBaseT snapshot;
	XInt readId = -1;
	XBindParamT *pBind = NULL;
	XInt iret = 0;

	slog_info(0, "XSZRebuild启动......");
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

#ifdef __BTEST__
	fpbckfile = fopen("bigOrder.csv", "w");
#endif

	readId = XGetReadCache(XSHMKEYCONECT(mktCache));
	slog_info(3, "开始重构订单薄行情[%d]......", readId);

	for (;;) {
		l2l = (XL2LT*) XPopCache(XSHMKEYCONECT(mktCache), readId);
		if (NULL == l2l || l2l->head.market != eXMarketSza) {
			continue;
		}

		switch (l2l->head.type) {

		case eMTickOrder:

			order = l2l->order;

			l_pMonitor->szChannel = order.channel;
			l_pMonitor->szBiz = order.bizIndex;
			OnOrder(&order);

#ifdef __BTEST__

				__atomic_store_n(&l_pMonitor->updateTime, order.updateTime, __ATOMIC_RELAXED);
#endif
			l_pMonitor->totalSzOrders++;

			break;

			// 只处理涨停价或跌停价的档位
		case eMTickTrade:

			trade = l2l->trade;

			l_pMonitor->szChannel = trade.channel;
			l_pMonitor->szBiz = trade.bizIndex;
			// 1、查看对应的原始委托是否存在，不存在报数据丢失;存放数据，等后续补发
			// 2、找到对应的委托然后找到价格档位，更新价格档位信息
			// 3、更新实时快照
			OnTrade(&trade);

#ifdef __BTEST__			

				__atomic_store_n(&l_pMonitor->updateTime, trade.updateTime, __ATOMIC_RELAXED);
#endif
			l_pMonitor->totalSzTrades++;
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
#ifdef __BTEST__
	fclose(fpbckfile);
#endif

	exit(0);
}
