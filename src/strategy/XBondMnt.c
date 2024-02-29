/*
 * @file XBondMnt.c
 * @brief 双低交易策略
 * @author kangyq
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 * 
1、价格接近目标价位差值为N时买入或卖出;
 	 双低值=可转债价格+转股溢价率*100
	https://baijiahao.baidu.com/s?id=1717370875435149262&wfr=spider&for=pc
1、择时，双低<160的，仓位30%;双低<155,仓位60%;双低<150,仓位100%；双低>170或<130调出；
2、中位数价格<110,单一转债价格>120且双低>125调出；中位价格>110,单一转债价格>125&双低>130,调出；要求调出的双低价要大于调入双低价>10;
 */

#include "XCom.h"
#include "XTimes.h"
#include "XBus.h"
#include "XLog.h"
#include "XCSV.h"

typedef struct _xBondCnv
{
	XChar market;
	XSecurityId securityId;
	XInt bs;
	XInt zgjz;				/** 转股价值 */
	XInt zgjyl;				/** 转股溢价率 */
	XPrice price;			/** 转债价格 */
	XPrice zPrice;			/** 正股价格 */
	XPrice convPx;			/** 转股价 */
	XInt   noneBuy;			/** 不买 */
	XMoney profit;          /** 收益 */
	XQty qty;				/** 卖出的数量 */
}XBondCnvT;

static XBondCnvT bondcnv[1000];
static XInt bSelect = 0;
static XInt iCntBuy = -1, needBuy = -1;

int cmp_by_zgjz(const void* e1, const void* e2)
{
	return ((XBondCnvT*)e1)->zgjz - ((XBondCnvT*)e2)->zgjz;
}

int cmp_by_profit(const void* e1, const void* e2)
{
	return ((XBondCnvT*)e1)->profit - ((XBondCnvT*)e2)->profit;
}

static XInt CalcJz(XMonitorT *pMonitor, XBondCnvT bondcnv[], XInt defaultQty)
{
	XRSnapshotT *pSnapshot = NULL, *pZSnapshot = NULL;
	XStockT *pStock = NULL;
	XIdx i = 0, j = 0;
	XInt zgjz, zgjyl;
	
	for(i = 0; i < pMonitor->iTOrderBook; i++)
	{
		pSnapshot = XFndVRSnapshotById(i + 1);
		if(NULL == pSnapshot)
		{
			continue;
		}

		pStock = XFndVStockByKey(pSnapshot->market, pSnapshot->securityId);

		if(NULL == pStock || pStock->subSecType != eXSubSecCCF || pStock->convPx == 0)
		{
			continue;
		}

		pZSnapshot = XFndVRSnapshotByKey(pStock->market, pStock->baseSecurityId);
		if(NULL == pZSnapshot)
		{
			continue;
		}

		if(pSnapshot->tradePx == 0 || pZSnapshot->tradePx == 0)
		{
			continue;
		}

		//转股价值 = 正股价/转股价 * 100
		zgjz = pZSnapshot->tradePx * 100 / pStock->convPx * 10000;

		
		//转股溢价率 = (转债最新价 - 转股价值) / 转股价值
		if(zgjz != 0)
		{
			zgjyl = (pSnapshot->tradePx - zgjz) * 10000 / zgjz ;
		}
		else
		{
			zgjyl = 0;
		}

		if(zgjz < 800000 || zgjz > 1600000)
		{
			continue;
		}
		if(zgjyl < -500 || zgjyl > 6000)
		{
			continue;
		}
		
		bondcnv[j].bs = eXBuy;
		bondcnv[j].price = pSnapshot->tradePx;
		bondcnv[j].zPrice = pZSnapshot->tradePx;
		bondcnv[j].convPx = pStock->convPx;
		bondcnv[j].market = pSnapshot->market;
		memcpy(&bondcnv[j].securityId, pSnapshot->securityId, strlen(pSnapshot->securityId));
		bondcnv[j].zgjz = zgjz;
		bondcnv[j].zgjyl = zgjyl;
		bondcnv[j].qty = defaultQty;
		j++;
		slog_debug(0, "[%d-%s],转股价值[%d],转股溢价率[%d]", pSnapshot->market, pSnapshot->securityId, zgjz, zgjyl);
	}

	qsort(bondcnv, j, sizeof(XBondCnvT), cmp_by_zgjz);

	return (j);
}


static XInt UpdZgj(XChar* convFile)
{
	XCsvHandleT handle;
	XChar* col = NULL;
	XInt iret = -1;
	XChar market = -1;
	XChar* securityId = NULL;
	XInt convPx = -1;
	XStockT *pStock = NULL;

	iret = XCsvOpen(&handle, convFile);
	if (iret)
	{
		slog_error(0, "文件不存在[%s]", convFile);
		return (0);
	}
	slog_debug(0, "更新转股价");

	while ((!XCsvReadLine(&handle)))
	{
		if(handle.colSize < 3)
		{
			continue;
		}
		col = handle.GetFieldByCol(&handle, 0);
		if(col)
		{
			market = col[0];
		}
		col = handle.GetFieldByCol(&handle, 1);
		if(col)
		{
			securityId = col;
		}
		col = handle.GetFieldByCol(&handle, 2);
		if(col)
		{
			convPx = atoi(col);
		}

		pStock = XFndVStockByKey(market, securityId);
		if(NULL != pStock)
		{
			pStock->convPx = convPx;
//			slog_debug(0, "更新转股价[%d-%s],[%d]", market, securityId, convPx);
		}

	}

	XCsvClose(&handle);

	return 0;
}

