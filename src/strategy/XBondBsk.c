/**
 * @file XBasket2.c
 *
篮子买卖：
1、价格接近目标价位差值为N时买入或卖出;
 	 双低值=可转债价格+转股溢价率*100
	https://baijiahao.baidu.com/s?id=1717370875435149262&wfr=spider&for=pc
1、择时，双低<160的，仓位30%;双低<155,仓位60%;双低<150,仓位100%；双低>170或<130调出；
2、中位数价格<110,单一转债价格>120且双低>125调出；中位价格>110,单一转债价格>125&双低>130,调出；要求调出的双低价要大于调入双低价>10;

 *  Created on: 2022年8月3日
 *      Author: DELL
 */

#include "XCom.h"
#include "XTimes.h"
#include "XBus.h"
#include "XLog.h"

static void BasketBuy(XStrategyT *pStrategy, XSnapshotT *snapshot,
		XStockT *pStock) {
	XQty allowBuyQty = -1;
	XNum idx = 0;
	XOrderReqT order = { 0 };
	XLocalId localid = 0;

	if (pStrategy->version == snapshot->version) {
		return;
	}
	pStrategy->version = snapshot->version;

	//离最新价差1毛时委托
	if (snapshot->tradePx > pStrategy->conPx || snapshot->tradePx + pStrategy->basket._aimDiff /*价格波动范围 */ < pStrategy->conPx) {
		return;
	}

	slog_debug(0, "[%lld-%d-%s] 1.判断最新价是否等于条件价,最新价[%d],条件价[%d]",
			pStrategy->plotid, snapshot->market, snapshot->securityId,
			snapshot->tradePx, pStrategy->conPx);

	slog_debug(0, "[%lld-%d-%s] 2.判断开盘价[%d]是否等于涨停价[%d]", pStrategy->plotid,
			snapshot->market, snapshot->securityId, snapshot->openPx,
			pStock->upperPrice);

	//开盘价是涨停价，策略暂停
	if (snapshot->openPx == pStock->upperPrice) {
		pStrategy->status = eXPlotStop;
		return;
	}


	slog_debug(0, "[%lld-%d-%s] 3.策略启动时[%d]，最新价[%d]是否等于涨停价[%d]",
			pStrategy->plotid, snapshot->market, snapshot->securityId,
			pStrategy->beginTime, snapshot->tradePx, pStock->upperPrice);

	//篮子开始的时间，最新价为涨停价，策略暂停
	if (snapshot->tradePx == pStock->upperPrice
			&& pStrategy->beginTime >= (snapshot->updateTime / 1000 * 1000)) {
		pStrategy->status = eXPlotStop;
		return;
	}

	allowBuyQty = XGetCanBuyQty(pStrategy, pStock->lmtBuyMinQty);

	if (allowBuyQty <= 0) {
		return;
	}
	idx = XGetBuyStorePos(pStrategy);
	if (idx < 0) {
		return;
	}
	memcpy(order.customerId, pStrategy->customerId, sizeof(XCustomer));
	order.market = pStrategy->market;
	memcpy(order.securityId, pStrategy->securityId, sizeof(XSecurityId));
	order.bsType = eXBuy;
	order.ordPrice = pStrategy->ordPx;
	order.ordQty = allowBuyQty;
	order.ordType = eXOrdLimit;
	order.acctType = pStrategy->acctType;
	order.plotType = pStrategy->plotType;
	order.plotid = pStrategy->plotid;
	order._lastPx = snapshot->tradePx;
	order._lastTime = snapshot->updateTime;
	order._mktTime = snapshot->_recvTime;
	order._bizIndex = snapshot->_bizIndex;

	memcpy(order.investId, pStrategy->investId, sizeof(XInvestId));
	localid = XPutOrderReq(&order);
	pStrategy->buyList[idx] = localid;
	pStrategy->_buySends += order.ordQty;
	pStrategy->_lastBuyPx = order.ordPrice;
	pStrategy->_buyLocTime = XGetClockTime();

#ifdef __TEST__
	slog_debug(0, "5.[%d-%s-%lld],委托本地编号[%d]", pStrategy->market,
			pStrategy->securityId, pStrategy->plotid, localid);
#endif

}

