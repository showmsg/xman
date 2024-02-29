
/*
 * @file XBondAvg.c
 * @brief 5分钟均线交易策略
 * @author kangyq
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 * 
 */
#include "XCom.h"
#include "XTimes.h"
#include "XBus.h"
#include "XLog.h"

static void MarketSell(XStrategyT *pStrategy, XRSnapshotT *snapshot,
                       XStockT *pStock, XHoldT *pHold)
{
    XOrderReqT sellorder = {0};
    XLocalId localid = 0;
    XQty allowSell = -1;
    XIdx sellidx = -1;
    XPrice ma5_20 = 0, ma5_5 = 0,  prePx = -1;
    XRatio rsi = 12;
    XKLineT* kline1 = NULL;

    if (pStrategy->version == snapshot->version)
    {
        return;
    }
    pStrategy->version = snapshot->version;

    kline1 = GetKlinesByBlock(snapshot->idx, 0);

    ma5_20 = MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1,  13);
    ma5_5 = MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1,  5);
    rsi = RSI(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1, 12);
    prePx = kline1[(SNAPSHOT_K1_CNT + snapshot->kcursor1 - 1) % SNAPSHOT_K1_CNT].close;

    if(!((rsi > 80 && snapshot->tradePx < ma5_5 && snapshot->tradePx < prePx) 
    || (snapshot->tradePx < prePx && snapshot->tradePx  > pStrategy->_lastBuyPx * 1.009)))
    {
        return;
    }


    /** 控制每笔报单之间的间隔 */
    if (XGetClockTime() < pStrategy->_sellLocTime + pStrategy->plot.ordGapTime * 1000000LL)
    {
        return;
    }

    // 控制频率
    sellidx = XGetSellStorePos(pStrategy);
    if (sellidx < 0)
    {
        return;
    }

    allowSell = pHold->sellAvlHld;

    CPStrategy2Ord(pStrategy, &sellorder);

    sellorder.bsType = eXSell;

    /***
    处理滑点
    */
    pStrategy->_slipSellTimes++;
    if (pStrategy->_slipSellTimes > pStrategy->plot.slipSellTimes)
    {
        pStrategy->_slipSellTimes = 1;
    }
    if(pStrategy->_lastSellPx > 1)
    {
        sellorder.ordPrice = pStrategy->_lastSellPx - pStrategy->setting.sellSlip; // 卖价减去滑点
    }
    else
    {
        sellorder.ordPrice = snapshot->tradePx - pStrategy->setting.sellSlip; // 卖价减去滑点
    }

    sellorder.ordQty = allowSell;
    sellorder.ordType = eXOrdLimit;
    sellorder._lastPx = snapshot->tradePx;
    sellorder._lastTime = snapshot->updateTime;
    sellorder._mktTime = snapshot->_recvTime;
    sellorder._bizIndex = snapshot->_bizIndex;

    localid = XPutOrderReq(&sellorder);
    pStrategy->_lastSellPx = sellorder.ordPrice;
    pStrategy->_lastBuyPx = 0;
    pStrategy->sellList[sellidx] = localid;
    pStrategy->_sellSends += sellorder.ordQty;
    pStrategy->_sellLocTime = XGetClockTime();
    pStrategy->_signal = 0;

    slog_info(0, ">>> [%d-%s]卖次数[%d],委托数量[%d],价格[%d(%d)], 均价[%d-%d]", pStrategy->setting.market,
              pStrategy->setting.securityId, pStrategy->_slipSellTimes,
              sellorder.ordQty,
              sellorder.ordPrice,  snapshot->tradePx, ma5_5, ma5_20);
    if(pStrategy->_lastSellPx < pStrategy->_lastBuyPx)
    {
        pStrategy->_bFirst++;
        if(pStrategy->_bFirst >= 3)
        {
            pStrategy->status = eXPlotStop;
        }
    }
}

/**
 * 1、开盘放量上涨的,买入
 * 2、开盘2分钟急速拉升大于2个点的买入
 * 3、RSI买入
 */
