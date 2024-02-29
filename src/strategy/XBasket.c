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
			slog_debug(0, "!!!!!! [%lld-%d-%s],策略暂停, 开盘涨幅不够[%d]",
					pStrategy->plotid, pStrategy->setting.market,
					pStrategy->setting.securityId, kpzdf);
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
				"<<< [%lld-%d-%s]:最新时间[%d] 最新价[%d],条件价[%d],开盘价[%d],卖一[%d-%lld],买一[%d-%lld],封单金额[%lld(%lld)],撤单率[%d(%d)],涨停次数[%d],封单占成交[%lld],封单占流通[%lld]",
				pStrategy->plotid, snapshot->market, snapshot->securityId,
				snapshot->updateTime, snapshot->tradePx,
				pStrategy->setting.conPx, snapshot->openPx, snapshot->ask[0],
				snapshot->askqty[0], snapshot->bid[0], snapshot->bidqty[0],
				totalBidAmt, pStrategy->setting.buyMoney, pStrategy->_cdl,
				pStrategy->setting.cdl, snapshot->_upperTimes, bidTradRatio,
				totalBidQty * 10000 / pStock->publicfloatShare);

		slog_debug(0, "[%lld-%d-%s] 当前买涨停价撤单数量[%lld] 当前买涨停价委托数量[%lld] =[%lld]",
				pStrategy->plotid, snapshot->market, snapshot->securityId,
				snapshot->_catchUpBidCQty, snapshot->_catchUpBidQty,
				snapshot->_catchUpBidQty > 0 ?
						snapshot->_catchUpBidCQty * 10000
								/ snapshot->_catchUpBidQty :
						0);

		slog_debug(0,
				"[%lld-%d-%s] 涨停后成交量[%lld] / 封单量[%lld] = [%lld],大单买[%d],大单撤[%d],涨停价大单[%d],涨停价买入次数[%d]",
				pStrategy->plotid, snapshot->market, snapshot->securityId,
				snapshot->_catchUpTrdQty, totalBidQty,
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
	if (snapshot->tradePx != pStrategy->setting.conPx
			&& snapshot->secUpBigBuyCnt <= 2) {
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
				"<<< [%lld-%d-%s]:最新时间[%d] 最新价[%d],条件价[%d], 开盘价[%d],卖一[%d-%lld],买一[%d-%lld], 封单金额[%lld(%lld)],撤单率[%d(%d)],涨停次数[%d],封单占成交[%lld],封单占流通[%lld]",
				pStrategy->plotid, snapshot->market, snapshot->securityId,
				snapshot->updateTime, snapshot->tradePx,
				pStrategy->setting.conPx, snapshot->openPx, snapshot->ask[0],
				snapshot->askqty[0], snapshot->bid[0], snapshot->bidqty[0],
				totalBidAmt, pStrategy->setting.buyMoney, pStrategy->_cdl,
				pStrategy->setting.cdl, snapshot->_upperTimes, bidTradRatio,
				totalBidQty * 10000 / pStock->publicfloatShare);

		slog_debug(0, "[%lld-%d-%s] 当前撤单数量[%lld] / 当前委托数量[%lld] =[%lld]",
				pStrategy->plotid, snapshot->market, snapshot->securityId,
				snapshot->_catchUpBidCQty, snapshot->_catchUpBidQty,
				snapshot->_catchUpBidQty > 0 ?
						snapshot->_catchUpBidCQty * 10000
								/ snapshot->_catchUpBidQty :
						0);

		slog_debug(0,
				"[%lld-%d-%s] 涨停后成交量[%lld] / 封单量[%lld] = [%lld], 大单买[%d],大单撤[%d]",
				pStrategy->plotid, snapshot->market, snapshot->securityId,
				snapshot->_catchUpTrdQty, totalBidQty,
				totalBidQty > 0 ?
						10000 * snapshot->_catchUpTrdQty / totalBidQty : 0,
				snapshot->_catchUpBidBuyCnt, snapshot->_catchUpBidCBuyCnt);
		slog_debug(0, "[%lld-%d-%s] 集合竞价成交量比[%d]", pStrategy->plotid,
				snapshot->market, snapshot->securityId,
				10000 * snapshot->auctionQty / pStock->publicfloatShare);
	}

	allowBuyQty = XGetCanBuyQty(pStrategy, pStock->lmtBuyMinQty);

	slog_debug(0, "<<< 1.[%lld-%d-%s]:允许买量[%d]", pStrategy->plotid,
			snapshot->market, snapshot->securityId, allowBuyQty);
	if (allowBuyQty <= 0) {
		return;
	}

	//如果有连续大单,则只要涨停不看封单为0
	slog_debug(0,
			"<<< 1.[%lld-%d-%s]:允许买量[%d], 卖一量[%lld],计算卖一量[%lld],设定卖一量[%lld],涨停价大胆[%d], 涨停价买入次数[%d]",
			pStrategy->plotid, snapshot->market, snapshot->securityId,
			allowBuyQty, snapshot->askqty[0],
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
		if (snapshot->tradePx != pStrategy->setting.conPx && snapshot->secUpBigBuyCnt <= 2) {
			return;
		}
	}

	totalBidQty =
			snapshot->bidqty[0] - snapshot->askqty[0] > 0 ?
					snapshot->bidqty[0] - snapshot->askqty[0] : 0;
	totalBidAmt = totalBidQty / 10000 * snapshot->bid[0];

	slog_debug(0,
			"<<< 2.[%lld-%d-%s]:达到卖一量条件, 最新时间[%d],最新价[%d],卖一价[%d],卖一量[%lld(%lld)]",
			pStrategy->plotid, snapshot->market, snapshot->securityId,
			snapshot->updateTime, snapshot->tradePx, snapshot->ask[0],
			snapshot->askqty[0], pStrategy->setting.askQty);

	slog_debug(0,
			"<<< 3.[%lld-%d-%s]:达到买一金额条件, 最新价[%d],买一价[%d] * 买一量[%lld] = 买一金额[%lld(%lld)]",
			pStrategy->plotid, snapshot->market, snapshot->securityId,
			snapshot->tradePx, snapshot->bid[0], snapshot->bidqty[0],
			totalBidAmt, pStrategy->setting.buyMoney);
	/** 涨停价,买一金额小于设定金额不触发 */
	if (0 != pStrategy->setting.buyMoney
			&& totalBidAmt < pStrategy->setting.buyMoney) {
		return;
	}

	slog_debug(0,
			"<<< 4.[%lld-%d-%s]:达到买一金额条件, 最新价[%d],买一价[%d] * 买一量[%lld] = 买一金额[%lld(%lld)]",
			pStrategy->plotid, snapshot->market, snapshot->securityId,
			snapshot->tradePx, snapshot->bid[0], snapshot->bidqty[0],
			totalBidAmt, pStrategy->setting.buyMoney);

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

		slog_debug(0,
				"<<<<<< [%lld-%d-%s],委托本地编号[%d],委托价格[%d],最新价[%d],最新时间[%d]",
				pStrategy->plotid, pStrategy->setting.market,
				pStrategy->setting.securityId, localid, order.ordPrice,
				snapshot->tradePx, snapshot->updateTime);
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

	slog_debug(0, "[%lld-%d-%s] 1. 涨幅[%d]", pStrategy->plotid,
			pStrategy->setting.market, pStrategy->setting.securityId, zdf);

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

		slog_debug(0,
				"<<<<<< [%lld-%d-%s],委托本地编号[%d],委托价格[%d],最新价[%d],最新时间[%d]",
				pStrategy->plotid, pStrategy->setting.market,
				pStrategy->setting.securityId, localid, order.ordPrice,
				snapshot->tradePx, snapshot->updateTime);
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
	XInt cursor = -1, lcursor = -1;
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
			"[%lld-%d-%s] 1. 今日涨停卖量[%lld] > 昨日涨停卖量[%lld] * 倍数[%d] * 0.0001 = (%f),时间[%d]",
			pStrategy->plotid, pStrategy->setting.market,
			pStrategy->setting.securityId, snapshot->upperOfferOrdQty,
			pStock->upperOfferOrdQty, pStrategy->setting.upperQtyMulty,
			pStrategy->setting.upperQtyMulty * 0.0001
					* pStock->upperOfferOrdQty, snapshot->updateTime);
	// TODO 涨停价卖挂单量 2023.10.6
	if (snapshot->upperOfferOrdQty
			< pStrategy->setting.upperQtyMulty /** 涨停与昨日倍量 */* 0.0001
					* pStock->upperOfferOrdQty) {
		return;
	}

	slog_debug(0,
			"[%lld-%d-%s] 2. 主买金额[%.2f]万元, 主卖金额[%.2f]万元, 流通市值[%.2f]万元, 时间[%d]",
			pStrategy->plotid, pStrategy->setting.market,
			pStrategy->setting.securityId, snapshot->outsideTrdAmt * 0.00000001,
			snapshot->insideTrdAmt * 0.00000001,
			0.00000001 * pStock->publicfloatShare * snapshot->tradePx,
			snapshot->updateTime);

	kline1 = GetKlinesByBlock(snapshot->idx, 0);

	cursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 1) % SNAPSHOT_K1_CNT;
	amt = kline1[cursor].amt;
	curPx = kline1[cursor].close;

	// 前3分钟成交
	cursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 2) % SNAPSHOT_K1_CNT;
	preamt = kline1[cursor].amt;

	if (kline1[cursor].close) {
		zs = (curPx - kline1[cursor].close) * 10000 / kline1[cursor].close;
	}

	cursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 3) % SNAPSHOT_K1_CNT;
	preamt += kline1[cursor].amt;

	cursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 4) % SNAPSHOT_K1_CNT;
	preamt += kline1[cursor].amt;

	if (snapshot->volumeTrade) {
		avgPx = snapshot->amountTrade / snapshot->volumeTrade;
	}

	if (snapshot->updateTime < 93500000) {
		slog_debug(0,
				"[%lld-%d-%s] 3. 1分钟成交金额[%.2f(%.2f)],5分钟成交金额[%.2f(%2.f)], 涨速[%d(%d)], 涨跌幅[%d],振幅[%d],时间[%d]",
				pStrategy->plotid, pStrategy->setting.market,
				pStrategy->setting.securityId, amt * 0.0001,
				pStrategy->setting.buyMoney * 1.5, preamt * 0.0001,
				pStrategy->setting.buyCtrlMoney * 1.0, zs,
				pStrategy->setting.cdl, zdf, zf, snapshot->updateTime);
		if (!((amt > pStrategy->setting.buyMoney * 15000
				|| preamt > pStrategy->setting.buyCtrlMoney * 15000)
				&& zs > pStrategy->setting.cdl && snapshot->tradePx > avgPx)) {
			return;
		}
	} else {
		slog_debug(0,
				"[%lld-%d-%s] 3. 1分钟成交金额[%.2f(%.2f)],5分钟成交金额[%.2f(%.2f)], 涨速[%d(%d)], 涨跌幅[%d],振幅[%d],时间[%d]",
				pStrategy->plotid, pStrategy->setting.market,
				pStrategy->setting.securityId, amt * 0.0001,
				pStrategy->setting.buyMoney * 1.0, preamt * 0.0001,
				pStrategy->setting.buyCtrlMoney * 1.0, zs,
				pStrategy->setting.cdl, zdf, zf, snapshot->updateTime);
		if (!((amt > pStrategy->setting.buyMoney * 10000
				|| preamt > pStrategy->setting.buyCtrlMoney * 10000)
				&& zs > pStrategy->setting.cdl)) {
			return;
		}
	}

	// TODO 当前涨停卖的委托量是前几分钟卖的多少倍 2023.10.6
	kline5 = GetKlinesByBlock(snapshot->idx, 1);
	cursor = (SNAPSHOT_K5_CNT + snapshot->kcursor5 - 1) % SNAPSHOT_K5_CNT;
	lcursor = (SNAPSHOT_K5_CNT + snapshot->kcursor5 - 1) % SNAPSHOT_K5_CNT;

	slog_debug(0, "[%lld-%d-%s] 4. 涨停卖量[%lld],上一次涨停卖量[%lld],涨停卖量[%d]",
			pStrategy->plotid, pStrategy->setting.market,
			pStrategy->setting.securityId, kline5[cursor].upperSellOrdQty,
			kline5[lcursor].upperSellOrdQty,
			pStrategy->setting.upperQtyMultyMin);
	if (kline5[cursor].upperSellOrdQty
			< pStrategy->setting.upperQtyMultyMin * 0.0001
					* kline5[lcursor].upperSellOrdQty) {
		return;
	}

	slog_debug(0,
			"[%lld-%d-%s]  5. 托单数量[%lld],压单数量[%lld], 压单/托单比例[%.2f(%lld)], 压单笔数[%d(%lld)]",
			pStrategy->plotid, pStrategy->setting.market,
			pStrategy->setting.securityId, snapshot->bigBuyOrdQty,
			snapshot->bigSellOrdQty,
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
		slog_debug(0,
				"<<<<<< [%lld-%d-%s],委托本地编号[%d],委托价格[%d],最新价[%d],最新时间[%d]",
				pStrategy->plotid, pStrategy->setting.market,
				pStrategy->setting.securityId, localid, order.ordPrice,
				snapshot->tradePx, snapshot->updateTime);
	}
}
/**
 * @brief
 *  1、篮子卖出同上；
 2、如果“卖出价格”为“0”，则以市价买一价格卖出，且单笔卖出委托数量不大于“全市场买一数量”。
 3、如果总卖出数量大于“全市场买一数量”，则分批委托，每次委托均不大于“全市场买一数量”。且每次委托时间间隔可统一设置。
 4、如果以“买一价格”卖出委托未成交或部分成交，则撤单。可设置撤单时间间隔。
 5、对篮子卖出执行时间可统一设置（beginTime和endTime）
 * @param pStrategy
 * @param snapshot
 * @param pStock
 */
