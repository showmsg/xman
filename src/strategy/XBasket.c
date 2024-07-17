/*
 * @file XBasket2.c
 * @brief 盘中抢单程序
 * @author kangyq
 * @version 2.0.0
 * @date 2022-12-16
 * @version 20230525  调整均线计算,不包括当前未完结的最新行情
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */

#include "XPlot.h"
#include "XLog.h"
#include "XTimes.h"
#include "XBasket.h"

/**
 *
 */
static XBool UpperBuy(XStrategyT *pStrategy, XRSnapshotT *snapshot) {
	XMoney totalBidAmt = 0;
	XSumQty totalBidQty = 0;

	//涨停买
	if(-1 == pStrategy->setting.askQty)
	{
		return false;
	}
	/**涨停价，卖一量大于设定值不触发,(卖一量 - 买一量) 大于设定值 */
	if (snapshot->askqty[0] - snapshot->bidqty[0] > pStrategy->setting.askQty) {
		return false;
	}
	totalBidQty =
			snapshot->bidqty[0] - snapshot->askqty[0] > 0 ?
					snapshot->bidqty[0] - snapshot->askqty[0] : 0;
	totalBidAmt = totalBidQty / 10000 * snapshot->bid[0];

	slog_debug(0,
			"[%s.%s-%lld]:2.达到卖一量条件, 最新时间[%d],最新价[%d],卖一价[%d],卖一量[%lld(%lld)]",
			pStrategy->setting.securityId,
			pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
			pStrategy->plotid, snapshot->updateTime, snapshot->tradePx,
			snapshot->ask[0], snapshot->askqty[0], pStrategy->setting.askQty);

	slog_debug(0,
			"[%s.%s-%lld]:3.达到买一金额条件, 最新价[%d],买一价[%d] * 买一量[%lld] = 买一金额[%lld(%lld)]",
			pStrategy->setting.securityId,
			pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
			pStrategy->plotid, snapshot->tradePx, snapshot->bid[0],
			snapshot->bidqty[0], totalBidAmt, pStrategy->setting.buyMoney);
	/** 涨停价,买一金额小于设定金额不触发 */
	if (0 != pStrategy->setting.buyMoney
			&& totalBidAmt < pStrategy->setting.buyMoney) {
		return false;
	}

	slog_debug(0,
			"[%s.%s-%lld]:4.达到买一金额条件, 最新价[%d],买一价[%d] * 买一量[%lld] = 买一金额[%lld(%lld)]",
			pStrategy->setting.securityId,
			pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
			pStrategy->plotid, snapshot->tradePx, snapshot->bid[0],
			snapshot->bidqty[0], totalBidAmt, pStrategy->setting.buyMoney);

	if (pStrategy->setting.nxtCtrlMoney > 0 && pStrategy->_hasCtrl
			&& (snapshot->_sealCursor + 1) == snapshot->kcursor1) {
		return false;
	}

	return true;
}

/**
 *
 */
static XBool RobBuy(XStrategyT *pStrategy, XRSnapshotT *snapshot) {

	return true;
}

/**
 * @brief 抢板买入
 */