static void MarketBuy(XStrategyT *pStrategy, XRSnapshotT *snapshot,
                      XStockT *pStock, XHoldT *pHold)
{
    XQty allowBuyQty = -1 /*计算允许买量*/, totalAllowQty = -1;
    XNum idx = 0;
    XOrderReqT order = {0};
    XLocalId localid = 0;
    XPrice ma5_20 = 0, ma5_5 = 0, prePx = -1;
    XRatio zs = -1;
    XInt zdf = 0, bzdf = 0;
    XRSnapshotT *pBaseSnap = NULL;
    XKLineT *kline1 = NULL;

    if (pStrategy->version == snapshot->version)
    {
        return;
    }
    pStrategy->version = snapshot->version;

    if (snapshot->volumeTrade <= 0)
    {
        return;
    }

    if(snapshot->updateTime > 100000000)
    {
        return;
    }

    //找到正股的信息
    pBaseSnap = XFndVRSnapshotByKey(pStock->market, pStock->baseSecurityId);
    if(NULL == pBaseSnap)
    {
        return;
    }

    bzdf = (pBaseSnap->tradePx - pBaseSnap->preClosePx) * 10000 / pBaseSnap->preClosePx;
    zdf = (snapshot->tradePx - snapshot->preClosePx) * 10000 / snapshot->preClosePx;
    
    kline1 = GetKlinesByBlock(snapshot->idx, 0);

    ma5_20 = MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1,  13);
    ma5_5 = MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1,  5);
    //rsi = RSI(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1, 12);
    if(snapshot->updateTime < 93100000)
    {
        prePx = snapshot->openPx;
    }
    else
    {
        prePx = kline1[(SNAPSHOT_K1_CNT + snapshot->kcursor1 - 1) % SNAPSHOT_K1_CNT].open;
    }
	if(prePx)
	{
		zs = (snapshot->tradePx - prePx) * 10000 / prePx;
	}

    if(!(((bzdf > 200 && bzdf > zdf) || zdf > 0) && prePx != -1 && pStrategy->_folwPx < -10 && snapshot->tradePx > prePx))
    {
        pStrategy->_folwPx = zs;
        return;
    }
    pStrategy->_folwPx = zs;
    /** 控制每笔报单之间的间隔 */
    
    if (XGetClockTime() < pStrategy->_buyLocTime + pStrategy->plot.ordGapTime * 1000000LL)
    {
        return;
    }

    if (NULL != pHold)
    {
        totalAllowQty = pStrategy->plot.allowHoldQty - XGetBuyedQty(pStrategy, pStock->lmtBuyMinQty) - pHold->sumHld;
    }
    else
    {
        totalAllowQty = pStrategy->plot.allowHoldQty - XGetBuyedQty(pStrategy, pStock->lmtBuyMinQty);
    }

    //	slog_debug(0, "达到条件，允许买入[%d],卖一量[%ld]", totalAllowQty, snapshot->askqty[0]);
    if (totalAllowQty <= 0)
    {
        return;
    }
    allowBuyQty = totalAllowQty;

    if (pStock->lmtBuyMinQty > 0)
    {
        allowBuyQty = allowBuyQty / pStock->lmtBuyMinQty * pStock->lmtBuyMinQty;
    }

    if (allowBuyQty <= 0)
    {
        return;
    }

    idx = XGetBuyStorePos(pStrategy);
    if (idx < 0)
    {
        return;
    }

    order.bsType = eXBuy;
    CPStrategy2Ord(pStrategy, &order);

    /***
    处理滑点
    */
    pStrategy->_slipBuyTimes++;
    if (pStrategy->_slipBuyTimes > pStrategy->plot.slipBuyTimes)
    {
        pStrategy->_slipBuyTimes = 1;
    }
    if(pStrategy->_lastBuyPx > 1)
    {
        order.ordPrice = pStrategy->_lastBuyPx + pStrategy->setting.buySlip; // 卖价减去滑点
    }
    else
    {
        order.ordPrice = snapshot->tradePx + pStrategy->setting.buySlip; // 买价减去滑点
    }

    order.ordQty = allowBuyQty;
    order.ordType = eXOrdLimit;
    order._lastPx = snapshot->tradePx;
    order._lastTime = snapshot->updateTime;
    order._mktTime = snapshot->_recvTime;
    order._bizIndex = snapshot->_bizIndex;

    memcpy(order.investId, pStrategy->investId, sizeof(XInvestId));
    localid = XPutOrderReq(&order);
    pStrategy->buyList[idx] = localid;
    pStrategy->_buySends += order.ordQty;
    pStrategy->_lastBuyPx = order.ordPrice;
    pStrategy->_lastSellPx = 0;
    pStrategy->_buyLocTime = XGetClockTime();
    pStrategy->_bFirst = true;
    pStrategy->_signal = 1;

    slog_info(0, "<<<<<< [%d-%s]买次数[%d],委托数量[%d], 价格[%d],最新[%d],均价[%d-%d]",
              pStrategy->setting.market, pStrategy->setting.securityId, pStrategy->_slipBuyTimes,
              order.ordQty, order.ordPrice,  snapshot->tradePx,  
               ma5_5, ma5_20);
}

