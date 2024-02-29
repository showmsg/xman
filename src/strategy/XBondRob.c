/*
 * @file XBondRob.c
 * @brief 可转债盘前或收盘抢板
 * @author kangyq
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */

#include "XCommon.h"
#include "XTimes.h"
#include "XBus.h"
#include "XLog.h"

XNum XGetBuyPos(XStrategyT *pStrategy)
{
    XOrderT *pOrder = NULL;
    XNum i = 0;
    XNum idx = -1;

    for (i = 0; i < 2; i++)
    {
        if (0 == pStrategy->buyList[i])
        {
            idx = i;
            break;
        }
        else
        {
            pOrder = XFndVOrderByLoc(pStrategy->plot.customerId, pStrategy->envno, pStrategy->buyList[i]);
            if (NULL != pOrder)
            {
                //全部成交，部分撤单，全部撤单，非法订单
                if (pOrder->ordStatus == eXOrdStatusInvalid)
                {
                    pStrategy->buyList[i] = 0;
                    idx = i;
                    break;
                }
            }
        }
    }

    return (idx);
}

XBool XGetValidPos(XStrategyT *pStrategy)
{
    XOrderT *pOrder = NULL;
    XNum i = 0;
    XBool bvalid = true; //默认允许报单

    for (i = 0; i < 2; i++)
    {
        //如果有挂单且挂单是废单时,允许报单，否则不允许报单
        if (0 != pStrategy->buyList[i])
        {
            bvalid = false;

            //特殊订单为废单时允许挂单
            pOrder = XFndVOrderByLoc(pStrategy->plot.customerId, pStrategy->envno, pStrategy->buyList[i]);
            if (NULL != pOrder && pOrder->ordStatus == eXOrdStatusInvalid)
            {
                //全部成交，部分撤单，全部撤单，非法订单           
                bvalid = true;        
            }
            break;
        }      
    }

    return (bvalid);
}

static XBool BasketBuy(XStrategyT *pStrategy, XStockT *pStock, XInt updateTime)
{
    XNum idx = 0;
    XOrderReqT order = {0};
    XLocalId localid = 0;

    idx = XGetBuyPos(pStrategy);
    if (idx < 0)
    {
        return (false);
    }

    CPStrategy2Ord(pStrategy, &order);

    order.bsType = eXBuy;
    order.ordPrice = pStrategy->setting.ordPx;
    order.ordQty = pStrategy->setting.ordQty;
    order.ordType = eXOrdLimit;

    localid = XPutOrderReq(&order);
    pStrategy->buyList[idx] = localid;
    pStrategy->_buySends += order.ordQty;
    pStrategy->_lastBuyPx = order.ordPrice;
    pStrategy->_buyLocTime = XGetClockTime();

    slog_info(0, "5.[%d-%s-%lld], 委托本地编号[%d], 委托时间[%d]", pStrategy->setting.market, pStrategy->setting.securityId, pStrategy->plotid, localid, updateTime);

    return (true);
}

static void Trade(XMonitorT *pMonitor, XMonitorMdT *pMonitorMd)
{
    XIdx i;
    XSnapshotT *pExSnapshot = NULL, exsnap = {0};
    XStrategyT *pStrategy = NULL;
    XStockT *pStock = NULL;
    XBool bSendOk = false;
    XLongTime lCurTime = 0;
    XShortTime iCurTIme = 0;

    for (i = 0; i < pMonitor->iTPlot; i++)
    {
        pStrategy = XFndVStrategyById(i + 1);
        if (NULL == pStrategy || pStrategy->status != eXPlotNormal || pStrategy->plot.plotType != eXMidRob)
        {
            continue;
        }
   
        //当前时间>9:14:57 && <9:15:57
        pStock = XFndVStockByKey(pStrategy->setting.market, pStrategy->setting.securityId);
        if (NULL == pStock)
        {
            continue;
        }

        /** 开盘前30ms没有报上去连续报 */
        else if (pStrategy->setting.sign == 0)
        {

             /** 在9:19:30看当前行情进行撤单 */
            if(pMonitorMd->updateTime >= pStrategy->plot.endTime)
            {
                pExSnapshot = XFndVSnapshotByKey(pStrategy->setting.market, pStrategy->setting.securityId);
                if (NULL == pExSnapshot)
                {
                    continue;
                }
                memcpy(&exsnap, pExSnapshot, XSNAPSHOT_SIZE);

                //只要在9:19:57之后，卖一有数量就撤单
                if(pMonitorMd->updateTime < 93000000)
                {
                    if(exsnap.askqty[1] != 0 || exsnap.bidqty[0] < pStrategy->setting.askQty /* 封单数量 */)
                    {
                        XCancelBuy(pStrategy, 1, 1);
                    }
                } 
                else
                {
                    if(exsnap.askqty[0] != 0 || exsnap.bid[0] != pStrategy->setting.conPx || 
                    (exsnap.bid[0] == pStrategy->setting.conPx && exsnap.bidqty[0] - exsnap.askqty[0] < pStrategy->setting.askQty))
                    {
                        XCancelBuy(pStrategy, 1, 1);
                    }
                }                
            }

            /** 用本地时间进行早盘抢单 */

            lCurTime = XGetClockTime();
            iCurTIme = XNsTime2I(lCurTime);

            if (iCurTIme >= pStrategy->plot.beginTime && iCurTIme <= pStrategy->plot.beginTime + 60 * 1000 && 
              eXBuy == pStrategy->setting.bsType)
            {
                bSendOk = XGetValidPos(pStrategy);
                //检查订单是否委托成功
                if (bSendOk)
                {
                    BasketBuy(pStrategy, pStock, pMonitorMd->updateTime);                       
                }
            }
        }
        //收盘集合竞价抢单,用行情的时间进行下单
        else if(pStrategy->setting.sign == 1)
        {
            if(pMonitorMd->updateTime > pStrategy->plot.beginTime && pMonitorMd->updateTime < pStrategy->plot.endTime && eXBuy == pStrategy->setting.bsType)
            {
                bSendOk = XGetValidPos(pStrategy);
                //检查订单是否委托成功
                if (bSendOk)
                {
                    BasketBuy(pStrategy, pStock, pMonitorMd->updateTime);                       
                }
            }
        }
        
    }
}

int main(int argc, char *argv[])
{
    XMonitorT *pMonitor = NULL;
    XMonitorMdT *pMonitorMd = NULL;

    xslog_init(XSHM_SDB_FILE, "xrob");
    XManShmLoad();

    slog_debug(0, "xbasket版本[%s]", __XMAN_VERSION__);

    pMonitor = XFndVMonitor();
    if (NULL == pMonitor)
    {
        return (-1);
    }
    pMonitorMd = XFndVMdMonitor(eXExchSec);

    for (;;)
    {
        if (isMktClosedTime(pMonitorMd->updateTime))
        {
            break;
        }
        Trade(pMonitor, pMonitorMd);
    }

    slog_info(3, "xbasket 关闭");

    return (0);
}
