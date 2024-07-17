/*
 * XSignals.c
 *
 *  Created on: 2024年6月27日
 *      Author: Administrator
 */
#include "XSignals.h"
#include "XLog.h"
#include "XCom.h"
#include "XTimes.h"
#include <getopt.h>
#include "XUtils.h"

#define XSIGNAL_ZSAMT_BUY_POS                (1)

/**
 * 集合竞价阶段涨速成交量买入信号
 */
void ZsAmtAucBuySignal(XSnapshotT *pSnapshot, XStockT *pStock, XSessioManageT *pSessionMan)
{
	XMoney bidMoney = 0;
	XInt flag = 0;
	XInt zdf = 0;
	XBlockT *pBlock = NULL;
	XSignalT signal;
	XTradeCache webCache =
	{ 0 };

	//记录9:20分的虚拟成交量,成交价格
	if (pSnapshot->updateTime <= 92000000)
	{
		pSessionMan->triggerPx = pSnapshot->bid[0];
		pSessionMan->triggerQty = pSnapshot->bidqty[0];
	}
	else if (pSnapshot->updateTime >= 92500000 && pSnapshot->updateTime < 93000000)
	{
		//超过9:29:54秒异动处理
		if (0 != pSnapshot->bid[1] && pSnapshot->bid[0] > pSessionMan->triggerPx && pSnapshot->bidqty[0] > 2 * pSessionMan->triggerQty)
		{

			setbit(pSessionMan->triggerBit, XSIGNAL_ZSAMT_BUY_POS);
		}
		else
		{
			clrbit(pSessionMan->triggerBit, XSIGNAL_ZSAMT_BUY_POS);
		}
		bidMoney = pSnapshot->bidqty[0] * pSnapshot->bid[0] / 100000000;

		flag = getbit(pSessionMan->handleBit, XSIGNAL_ZSAMT_BUY_POS);

		zdf = (pSnapshot->bid[0] - pStock->preClose) * 10000 / pStock->preClose;

		if (!flag && pSessionMan->handleBit && bidMoney > 500)
		{
			pBlock = XFndVBlockById(pStock->industryIdx);

			slog_debug(0, "[%d-%s] version[%lld], updatetime[%d], 前收盘[%d], 买一价[%d],买一量[%lld] 买二量[%d], 9:20价格[%d],9:20量[%lld], 匹配金额[%d],涨跌幅[%.2f]", pSnapshot->market, pSnapshot->securityId, pSnapshot->version, pSnapshot->updateTime, pStock->preClose, pSnapshot->bid[0], pSnapshot->bidqty[0], pSnapshot->bid[1], pSessionMan->triggerPx, pSessionMan->triggerQty, bidMoney, zdf
					* 0.01);

			setbit(pSessionMan->handleBit, XSIGNAL_ZSAMT_BUY_POS);

			memset(&signal, 0, sizeof(XSignalT));
			signal.traday = pSnapshot->traday;
			signal.market = pSnapshot->market;
			signal.updateTime = pSnapshot->updateTime;
			memcpy(signal.securityId, pSnapshot->securityId, strlen(pSnapshot->securityId));
			signal.bsType = eXBuy;
			signal.idx = 0;
			signal.tradePx = pSnapshot->tradePx;
			signal.signalType = 0;
			sprintf(signal.remark, "[%s]涨速[%.2f] 成交额[%lld]万元 涨跌幅[%.2f],板块[%s]", pStock->securityName, 0.0, bidMoney, zdf * 0.01, pBlock->blockName);

			webCache.head.type = eSignal;
			webCache.head.dataLen = sizeof(XSignalT);
			webCache.signal = signal;

			XPushCache(XSHMKEYCONECT(rtnCache), &webCache);

		}
	}

}

/**
 * 连续竞价阶段涨速成交量买入信号
 */