static inline void BasketSell(XStrategyT *pStrategy, XSnapshotT *snapshot,
		XStockT *pStock) {
	XQty allowSell = 0, calQty = 0;
	XOrderReqT order = { 0 };
	XLocalId localid = 0;
	XHoldT *pHold = NULL;
	XIdx idx = -1;

	if (pStrategy->version == snapshot->version) {
		return;
	}
	pStrategy->version = snapshot->version;

	if (XGetClockTime()
			< pStrategy->_sellLocTime
					+ pStrategy->plot.ordGapTime * 1000000LL) {
		return;
	}

	if (snapshot->bidqty[0] <= 0) {
		return;
	}

	/** 触发限价卖出价格 */
	if (0 != pStrategy->setting.conPx
			&& snapshot->tradePx != pStrategy->setting.conPx) {
		return;
	}

	pHold = XFndVHoldByKey(pStrategy->plot.customerId, pStrategy->investId,
			pStrategy->setting.market, pStrategy->setting.securityId);
	if (NULL == pHold) {
		pStrategy->status = eXPlotStop;
		return;
	}

	calQty = XGetCanSellQty(pStrategy, pStock->lmtBuyMinQty);
	if (calQty <= 0) {
		return;
	}
	// 计算数量大于买一量则以买一辆卖出
	if (calQty > snapshot->bidqty[0]) {
		calQty = snapshot->bidqty[0];
	}

	if (calQty > pStock->lmtSellMaxQty) {
		calQty = pStock->lmtSellMaxQty;
	}
	if (calQty < pHold->sellAvlHld && pStock->lmtBuyMinQty > 0) {
		allowSell = calQty / pStock->lmtBuyMinQty * pStock->lmtBuyMinQty;
	} else {
		allowSell = pHold->sellAvlHld;
	}
	if (allowSell <= 0) {
		return;
	}

	// 控制频率
	idx = XGetSellStorePos(pStrategy);
	if (idx < 0) {
		return;
	}

	CPStrategy2Ord(pStrategy, &order);

	order.bsType = eXSell;

	order.ordPrice = pStrategy->setting.ordPx; // 委托价格

	order.ordQty = allowSell;
	if (0 == pStrategy->setting.conPx) {
		order.ordType = eXOrdBest5FAK;
	} else {
		order.ordType = eXOrdLimit;
	}

	order._lastPx = snapshot->tradePx;
	order._lastTime = snapshot->updateTime;
	order._mktTime = snapshot->_recvTime;
	order._bizIndex = snapshot->_bizIndex;

	localid = XPutOrderReq(&order);
	pStrategy->sellList[idx] = localid;
	pStrategy->_sellSends += order.ordQty;
	pStrategy->_sellLocTime = XGetClockTime();
	pStrategy->_lastSellPx = order.ordPrice;

	slog_debug(0, ">>>>>> [%lld-%d-%s],委托本地编号[%d],委托价格[%d],最新价[%d],最新时间[%d]",
			pStrategy->plotid, pStrategy->setting.market,
			pStrategy->setting.securityId, localid, order.ordPrice,
			snapshot->tradePx, snapshot->updateTime);
}

