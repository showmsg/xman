/*
 * Test.c
 *
 *  Created on: 2024年7月12日
 *      Author: Administrator
 */
#include<fcntl.h>
#include "XStore.h"
#include "XExport.h"
#include "XLog.h"
#include "XTimes.h"
#include "XUtils.h"

#define XSIGNAL_ZSAMT_BUY_POS                (1)

void ZsConBuySignal(XRSnapshotT *snapshot, XStockT *pStock, XSessioManageT *pSessionMan)
{
	XKLineT *kline1 = NULL;
	XInt cursor = -1;
	XPrice px = 0;

	kline1 = GetKlinesByBlock(snapshot->idx, 0);

	cursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 1) & (SNAPSHOT_K1_CNT - 1);

	px = kline1[cursor].close;


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
	if (snapshot->updateTime < 93100000)
	{
		return;
	}

	if (snapshot->version == pSessionMan->version)
	{
		return;
	}

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
	sprintf(signal.remark, "[%s]涨速[%.2f] 涨跌幅[%.2f] 最近1分钟成交额[%.2f]万元 最近3分钟成交额[%.2f]万元", pStock->securityName, zs * 0.01, zdf * 0.01, amt * 0.0001 * 0.0001, preamt * 0.0001 * 0.0001);

	slog_debug(0, "%s", signal.remark);
	webCache.head.type = eSignal;
	webCache.head.dataLen = sizeof(XSignalT);
	webCache.signal = signal;

	XPushCache(XSHMKEYCONECT(rtnCache), &webCache);

	pSessionMan->kcursor1 = cursor;
	setbit(pSessionMan->handleBit, XSIGNAL_ZSAMT_BUY_POS);

}

XVoid XDealSignal(XVoid *params)
{
	XMonitorT *pMonitor = NULL;
	XIdx i = 0;
	XRSnapshotT *pRSnapshot = NULL, snapshot;
	XSnapshotT *pSnapshot = NULL;
	XStockT *pStock = NULL;
	XSessioManageT sessionMan, *pSessionMan = NULL;
	XMonitorMdT *pMonitorMd = NULL;

	pMonitor = XFndVMonitorS(__FILE__, __LINE__);
	if (NULL == pMonitor)
	{
		return;
	}
	pMonitorMd = XFndVMdMonitor(eXExchSec);

	slog_info(0, "信号触发......");
	while (1)
	{
		/**
		if (pMonitorMd->updateTime < 93000000)
		{
			for (i = 0; i < pMonitor->iTSnapshot; i++)
			{
				pSnapshot = XFndVSnapshotById(i + 1);
				if (NULL == pSnapshot)
				{
					continue;
				}

				if (pSnapshot->updateTime > 92500000)
				{
					continue;
				}
				pStock = XFndVStockByKey(pSnapshot->market, pSnapshot->securityId);
				if (NULL == pStock)
				{
					continue;
				}

				pSessionMan = (XSessioManageT*) XFndVSessionManByKey(0, pSnapshot->market, pSnapshot->securityId);
				if (NULL != pSessionMan)
				{
					if (pSnapshot->version == pSessionMan->version)
					{
						continue;
					}
					memcpy(&sessionMan, pSessionMan, sizeof(XSessioManageT));
				}
				else
				{
					memset(&sessionMan, 0, sizeof(XSessioManageT));
				}

				sessionMan.version = pSnapshot->version;

				//信号处理
				ZsAmtAucBuySignal(pSnapshot, pStock, &sessionMan);

				XPutOrUpdVSessionMan(0, pSnapshot->market, pSnapshot->securityId, &sessionMan);
			}
		}
		else
		*/
		{
			for (i = 0; i < pMonitor->iTOrderBook; i++)
			{
				pRSnapshot = XFndVRSnapshotById(i + 1);
				if (NULL == pRSnapshot)
				{
					continue;
				}
				memcpy(&snapshot, pRSnapshot, sizeof(XRSnapshotT));
				pStock = XFndVStockByKey(snapshot.market, snapshot.securityId);
				if (NULL == pStock || eXStock != pStock->secType)
				{
					continue;
				}

				pSessionMan = (XSessioManageT*) XFndVSessionManByKey(0, snapshot.market, snapshot.securityId);
				if (NULL != pSessionMan)
				{
					if (snapshot.version == pSessionMan->version)
					{
						continue;
					}
					memcpy(&sessionMan, pSessionMan, sizeof(XSessioManageT));
				}
				else
				{
					memset(&sessionMan, 0, sizeof(XSessioManageT));
				}

				sessionMan.version = snapshot.version;

				//处理信号
				ZsAmtConBuySignal(&snapshot, pStock, &sessionMan);

				XPutOrUpdVSessionMan(0, pRSnapshot->market, pRSnapshot->securityId, &sessionMan);
			}
		}
		//sched_yield();
	}
}


int main(int argc, char *argv[]) {

	xslog_init(NULL, "test");
	XManShmLoad();

	XDealSignal(NULL);
	return 0;
}