static void BasketSell(XStrategyT *pStrategy, XSnapshotT *snapshot,
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

	//离最新价差1毛时委托
	if(snapshot->tradePx + pStrategy->basket._aimDiff < pStrategy->conPx )
	{
		return;
	}

	if(XGetClockTime() < pStrategy->_buyLocTime + pStrategy->ordGapTime * 1000000LL)
	{
		return;
	}

	pHold = XFndVHoldByKey(pStrategy->customerId, pStrategy->investId, pStrategy->market, pStrategy->securityId);
	if(NULL == pHold)
	{
		pStrategy->status = eXPlotStop;
		return;
	}

	calQty = pStrategy->ordQty - pStrategy->_sellSends + pStrategy->_sellValid;
	if(calQty <= 0)
	{
		return;
	}

	if(calQty < pHold->sellAvlHld && pStock->lmtBuyMinQty > 0)
	{
		allowSell = calQty / pStock->lmtBuyMinQty * pStock->lmtBuyMinQty;
	}
	else
	{
		allowSell = pHold->sellAvlHld;
	}
	if(allowSell <= 0)
	{
		return;
	}

	//控制频率
	idx = XGetSellStorePos(pStrategy);
	if (idx < 0) {
		return;
	}

	memcpy(order.customerId, pStrategy->customerId, sizeof(XCustomer));
	order.market = pStrategy->market;
	memcpy(order.securityId, pStrategy->securityId, sizeof(XSecurityId));
	order.bsType = eXSell;
	order.ordPrice = pStrategy->ordPx;
	order.ordQty = allowSell;
	order.ordType = eXOrdLimit;
	order.acctType = pStrategy->acctType;
	order.plotType = pStrategy->plotType;
	order.plotid = pStrategy->plotid;
	order._lastPx = snapshot->tradePx;
	order._lastTime = snapshot->updateTime;
	order._mktTime = snapshot->_recvTime;
	order._bizIndex = snapshot->_bizIndex;

	memcpy(order.investId, pStrategy->investId, sizeof(XInvestId));
	localid = XPutOrderReq(&order);
	pStrategy->sellList[idx] = localid;
	pStrategy->_sellSends += order.ordQty;
	pStrategy->_sellLocTime = XGetClockTime();
	pStrategy->_lastSellPx = order.ordPrice;
}

static void Trade(XMonitorT *pMonitor, XMonitorMdT *pMonitorMd) {
	XIdx i;
	XSnapshotT *pSnapshot = NULL, ticksnap = { 0 };
	XStrategyT *pStrategy = NULL;
	XStockT *pStock = NULL;

	for (i = 0; i < pMonitor->iTPlot; i++) {
		pStrategy = XFndVStrategyById(i + 1);
		if (NULL == pStrategy || pStrategy->status != eXPlotNormal
				|| pStrategy->plotType != eXBondHold) {
			continue;
		}

		//找到快照位置
		pSnapshot = XFndVTSnapshotByKey(pStrategy->market,
				pStrategy->securityId);
		if (NULL == pSnapshot) {
			continue;
		}
		memcpy(&ticksnap, pSnapshot, sizeof(XSnapshotT));

		//不在时间范围的策略不运行
		if ((pMonitorMd->updateTime < pStrategy->beginTime
				&& 0 != pStrategy->beginTime)
				|| (pMonitorMd->updateTime > pStrategy->endTime
						&& 0 != pStrategy->endTime)) {
			continue;
		}

		pStock = XFndVStockByKey(pStrategy->market, pStrategy->securityId);
		if (NULL == pStock) {
			continue;
		}

		if (eXBuy == pStrategy->bsType) {
			XCancelBuy(pStrategy, &ticksnap, pStock, 0);
			BasketBuy(pStrategy, &ticksnap, pStock);
		} else {
			XCancelSell(pStrategy, &ticksnap, pStock, 0);
			BasketSell(pStrategy, &ticksnap, pStock);
		}
	}
}

int main(int argc, char *argv[]) {
	XMonitorT *pMonitor = NULL;
	XMonitorMdT *pMonitorMd = NULL;

	XManShmLoad();
	xslog_init(XSHM_SDB_FILE, "xbondbsk");

	slog_info(0, "xbondbsk 启动......");

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return (-1);
	}
	pMonitorMd = XFndVMdMonitor(eXExchSec);

	for (;;) {
		if (isMktClosedTime(pMonitorMd->updateTime)) {
			break;
		}
		Trade(pMonitor, pMonitorMd);
	}

	slog_info(3, "xbasket 关闭");

	return (0);
}