void BasketBuy4(XStrategyT *pStrategy, XRSnapshotT *snapshot, XStockT *pStock) {
	XQty allowBuyQty = -1, realOrdQty = -1;
	XNum idx = 0;
	XOrderReqT order = { 0 };
	XLocalId localid = 0;
	XMoney totalBidAmt = 0;
	XRatio bidTradRatio = 0;
	XSumQty totalBidQty = 0;
	XRatio zdf = 0;
	XRatio kpzdf = -1;
	XBool bSignal = false;

	if (pStrategy->version == snapshot->version) {
		return;
	}
	pStrategy->version = snapshot->version;

	//判断当前涨跌幅是否离涨停价2%
	if((snapshot->upperPx - snapshot->tradePx) * 10000 / snapshot->preClosePx > 200)
	{
		return;
	}

	if (pStrategy->setting.kpzf >= -2000) {
		// 开盘涨跌幅低于设置值不抢
		kpzdf = (snapshot->openPx - snapshot->preClosePx) * 10000
				/ snapshot->preClosePx;
		if (kpzdf < pStrategy->setting.kpzf) {
			slog_debug(0, "[%s.%s-%lld]:!!!!!! 策略暂停, 开盘涨幅不够[%d]",
					pStrategy->setting.securityId,
					pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
					pStrategy->plotid, kpzdf);
			pStrategy->status = eXPlotStop;
			return;
		}
	}

	if (pStrategy->_signal) {
		slog_debug(0, "--------------------------------------------------------"
				"------------------------------------");

		totalBidQty =
				snapshot->bidqty[0] - snapshot->askqty[0] > 0 ?
						snapshot->bidqty[0] - snapshot->askqty[0] : 0;
		totalBidAmt = totalBidQty / 10000 * snapshot->bid[0];
		if (snapshot->amountTrade != 0) {
			bidTradRatio = totalBidAmt * 10000 * 10000 / snapshot->amountTrade;
		}

		slog_debug(0,
				"[%s.%s-%lld]:最新时间[%d] 最新价[%d],条件价[%d],开盘价[%d],卖一[%d-%lld],买一[%d-%lld],封单金额[%lld(%lld)],撤单率[%d(%d)],涨停次数[%d],封单占成交[%lld],封单占流通[%lld]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, snapshot->updateTime, snapshot->tradePx,
				pStrategy->setting.conPx, snapshot->openPx, snapshot->ask[0],
				snapshot->askqty[0], snapshot->bid[0], snapshot->bidqty[0],
				totalBidAmt, pStrategy->setting.buyMoney, pStrategy->_cdl,
				pStrategy->setting.cdl, snapshot->_upperTimes, bidTradRatio,
				totalBidQty * 10000 / pStock->publicfloatShare);

		slog_debug(0, "[%s.%s-%lld]: 当前买涨停价撤单数量[%lld] 当前买涨停价委托数量[%lld] =[%lld]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, snapshot->_catchUpBidCQty,
				snapshot->_catchUpBidQty,
				snapshot->_catchUpBidQty > 0 ?
						snapshot->_catchUpBidCQty * 10000
								/ snapshot->_catchUpBidQty :
						0);

		slog_debug(0,
				"[%s.%s-%lld]: 涨停后成交量[%lld] / 封单量[%lld] = [%lld],大单买[%d],大单撤[%d],涨停价大单[%d],涨停价买入次数[%d]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, snapshot->_catchUpTrdQty, totalBidQty,
				totalBidQty > 0 ?
						10000 * snapshot->_catchUpTrdQty / totalBidQty : 0,
				snapshot->_catchUpBidBuyCnt, snapshot->_catchUpBidCBuyCnt,
				snapshot->secUpBigBuyCnt, snapshot->secUpBuyTimes);
	}

	//异动抢板: 临近涨停，有大单抢买,买入;涨停5%,10%,20%
	if (0 <= pStrategy->setting.askQty && snapshot->secUpBigBuyCnt > 2) {
		zdf = (snapshot->highPx - snapshot->tradePx) * 10000 / snapshot->preClosePx;
		if (zdf < 200)
		{
				bSignal = RobBuy(pStrategy, snapshot);

				slog_debug(0,
								"[%s.%s-%lld]: 异动大单提前买入,价格[%d],时间[%d],大单次数[%d]",
								pStrategy->setting.securityId,
								pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
								pStrategy->plotid, snapshot->tradePx, snapshot->updateTime, snapshot->secUpBigBuyCnt);
		}
	}

	//涨停抢板
	if (!bSignal && snapshot->tradePx == pStrategy->setting.conPx) {
		bSignal = UpperBuy(pStrategy, snapshot);
	}
	if (!bSignal) {
		return;
	}

	// 只要是涨停,都打印
	if (!pStrategy->_signal) {
		slog_debug(0,
				"--------------------------------------------------------------------------------------------");

		totalBidQty =
				snapshot->bidqty[0] - snapshot->askqty[0] > 0 ?
						snapshot->bidqty[0] - snapshot->askqty[0] : 0;
		totalBidAmt = totalBidQty / 10000 * snapshot->bid[0];
		if (snapshot->amountTrade != 0) {
			bidTradRatio = totalBidAmt * 10000 * 10000 / snapshot->amountTrade;
		}

		slog_debug(0,
				"[%s.%s-%lld]:最新时间[%d] 最新价[%d],条件价[%d], 开盘价[%d],卖一[%d-%lld],买一[%d-%lld], 封单金额[%lld(%lld)],撤单率[%d(%d)],涨停次数[%d],封单占成交[%lld],封单占流通[%lld]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, snapshot->updateTime, snapshot->tradePx,
				pStrategy->setting.conPx, snapshot->openPx, snapshot->ask[0],
				snapshot->askqty[0], snapshot->bid[0], snapshot->bidqty[0],
				totalBidAmt, pStrategy->setting.buyMoney, pStrategy->_cdl,
				pStrategy->setting.cdl, snapshot->_upperTimes, bidTradRatio,
				totalBidQty * 10000 / pStock->publicfloatShare);

		slog_debug(0, "[%s.%s-%lld]: 当前撤单数量[%lld] / 当前委托数量[%lld] =[%lld]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, snapshot->_catchUpBidCQty,
				snapshot->_catchUpBidQty,
				snapshot->_catchUpBidQty > 0 ?
						snapshot->_catchUpBidCQty * 10000
								/ snapshot->_catchUpBidQty :
						0);

		slog_debug(0,
				"[%s.%s-%lld]: 涨停后成交量[%lld] / 封单量[%lld] = [%lld], 大单买[%d],大单撤[%d]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, snapshot->_catchUpTrdQty, totalBidQty,
				totalBidQty > 0 ?
						10000 * snapshot->_catchUpTrdQty / totalBidQty : 0,
				snapshot->_catchUpBidBuyCnt, snapshot->_catchUpBidCBuyCnt);
		slog_debug(0, "[%s.%s-%lld]: 集合竞价成交量比[%d] askqty[%lld], bigbuycnt[%d]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid,
				10000 * snapshot->auctionQty / pStock->publicfloatShare,
				pStrategy->setting.askQty, snapshot->secUpBigBuyCnt);
	}

	allowBuyQty = XGetCanBuyQty(pStrategy, pStock->lmtBuyMinQty);

	slog_debug(0, "[%s.%s-%lld]:1.允许买量[%d]", pStrategy->setting.securityId,
			pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
			pStrategy->plotid, allowBuyQty);
	if (allowBuyQty <= 0) {
		return;
	}

	//超过最大报单数量的下单拆单
	while (allowBuyQty > 0) {
		if (allowBuyQty > pStock->lmtBuyMaxQty) {
			realOrdQty = pStock->lmtBuyMaxQty;
		} else {
			realOrdQty = allowBuyQty;
		}

		idx = XGetBuyStorePos(pStrategy);
		if (idx < 0) {
			return;
		}

		CPStrategy2Ord(pStrategy, &order);

		order.bsType = eXBuy;
		order.ordPrice = pStrategy->setting.ordPx;
		order.ordQty = realOrdQty;
		order.ordType = eXOrdLimit;

		order._lastPx = snapshot->tradePx;
		order._lastTime = snapshot->updateTime;
		order._mktTime = snapshot->_recvTime;
		order._bizIndex = snapshot->_bizIndex;

		localid = XPutOrderReq(&order);
		pStrategy->buyList[idx] = localid;
		pStrategy->_buySends += order.ordQty;
		pStrategy->_lastBuyPx = order.ordPrice;
		pStrategy->_buyLocTime = XGetClockTime();
		pStrategy->_signal = 1;

		XST_BUY_LOG(pStrategy, order);

		allowBuyQty -= realOrdQty;
	}
}

/**
 * @brief 抢板买入
 */
