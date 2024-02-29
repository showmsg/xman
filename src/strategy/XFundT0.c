
/*
 * @file XFundT0.c
 * @brief T0交易基金
 * @author kangyq
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 * 
 */
#include "XCommon.h"
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
    XPrice ma1_5 = 0,  avgPx = -1, pma1_5 = 0, prePx = -1;
    XInt cursor = 0, lcursor = 0, llcursor = 0;
    XKLineT* kline1 = NULL;
    XBool bSell = false;
    XPrice p ;
    XInt SOIR, SOIR1, SOIR2, SOIR3, SOIR4, SOIR5;

    if (pStrategy->version == snapshot->version)
    {
        return;
    }
    pStrategy->version = snapshot->version;

    //1. 放量且最近1根K线在最高点，最新价小于上一根K线最低点卖出
    //2. 当前放量大于1000万且离最近1跟K线的低点卖出
    //3. 最近5分钟K线趋势往下,卖出

    kline1 = GetKlinesByBlock(snapshot->idx, 0);

    cursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 2) % SNAPSHOT_K1_CNT;
    lcursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 3) % SNAPSHOT_K1_CNT;
    llcursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 4) % SNAPSHOT_K1_CNT;

    prePx = kline1[cursor].close;
    pma1_5 = MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1 - 1,  5);

    ma1_5 = MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1,  5);

    avgPx = snapshot->amountTrade / snapshot->volumeTrade;
    
    //价格预测
	p = (snapshot->bid[0] + snapshot->ask[0]) / 2 + (snapshot->ask[0] - snapshot->bid[0]) * snapshot->bidqty[0] / ((snapshot->bidqty[0] + snapshot->askqty[0]) * 2);

    SOIR1 = (snapshot->bidqty[0] - snapshot->askqty[0]) * 100 / (snapshot->bidqty[0] + snapshot->askqty[0]);
    SOIR2 = (snapshot->bidqty[1] - snapshot->askqty[1]) * 100 / (snapshot->bidqty[1] + snapshot->askqty[1]);
    SOIR3 = (snapshot->bidqty[2] - snapshot->askqty[2]) * 100 / (snapshot->bidqty[2] + snapshot->askqty[2]);
    SOIR4 = (snapshot->bidqty[3] - snapshot->askqty[3]) * 100 / (snapshot->bidqty[3] + snapshot->askqty[3]);
    SOIR5 = (snapshot->bidqty[4] - snapshot->askqty[4]) * 100 / (snapshot->bidqty[4] + snapshot->askqty[4]);

    SOIR = (5 * SOIR1 + 4 * SOIR2 + 3 * SOIR3  + 2 * SOIR4  + SOIR5 ) / (5 + 4 + 3 + 2 + 1);

    //均线变换
    if((kline1[cursor].close < kline1[lcursor].close && kline1[lcursor].close < kline1[llcursor].close && SOIR < -20))
    {
        bSell = true;
    }	

    if(!bSell)
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

    slog_info(0, ">>> [%d-%s]卖次数[%d],委托数量[%d],价格[%d(%d)]", pStrategy->setting.market,
              pStrategy->setting.securityId, pStrategy->_slipSellTimes,
              sellorder.ordQty,
              sellorder.ordPrice,  snapshot->tradePx);
    
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
    XPrice ma1_5 = 0,  avgPx = -1, pma1_5 = 0, prePx = -1;
    XInt cursor = 0;
    XPrice p ;
    XInt SOIR, SOIR1, SOIR2, SOIR3, SOIR4, SOIR5;

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

    //9:50开始交易
    if(snapshot->updateTime < 93100000)
    {
        return;
    }
    
    //1. 涨跌幅在-1% ～ 5%
    //2. 成交量放量1000万并且涨速大于2%
    //3. 量要大于前量
    //3. 最新价突破前5分钟最高点,买入




    kline1 = GetKlinesByBlock(snapshot->idx, 0);

    cursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 2) % SNAPSHOT_K1_CNT;

    prePx = kline1[cursor].close;
    pma1_5 = MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1 - 1,  5);

    ma1_5 = MA(kline1, SNAPSHOT_K1_CNT, snapshot->kcursor1,  5);

    avgPx = snapshot->amountTrade / snapshot->volumeTrade;
    
    //价格预测
	p = (snapshot->bid[0] + snapshot->ask[0]) / 2 + (snapshot->ask[0] - snapshot->bid[0]) * snapshot->bidqty[0] / ((snapshot->bidqty[0] + snapshot->askqty[0]) * 2);

    SOIR1 = (snapshot->bidqty[0] - snapshot->askqty[0]) * 100 / (snapshot->bidqty[0] + snapshot->askqty[0]);
    SOIR2 = (snapshot->bidqty[1] - snapshot->askqty[1]) * 100 / (snapshot->bidqty[1] + snapshot->askqty[1]);
    SOIR3 = (snapshot->bidqty[2] - snapshot->askqty[2]) * 100 / (snapshot->bidqty[2] + snapshot->askqty[2]);
    SOIR4 = (snapshot->bidqty[3] - snapshot->askqty[3]) * 100 / (snapshot->bidqty[3] + snapshot->askqty[3]);
    SOIR5 = (snapshot->bidqty[4] - snapshot->askqty[4]) * 100 / (snapshot->bidqty[4] + snapshot->askqty[4]);

    SOIR = (5 * SOIR1 + 4 * SOIR2 + 3 * SOIR3  + 2 * SOIR4  + SOIR5 ) / (5 + 4 + 3 + 2 + 1);
    
    slog_debug(0, "[%d-%s],时间[%d],最新价[%d],预测价[%d], 1分钟成交金额[%.2f]", snapshot->market, snapshot->securityId, 
    snapshot->updateTime, snapshot->tradePx,
    p, kline1[cursor].amt * 0.0001);

    if(snapshot->tradePx > p || snapshot->tradePx == snapshot->highPx || kline1[cursor].amt < 100000000000LL)
    {
        return;
    }
    
    if(NULL != pHold)
    {
        slog_debug(0, "[%d-%s] 可买数量[%d], 持仓数量[%d]", snapshot->market, snapshot->securityId, XGetBuyedQty(pStrategy, pStock->lmtBuyMinQty), pHold->sumHld);
    }
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

    slog_info(0, "<<<<<< [%d-%s]买次数[%d],委托数量[%d], 价格[%d],最新[%d]",
              pStrategy->setting.market, pStrategy->setting.securityId, pStrategy->_slipBuyTimes,
              order.ordQty, order.ordPrice,  snapshot->tradePx);
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
            //XCancelSell(pStrategy, 0, 0); //如果发起撤单,重置
        }
        else if (NULL == pHold || pHold->sumHld == 0)
        {
            pStrategy->_slipSellTimes = 0;
            MarketBuy(pStrategy, &ticksnap, pStock, pHold);
            //XCancelBuy(pStrategy, 0, 0);
        }    
    }
}

int main(int argc, char *argv[])
{
    XMonitorT *pMonitor = NULL;
    XMonitorMdT *pMonitorMd = NULL;

    XManShmLoad();
    xslog_init(XSHM_SDB_FILE, "xfundt0");
    slog_info(0, "xfundt0 启动......");

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

    slog_info(3, "xfundt0 关闭");

    return (0);
}