static void BasketSell(XStrategyT *pStrategy, XSnapshotT *snapshot,
		XStockT *pStock) {
	XQty allowSell = 0, calQty = 0;
	XOrderReqT order = { 0 };
	XLocalId localid = 0;
	XHoldT *pHold = NULL;
	XIdx idx = -1;
	XInt zdf = -1;
	XInt fy = -1;

	if (pStrategy->version == snapshot->version) {
		return;
	}
	pStrategy->version = snapshot->version;

	
	pHold = XFndVHoldByKey(pStrategy->plot.customerId, pStrategy->investId, pStrategy->setting.market, pStrategy->setting.securityId);
	if(NULL == pHold)
	{
//		pStrategy->status = eXPlotStop;
		return;
	}
	
	zdf = (snapshot->tradePx - snapshot->preClosePx) * 10000 / snapshot->preClosePx;

	if(pHold->costPrice > 0)
	{
		fy = (snapshot->tradePx - pHold->costPrice) * 10000 / pHold->costPrice;
	}

	slog_debug(0, "[%d-%s-%lld]1.涨跌幅[%d],浮盈[%d]", snapshot->market,snapshot->securityId, pStrategy->plotid ,zdf, fy);

	if(!(zdf < pStrategy->setting.stopLoss || zdf > pStrategy->setting.stopProfit || fy >= 400))
	{
		return;
	}

	if(XGetClockTime() < pStrategy->_buyLocTime + pStrategy->plot.ordGapTime * XTIMS_MS4NS)
	{
		return;
	}

	calQty = pStrategy->setting.ordQty - pStrategy->_sellSends + pStrategy->_sellValid;
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

	CPStrategy2Ord(pStrategy, &order);
	
	order.bsType = eXSell;
	order.ordPrice = snapshot->tradePx - pStrategy->setting.sellSlip;
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
	slog_debug(0, ">>> 2.[%d-%s-%lld],委托本地编号[%d]", pStrategy->setting.market,
			pStrategy->setting.securityId, pStrategy->plotid, localid);
}

//根据持仓收益排名
static XSumQty CalcClear(XMonitorT *pMonitor, XBondCnvT bondcnv[], XInt *ibondcnv)
{
	XHoldT* pHold = NULL;
	XIdx i = -1, j = -1;
	XStockT *pStock = NULL;
	XInt iCnt = 0;
	XInt bSell = 1;
	XSumQty qty = 0;
	XBondCnvT bond[1000];
	XRSnapshotT *pSnapshot = NULL;

	for(i = 0; i < pMonitor->iTHold; i++)
	{
		pHold = XFndVHoldById(i + 1);
		if(NULL == pHold || pHold->sumHld == 0)
		{
			continue;
		}
		pStock = XFndVStockByKey(pHold->market, pHold->securityId);
		if (NULL == pStock || eXSubSecCCF != pStock->subSecType) {
			continue;
		}

		pSnapshot = XFndVRSnapshotByKey(pHold->market, pHold->securityId);
		if(NULL == pSnapshot)
		{
			continue;
		}

		bond[iCnt].market = pHold->market;
		memcpy(bond[iCnt].securityId, pHold->securityId, sizeof(pHold->securityId));
		bond[iCnt].qty = pHold->sumHld;
		bond[iCnt].bs = eXSell;
		bond[iCnt].profit = (pSnapshot->tradePx - pHold->costPrice) * pHold->sumHld;
		iCnt++;
	}

	qsort(bond, iCnt, sizeof(XBondCnvT), cmp_by_profit);

	//遍历卖出的，取10%的名单
	for(i = 0; i < iCnt; i++)
	{
		if(i > iCnt / 10)
		{
			break;
		}
		bSell = 1;
		for(j = 0; j < *ibondcnv; j++)
		{
			if(0 == strcmp(bond[i].securityId, bondcnv[j].securityId))
			{
				bondcnv[j].noneBuy = 1;
				bSell = 0;
				break;
			}
		}
		if(bSell)
		{
			qty += bond[i].qty;
			bondcnv[*ibondcnv].market = bond[i].market;
			memcpy(bond[*ibondcnv].securityId, bond[i].securityId, sizeof(bond[i].securityId));
			bondcnv[*ibondcnv].bs = eXSell;
			bondcnv[*ibondcnv].qty = bond[i].qty;
		}
	}

	return (qty);
}