void BasketBuy(XStrategyT *pStrategy, XRSnapshotT *snapshot, XStockT *pStock) {
	XQty allowBuyQty = -1, realOrdQty = -1;
	XNum idx = 0;
	XOrderReqT order = { 0 };
	XLocalId localid = 0;
	XMoney totalBidAmt = 0;
	XRatio bidTradRatio = 0;
	XSumQty totalBidQty = 0;
	//	XSumQty ysVolTrades = 0;
	XRatio kpzdf = -1;

	if (pStrategy->version == snapshot->version) {
		return;
	}
	pStrategy->version = snapshot->version;

	if (pStrategy->setting.kpzf >= -2000) {
		// 开盘涨跌幅低于设置值不抢
		kpzdf = (snapshot->openPx - snapshot->preClosePx) * 10000
				/ snapshot->preClosePx;
		if (kpzdf < pStrategy->setting.kpzf) {
			slog_debug(0, "[%s.%s-%lld]:!!!!!! 策略暂停, 开盘涨幅不够[%d]",
					pStrategy->setting.securityId,
					pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
					pStrategy->plotid, kpzdf);
			pStrategy->status = eXPlotStop;
			return;
		}
	}

	if (pStrategy->_signal) {
		slog_debug(0, "--------------------------------------------------------"
				"------------------------------------");

		totalBidQty =
				snapshot->bidqty[0] - snapshot->askqty[0] > 0 ?
						snapshot->bidqty[0] - snapshot->askqty[0] : 0;
		totalBidAmt = totalBidQty / 10000 * snapshot->bid[0];
		if (snapshot->amountTrade != 0) {
			bidTradRatio = totalBidAmt * 10000 * 10000 / snapshot->amountTrade;
		}

		slog_debug(0,
				"[%s.%s-%lld]:最新时间[%d] 最新价[%d],条件价[%d],开盘价[%d],卖一[%d-%lld],买一[%d-%lld],封单金额[%lld(%lld)],撤单率[%d(%d)],涨停次数[%d],封单占成交[%lld],封单占流通[%lld]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, snapshot->updateTime, snapshot->tradePx,
				pStrategy->setting.conPx, snapshot->openPx, snapshot->ask[0],
				snapshot->askqty[0], snapshot->bid[0], snapshot->bidqty[0],
				totalBidAmt, pStrategy->setting.buyMoney, pStrategy->_cdl,
				pStrategy->setting.cdl, snapshot->_upperTimes, bidTradRatio,
				totalBidQty * 10000 / pStock->publicfloatShare);

		slog_debug(0, "[%s.%s-%lld]: 当前买涨停价撤单数量[%lld] 当前买涨停价委托数量[%lld] =[%lld]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, snapshot->_catchUpBidCQty,
				snapshot->_catchUpBidQty,
				snapshot->_catchUpBidQty > 0 ?
						snapshot->_catchUpBidCQty * 10000
								/ snapshot->_catchUpBidQty :
						0);

		slog_debug(0,
				"[%s.%s-%lld]: 涨停后成交量[%lld] / 封单量[%lld] = [%lld],大单买[%d],大单撤[%d],涨停价大单[%d],涨停价买入次数[%d]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, snapshot->_catchUpTrdQty, totalBidQty,
				totalBidQty > 0 ?
						10000 * snapshot->_catchUpTrdQty / totalBidQty : 0,
				snapshot->_catchUpBidBuyCnt, snapshot->_catchUpBidCBuyCnt,
				snapshot->secUpBigBuyCnt, snapshot->secUpBuyTimes);
	}

	// 最新价不等于条件价
	/**
	 if (snapshot->tradePx != pStrategy->setting.conPx && !snapshot->secUpBigBuyCnt && !snapshot->secUpBuyTimes) {
	 return;
	 }
	 */
	if (!(snapshot->tradePx == pStrategy->setting.conPx
			|| (-1 == pStrategy->setting.askQty && snapshot->secUpBigBuyCnt > 2))) {
		return;
	}
	//10点后，涨多跌少的，指数为正，放宽条件买入

	// 昨日涨停且放量
	/**
	 if(pStock->upperTimes > 0 && pStock->ysMultiple)
	 {
	 if(snapshot->updateTime >= 130000000)
	 {
	 ysVolTrades = pStock->ysVolumeTrade *
	 (XMsC2S(snapshot->updateTime) / 1000 - 90 * 60 - XMsC2S(93000000) / 1000) /
	 240;
	 }
	 else
	 {
	 ysVolTrades = pStock->ysVolumeTrade *
	 (XMsC2S(snapshot->updateTime) / 1000 -  XMsC2S(93000000) / 1000) / 240;
	 }


	 //前一个板放量,缩量才下单
	 if(ysVolTrades < 2 * snapshot->volumeTrade)
	 {
	 return;
	 }
	 slog_debug(0, "[%lld-%d-%s] 涨停次数[%d]
	 昨日是否放量[%d],今日量[%lld]", pStrategy->plotid, snapshot->market,
	 snapshot->securityId, pStock->upperTimes, pStock->ysMultiple, ysVolTrades);

	 }
	 */
	// 只要是涨停,都打印
	if (!pStrategy->_signal) {
		slog_debug(0,
				"--------------------------------------------------------------------------------------------");

		totalBidQty =
				snapshot->bidqty[0] - snapshot->askqty[0] > 0 ?
						snapshot->bidqty[0] - snapshot->askqty[0] : 0;
		totalBidAmt = totalBidQty / 10000 * snapshot->bid[0];
		if (snapshot->amountTrade != 0) {
			bidTradRatio = totalBidAmt * 10000 * 10000 / snapshot->amountTrade;
		}

		slog_debug(0,
				"[%s.%s-%lld]:最新时间[%d] 最新价[%d],条件价[%d], 开盘价[%d],卖一[%d-%lld],买一[%d-%lld], 封单金额[%lld(%lld)],撤单率[%d(%d)],涨停次数[%d],封单占成交[%lld],封单占流通[%lld]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, snapshot->updateTime, snapshot->tradePx,
				pStrategy->setting.conPx, snapshot->openPx, snapshot->ask[0],
				snapshot->askqty[0], snapshot->bid[0], snapshot->bidqty[0],
				totalBidAmt, pStrategy->setting.buyMoney, pStrategy->_cdl,
				pStrategy->setting.cdl, snapshot->_upperTimes, bidTradRatio,
				totalBidQty * 10000 / pStock->publicfloatShare);

		slog_debug(0, "[%s.%s-%lld]: 当前撤单数量[%lld] / 当前委托数量[%lld] =[%lld]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, snapshot->_catchUpBidCQty,
				snapshot->_catchUpBidQty,
				snapshot->_catchUpBidQty > 0 ?
						snapshot->_catchUpBidCQty * 10000
								/ snapshot->_catchUpBidQty :
						0);

		slog_debug(0,
				"[%s.%s-%lld]: 涨停后成交量[%lld] / 封单量[%lld] = [%lld], 大单买[%d],大单撤[%d]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, snapshot->_catchUpTrdQty, totalBidQty,
				totalBidQty > 0 ?
						10000 * snapshot->_catchUpTrdQty / totalBidQty : 0,
				snapshot->_catchUpBidBuyCnt, snapshot->_catchUpBidCBuyCnt);
		slog_debug(0, "[%s.%s-%lld]: 集合竞价成交量比[%d]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid,
				10000 * snapshot->auctionQty / pStock->publicfloatShare);
	}

	allowBuyQty = XGetCanBuyQty(pStrategy, pStock->lmtBuyMinQty);

	slog_debug(0, "[%s.%s-%lld]:1.允许买量[%d]", pStrategy->setting.securityId,
			pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
			pStrategy->plotid, allowBuyQty);
	if (allowBuyQty <= 0) {
		return;
	}

	//如果有连续大单,则只要涨停不看封单为0
	slog_debug(0,
			"[%s.%s-%lld]:1.允许买量[%d], 卖一量[%lld],计算卖一量[%lld],设定卖一量[%lld],涨停价大胆[%d], 涨停价买入次数[%d]",
			pStrategy->setting.securityId,
			pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
			pStrategy->plotid, allowBuyQty, snapshot->askqty[0],
			snapshot->askqty[0] - snapshot->bidqty[0],
			pStrategy->setting.askQty, snapshot->secUpBigBuyCnt,
			snapshot->secUpBuyTimes);
	/**涨停价，卖一量大于设定值不触发,(卖一量 - 买一量) 大于设定值 */
	/** 只有设置卖一不为0时才看连续大单 */
	if (-1 != pStrategy->setting.askQty) {
		if (snapshot->askqty[0] - snapshot->bidqty[0]
				> pStrategy->setting.askQty) {
			return;
		}
	} else {
		if (snapshot->tradePx != pStrategy->setting.conPx
				&& snapshot->secUpBigBuyCnt <= 2) {
			return;
		}
		slog_debug(0, "[%s.%s-%lld]: 1.tradepx[%d], secupbigbuycnt[%d]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, snapshot->tradePx, snapshot->secUpBigBuyCnt);
	}

	totalBidQty =
			snapshot->bidqty[0] - snapshot->askqty[0] > 0 ?
					snapshot->bidqty[0] - snapshot->askqty[0] : 0;
	totalBidAmt = totalBidQty / 10000 * snapshot->bid[0];

	slog_debug(0,
			"[%s.%s-%lld]:2.达到卖一量条件, 最新时间[%d],最新价[%d],卖一价[%d],卖一量[%lld(%lld)]",
			pStrategy->setting.securityId,
			pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
			pStrategy->plotid, snapshot->updateTime, snapshot->tradePx,
			snapshot->ask[0], snapshot->askqty[0], pStrategy->setting.askQty);

	slog_debug(0,
			"[%s.%s-%lld]:3.达到买一金额条件, 最新价[%d],买一价[%d] * 买一量[%lld] = 买一金额[%lld(%lld)]",
			pStrategy->setting.securityId,
			pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
			pStrategy->plotid, snapshot->tradePx, snapshot->bid[0],
			snapshot->bidqty[0], totalBidAmt, pStrategy->setting.buyMoney);
	/** 涨停价,买一金额小于设定金额不触发 */
	if (0 != pStrategy->setting.buyMoney
			&& totalBidAmt < pStrategy->setting.buyMoney) {
		return;
	}

	slog_debug(0,
			"[%s.%s-%lld]:4.达到买一金额条件, 最新价[%d],买一价[%d] * 买一量[%lld] = 买一金额[%lld(%lld)]",
			pStrategy->setting.securityId,
			pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
			pStrategy->plotid, snapshot->tradePx, snapshot->bid[0],
			snapshot->bidqty[0], totalBidAmt, pStrategy->setting.buyMoney);

	if (pStrategy->setting.nxtCtrlMoney > 0 && pStrategy->_hasCtrl
			&& (snapshot->_sealCursor + 1) == snapshot->kcursor1) {
		return;
	}
	//超过最大报单数量的下单拆单
	while (allowBuyQty > 0) {
		if (allowBuyQty > pStock->lmtBuyMaxQty) {
			realOrdQty = pStock->lmtBuyMaxQty;
		} else {
			realOrdQty = allowBuyQty;
		}

		idx = XGetBuyStorePos(pStrategy);
		if (idx < 0) {
			return;
		}

		CPStrategy2Ord(pStrategy, &order);

		order.bsType = eXBuy;
		order.ordPrice = pStrategy->setting.ordPx;
		order.ordQty = realOrdQty;
		order.ordType = eXOrdLimit;

		order._lastPx = snapshot->tradePx;
		order._lastTime = snapshot->updateTime;
		order._mktTime = snapshot->_recvTime;
		order._bizIndex = snapshot->_bizIndex;

		localid = XPutOrderReq(&order);
		pStrategy->buyList[idx] = localid;
		pStrategy->_buySends += order.ordQty;
		pStrategy->_lastBuyPx = order.ordPrice;
		pStrategy->_buyLocTime = XGetClockTime();
		pStrategy->_signal = 1;

		XST_BUY_LOG(pStrategy, order);

		allowBuyQty -= realOrdQty;
	}
	// TODO 记录下单时的大单信息，在发起撤单时，如果大单成交或者撤单，不再下单
}

/**
 * 低吸:935-9:40
 * 已有持仓，下跌超-4%补仓1/3；
 *
 * 收盘后
 * 策略2收盘后调整：
 * 1，盈利5%以上，归“策略1”；
 * 2，亏损-5%，归“策略1”；
 * 3，买入后10个交易日未达止盈止损，归“策略1”；
 */
void BasketBuy2(XStrategyT *pStrategy, XRSnapshotT *snapshot, XStockT *pStock) {
	XQty allowBuyQty = -1, realOrdQty = -1;
	XNum idx = 0;
	XOrderReqT order = { 0 };
	XLocalId localid = 0;
	XInt zdf = 0;
	XHoldT *pHold = NULL;

	if (pStrategy->version == snapshot->version) {
		return;
	}
	pStrategy->version = snapshot->version;

	// 如果涨跌幅 > -4 买入
	zdf = (snapshot->tradePx - snapshot->preClosePx) * 10000
			/ snapshot->preClosePx;
	if (zdf > -400) {
		return;
	}

	slog_debug(0, "[%s.%s-%lld]: 1. 涨幅[%d]", pStrategy->setting.securityId,
			pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
			pStrategy->plotid, zdf);

	pHold = XFndVHoldByKey(pStrategy->plot.customerId, pStrategy->investId,
			pStrategy->setting.market, pStrategy->setting.securityId);
	if (NULL != pHold) {
		if (pHold->orgAvlHld <= 0) {
			return;
		}
		pStrategy->setting.ordQty = pHold->orgAvlHld / 3;
	}

	allowBuyQty = XGetCanBuyQty(pStrategy, pStock->lmtBuyMinQty);

	if (allowBuyQty <= 0) {
		return;
	}

	while (allowBuyQty > 0) {
		if (allowBuyQty > pStock->lmtBuyMaxQty) {
			realOrdQty = pStock->lmtBuyMaxQty;
		} else {
			realOrdQty = allowBuyQty;
		}
		idx = XGetBuyStorePos(pStrategy);
		if (idx < 0) {
			return;
		}

		CPStrategy2Ord(pStrategy, &order);

		order.bsType = eXBuy;
		order.ordPrice = snapshot->tradePx;
		order.ordQty = realOrdQty;
		order.ordType = eXOrdLimit;

		order._lastPx = snapshot->tradePx;
		order._lastTime = snapshot->updateTime;
		order._mktTime = snapshot->_recvTime;
		order._bizIndex = snapshot->_bizIndex;

		localid = XPutOrderReq(&order);
		pStrategy->buyList[idx] = localid;
		pStrategy->_buySends += order.ordQty;
		pStrategy->_lastBuyPx = order.ordPrice;
		pStrategy->_buyLocTime = XGetClockTime();
		pStrategy->_signal = 1;

		XST_BUY_LOG(pStrategy, order);

		allowBuyQty -= realOrdQty;
	}
}

/**
 * 科创板和创业板半路板
 * 一分钟放量1千万或3分钟放量5000千万,涨速> 200且当前涨幅>3%
 */
void BasketBuy3(XStrategyT *pStrategy, XRSnapshotT *snapshot, XStockT *pStock) {
	XQty allowBuyQty = -1, realOrdQty = -1;
	XNum idx = 0;
	XOrderReqT order = { 0 };
	XLocalId localid = 0;
	XInt zdf = 0, zs = 0;
	XKLineT *kline1 = NULL, *kline5 = NULL;
	XInt cursor = -1, lcursor = -1, lcursor5 = -1, cursor5 = -1;
	XMoney amt = 0, preamt = 0;
	XPrice curPx = 0;
	XRatio zf = 0;
	XPrice avgPx = -1;

	if (pStrategy->version == snapshot->version) {
		return;
	}
	pStrategy->version = snapshot->version;

	// 9:31分之前涨速计算存在历史K线数据不正确
	if (snapshot->updateTime < 93100000) {
		return;
	}

	// 如果涨跌幅 > -4 买入
	zdf = (snapshot->tradePx - snapshot->preClosePx) * 10000
			/ snapshot->preClosePx;

	// 振幅
	zf = (snapshot->highPx - snapshot->lowPx) * 10000 / snapshot->preClosePx;

	if (zdf < pStrategy->setting.conPx /* 最低涨幅 */) {
		return;
	}

	// 根据证券类型判断在高位的时候不做半路板
	if (pStock->subSecType == eXSubSecASH || pStock->subSecType == eXSubSecSME
			|| pStock->subSecType == eXSubSecCDR
			|| pStock->subSecType == eXSubSecHLT) {
		if (zdf > pStrategy->setting.ordPx /* 最高涨幅 */) {
			return;
		}
	} else {
		if (zdf > pStrategy->setting.ordPx + 700 /* 最高涨幅 */) {
			return;
		}
	}

	//大单净买入额
	/**
	 if (snapshot->netBigTrdMoney <= 0) {
	 return;
	 }
	 */
	/**
	 slog_debug(0, "[%lld-%d-%s] 1. 最高[%d],最低[%d],昨收[%d],振幅[%d],涨幅[%d]",
	 pStrategy->plotid, pStrategy->setting.market, pStrategy->setting.securityId,
	 snapshot->highPx, snapshot->lowPx, snapshot->preClosePx, zf, zdf);

	 //大单交易量比较大
	 if((snapshot->bigBuyOrdAmt + snapshot->bigSellOrdAmt) < 0.3 *
	 snapshot->amountTrade)
	 {
	 return;
	 }

	 slog_debug(0, "[%lld-%d-%s] 2. 大单交易量[%.2f], 成交量[%.2f]",
	 pStrategy->plotid, pStrategy->setting.market, pStrategy->setting.securityId,
	 (snapshot->bigBuyOrdAmt + snapshot->bigSellOrdAmt) * 0.0001,
	 snapshot->amountTrade * 0.0001);

	 //卖单>买单
	 if(1.3 * snapshot->totalSellCnt < snapshot->totalBuyCnt)
	 {
	 return;
	 }
	 slog_debug(0, "[%lld-%d-%s] 3. 卖单次数[%d], 买单次数[%d]",
	 pStrategy->plotid, pStrategy->setting.market, pStrategy->setting.securityId,
	 snapshot->totalSellCnt, snapshot->totalBuyCnt);
	 */

	if (snapshot->updateTime > 93000000) {
		// 外盘 > 内盘 且 外盘成交 > 流通盘的* 0.3
		if (!(snapshot->outsideTrdAmt > snapshot->insideTrdAmt
				&& snapshot->outsideTrdAmt > 0.00006 * pStrategy->setting.askQty /** 内盘与流通盘比例 */
				* pStock->publicfloatShare * snapshot->tradePx)) {
			return;
		}
	} else {
		// 外盘 > 内盘 且 外盘成交 > 流通盘的* 0.3
		if (!(snapshot->outsideTrdAmt > snapshot->insideTrdAmt
				&& snapshot->outsideTrdAmt > 0.0001 * pStrategy->setting.askQty /** 内盘与流通盘比例 */
				* pStock->publicfloatShare * snapshot->tradePx)) {
			return;
		}
	}
	slog_debug(0,
			"[%s.%s-%lld]: 1. 今日涨停卖量[%lld] > 昨日涨停卖量[%lld] * 倍数[%d] * 0.0001 = (%lld),时间[%d]",
			pStrategy->setting.securityId,
			pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
			pStrategy->plotid, snapshot->upperOfferOrdQty,
			pStock->upperOfferOrdQty, pStrategy->setting.upperQtyMulty,
			pStrategy->setting.upperQtyMulty
					* pStock->upperOfferOrdQty / 10000, snapshot->updateTime);
	// TODO 涨停价卖挂单量 2023.10.6
	if (snapshot->upperOfferOrdQty
			< pStock->upperOfferOrdQty * pStrategy->setting.upperQtyMulty /** 涨停与昨日倍量 */
					 / 10000) {
		return;
	}

	slog_debug(0,
			"[%s.%s-%lld]: 2. 主买金额[%.2f]万元, 主卖金额[%.2f]万元, 流通市值[%.2f]万元, 时间[%d]",
			pStrategy->setting.securityId,
			pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
			pStrategy->plotid, snapshot->outsideTrdAmt * 0.00000001,
			snapshot->insideTrdAmt * 0.00000001,
			0.00000001 * pStock->publicfloatShare * snapshot->tradePx,
			snapshot->updateTime);

	kline1 = GetKlinesByBlock(snapshot->idx, 0);

	cursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 1) & (SNAPSHOT_K1_CNT - 1);
	amt = kline1[cursor].amt;
	curPx = kline1[cursor].close;

	// 前3分钟成交
	lcursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 2) & (SNAPSHOT_K1_CNT - 1);
	preamt = kline1[lcursor].amt;

	if (kline1[lcursor].close) {
		zs = (curPx - kline1[lcursor].close) * 10000 / kline1[lcursor].close;
	}

	lcursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 3) & (SNAPSHOT_K1_CNT - 1);
	preamt += kline1[lcursor].amt;

	lcursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 4) % (SNAPSHOT_K1_CNT - 1);
	preamt += kline1[lcursor].amt;

	if (snapshot->volumeTrade) {
		avgPx = snapshot->amountTrade / snapshot->volumeTrade;
	}

	if (snapshot->updateTime < 93500000) {
		slog_debug(0,
				"[%s.%s-%lld]: 3. 1分钟成交金额[%.2f(%.2f)],5分钟成交金额[%.2f(%2.f)], 涨速[%d(%d)], 涨跌幅[%d],振幅[%d],时间[%d]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, amt * 0.0001,
				pStrategy->setting.buyMoney * 1.5, preamt * 0.0001,
				pStrategy->setting.buyCtrlMoney * 1.0, zs,
				pStrategy->setting.cdl, zdf, zf, snapshot->updateTime);
		if (!((amt > pStrategy->setting.buyMoney * 15000
				|| preamt > pStrategy->setting.buyCtrlMoney * 15000)
				&& zs > pStrategy->setting.cdl && snapshot->tradePx > avgPx
				&& snapshot->tradePx > kline1[cursor].open
				&& snapshot->tradePx > kline1[cursor].close)) {
			return;
		}
	} else {
		slog_debug(0,
				"[%s.%s-%lld]: 3. 1分钟成交金额[%.2f(%.2f)],5分钟成交金额[%.2f(%.2f)], 涨速[%d(%d)], 涨跌幅[%d],振幅[%d],时间[%d]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, amt * 0.0001,
				pStrategy->setting.buyMoney * 1.0, preamt * 0.0001,
				pStrategy->setting.buyCtrlMoney * 1.0, zs,
				pStrategy->setting.cdl, zdf, zf, snapshot->updateTime);
		if (!((amt > pStrategy->setting.buyMoney * 10000
				|| preamt > pStrategy->setting.buyCtrlMoney * 10000)
				&& zs > pStrategy->setting.cdl
				&& snapshot->tradePx > kline1[cursor].open
				&& snapshot->tradePx > kline1[cursor].close)) {
			return;
		}
	}

	// 当前涨停卖的委托量是前几分钟卖的多少倍 2023.10.6
	kline5 = GetKlinesByBlock(snapshot->idx, 1);
	cursor5 = (SNAPSHOT_K5_CNT + snapshot->kcursor5 - 1) % SNAPSHOT_K5_CNT;
	lcursor5 = (SNAPSHOT_K5_CNT + snapshot->kcursor5 - 4) % SNAPSHOT_K5_CNT;

	slog_debug(0, "[%s.%s-%lld]: 4. 涨停卖量[%lld],上一次涨停卖量[%lld],涨停卖量[%d]",
			pStrategy->setting.securityId,
			pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
			pStrategy->plotid, kline5[cursor5].upperSellOrdQty,
			kline5[lcursor5].upperSellOrdQty,
			pStrategy->setting.upperQtyMultyMin);
	if (kline5[cursor5].upperSellOrdQty
			< kline5[lcursor5].upperSellOrdQty * pStrategy->setting.upperQtyMultyMin
					 / 10000) {
		return;
	}

	slog_debug(0,
			"[%s.%s-%lld]:  5. 托单数量[%lld],压单数量[%lld], 压单/托单比例[%.2f(%lld)], 压单笔数[%d(%lld)]",
			pStrategy->setting.securityId,
			pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
			pStrategy->plotid, snapshot->bigBuyOrdQty, snapshot->bigSellOrdQty,
			snapshot->bigBuyOrdQty ?
					snapshot->bigSellOrdQty * 1.0 / snapshot->bigBuyOrdQty : 0,
			pStrategy->setting.nxtCtrlMoney, snapshot->bigSellOrdCnt,
			pStrategy->setting.followCtrlMoney);
	//托单压单
	if (snapshot->bigSellOrdCnt
			< pStrategy->setting.followCtrlMoney/** 压单量 */) {
		return;
	}

	//比例
	if (snapshot->bigBuyOrdQty > 0
			&& snapshot->bigSellOrdQty * 10000 / snapshot->bigBuyOrdQty
					< pStrategy->setting.nxtCtrlMoney) {
		return;
	}

	allowBuyQty = XGetCanBuyQty(pStrategy, pStock->lmtBuyMinQty);

	if (allowBuyQty <= 0) {
		return;
	}

	while (allowBuyQty > 0) {
		if (allowBuyQty > pStock->lmtBuyMaxQty) {
			realOrdQty = pStock->lmtBuyMaxQty;
		} else {
			realOrdQty = allowBuyQty;
		}
		idx = XGetBuyStorePos(pStrategy);
		if (idx < 0) {
			return;
		}

		CPStrategy2Ord(pStrategy, &order);

		order.bsType = eXBuy;
		order.ordPrice = snapshot->tradePx;
		order.ordQty = realOrdQty;
		order.ordType = eXOrdLimit;

		order._lastPx = snapshot->tradePx;
		order._lastTime = snapshot->updateTime;
		order._mktTime = snapshot->_recvTime;
		order._bizIndex = snapshot->_bizIndex;

		localid = XPutOrderReq(&order);
		pStrategy->buyList[idx] = localid;
		pStrategy->_buySends += order.ordQty;
		pStrategy->_lastBuyPx = order.ordPrice;
		pStrategy->_buyLocTime = XGetClockTime();
		pStrategy->_signal = 1;

		allowBuyQty -= realOrdQty;

		XST_BUY_LOG(pStrategy, order);

	}
}