void ZsAmtConBuySignal(XRSnapshotT *snapshot, XStockT *pStock, XSessioManageT *pSessionMan)
{

	XInt zdf = 0, zs = 0;
	XPrice curPx = 0, lastPx = 0, openPx = 0;
	XInt flag = 0;
	XSignalT signal;
	XTradeCache webCache = { 0 };
	XBlockT *pBlock = NULL;
	XBool bSignal = false;
	XKLineT *kline1 = NULL;
	XInt cursor = -1, lcursor = -1;
	XMoney amt = 0, preamt = 0, preamt3 = 0;

	// 9:31分之前涨速计算存在历史K线数据不正确
	//if (snapshot->updateTime < 93100000)
	//{
	//	return;
	//}
	/**
	if (snapshot->version == pSessionMan->version)
	{
		return;
	}
	slog_debug(0, "############");
	pSessionMan->version = snapshot->version;

	XPutOrUpdVSessionMan(0, pRSnapshot->market, pRSnapshot->securityId, &sessionMan);
	*/

	kline1 = GetKlinesByBlock(snapshot->idx, 0);

	cursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 1) & (SNAPSHOT_K1_CNT - 1);

	amt = kline1[cursor].amt;


	if (cursor == pSessionMan->kcursor1)
	{
		return;
	}

	flag = getbit(pSessionMan->handleBit, XSIGNAL_ZSAMT_BUY_POS);
	if (flag)
	{
		return;
	}


	curPx = kline1[cursor].close;
	openPx = kline1[cursor].open;

	// 前3分钟成交
	lcursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 2) & (SNAPSHOT_K1_CNT - 1);
	preamt = kline1[lcursor].amt;

	preamt3 = kline1[lcursor].driverBuyAmt - kline1[lcursor].driverSellAmt;
	lastPx = kline1[lcursor].close;
	if (lastPx <= 0)
	{
		return;
	}

	if (kline1[lcursor].close)
	{
		zs = (curPx - kline1[lcursor].close) * 10000 / kline1[lcursor].close;
	}

	// 前3分钟成交

	if (kline1[lcursor].close)
	{
		zs = (curPx - kline1[lcursor].close) * 10000 / kline1[lcursor].close;
	}

	lcursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 3) & (SNAPSHOT_K1_CNT - 1);
	preamt += kline1[lcursor].amt;
	preamt3 += kline1[lcursor].driverBuyAmt - kline1[lcursor].driverSellAmt;

	lcursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 4) & (SNAPSHOT_K1_CNT - 1);
	preamt += kline1[lcursor].amt;
	preamt3 += kline1[lcursor].driverBuyAmt - kline1[lcursor].driverSellAmt;

	if (amt > 150000000000 && zs > 150 && curPx > openPx && preamt3 > 0)
	{
		bSignal = true;
	}
	if (!bSignal && amt > 150000000000 && zs < -150 && curPx > openPx)
	{
		bSignal = true;
	}

	if (!bSignal)
	{
		return;
	}

	pBlock = XFndVBlockById(pStock->industryIdx);

	zdf = (snapshot->tradePx - pStock->preClose) * 10000 / pStock->preClose;

	memset(&signal, 0, sizeof(XSignalT));
	signal.traday = snapshot->traday;
	signal.market = snapshot->market;
	signal.updateTime = snapshot->updateTime;
	memcpy(signal.securityId, snapshot->securityId, strlen(snapshot->securityId));
	signal.bsType = eXBuy;
	signal.idx = 0;
	signal.tradePx = snapshot->tradePx;
	signal.signalType = 0;
	sprintf(signal.remark, "[%s]涨速[%.2f] 涨跌幅[%.2f] 最近1分钟成交额[%.2f]万元 最近3分钟成交额[%.2f]万元,板块[%s]", pStock->securityName, zs * 0.01, zdf * 0.01, amt * 0.0001 * 0.0001, preamt * 0.0001 * 0.0001, pBlock->blockName);

	slog_debug(0, "%s", signal.remark);
	webCache.head.type = eSignal;
	webCache.head.dataLen = sizeof(XSignalT);
	webCache.signal = signal;

	XPushCache(XSHMKEYCONECT(rtnCache), &webCache);

	pSessionMan->kcursor1 = cursor;
	setbit(pSessionMan->handleBit, XSIGNAL_ZSAMT_BUY_POS);

}