static XQty SellOpt(int sellOpt, XHoldT *pHold, XStrategyT *pStrategy,
		XStockT *pStock) {
	XQty allowSell = 0;

	allowSell = (pStrategy->setting.ordQty - pStrategy->_sellSends
			+ pStrategy->_sellRtn) * sellOpt / (12 * pStock->lmtBuyMinQty)
			* pStock->lmtBuyMinQty;
	allowSell = allowSell > pHold->sellAvlHld ? pHold->sellAvlHld : allowSell;
	slog_debug(0,
			"[%lld-%d-%s]总持仓[%lld],初始持仓[%lld],可卖[%lld],sellSends[%lld], _sellRtn[%lld], _sellValid[%lld],委托数量[%d] 卖出份数[%d]",
			pStrategy->plotid, pStrategy->setting.market,
			pStrategy->setting.securityId, pHold->sumHld, pHold->orgAvlHld,
			pHold->sellAvlHld, pStrategy->_sellSends, pStrategy->_sellRtn,
			pStrategy->_sellValid, pStrategy->setting.ordQty, sellOpt);
	return (allowSell);
}

/**
 * 常规卖出
 */
void BasketSell2(XStrategyT *pStrategy, XSnapshotT *snapshot, XStockT *pStock) {
	XQty allowSell = 0;
	XOrderReqT order = { 0 };
	XLocalId localid = 0;
	XHoldT *pHold = NULL;
	XIdx idx = -1;
	XInt zdf = 0, kpzdf = 0, hl = 0;
	XPrice avgPx = -1;
	XPrice ma1_5 = -1, ma1_13 = -1, ma5_5 = -1, ma5_60 = -1;
	XInt flag = 0;
	XKLineT *kline1 = NULL, *kline5 = NULL;
	//	XSumQty ysVolTrades = 0;
	XInt cursor = 0, cursor1 = 0;
	XInt zs = 0;
	XQty sendedQty = 0;

	if (pStrategy->version == snapshot->version) {
		return;
	}
	pStrategy->version = snapshot->version;
	// 已经发送的订单超过允许委托的数量
	sendedQty = XGetSendedQty(pStrategy);

	if (sendedQty >= pStrategy->setting.ordQty) {
		return;
	}

	if (XGetClockTime()
			< pStrategy->_sellLocTime
					+ pStrategy->plot.ordGapTime * 1000000LL) {
		return;
	}

	pHold = XFndVHoldByKey(pStrategy->plot.customerId, pStrategy->investId,
			pStrategy->setting.market, pStrategy->setting.securityId);
	if (NULL == pHold || pHold->sellAvlHld <= 0) {
		return;
	}

	kpzdf = (snapshot->openPx - snapshot->preClosePx) * 10000
			/ snapshot->preClosePx;
	zdf = (snapshot->tradePx - snapshot->preClosePx) * 10000
			/ snapshot->preClosePx;

	if (snapshot->updateTime / 100000 * 100000
			<= pStrategy->setting.conPx / 100000 * 100000) {
		pStrategy->zdf1 = zdf;
	}
	if (snapshot->updateTime / 100000 * 100000
			== pStrategy->setting.ordPx / 100000 * 100000) {
		pStrategy->zdf2 = zdf;
	}

	hl = (snapshot->highPx - snapshot->tradePx) * 10000 / snapshot->preClosePx;

	if (snapshot->volumeTrade > 0) {
		avgPx = snapshot->amountTrade / snapshot->volumeTrade;
	}
	kline1 = GetKlinesByBlock(snapshot->idx, 0);
	kline5 = GetKlinesByBlock(snapshot->idx, 1);

	cursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 1) % SNAPSHOT_K1_CNT;
	cursor1 = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 2) % SNAPSHOT_K1_CNT;
	//	cursor5 = (SNAPSHOT_K5_CNT + snapshot->kcursor5 - 2) % SNAPSHOT_K5_CNT;
	// 取临近的数量
	ma1_5 = MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1, 5);
	ma1_13 = MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1, 13);
	ma5_5 = MA(kline5, SNAPSHOT_K5_CNT, snapshot->kcursor5, 5);
	ma5_60 = MA(kline5, SNAPSHOT_K5_CNT, snapshot->kcursor5, 60);
	//	ma5_20 = MA(snapshot->kline5, SNAPSHOT_K5_CNT, snapshot->kcursor5, 20);
	//	obv5_5 = OBV_MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1,  5);
	//	obv5_20 = OBV_MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1,  20);

	// 计算涨速和放量达到的时间和涨幅,如果在10秒后涨幅未提高则卖出

	slog_debug(0,
			"[%lld-%d-%s] 1. 时间[%d],托单数量[%lld],压单数量[%lld],托单/压单比例[%.2f (%lld)], 托单笔数[%ld(%lld)",
			pStrategy->plotid, pStrategy->setting.market,
			pStrategy->setting.securityId, snapshot->updateTime,
			snapshot->bigBuyOrdQty, snapshot->bigSellOrdQty,
			snapshot->bigSellOrdQty ?
					(snapshot->bigBuyOrdQty * 1.0) / snapshot->bigSellOrdQty :
					0, pStrategy->setting.nxtCtrlMoney, snapshot->bigBuyOrdCnt,
			pStrategy->setting.followCtrlMoney);

	if (snapshot->bigBuyOrdCnt
			< pStrategy->setting.followCtrlMoney /** 托单量 */) {
		return;
	}
	if (pStrategy->setting.nxtCtrlMoney > 0 && snapshot->bigSellOrdQty > 0
			&& snapshot->bigBuyOrdQty * 10000 / snapshot->bigSellOrdQty
					< pStrategy->setting.nxtCtrlMoney/** 托单/压单比 */) {
		return;
	}
	if (kline1[cursor1].close) {
		zs = (snapshot->tradePx - kline1[cursor1].close) * 10000
				/ kline1[cursor1].close;
	}

	if (pStrategy->setting.buyMoney /**1分钟成交金额 */> 0
			&& zs > pStrategy->setting.cdl /** 涨速 */
			&& (kline1[cursor].amt > pStrategy->setting.buyMoney * 10000 /** 最近成交金额 > 1分钟成交金 */
					|| (kline1[cursor1].amt + kline1[cursor].amt)
							> 1.8 * pStrategy->setting.buyMoney * 10000)) {
		pStrategy->_cdl = zdf;                      // 涨跌幅字段
		pStrategy->kversion = snapshot->updateTime; // 时间
	}

	slog_debug(0,
			">>> [%lld-%d-%s] 2. 开盘涨跌幅[%d],当前涨跌幅[%d],回落[%d],最新价[%d-%d],avgPx[%d],MA1_5[%d],MA1_13[%d],ma5_5[%d],ma5_60[%d],主动买成交额[%.2f]主动卖成交额[%.2f],大单买成交额[%.2f],大单卖成交额[%.2f]",
			pStrategy->plotid, snapshot->market, snapshot->securityId, kpzdf,
			zdf, hl, snapshot->updateTime, snapshot->tradePx, avgPx, ma1_5,
			ma1_13, ma5_5, ma5_60, snapshot->outsideTrdAmt * 0.0001,
			snapshot->insideTrdAmt * 0.0001, snapshot->bigBuyTrdAmt * 0.0001,
			snapshot->bigSellTrdAmt * 0.0001);

	// 1、开盘涨停打开涨停，卖出1/2;回落2%卖出1/4,剩余涨幅小于8%且MA1_5 <
	// MA1_13卖出; 2、冲到涨停价但回落2%，卖出3/4;剩余跌到5%且MA1_5 < MA1_13卖出;
	// 3、开盘高开2%一个点以上,9：40后回落1.8%卖出2/3;剩余涨幅小于8%,MA1_5 <
	// MA1_13卖出,涨幅大于8%, 回落卖出; 4、开盘在2%以下,
	// 10点涨幅>2%回落1%的卖出2/3;10点后涨幅<-8%均线向下且回落1%卖出；
	// 5、当天未全部卖出的，第二天按ma5_5 < ma5_60 且最新价 < ma5_60 均线卖出；

	// 1、不是涨停,最新价大于设置时间1,最新价小于MA5_60且ma5_5小于ma5_60且主动买成交>主动卖成交
	if (snapshot->tradePx != snapshot->upperPx && snapshot->tradePx < ma5_60
			&& ma5_5 < ma5_60) {
		if (pStrategy->_signal != 1) {
			slog_debug(0, "[%lld-%d-%s] 3. 破均线,[%d]抛出", pStrategy->plotid,
					pStrategy->setting.market, pStrategy->setting.securityId,
					snapshot->updateTime);
			flag = 12;
			pStrategy->_signal = 1;
		}
	}
	//2.开盘涨停，打开涨停卖出1/2，剩余在设置时间1后回落2%卖出1/4,在设置时间1后涨跌幅<8%且ma1_5<ma1_13卖出全部
	else if (snapshot->openPx == snapshot->upperPx
			&& snapshot->tradePx != snapshot->upperPx) {
		slog_debug(0,
				"[%lld-%d-%s], 开盘涨停 已发送量[%d],time[%d], hl[%d], 设置卖出时间[%d],signal[%d],ma1_5[%d],ma1_13[%d],zdf[%d]",
				pStrategy->plotid, pStrategy->setting.market,
				pStrategy->setting.securityId, sendedQty, snapshot->updateTime,
				hl, pStrategy->setting.conPx, pStrategy->_signal, ma1_5, ma1_13,
				zdf);
		// 如果没有委托出去,先抛一半,如果委托出去根据情况进行抛出
		if (sendedQty == 0) {
			if (pStrategy->_signal != 2) {
				slog_debug(0, "[%lld-%d-%s]3. 开盘涨停,[%d]打开涨停抛出1/2",
						pStrategy->plotid, pStrategy->setting.market,
						pStrategy->setting.securityId, snapshot->updateTime);
				flag = 6;
				pStrategy->_signal = 2;
			}
		} else {
			if (pStrategy->_signal != 3
					&& snapshot->updateTime > pStrategy->setting.conPx /**时间1 */
					&& hl > 200) {

				slog_debug(0, "[%lld-%d-%s]3. 开盘涨停,[%d]回落2%抛出1/4",
						pStrategy->plotid, pStrategy->setting.market,
						pStrategy->setting.securityId, snapshot->updateTime);
				pStrategy->_signal = 3;
				flag = 3;

			}
			// 1/4
			else if (pStrategy->_signal != 4
					&& snapshot->updateTime > pStrategy->setting.conPx /**时间1 */
					&& zdf < 800 && ma1_5 < ma1_13) {
				slog_debug(0, "[%lld-%d-%s]3. 开盘涨停,[%d]全部抛出", pStrategy->plotid,
						pStrategy->setting.market,
						pStrategy->setting.securityId, snapshot->updateTime);
				pStrategy->_signal = 4;
				flag = 12;
			}
		}
	}
	// 3.触发涨停回落2%抛出3/4,剩余在设置的时间2，涨跌幅<5%且ma1_5 < ma1_13全部抛出
	else if (snapshot->openPx != snapshot->upperPx
			&& snapshot->highPx == snapshot->upperPx && hl > 200) {

		slog_debug(0,
				"[%lld-%d-%s], 开盘涨停 已发送量[%d],time[%d], hl[%d], 设置卖出时间[%d]",
				pStrategy->plotid, pStrategy->setting.market,
				pStrategy->setting.securityId, sendedQty, snapshot->updateTime,
				hl, pStrategy->setting.conPx);
		if (sendedQty == 0) {
			if (pStrategy->_signal != 5) {
				slog_debug(0, "[%lld-%d-%s]3. 涨停过,[%d]打开涨停回落抛出3/4",
						pStrategy->plotid, pStrategy->setting.market,
						pStrategy->setting.securityId, snapshot->updateTime);
				flag = 9;
				pStrategy->_signal = 5;
			}
		} else {
			if (pStrategy->_signal != 6
					&& snapshot->updateTime > pStrategy->setting.ordPx /**时间2 */
					&& zdf < 500 && ma1_5 < ma1_13) {

				slog_debug(0, "[%lld-%d-%s]3. 开盘涨停,[%d]全部抛出", pStrategy->plotid,
						pStrategy->setting.market,
						pStrategy->setting.securityId, snapshot->updateTime);
				flag = 12;
				pStrategy->_signal = 6;

			}
		}
	}
	// 4.振幅超过5%，且回落2%,卖出全部
	else if (pStrategy->setting.upperQtyMulty > 0
			&& zdf - kpzdf > pStrategy->setting.upperQtyMulty /** 拉高涨幅  */
			&& hl >= pStrategy->setting.upperQtyMultyMin /** 回落幅度 */) {

		if (pStrategy->_signal != 7) {
			slog_debug(0, "[%lld-%d-%s]3. 振幅超过5%,[%d]全部抛出", pStrategy->plotid,
					pStrategy->setting.market, pStrategy->setting.securityId,
					snapshot->updateTime);
			flag = 12;
			pStrategy->_signal = 7;
		}
	}
	// 5.当涨速大于设置涨速或小于设置涨速,上一分钟成交量小于设置1分钟成交量，全部卖出
	else if (zs > pStrategy->setting.cdl /** 涨速 */
	&& zs < pStrategy->setting.buyCtrlMoney
			&& kline1[cursor1].amt
					< pStrategy->setting.buyMoney * 100000000 /** 1分钟成交金额 */) {

		if (pStrategy->_signal != 8) {
			slog_debug(0, "[%lld-%d-%s]3. 涨速过大但成交金额不足,[%d]全部抛出",
					pStrategy->plotid, pStrategy->setting.market,
					pStrategy->setting.securityId, snapshot->updateTime);
			flag = 12;
			pStrategy->_signal = 8;
		}
	}
	//6.开盘涨幅>2%,如果时间大于设置时间2,回落1.8%且最新价<ma1_5,卖出2/3；剩余如果时间大于设置时间2,涨跌幅<8%,ma1_5 < ma1_13全部卖出;涨跌幅>8%，时间大于设置时间1
	//回落大于1%，且最新价<ma1_5且最新价<均价,全部抛出
	else if (kpzdf >= 200) // cdl 用来处理当前证券不适合该指标卖出
			{
		/**
		 if (snapshot->outsideTrdAmt > 1.2 * snapshot->insideTrdAmt) {
		 return;
		 }
		 */
		// 回落大于1%卖出2/3
		if (sendedQty == 0) {
			// 如果放量就立即卖
			if ((pStrategy->kversion != 0
					&& (snapshot->updateTime - pStrategy->kversion) > 10000
					&& zdf < pStrategy->_cdl)
					|| (snapshot->updateTime > pStrategy->setting.conPx /**时间1 */
					&& hl > 180 && snapshot->tradePx < ma1_5)) {
				if (pStrategy->_signal != 9) {
					slog_debug(0, "[%lld-%d-%s]3. 开盘涨幅大于2%,回落[%d],[%d]卖出2/3",
							pStrategy->plotid, pStrategy->setting.market,
							pStrategy->setting.securityId, hl,
							snapshot->updateTime);
					flag = 8;
					pStrategy->_signal = 9;
				}
			}
		} else {
			if (zdf < 800) {
				if (pStrategy->_signal != 10
						&& snapshot->updateTime > pStrategy->setting.ordPx /**时间2 */
						&& ma1_5 < ma1_13) {
					slog_debug(0, "[%lld-%d-%s]3. 开盘涨幅大于2%,[%d]全部抛出",
							pStrategy->plotid, pStrategy->setting.market,
							pStrategy->setting.securityId, snapshot->updateTime);
					flag = 4;
					pStrategy->_signal = 10;
				}
			} else {
				if (pStrategy->_signal != 11
						&& snapshot->updateTime > pStrategy->setting.conPx /**时间1 */
						&& hl > 100 && snapshot->tradePx < ma1_5
						&& snapshot->tradePx < avgPx) {
					slog_debug(0, "[%lld-%d-%s]3. 开盘涨幅大于2%,[%d]全部抛出",
							pStrategy->plotid, pStrategy->setting.market,
							pStrategy->setting.securityId, snapshot->updateTime);
					flag = 12;
					pStrategy->_signal = 11;
				}
			}
		}
	}
	//7.开盘涨跌幅<2%,如果开盘涨跌幅<设置涨跌幅且最新时间大于设置时间2且在设置时间1和时间2趋势向下，全部卖出;如果主动买成交<主动卖,
	//涨跌幅>2%且大于设置涨跌幅且最新时间大于设置时间2且最新价<ma1_5,卖出2/3；前一分钟收盘价<ma1_5或最新价<均价且时间大于设置时间1，全部抛出；
	//开盘涨跌幅<0且前一分钟收盘价<ma1_5且亏损7%,全部抛出
	else if (kpzdf < 200) // cdl 用来处理当前证券不适合该指标卖出
			{

		if (pStrategy->_signal != 12 && kpzdf < pStrategy->setting.kpzf /** 开盘涨跌幅 */
		&& snapshot->updateTime > pStrategy->setting.ordPx /** 时间批次2 */
		&& pStrategy->zdf2 < pStrategy->zdf1) {
			slog_debug(0, "[%lld-%d-%s]3. 9:40-9:50趋势向下,卖出", pStrategy->plotid,
					pStrategy->setting.market, pStrategy->setting.securityId);
			flag = 12;
			pStrategy->_signal = 12;
			goto END_STRATEGY;
		}

		/**
		 * 买的笔数 > 卖的笔数
		 * 大单卖的金额 > 大单买的金额
		 */
		/**
		 if (snapshot->outsideTrdAmt > snapshot->insideTrdAmt) {
		 return;
		 }
		 */
		slog_debug(0, "[%lld-%d-%s] 4. 买笔数[%d],卖笔数[%d],买金额[%lld],卖金额[%lld]",
				pStrategy->plotid, pStrategy->setting.market,
				pStrategy->setting.securityId, snapshot->totalBuyOrdCnt,
				snapshot->totalSellOrdCnt, snapshot->bigBuyTrdAmt,
				snapshot->bigSellTrdAmt);
		if (zdf > 200) {
			if ((pStrategy->kversion != 0
					&& (snapshot->updateTime - pStrategy->kversion) > 10000
					&& zdf < pStrategy->_cdl)
					|| (snapshot->updateTime > pStrategy->setting.ordPx /**时间2 */
					&& snapshot->tradePx < ma1_5)) {
				if (pStrategy->_signal != 13) {
					slog_debug(0, "[%lld-%d-%s] 5. 开盘涨小于2%,[%d]回落1%抛出2/3",
							pStrategy->plotid, pStrategy->setting.market,
							pStrategy->setting.securityId, snapshot->updateTime);
					flag = 8;
					pStrategy->setting.cdl = 1;
					pStrategy->_signal = 13;
				}
			}
		}
		// 只能拿1分钟均线，否则很容易抛出，中间毛刺太多了
		else if ((kline1[cursor1].close < ma1_5 || snapshot->tradePx < avgPx)
				&& snapshot->updateTime > pStrategy->setting.conPx /**时间1 */) {
			if (pStrategy->_signal != 14) {
				slog_debug(0, "[%lld-%d-%s]5. 开盘涨小于-5%,[%d]低开5分钟未上涨抛出全部",
						pStrategy->plotid, pStrategy->setting.market,
						pStrategy->setting.securityId, snapshot->updateTime);
				flag = 12;
				pStrategy->_signal = 14;
			}
		} else if (kpzdf < 0 && kline1[cursor1].close < ma1_5
				&& snapshot->tradePx < pHold->costPrice * 0.93) {
			if (pStrategy->_signal != 15) {
				slog_debug(0,
						"[%lld-%d-%s]5. 开盘涨小于0%,[%d]亏损7%抛出全部,最新价[%d],成本价[%d], zdf[%d]",
						pStrategy->plotid, pStrategy->setting.market,
						pStrategy->setting.securityId, snapshot->updateTime,
						snapshot->tradePx, pHold->costPrice, zdf);
				flag = 12;
				pStrategy->_signal = 15;
			}
		}
	}

	END_STRATEGY: if (!flag) {
		return;
	}

	allowSell = SellOpt(flag, pHold, pStrategy, pStock);

	if (allowSell <= 0) {
		return;
	}

	slog_debug(0, ">>> 7.[%lld-%d-%s]可卖数量[%lld],策略委托数量[%d]-当前数量[%d]",
			pStrategy->plotid, pStrategy->setting.market,
			pStrategy->setting.securityId, pHold->sellAvlHld,
			pStrategy->setting.ordQty, allowSell);

	// 控制频率
	idx = XGetSellStorePos(pStrategy);
	if (idx < 0) {
		return;
	}

	CPStrategy2Ord(pStrategy, &order);

	order.bsType = eXSell;

	order.ordPrice = GetSlipSellPx(snapshot->tradePx, pStrategy->_slipSellTimes,
			pStock->lowerPrice, pStock->priceTick);

	order.ordQty = allowSell;

	order.ordType = eXOrdLimit;

	order._lastPx = order.ordPrice;
	order._lastTime = snapshot->updateTime;
	order._mktTime = snapshot->_recvTime;
	order._bizIndex = snapshot->_bizIndex;

	localid = XPutOrderReq(&order);
	pStrategy->sellList[idx] = localid;
	pStrategy->_sellSends += order.ordQty;
	pStrategy->_sellLocTime = XGetClockTime();
	pStrategy->_lastSellPx = order.ordPrice;

	slog_debug(0, ">>>>>> [%lld-%d-%s],委托本地编号[%d],委托价格[%d],委托数量[%d],最新价[%d-%d]",
			pStrategy->plotid, pStrategy->setting.market,
			pStrategy->setting.securityId, localid, order.ordPrice,
			order.ordQty, snapshot->tradePx, snapshot->updateTime);
}
/**
 * 不盈利不卖出
 */