/**
 * 科创板和创业板半路板
 * 一分钟放量1千万或3分钟放量5000千万,涨速> 200且当前涨幅>3%
 */
void BasketBuy5(XStrategyT *pStrategy, XRSnapshotT *snapshot, XStockT *pStock) {
	XQty allowBuyQty = -1, realOrdQty = -1;
	XNum idx = 0;
	XOrderReqT order = { 0 };
	XLocalId localid = 0;
	XInt zdf = 0, zs = 0;
	XKLineT *kline1 = NULL;
	XInt cursor = -1, lcursor = -1;
	XMoney amt = 0, preamt = 0, preamt3 = 0;
	XPrice curPx = 0;
	XRatio zf = 0;

	if (pStrategy->version == snapshot->version) {
		return;
	}
	pStrategy->version = snapshot->version;

	// 9:31分之前涨速计算存在历史K线数据不正确
	if (snapshot->updateTime < 93100000) {
		return;
	}

	//如果买入是同一分钟,不再买入
	if (pStrategy->updateTime / 100000 == snapshot->updateTime / 100000) {
		return;
	}

	// 如果涨跌幅 > -4 买入
	zdf = (snapshot->tradePx - snapshot->preClosePx) * 10000
			/ snapshot->preClosePx;

	// 振幅
	zf = (snapshot->highPx - snapshot->lowPx) * 10000 / snapshot->preClosePx;

	if (zdf < pStrategy->setting.conPx /* 最低涨幅 */) {
		return;
	}

	// 根据证券类型判断在高位的时候不做半路板
	if (pStock->subSecType == eXSubSecASH || pStock->subSecType == eXSubSecSME
			|| pStock->subSecType == eXSubSecCDR
			|| pStock->subSecType == eXSubSecHLT) {
		if (zdf > pStrategy->setting.ordPx /* 最高涨幅 */) {
			return;
		}
	} else {
		if (zdf > pStrategy->setting.ordPx + 700 /* 最高涨幅 */) {
			return;
		}
	}

	kline1 = GetKlinesByBlock(snapshot->idx, 0);

	cursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 1) & (SNAPSHOT_K1_CNT - 1);
	amt = kline1[cursor].amt;
	curPx = kline1[cursor].close;

	// 前3分钟成交
	lcursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 2) &(SNAPSHOT_K1_CNT - 1);
	preamt = kline1[lcursor].amt;
	preamt3 = kline1[lcursor].driverBuyAmt - kline1[lcursor].driverSellAmt;

	if (kline1[lcursor].close) {
		zs = (curPx - kline1[lcursor].close) * 10000 / kline1[lcursor].close;
	}

	lcursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 3) &(SNAPSHOT_K1_CNT - 1);
	preamt += kline1[lcursor].amt;
	preamt3 += kline1[lcursor].driverBuyAmt - kline1[lcursor].driverSellAmt;

	lcursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 4) & (SNAPSHOT_K1_CNT - 1);
	preamt += kline1[lcursor].amt;
	preamt3 += kline1[lcursor].driverBuyAmt - kline1[lcursor].driverSellAmt;

	if (!((amt > pStrategy->setting.buyMoney * 10000
			|| preamt > pStrategy->setting.buyCtrlMoney * 10000)
			&& zs > pStrategy->setting.cdl
			&& snapshot->tradePx > kline1[cursor].open && preamt3 > 0)) {
		return;
	}

	slog_debug(0,
			"[%s.%s-%lld]: 3. 1分钟成交金额[%.2f(%.2f)],5分钟成交金额[%.2f(%.2f)], 涨速[%d(%d)], 涨跌幅[%d],振幅[%d],时间[%d]",
			pStrategy->setting.securityId,
			pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
			pStrategy->plotid, amt * 0.0001, pStrategy->setting.buyMoney * 1.0,
			preamt * 0.0001, pStrategy->setting.buyCtrlMoney * 1.0, zs,
			pStrategy->setting.cdl, zdf, zf, snapshot->updateTime);

	allowBuyQty = pStrategy->setting.ordQty;

	pStrategy->_slipBuyTimes++;
	pStrategy->updateTime = snapshot->updateTime;

	if (pStrategy->_slipBuyTimes < 2 || pStrategy->_slipBuyTimes > 6) {
		return;
	}

	while (allowBuyQty > 0) {
		if (allowBuyQty > pStock->lmtBuyMaxQty) {
			realOrdQty = pStock->lmtBuyMaxQty;
		} else {
			realOrdQty = allowBuyQty;
		}
		idx = XGetBuyStorePos(pStrategy);
		if (idx < 0) {
			return;
		}

		CPStrategy2Ord(pStrategy, &order);

		order.bsType = eXBuy;
		order.ordPrice = snapshot->tradePx;
		order.ordQty = realOrdQty;
		order.ordType = eXOrdLimit;

		order._lastPx = snapshot->tradePx;
		order._lastTime = snapshot->updateTime;
		order._mktTime = snapshot->_recvTime;
		order._bizIndex = snapshot->_bizIndex;

		localid = XPutOrderReq(&order);
		pStrategy->buyList[idx] = localid;
		pStrategy->_buySends += order.ordQty;
		pStrategy->_lastBuyPx = order.ordPrice;
		pStrategy->_buyLocTime = XGetClockTime();
		pStrategy->_signal = 1;

		allowBuyQty -= realOrdQty;

		XST_BUY_LOG(pStrategy, order);
	}
}

