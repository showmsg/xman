/*
 * XBond.c
 *
 *  Created on: 2024年1月5日
 *      Author: Administrator
 */

#include "XBond.h"
#include "XLog.h"
#include "XTimes.h"
#include "XBus.h"

static void Sell(XRSnapshotT *snapshot, XStrategyT *pStrategy, XHoldT *pHold,
		XBondTradeParamT *param) {
//	XMoney amt = 0;
	XPrice p = 0;
	XInt SOIR, SOIR1, SOIR2, SOIR3, SOIR4, SOIR5;
	XSumQty bidqty, askqty;
	XKLineT *k1 = NULL;
	XNum kcursor1 = -1;
	XIdx idx = -1;
	XOrderReqT order = { 0 };
	XLocalId localid = 0;
//	XQty sendedQty = 0;
//	XOrderT* pOrder = NULL;
//	XBool bCon = false;
	XSumQty sendQty = 0;

	if (pStrategy->version == snapshot->version) {
		return;
	}

	if(NULL == pHold || 0 == pHold->sellAvlHld || 0 == pHold->sumHld)
	{
		return;
	}

	sendQty = pStrategy->_sellSends - pStrategy->_sellRtn + pStrategy->_sellValid - pStrategy->_sellTrades;
	if(sendQty >= pHold->sumHld)
	{
		return;
	}
	sendQty  = pHold->sumHld - sendQty;

	bidqty = snapshot->bidqty[0] + snapshot->bidqty[1] + snapshot->bidqty[2]
			+ snapshot->bidqty[3] + snapshot->bidqty[4];
	askqty = snapshot->askqty[0] + snapshot->askqty[1] + snapshot->askqty[2]
			+ snapshot->askqty[3] + snapshot->askqty[4];

	//价格预测
	p = (snapshot->bid[0] + snapshot->ask[0]) / 2
			+ (snapshot->ask[0] - snapshot->bid[0]) * snapshot->bidqty[0]
					/ ((snapshot->bidqty[0] + snapshot->askqty[0]) * 2);

	//订单失衡SOIR
	//SOIR = (Bvi - Avi)/(Bvi + Avi)
	//权重 w = 1 - (i - 1)/5
	//加强 SOIR = ((Bv1 - Av1)/(Bv1 + Av1) + 4/5(Bv2 - Av2)/(Bv2 + Av2) + 3/5(Bv3 - Av3)/(Bv3 + Av3) + 2/5(B4 - A4)/(B4 + A4) + 1/5(B5 - A5)/(B5 + A5)) / ( 1 + 4/5 + 3/5 + 2/5 + 1/5)
	//SOIR为正说明市场买压大于卖压，未来价格趋向上涨，且SOIR的值越大，上涨的概率越高，反之亦然

	//中间价变化率MPC,MPC为正说明股票未来短期价格趋向上涨，且MPC的值越大，其上涨的概率越高，反之亦然
	//MPC = (A + B) - (A1 + B1) / (A1 + B1)

	SOIR1 = (snapshot->bidqty[0] - snapshot->askqty[0]) * 100
			/ (snapshot->bidqty[0] + snapshot->askqty[0]);
	SOIR2 = (snapshot->bidqty[1] - snapshot->askqty[1]) * 100
			/ (snapshot->bidqty[1] + snapshot->askqty[1]);
	SOIR3 = (snapshot->bidqty[2] - snapshot->askqty[2]) * 100
			/ (snapshot->bidqty[2] + snapshot->askqty[2]);
	SOIR4 = (snapshot->bidqty[3] - snapshot->askqty[3]) * 100
			/ (snapshot->bidqty[3] + snapshot->askqty[3]);
	SOIR5 = (snapshot->bidqty[4] - snapshot->askqty[4]) * 100
			/ (snapshot->bidqty[4] + snapshot->askqty[4]);

	SOIR = (5 * SOIR1 + 4 * SOIR2 + 3 * SOIR3 + 2 * SOIR4 + SOIR5)
			/ (5 + 4 + 3 + 2 + 1);

	/**
	 slog_debug(0, "[%d-%s] [%d] 当前最新价[%d] 预测股价[%d], SOIR[%d]", snapshot->market, snapshot->securityId,
	 snapshot->updateTime, snapshot->tradePx, p, SOIR);
	 */

	pStrategy->_folwPx = snapshot->tradePx;

	kcursor1 = (kcursor1 - 2) & (SNAPSHOT_K1_CNT - 1);
	k1 = GetKlinesByBlock(snapshot->idx, 0);

	//必须受到成交后再触发卖单，此处以时间控制
	//严进松出,在买入时价格在最高+最低的一半以下
	if (p < snapshot->tradePx - 500
			&& snapshot->ask[0] - snapshot->bid[0] > param->sellGapPrice
			&& SOIR < param->sellSoir
			&& askqty > param->sellQtyRatio * 0.0001 * (askqty + bidqty)) {

		// 控制频率
		idx = XGetSellStorePos(pStrategy);
		if (idx < 0) {
			return;
		}

		CPStrategy2Ord(pStrategy, &order);

		order.bsType = eXSell;

		order.ordPrice = snapshot->bid[0];

		if(snapshot->market == eXMarketSha)
		{
			order.ordQty = sendQty > 10 * snapshot->bidqty[0] ? 10 * snapshot->bidqty[0] : sendQty;
		}
		else
		{
			order.ordQty = sendQty >  snapshot->bidqty[0] ?  snapshot->bidqty[0] : sendQty;
		}
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

		//发送委托
		slog_debug(0, "<<<<<< 卖委托[%lld-%d-%s],时间[%d],本地单号[%d],委托价格[%d],委托数量[%d]",
				pStrategy->plotid, snapshot->market, snapshot->securityId, snapshot->updateTime,
				localid, order.ordPrice, order.ordQty);
	}

}