static void CalcExec(XBondCnvT bondcnv[], XInt ibondcnv, XInt buyCnt)
{
	XInt i = 0, iBuyed = 0;

	for(i = 0; i < ibondcnv; i++)
	{
		if(bondcnv[i].bs == eXBuy)
		{
			if(iBuyed < buyCnt && !bondcnv[i].noneBuy)
			{
				slog_info(0, "[%d-%s],买卖[%d], 价格[%d], 数量[%d]", bondcnv[i].market, bondcnv[i].securityId, bondcnv[i].bs,
				bondcnv[i].price, bondcnv[i].qty);
				iBuyed++;
			}
		}
		else
		{
			slog_info(0, "[%d-%s],买卖[%d], 价格[%d], 数量[%d]", bondcnv[i].market, bondcnv[i].securityId, bondcnv[i].bs,
				bondcnv[i].price, bondcnv[i].qty);
		}	
	}
}

static XSumQty CalcTrade(XMonitorT *pMonitor)
{
	XSumQty qty = 0;
	XIdx i = 0;
	XTradeT *pTrade = NULL;
	XStockT *pStock = NULL;

	for (i = 0; i < pMonitor->iTTrade; i++) {
		pTrade = XFndVTradeById(i + 1);
		if(NULL == pTrade)
		{
			continue;
		}
		pStock = XFndVStockByKey(pTrade->market, pTrade->securityId);
		if (NULL == pStock || eXSubSecCCF != pStock->subSecType) {
			continue;
		}
		
		qty += pTrade->trdQty;
	}

	return (qty);
}
static void Trade(XMonitorT *pMonitor, XMonitorMdT *pMonitorMd, XQty defaultQty) {
	XIdx i;
	XRSnapshotT *pSnapshot = NULL, ticksnap = { 0 };
	XStrategyT *pStrategy = NULL;
	XStockT *pStock = NULL;
	XSumQty  qty = -1, qtyclear = -1;
	
	
	if(pMonitorMd->updateTime < 145600000)
	{
		//有持仓的监控行情涨跌，卖出
		for (i = 0; i < pMonitor->iTPlot; i++) {
			pStrategy = XFndVStrategyById(i + 1);
			if (NULL == pStrategy || pStrategy->status != eXPlotNormal
					|| pStrategy->plot.plotType != eXBondHold) {
				continue;
			}

			//找到快照位置
			pSnapshot = XFndVRSnapshotByKey(pStrategy->setting.market,
					pStrategy->setting.securityId);
			if (NULL == pSnapshot) {
				continue;
			}
			memcpy(&ticksnap, pSnapshot, sizeof(XSnapshotT));

			//不在时间范围的策略不运行
			if ((pMonitorMd->updateTime < pStrategy->plot.beginTime
					&& 0 != pStrategy->plot.beginTime)
					|| (pMonitorMd->updateTime > pStrategy->plot.endTime
							&& 0 != pStrategy->plot.endTime)) {
				continue;
			}


			pStock = XFndVStockByKey(pStrategy->setting.market, pStrategy->setting.securityId);
			if (NULL == pStock) {
				continue;
			}
			
			if(pStrategy->setting.bsType == eXSell)
			{
				XCancelSell(pStrategy, 0, pStrategy->plot.isCtrlStop);
				BasketSell(pStrategy, &ticksnap, pStock);
			}
		}
	}
	//计算当前转股价值转股溢价率
	else
	
	{
		if(!bSelect)
		{
			memset(&bondcnv, 0, 1000 * sizeof(XBondCnvT));
			//1、按转股价值计算排名
			iCntBuy = CalcJz(pMonitor, bondcnv, defaultQty);
			slog_info(0, "按转股价值计算排名[%d]", iCntBuy);
			//2、如果今天是换仓日,则调整收益后的10%卖出
			qtyclear = CalcClear(pMonitor, bondcnv, &iCntBuy);
			slog_info(0, "在换仓日计算调仓数据");
			//3、获取可转债今天卖出的数量,遍历成交,获取可转债卖出数量
			qty = CalcTrade(pMonitor);	
			slog_info(0, "计算今天调仓出去的数量");
			//找今天卖了多少,遍历成交，获取成交数量
			
			needBuy = (qtyclear + qty) / defaultQty;
			slog_info(0, "计算当前需要买入的证券只数[%d]", needBuy);
			bSelect = 1;
			CalcExec(bondcnv, iCntBuy, needBuy);
			slog_info(0, "调仓完毕");
		}
		//未成交撤单继续买卖
		
	}
}

int main(int argc, char *argv[]) {
	XMonitorT *pMonitor = NULL;
	XMonitorMdT *pMonitorMd = NULL;

	XManShmLoad();
	xslog_init(XSHM_SDB_FILE, "xbondmnt");

	slog_info(0, "xbondmnt 启动......");

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return (-1);
	}
	pMonitorMd = XFndVMdMonitor(eXExchSec);

	UpdZgj("../data/outer/convbond.csv");
	//更新转股价,转股价值和转股溢价率交易时计算,抛除强赎的不买,stock中增加两个信息 转股价值，是否强赎标志
	
	for (;;) {
		if (isMktClosedTime(pMonitorMd->updateTime)) {
			break;
		}

		
		Trade(pMonitor, pMonitorMd, 20);
		
	}

	slog_info(3, "xbasket 关闭");

	return (0);
}