static XBool BuyCtrl(XStrategyT *pStrategy, XRSnapshotT *snapshot,
		XStockT *pStock) {
	XLongTime curTime = 0;
	XMoney totalBidMoney = 0;
	XSumQty totalBidQty = 0;
	XNum kcursor1, kcursor2;
	XKLineT *k1 = NULL;

	if (5 != pStrategy->setting.sign
			&& pStrategy->_buyTrades >= pStrategy->setting.ordQty) {
		slog_debug(0, "[%s.%s-%lld]:!!!!!! 策略暂停, 更新时间[%d]已全部成交[%d]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, snapshot->updateTime,
				pStrategy->setting.ordQty);
		pStrategy->status = eXPlotStop;
		return (true);
	}

	if (pStrategy->plot.slipBuyTimes != 0
			&& pStrategy->_slipBuyTimes >= pStrategy->plot.slipBuyTimes) {
		pStrategy->status = eXPlotStop;
		slog_debug(0, "[%s.%s-%lld]:!!!!!! 策略暂停, 超过滑点次数[%d(%d)],时间[%d]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, pStrategy->_slipBuyTimes,
				pStrategy->plot.slipBuyTimes, snapshot->updateTime);
		return (true);
	}

	if (0 == pStrategy->setting.sign) {
		/** 开盘涨停的不买 */
		if (pStrategy->plot.isOpenStop && snapshot->openPx == pStock->upperPrice) {
			pStrategy->status = eXPlotStop;
			slog_debug(0, "[%s.%s-%lld]:!!!!!! 策略暂停, 开盘价[%d]=涨停价[%d]",
					pStrategy->setting.securityId,
					pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
					pStrategy->plotid, snapshot->openPx, pStock->upperPrice);
			return (true);
		}

		/** 启动前已经涨停 */
		if (pStrategy->plot.isOpenStop && snapshot->_upperTimes > 0
				&& snapshot->updateTime < pStrategy->plot.beginTime) {
			pStrategy->status = eXPlotStop;
			slog_debug(0,
					"[%s.%s-%lld]:!!!!!! 策略暂停,在开始交易时间前已经涨停,当前时间[%d],开始时间[%d]",
					pStrategy->setting.securityId,
					pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
					pStrategy->plotid, snapshot->updateTime,
					pStrategy->plot.beginTime);
			return (true);
		}

		/** 涨停打开的不买 */
		if (pStrategy->plot.isUpperStop && snapshot->_upperTimes > 1) {
			pStrategy->status = eXPlotStop;
			slog_debug(0, "[%s.%s-%lld]:!!!!!! 策略暂停, 多次打开涨停[%d]",
					pStrategy->setting.securityId,
					pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
					pStrategy->plotid, snapshot->_upperTimes);
			return (true);
		}

		/** 计算实际撤单率,(卖单+买单) / 买委托量 */
		if (0 != snapshot->_catchUpBidQty) {
			pStrategy->_cdl = snapshot->_catchUpBidCQty * 10000
					/ snapshot->_catchUpBidQty;
		} else {
			pStrategy->_cdl = -1;
		}

		// 最大的买一委托金额
		if (pStrategy->_signal && pStrategy->plot.isAutoCtrl) {
			totalBidQty =
					snapshot->bidqty[0] - snapshot->askqty[0] > 0 ?
							snapshot->bidqty[0] - snapshot->askqty[0] : 0;
			totalBidMoney = totalBidQty / 10000 * snapshot->bid[0];
			pStrategy->_buyCtrlMoney =
					pStrategy->_buyCtrlMoney > totalBidMoney ?
							pStrategy->_buyCtrlMoney : totalBidMoney;

			// 下单后封单金额不足撤单
			if (pStrategy->setting.buyMoney > 0
					&& totalBidMoney < pStrategy->setting.buyMoney) {
				curTime = XGetClockTime();
				if (pStrategy->setting.market
						== eXMarketSha&& curTime > pStrategy->_buyLocTime + 30 * XTIMS_MS4NS && curTime < pStrategy->_buyLocTime + 1000 * XTIMS_MS4NS) {

					if (XCancelBuy(pStrategy, 1, __LINE__)) {
						slog_debug(0,
								"[%s.%s-%lld]: 下单后30ms封单金额不足,最新时间[%d - %lld - %lld],最新价格[%d],当前最大封单金额[%lld],最新封单金额[%lld]",
								pStrategy->setting.securityId,
								pStrategy->setting.market == eXMarketSha ?
										"SH" : "SZ", pStrategy->plotid,
								snapshot->updateTime, curTime,
								pStrategy->_buyLocTime, snapshot->tradePx,
								pStrategy->_buyCtrlMoney, totalBidMoney);
						pStrategy->setting.buyMoney =
								pStrategy->setting.buyMoney
										+ pStrategy->setting.buyMoney
												* pStrategy->plot.ctrlUpRatio
												* 0.0001; // 撤单后再次下单封单金额加0.5倍
						pStrategy->_signal = 0;
						pStrategy->_slipBuyTimes++;
					}
				} else if (pStrategy->setting.market
						== eXMarketSza&& curTime > pStrategy->_buyLocTime + 3 * XTIMS_MS4NS && curTime < pStrategy->_buyLocTime + 1000 * XTIMS_MS4NS) {

					if (XCancelBuy(pStrategy, 1, __LINE__)) {
						slog_debug(0,
								"[%s.%s-%lld]:下单后3ms封单金额不足,最新时间[%d - %lld - %lld],最新价格[%d],当前最大封单金额[%lld],最新封单金额[%lld]",
								pStrategy->setting.securityId,
								pStrategy->setting.market == eXMarketSha ?
										"SH" : "SZ", pStrategy->plotid,
								snapshot->updateTime, curTime,
								pStrategy->_buyLocTime, snapshot->tradePx,
								pStrategy->_buyCtrlMoney, totalBidMoney);
						pStrategy->setting.buyMoney =
								pStrategy->setting.buyMoney
										+ pStrategy->setting.buyMoney
												* pStrategy->plot.ctrlUpRatio
												* 0.0001; // 撤单后再次下单封单金额加0.5倍
						pStrategy->_signal = 0;
						pStrategy->_slipBuyTimes++;
					}
				}
			}

			// 如果价格不是条件价格,当前封单距最大封单不足10%撤单
			if ((pStrategy->setting.conPx != snapshot->tradePx
					|| (pStrategy->_buyCtrlMoney
							>= pStrategy->setting.buyCtrlMoney
							&& (totalBidMoney < pStrategy->setting.buyCtrlMoney
									|| (totalBidMoney
											< 0.1 * pStrategy->_buyCtrlMoney
											&& XMsC2S(snapshot->updateTime)
													- XMsC2S(
															snapshot->_sealTime)
													< 20000))))) {

				if (XCancelBuy(pStrategy, 1, __LINE__)) {
					slog_debug(0,
							"[%s.%s-%lld]: 最新时间[%d],最新价格[%d],当前最大封单金额[%lld],最新封单金额[%lld]",
							pStrategy->setting.securityId,
							pStrategy->setting.market == eXMarketSha ?
									"SH" : "SZ", pStrategy->plotid,
							snapshot->updateTime, snapshot->tradePx,
							pStrategy->_buyCtrlMoney, totalBidMoney);

					pStrategy->setting.buyMoney = pStrategy->setting.buyMoney
							+ pStrategy->setting.buyMoney
									* pStrategy->plot.ctrlUpRatio * 0.0001; // 撤单后再次下单封单金额加0.5倍
					pStrategy->_buyCtrlMoney = 0;
					pStrategy->_buyCtrlMoney = totalBidMoney;
					pStrategy->_signal = 0;
					pStrategy->_slipBuyTimes++;
				}
			}

			// 板上成交或撤单/委托大于80%且小于3000千万撤单
			if (pStrategy->setting.cdl > 0
					&& pStrategy->_cdl > pStrategy->setting.cdl
					&& XMsC2S(snapshot->updateTime)
							- XMsC2S(snapshot->_sealTime) > 60000
					&& totalBidMoney < pStrategy->setting.buyCtrlMoney) {

				if (XCancelBuy(pStrategy, 1, __LINE__)) {
					pStrategy->setting.buyMoney = pStrategy->setting.buyMoney
							+ pStrategy->setting.buyMoney
									* pStrategy->plot.ctrlUpRatio * 0.0001; // 撤单后再次下单封单金额加0.5倍
					slog_debug(0,
							"[%s.%s-%lld]: 板上撤单or成交过大,最新时间[%d - %lld],最新价格[%d],当前封单金额[%lld],当前撤单率[%d]",
							pStrategy->setting.securityId,
							pStrategy->setting.market == eXMarketSha ?
									"SH" : "SZ", pStrategy->plotid,
							snapshot->updateTime, pStrategy->_buyLocTime,
							snapshot->tradePx, totalBidMoney, pStrategy->_cdl);
					pStrategy->_signal = 0;
					pStrategy->_slipBuyTimes++;
				}
			}

			// 如果是封涨停,下一分钟成交量大于1000万撤单，后续任意连续两分钟成交量大于800万撤单
			if (snapshot->_sealCursor != 0) {
				kcursor1 = (snapshot->kcursor1 - 1) & (SNAPSHOT_K1_CNT - 1);
				k1 = GetKlinesByBlock(snapshot->idx, 0);
				if (pStrategy->setting.nxtCtrlMoney > 0
						&& (snapshot->_sealCursor + 1) == snapshot->kcursor1) {
					if (k1[kcursor1].amt * 0.00000001
							>= pStrategy->setting.nxtCtrlMoney) {
						if (XCancelBuy(pStrategy, 1, __LINE__)) {
							pStrategy->setting.buyMoney =
									pStrategy->setting.buyMoney; // 撤单后再次下单封单金额加0.5倍
							slog_debug(0,
									"[%s.%s-%lld]: 封涨后1分钟成交量破千万,最新时间[%d - %lld],最新价格[%d],当前封单金额[%lld],当前撤单率[%d]",
									pStrategy->setting.securityId,
									pStrategy->setting.market == eXMarketSha ?
											"SH" : "SZ", pStrategy->plotid,
									snapshot->updateTime,
									pStrategy->_buyLocTime, snapshot->tradePx,
									totalBidMoney, pStrategy->_cdl);
							pStrategy->_signal = 0;
							pStrategy->_slipBuyTimes++;
							pStrategy->_hasCtrl = true;
						}
					}
				}

				//如果刚封涨停的那刻一分钟成交

				if (pStrategy->setting.followCtrlMoney > 0
						&& snapshot->kcursor1 > snapshot->_sealCursor + 1) {
					// 如果连续2分钟破800万，不下单了
					kcursor2 = (snapshot->kcursor1 - 2) & (SNAPSHOT_K1_CNT - 1);
					if (k1[kcursor1].amt * 0.00000001
							>= pStrategy->setting.followCtrlMoney
							&& k1[kcursor2].amt * 0.00000001
									>= pStrategy->setting.followCtrlMoney) {
						if (XCancelBuy(pStrategy, 1, __LINE__)) {
							pStrategy->setting.buyMoney =
									pStrategy->setting.buyMoney
											+ pStrategy->setting.buyMoney
													* pStrategy->plot.ctrlUpRatio
													* 0.0001; // 撤单后再次下单封单金额加0.5倍
							slog_debug(0,
									"[%s.%s-%lld]: 连续2分钟成交量破800万,最新时间[%d - %lld],最新价格[%d],当前封单金额[%lld],当前撤单率[%d]",
									pStrategy->setting.securityId,
									pStrategy->setting.market == eXMarketSha ?
											"SH" : "SZ", pStrategy->plotid,
									snapshot->updateTime,
									pStrategy->_buyLocTime, snapshot->tradePx,
									totalBidMoney, pStrategy->_cdl);
							pStrategy->_signal = 0;
							pStrategy->_slipBuyTimes++;
							pStrategy->status = eXPlotStop;
							return (true);
						}
					}
				}
			}
		}
	}

	return (false);
}