void BasketSell8(XStrategyT *pStrategy, XSnapshotT *snapshot, XStockT *pStock) {
	XQty allowSell = 0;
	XOrderReqT order = { 0 };
	XLocalId localid = 0;
	XHoldT *pHold = NULL;
	XIdx idx = -1;
	XInt zdf = 0, kpzdf = 0, hl = 0;
	XPrice avgPx = -1;
	XPrice ma1_5 = -1, ma1_13 = -1;
	XRatio bias = -1;
	XInt flag = 0;
	XKLineT *kline1 = NULL;
	XQty sendedQty = 0;

	if (pStrategy->version == snapshot->version) {
		return;
	}
	pStrategy->version = snapshot->version;

	// 已经发送的订单超过允许委托的数量
	sendedQty = XGetSendedQty(pStrategy);
	if (sendedQty >= pStrategy->setting.ordQty) {
		return;
	}

	if (XGetClockTime()
			< pStrategy->_sellLocTime
					+ pStrategy->plot.ordGapTime * 1000000LL) {
		return;
	}

	pHold = XFndVHoldByKey(pStrategy->plot.customerId, pStrategy->investId,
			pStrategy->setting.market, pStrategy->setting.securityId);
	if (NULL == pHold || pHold->sellAvlHld <= 0) {
		return;
	}

	if (snapshot->tradePx < pHold->costPrice) {
		return;
	}

	kpzdf = (snapshot->openPx - snapshot->preClosePx) * 10000
			/ snapshot->preClosePx;
	zdf = (snapshot->tradePx - snapshot->preClosePx) * 10000
			/ snapshot->preClosePx;
	hl = (snapshot->highPx - snapshot->tradePx) * 10000 / snapshot->preClosePx;

	if (snapshot->volumeTrade > 0) {
		avgPx = snapshot->amountTrade / snapshot->volumeTrade;
		bias = (snapshot->tradePx - avgPx) * 10000 / avgPx;
	}
	kline1 = GetKlinesByBlock(snapshot->idx, 0);
	//	kline5 = GetKlinesByBlock(snapshot->idx, 1);
	// 取临近的数量
	ma1_5 = MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1, 5);
	ma1_13 = MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1, 13);
	//	ma5_5 = MA(kline5, SNAPSHOT_K5_CNT, snapshot->kcursor5,  5);
	//	ma5_60 =  MA(kline5, SNAPSHOT_K5_CNT, snapshot->kcursor5, 60);
	//	ma5_20 = MA(snapshot->kline5, SNAPSHOT_K5_CNT, snapshot->kcursor5, 20);
	//	obv5_5 = OBV_MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1,  5);
	//	obv5_20 = OBV_MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1,  20);

	// 更新最近bias
	if (bias < -300) {
		pStrategy->_cdl = bias;
		pStrategy->kversion = snapshot->kcursor1;
	}
	// 如果最近的最大乖离率没有达到-50,则乖离率为0
	if (pStrategy->kversion - snapshot->kcursor1 >= 5) {
		pStrategy->_cdl = 0;
		pStrategy->kversion = snapshot->kcursor1;
	}

	slog_debug(0,
			">>> [%lld-%d-%s],开盘涨跌幅[%d],当前涨跌幅[%d],回落[%d], 最新价[%d-%d],avgPx[%d],MA1_5[%d],MA1_13[%d], BIAS[%d]",
			pStrategy->plotid, snapshot->market, snapshot->securityId, kpzdf,
			zdf, hl, snapshot->updateTime, snapshot->tradePx, avgPx, ma1_5,
			ma1_13, bias);

	// 1、开盘涨停打开涨停，卖出1/2;回落2%卖出1/4,剩余涨幅小于8%且MA1_5 <
	// MA1_13卖出; 2、冲到涨停价但回落2%，卖出3/4;剩余跌到5%且MA1_5 < MA1_13卖出;
	// 3、开盘高开2%一个点以上,9：40后回落1.8%卖出2/3;剩余涨幅小于8%,MA1_5 <
	// MA1_13卖出,涨幅大于8%, 回落卖出; 4、开盘在2%以下,
	// 10点涨幅>2%回落1%的卖出2/3;10点后涨幅<-8%均线向下且回落1%卖出；
	// 5、当天未全部卖出的，第二天按ma5_5 < ma5_60 且最新价 < ma5_60 均线卖出；

	if (snapshot->openPx == snapshot->upperPx
			&& snapshot->tradePx != snapshot->upperPx) {

		if (sendedQty == 0) {
			if (pStrategy->_signal != 1) {
				slog_debug(0, "[%lld-%d-%s]开盘涨停,[%d]打开涨停抛出1/2",
						pStrategy->plotid, pStrategy->setting.market,
						pStrategy->setting.securityId, snapshot->updateTime);
				flag = 6;
				pStrategy->_signal = 1;
			}
		} else {
			// 1/4
			if (snapshot->updateTime > pStrategy->setting.conPx /**时间1 */
			&& hl > 200) {
				if (pStrategy->_signal != 2) {
					slog_debug(0, "[%lld-%d-%s]开盘涨停,[%d]回落2%抛出1/4",
							pStrategy->plotid, pStrategy->setting.market,
							pStrategy->setting.securityId, snapshot->updateTime);
					pStrategy->_signal = 2;
					flag = 3;
				}
			}
			// 1/4
			else if (snapshot->updateTime > pStrategy->setting.conPx /**时间1 */
			&& zdf < 800 && ma1_5 < ma1_13) {
				if (pStrategy->_signal != 3) {
					slog_debug(0, "[%lld-%d-%s]开盘涨停,[%d]全部抛出",
							pStrategy->plotid, pStrategy->setting.market,
							pStrategy->setting.securityId, snapshot->updateTime);
					pStrategy->_signal = 3;
					flag = 12;
				}
			}
		}
	}
	// 触发涨停回落2%抛出3/4
	else if (snapshot->openPx != snapshot->upperPx
			&& snapshot->highPx == snapshot->upperPx && hl > 200) {
		if (sendedQty == 0) {
			if (pStrategy->_signal != 4) {
				slog_debug(0, "[%lld-%d-%s]涨停过,[%d]打开涨停回落抛出3/4",
						pStrategy->plotid, pStrategy->setting.market,
						pStrategy->setting.securityId, snapshot->updateTime);
				flag = 9;
				pStrategy->_signal = 4;
			}
		} else {
			if (snapshot->updateTime > pStrategy->setting.conPx /**时间1 */
			&& zdf < 500 && ma1_5 < ma1_13) {
				if (pStrategy->_signal != 5) {
					slog_debug(0, "[%lld-%d-%s]开盘涨停,[%d]全部抛出",
							pStrategy->plotid, pStrategy->setting.market,
							pStrategy->setting.securityId, snapshot->updateTime);
					flag = 12;
					pStrategy->_signal = 5;
				}
			}
		}
	} else if (kpzdf >= 200) // cdl 用来处理当前证券不适合该指标卖出
			{
		// 回落大于1%卖出2/3
		if (sendedQty == 0) {
			if (snapshot->updateTime > pStrategy->setting.conPx /**时间1 */
			&& hl > 180 && snapshot->tradePx < ma1_5) {
				if (pStrategy->_signal != 6) {
					slog_debug(0, "[%lld-%d-%s]开盘涨幅大于2%,回落[%d],[%d]卖出2/3",
							pStrategy->plotid, pStrategy->setting.market,
							pStrategy->setting.securityId, hl,
							snapshot->updateTime);
					flag = 8;
					pStrategy->_signal = 6;
				}
			}
		} else {
			if (zdf < 800) {
				if (snapshot->updateTime > pStrategy->setting.conPx /**时间1 */
				&& ma1_5 < ma1_13) {
					if (pStrategy->_signal != 7) {
						slog_debug(0, "[%lld-%d-%s]开盘涨幅大于2%,[%d]全部抛出",
								pStrategy->plotid, pStrategy->setting.market,
								pStrategy->setting.securityId,
								snapshot->updateTime);
						flag = 4;
						pStrategy->_signal = 7;
					}
				}
			} else {
				if (snapshot->updateTime > pStrategy->setting.conPx /**时间1 */
				&& hl > 100 && snapshot->tradePx < ma1_5
						&& snapshot->tradePx < avgPx) {
					if (pStrategy->_signal != 8) {
						slog_debug(0, "[%lld-%d-%s]开盘涨幅大于2%,[%d]全部抛出",
								pStrategy->plotid, pStrategy->setting.market,
								pStrategy->setting.securityId,
								snapshot->updateTime);
						flag = 12;
						pStrategy->_signal = 8;
					}
				}
			}
		}
	} else if (kpzdf < 200) {

		/**
		 if (snapshot->outsideTrdAmt > 1.2 * snapshot->insideTrdAmt) {
		 return;
		 }
		 */
		if (snapshot->updateTime > pStrategy->setting.ordPx /**时间2 */
		&& snapshot->tradePx < ma1_5) {
			if (pStrategy->_signal != 9) {
				slog_debug(0, "[%lld-%d-%s]开盘涨幅小于2%,[%d]盈利全部抛出",
						pStrategy->plotid, pStrategy->setting.market,
						pStrategy->setting.securityId, snapshot->updateTime);
				flag = 12;
				pStrategy->_signal = 9;
			}
		}
	}

	if (!flag) {
		return;
	}

	allowSell = SellOpt(flag, pHold, pStrategy, pStock);
	if (allowSell <= 0) {
		return;
	}

	slog_debug(0, ">>> 7.[%lld-%d-%s]可卖持仓[%lld],策略委托数量[%lld]-发送数量[%d]",
			pStrategy->plotid, pStrategy->setting.market,
			pStrategy->setting.securityId, pHold->sellAvlHld,
			pStrategy->setting.ordQty, allowSell);

	// 控制频率
	idx = XGetSellStorePos(pStrategy);
	if (idx < 0) {
		return;
	}

	CPStrategy2Ord(pStrategy, &order);

	order.bsType = eXSell;

	order.ordPrice = GetSlipSellPx(snapshot->tradePx, pStrategy->_slipSellTimes,
			pStock->lowerPrice, pStock->priceTick);

	order.ordQty = allowSell;

	order.ordType = eXOrdLimit;

	order._lastPx = order.ordPrice;
	order._lastTime = snapshot->updateTime;
	order._mktTime = snapshot->_recvTime;
	order._bizIndex = snapshot->_bizIndex;

	localid = XPutOrderReq(&order);
	pStrategy->sellList[idx] = localid;
	pStrategy->_sellSends += order.ordQty;
	pStrategy->_sellLocTime = XGetClockTime();
	pStrategy->_lastSellPx = order.ordPrice;

	slog_debug(0, ">>>>>> [%lld-%d-%s],委托本地编号[%d],委托价格[%d],最新价[%d],最新时间[%d]",
			pStrategy->plotid, pStrategy->setting.market,
			pStrategy->setting.securityId, localid, order.ordPrice,
			snapshot->tradePx, snapshot->updateTime);
}