static void Buy(XRSnapshotT *snapshot, XStrategyT *pStrategy,
		XHoldT *pHold, XBondTradeParamT *param) {
//	XMoney amt = 0;
	XPrice p = 0;
	XInt SOIR, SOIR1, SOIR2, SOIR3, SOIR4, SOIR5;
	XSumQty bidqty, askqty;
	XKLineT *k1 = NULL, *k5 = NULL;
	XNum kcursor1 = -1, kcursor5 = -1;
	XInt i;
	XSumQty sum, div;
	XInt zdf = 0, openzdf;
	XOrderReqT order = { 0 };
	XNum idx = 0;
	XLocalId localid = 0;
	XQty allowBuyQty;
//	XOrderT* pOrder = NULL;
//	XBool bCon = false;
	XSumQty sendQty = 0, sumhld = 0;

	if (pStrategy->version == snapshot->version) {
		return;
	}

	//高价股不做
	if (snapshot->tradePx > param->price) {
		return;
	}

	// 如果14:55不再开仓
	if (snapshot->updateTime > 145000000) {
		return;
	}

	sumhld = (NULL == pHold) ? 0 : pHold->sumHld;
	sendQty = pStrategy->_buySends - pStrategy->_buyRtn + pStrategy->_buyValid - pStrategy->_buyTrades;

	if(sendQty + sumhld >= pStrategy->plot.allowHoldQty)
	{
		return;
	}
	/**
	slog_debug(0, "[%lld-%d-%s], sendQty:[%lld], sumhld[%lld], buySend[%lld],buyrnt[%lld],buyvalid[%lld],buytrade[%lld]",
			pStrategy->plotid, pStrategy->setting.market, pStrategy->setting.securityId, sendQty, sumhld, pStrategy->_buySends,
			pStrategy->_buyRtn, pStrategy->_buyValid, pStrategy->_buyTrades);
	*/
	//盘口订单失衡
	for (i = 0; i < 5; i++) {
		sum += (5 - i) * (snapshot->bid[i] - snapshot->askqty[i]);
		div += (5 - i) * (snapshot->bid[i] + snapshot->askqty[i]);
	}
	//      pt = sum / div;
	//订单失衡
	//与上一根K线的比较

	//买单量大于卖单量

	bidqty = snapshot->bidqty[0] + snapshot->bidqty[1] + snapshot->bidqty[2]
			+ snapshot->bidqty[3] + snapshot->bidqty[4];
	askqty = snapshot->askqty[0] + snapshot->askqty[1] + snapshot->askqty[2]
			+ snapshot->askqty[3] + snapshot->askqty[4];

	//价格预测
	p = (snapshot->bid[0] + snapshot->ask[0]) / 2
			+ (snapshot->ask[0] - snapshot->bid[0]) * snapshot->bidqty[0]
					/ ((snapshot->bidqty[0] + snapshot->askqty[0]) * 2);

	//订单失衡SOIR
	//SOIR = (Bvi - Avi)/(Bvi + Avi)
	//权重 w = 1 - (i - 1)/5
	//加强 SOIR = ((Bv1 - Av1)/(Bv1 + Av1) + 4/5(Bv2 - Av2)/(Bv2 + Av2) + 3/5(Bv3 - Av3)/(Bv3 + Av3) + 2/5(B4 - A4)/(B4 + A4) + 1/5(B5 - A5)/(B5 + A5)) / ( 1 + 4/5 + 3/5 + 2/5 + 1/5)
	//SOIR为正说明市场买压大于卖压，未来价格趋向上涨，且SOIR的值越大，上涨的概率越高，反之亦然

	//中间价变化率MPC,MPC为正说明股票未来短期价格趋向上涨，且MPC的值越大，其上涨的概率越高，反之亦然
	//MPC = (A + B) - (A1 + B1) / (A1 + B1)

	SOIR1 = (snapshot->bidqty[0] - snapshot->askqty[0]) * 100
			/ (snapshot->bidqty[0] + snapshot->askqty[0]);
	SOIR2 = (snapshot->bidqty[1] - snapshot->askqty[1]) * 100
			/ (snapshot->bidqty[1] + snapshot->askqty[1]);
	SOIR3 = (snapshot->bidqty[2] - snapshot->askqty[2]) * 100
			/ (snapshot->bidqty[2] + snapshot->askqty[2]);
	SOIR4 = (snapshot->bidqty[3] - snapshot->askqty[3]) * 100
			/ (snapshot->bidqty[3] + snapshot->askqty[3]);
	SOIR5 = (snapshot->bidqty[4] - snapshot->askqty[4]) * 100
			/ (snapshot->bidqty[4] + snapshot->askqty[4]);

	SOIR = (5 * SOIR1 + 4 * SOIR2 + 3 * SOIR3 + 2 * SOIR4 + SOIR5)
			/ (5 + 4 + 3 + 2 + 1);

	/**
	 slog_debug(0, "[%d-%s] [%d] 当前最新价[%d] 预测股价[%d], SOIR[%d],买一量[%lld]", snapshot->market, snapshot->securityId,
	 snapshot->updateTime, snapshot->tradePx, p, SOIR, snapshot->bidqty[0]);
	 */
	pStrategy->_folwPx = snapshot->tradePx;

	kcursor1 = (kcursor1 - 2) & (SNAPSHOT_K1_CNT - 1);
	k1 = GetKlinesByBlock(snapshot->idx, 0);

	kcursor5 = (kcursor5 - 2) & (SNAPSHOT_K5_CNT - 1);
	k5 = GetKlinesByBlock(snapshot->idx, 1);

	zdf = (snapshot->tradePx - snapshot->preClosePx) * 10000
			/ snapshot->preClosePx;

	openzdf = (snapshot->openPx - snapshot->preClosePx) * 10000
			/ snapshot->preClosePx;
	//如果短周期不在长周期上不开仓
	//必须受到成交后再触发卖单，此处以时间控制
	//严进松出,在买入时价格在最高+最低的一半以下
	//量比
	//上一交易日振幅
	//K线位置

	//当前股票价格
	if (openzdf >= param->openZdf && zdf > param->zdf
			&& abs(openzdf - zdf) < param->zfRatio && p > snapshot->tradePx + 500
			&& snapshot->bidqty[0] > param->buyQtyLimit /** 买一量要大于100 */
			&& (snapshot->ask[0] - snapshot->bid[0]) > param->buyGapPrice
			&& SOIR > param->buySoir
			&& bidqty > param->buyQtyRatio * 0.0001 * (askqty + bidqty)) {

		idx = XGetBuyStorePos(pStrategy);
		if (idx < 0) {
			return;
		}

		allowBuyQty = pStrategy->plot.allowHoldQty - sendQty - sumhld;
		if(allowBuyQty > pStrategy->setting.ordQty)
		{
			allowBuyQty = pStrategy->setting.ordQty;
		}

		CPStrategy2Ord(pStrategy, &order);

		order.bsType = eXBuy;
		order.ordPrice = snapshot->ask[0];
		if(snapshot->market == eXMarketSha)
		{
			order.ordQty = allowBuyQty > 10 * snapshot->askqty[0] ? 10 * snapshot->askqty[0] : allowBuyQty;   //如果盘口数量少于委托数量以委托数量下单
		}
		else
		{
			order.ordQty = allowBuyQty > snapshot->askqty[0] ? snapshot->askqty[0] : allowBuyQty;
		}
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

		//发送委托
		slog_debug(0,
				">>>>>> 买委托[%lld-%d-%s],时间[%d],委托本地[%d],委托价格[%d],委托数量[%d]",
				pStrategy->plotid, snapshot->market, snapshot->securityId, snapshot->updateTime,
				localid, order.ordPrice, order.ordQty);
	}

}

XBool BondTrade(XRSnapshotT *snapshot, XStrategyT *pStrategy,
		XBondTradeParamT *param) {
	XHoldT *pHold = NULL;

	pHold = XFndVHoldByKey(pStrategy->plot.customerId, pStrategy->investId,
			pStrategy->setting.market, pStrategy->setting.securityId);

	Buy(snapshot, pStrategy, pHold, param);

	Sell(snapshot, pStrategy, pHold, param);

	pStrategy->version = snapshot->version;

	return (true);
}