XBool BasketTrade(XRSnapshotT *snapshot, XStrategyT *pStrategy, XStockT *pStock) {

	XBool bCtrl = false, bSend = false;

	if (snapshot->updateTime > pStrategy->plot.endTime) {
		slog_debug(0, "[%s.%s-%lld]:!!!!!! 策略暂停, 已超过运行时间[%d-%d]",
				pStrategy->setting.securityId,
				pStrategy->setting.market == eXMarketSha ? "SH" : "SZ",
				pStrategy->plotid, snapshot->updateTime,
				pStrategy->plot.endTime);
		pStrategy->status = eXPlotStop;
		return (true);
	}

	// 处理买
	if (eXBuy == pStrategy->setting.bsType) {
		bCtrl = XGetBuyCtrlStatus(pStrategy); // 获取撤单状态,如果已经撤单或者全部成交,把策略记录订单清掉

		//获得撤单后的状态是否撤单成功,撤单成功后才能继续申报,发送过撤单或未收到响应不再申报
		if (pStrategy->plot.isAutoCtrl && bCtrl) {
			pStrategy->_slipBuyTimes++;
			if (pStrategy->plot.isCtrlStop) {
				pStrategy->status = eXPlotStop;
				return (true);
			}
		}
		//根据条件处理撤单,撤单成功后如果还满足下单条件继续下单
		if (BuyCtrl(pStrategy, snapshot, pStock)) {
			return (true);
		}
	}

	// 不在时间范围的策略不运行
	if (snapshot->updateTime < pStrategy->plot.beginTime) {
		return (true);
	}

	/** 买 */
	if (eXBuy == pStrategy->setting.bsType) {

		//看能不能发送订单
		bSend = CanBuySend(pStrategy);
		if (!bSend) {
			return (true);
		}
		switch (pStrategy->setting.sign) {
		/** 抢板 */
		case 0:
			BasketBuy4(pStrategy, snapshot, pStock);
			break;
		/**
		//低吸
		case 1:
			BasketBuy2(pStrategy, snapshot, pStock);
			break;
		//科创板和创业板半路板
		case 2:
			BasketBuy3(pStrategy, snapshot, pStock);
			break;
		//半路板，多次买入
		case 5:
			BasketBuy5(pStrategy, snapshot, pStock);
			break;
		*/
		default:
			break;
		}
	}

	return (false);
}