static XBool BuyCtrl(XStrategyT *pStrategy, XRSnapshotT *snapshot,
		XStockT *pStock) {
	XLongTime curTime = 0;
	XMoney totalBidMoney = 0;
	XSumQty totalBidQty = 0;
	XNum kcursor1, kcursor2;
	XKLineT *k1 = NULL;

	if (pStrategy->_buyTrades >= pStrategy->setting.ordQty) {
		slog_debug(0, "!!!!!! [%lld-%d-%s],策略暂停, 更新时间[%d]已全部成交[%d]",
				pStrategy->plotid, pStrategy->setting.market,
				pStrategy->setting.securityId, snapshot->updateTime,
				pStrategy->setting.ordQty);
		pStrategy->status = eXPlotStop;
		return (true);
	}

	if (pStrategy->plot.slipBuyTimes != 0
			&& pStrategy->_slipBuyTimes >= pStrategy->plot.slipBuyTimes) {
		pStrategy->status = eXPlotStop;
		slog_debug(0, "!!!!!! [%lld-%d-%s],策略暂停, 超过滑点次数[%d(%d)],时间[%d]",
				pStrategy->plotid, snapshot->market, snapshot->securityId,
				pStrategy->_slipBuyTimes, pStrategy->plot.slipBuyTimes,
				snapshot->updateTime);
		return (true);
	}

	if (0 == pStrategy->setting.sign) {
		/** 开盘涨停的不买 */
		if (snapshot->openPx == pStock->upperPrice) {
			pStrategy->status = eXPlotStop;
			slog_debug(0, "!!!!!! [%lld-%d-%s],策略暂停, 开盘价[%d]=涨停价[%d]",
					pStrategy->plotid, snapshot->market, snapshot->securityId,
					snapshot->openPx, pStock->upperPrice);
			return (true);
		}

		/** 启动前已经涨停 */
		if (snapshot->_upperTimes > 0
				&& snapshot->updateTime < pStrategy->plot.beginTime) {
			pStrategy->status = eXPlotStop;
			slog_debug(0,
					"!!!!!! [%lld-%d-%s],策略暂停,在开始交易时间前已经涨停,当前时间[%d],开始时间[%d]",
					pStrategy->plotid, snapshot->market, snapshot->securityId,
					snapshot->updateTime, pStrategy->plot.beginTime);
			return (true);
		}

		/** 涨停打开的不买 */
		if (pStrategy->plot.isUpperStop && snapshot->_upperTimes > 1) {
			pStrategy->status = eXPlotStop;
			slog_debug(0, "!!!!!! [%lld-%d-%s],策略暂停, 多次打开涨停[%d]",
					pStrategy->plotid, snapshot->market, snapshot->securityId,
					snapshot->_upperTimes);
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

					if (XCancelBuy(pStrategy, 1)) {
						slog_debug(0,
								"[%lld-%d-%s],下单后30ms封单金额不足,最新时间[%d - %lld - %lld],最新价格[%d],当前最大封单金额[%lld],最新封单金额[%lld]",
								pStrategy->plotid, pStrategy->setting.market,
								pStrategy->setting.securityId,
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

					if (XCancelBuy(pStrategy, 1)) {
						slog_debug(0,
								"[%lld-%d-%s],下单后3ms封单金额不足,最新时间[%d - %lld - %lld],最新价格[%d],当前最大封单金额[%lld],最新封单金额[%lld]",
								pStrategy->plotid, pStrategy->setting.market,
								pStrategy->setting.securityId,
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

				if (XCancelBuy(pStrategy, 1)) {
					slog_debug(0,
							"[%lld-%d-%s],最新时间[%d],最新价格[%d],当前最大封单金额[%lld],最新封单金额[%lld]",
							pStrategy->plotid, pStrategy->setting.market,
							pStrategy->setting.securityId, snapshot->updateTime,
							snapshot->tradePx, pStrategy->_buyCtrlMoney,
							totalBidMoney);

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

				if (XCancelBuy(pStrategy, 1)) {
					pStrategy->setting.buyMoney = pStrategy->setting.buyMoney
							+ pStrategy->setting.buyMoney
									* pStrategy->plot.ctrlUpRatio * 0.0001; // 撤单后再次下单封单金额加0.5倍
					slog_debug(0,
							"[%lld-%d-%s],板上撤单or成交过大,最新时间[%d - %lld],最新价格[%d],当前封单金额[%lld],当前撤单率[%d]",
							pStrategy->plotid, pStrategy->setting.market,
							pStrategy->setting.securityId, snapshot->updateTime,
							pStrategy->_buyLocTime, snapshot->tradePx,
							totalBidMoney, pStrategy->_cdl);
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
						if (XCancelBuy(pStrategy, 1)) {
							pStrategy->setting.buyMoney =
									pStrategy->setting.buyMoney; // 撤单后再次下单封单金额加0.5倍
							slog_debug(0,
									"[%lld-%d-%s],封涨后1分钟成交量破千万,最新时间[%d - %lld],最新价格[%d],当前封单金额[%lld],当前撤单率[%d]",
									pStrategy->plotid,
									pStrategy->setting.market,
									pStrategy->setting.securityId,
									snapshot->updateTime,
									pStrategy->_buyLocTime, snapshot->tradePx,
									totalBidMoney, pStrategy->_cdl);
							pStrategy->_signal = 0;
							pStrategy->_slipBuyTimes++;
							pStrategy->_hasCtrl = true;
						}
					}
				}

				if (pStrategy->setting.followCtrlMoney > 0
						&& snapshot->kcursor1 > snapshot->_sealCursor + 1) {
					// 如果连续2分钟破800万，不下单了
					kcursor2 = (snapshot->kcursor1 - 2) & (SNAPSHOT_K1_CNT - 1);
					if (k1[kcursor1].amt * 0.00000001
							>= pStrategy->setting.followCtrlMoney
							&& k1[kcursor2].amt * 0.00000001
									>= pStrategy->setting.followCtrlMoney) {
						if (XCancelBuy(pStrategy, 1)) {
							pStrategy->setting.buyMoney =
									pStrategy->setting.buyMoney
											+ pStrategy->setting.buyMoney
													* pStrategy->plot.ctrlUpRatio
													* 0.0001; // 撤单后再次下单封单金额加0.5倍
							slog_debug(0,
									"[%lld-%d-%s],连续2分钟成交量破800万,最新时间[%d - %lld],最新价格[%d],当前封单金额[%lld],当前撤单率[%d]",
									pStrategy->plotid,
									pStrategy->setting.market,
									pStrategy->setting.securityId,
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

static XBool SellCtrl(XStrategyT *pStrategy, XRSnapshotT *snapshot,
		XStockT *pStock) {
	//超过滑点卖出次数的暂停
	if (pStrategy->plot.slipSellTimes != 0
			&& pStrategy->_slipSellTimes >= pStrategy->plot.slipSellTimes) {
		pStrategy->status = eXPlotStop;
		slog_debug(0, "[%lld-%d-%s],超过滑点次数[%d(%d)],时间[%d]", pStrategy->plotid,
				pStrategy->setting.market, pStrategy->setting.securityId,
				pStrategy->_slipSellTimes, pStrategy->plot.slipSellTimes,
				snapshot->updateTime);
		return (true);
	}

	//处理撤单,如果超过时间没有卖出,发起撤单
	//如果下单成功后多少秒没有成交,撤单
	if (XGetClockTime() > pStrategy->_sellLocTime + pStrategy->plot.ctrGapTime * XTIMS_MS4NS) {
		//如果当前价格是跌停价且委托价格是跌停价,不撤单
		if (snapshot->tradePx != snapshot->lowerPx
				&& XCancelSell(pStrategy, 0)) {
			return (true);
		}
	}
	return (false);
}

XBool BasketTrade(XRSnapshotT *snapshot, XStrategyT *pStrategy, XStockT *pStock) {

	XBool bCtrl = false, bSend = false;

	if (snapshot->updateTime > pStrategy->plot.endTime) {
		slog_debug(0, "!!!!!! [%lld-%d-%s],策略暂停, 已超过运行时间[%d-%d]",
				pStrategy->plotid, pStrategy->setting.market,
				pStrategy->setting.securityId, snapshot->updateTime,
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
	//处理卖
	else {

		bCtrl = XGetSellCtrlStatus(pStrategy);
		//下单后多少时间没有成交撤单
		if (pStrategy->plot.isAutoCtrl && bCtrl) {
			// 发送撤单后,滑点次数++
			pStrategy->_slipSellTimes++;
			pStrategy->_signal = 0;
			if (pStrategy->plot.isCtrlStop) {
				pStrategy->status = eXPlotStop;
				return (true);
			}
		}
		if (SellCtrl(pStrategy, snapshot, pStock)) {
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
			BasketBuy(pStrategy, snapshot, pStock);
			break;
			/** 低吸 */
		case 1:
			BasketBuy2(pStrategy, snapshot, pStock);
			break;
			/** 科创板和创业板半路板 */
		case 2:
			BasketBuy3(pStrategy, snapshot, pStock);
			break;
		default:
			break;
		}

	}

	/** 卖 */
	else {
		bSend = CanSellSend(pStrategy);
		if (!bSend) {
			return (true);
		}

		switch (pStrategy->setting.sign) {
		// 正常卖出
		case 0:
			BasketSell2(pStrategy, snapshot, pStock);
			break;
		case 6:
			BasketSell8(pStrategy, snapshot, pStock);
			break;
		default:
			break;
		}
	}

	return (false);
}