static void Trade(XMonitorT *pMonitor, XMonitorMdT *pMonitorMd)
{
    XIdx i;
    XRSnapshotT *pSnapshot = NULL, ticksnap = {0};
    XStrategyT *pStrategy = NULL;
    XStockT *pStock = NULL;
    XHoldT *pHold = NULL;

    for (i = 0; i < pMonitor->iTPlot; i++)
    {
        pStrategy = XFndVStrategyById(i + 1);
        if (NULL == pStrategy || pStrategy->status != eXPlotNormal || pStrategy->plot.plotType != eXBondAvg)
        {
            continue;
        }

        // 找到快照位置
        pSnapshot = XFndVRSnapshotByKey(pStrategy->setting.market, pStrategy->setting.securityId);
        if (NULL == pSnapshot)
        {
            continue;
        }
        memcpy(&ticksnap, pSnapshot, sizeof(XSnapshotT));

        // 不在时间范围的策略不运行
        if ((pMonitorMd->updateTime < pStrategy->plot.beginTime && 0 != pStrategy->plot.beginTime) 
        || (pMonitorMd->updateTime > pStrategy->plot.endTime && 0 != pStrategy->plot.endTime))
        {
            continue;
        }

        //TODO 收盘清仓
        
        pStock = XFndVStockByKey(pStrategy->setting.market, pStrategy->setting.securityId);
        if (NULL == pStock)
        {
            continue;
        }

        pHold = XFndVHoldByKey(pStrategy->plot.customerId, pStrategy->investId,
                           pStrategy->setting.market, pStrategy->setting.securityId);

        if (NULL != pHold && pHold->sellAvlHld > 0)
        {
            pStrategy->_slipBuyTimes = 0;
            MarketSell(pStrategy, &ticksnap, pStock, pHold);
            XCancelSell(pStrategy, 0, 0); //如果发起撤单,重置
        }
        else if (NULL == pHold || pHold->sumHld == 0)
        {
            pStrategy->_slipSellTimes = 0;
            MarketBuy(pStrategy, &ticksnap, pStock, pHold);
            XCancelBuy(pStrategy, 0, 0);
        }    
    }
}

int main(int argc, char *argv[])
{
    XMonitorT *pMonitor = NULL;
    XMonitorMdT *pMonitorMd = NULL;

    XManShmLoad();
    xslog_init(XSHM_SDB_FILE, "xbondavg");
    slog_info(0, "xbondavg 启动......");

    pMonitor = XFndVMonitor();
    if (NULL == pMonitor)
    {
        return (-1);
    }
    pMonitorMd = XFndVMdMonitor(eXExchSec);

    for (;;)
    {
        if(NULL == pMonitorMd)
        {
            continue;
        }
        if (isMktClosedTime(pMonitorMd->updateTime))
        {
            break;
        }
        Trade(pMonitor, pMonitorMd);
    }

    slog_info(3, "xbondavg 关闭");

    return (0);
}