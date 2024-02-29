/**
 * @author your name (you@domain.com)
 * @brief
 * 网格交易策略
 * @version 0.1
 * @date 2022-12-16
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "XCommon.h"
#include "XTimes.h"
#include "XBus.h"


static void Trade(XMonitorT *pMonitor, XMonitorMdT *pMonitorMd)
{
    XIdx i;
    XSnapshotT *pSnapshot = NULL, ticksnap = {0};
    XStrategyT *pStrategy = NULL;
    XStockT *pStock = NULL;
    XHoldT *pHold = NULL;
    XInt zdf = 0;

    for (i = 0; i < pMonitor->iTPlot; i++)
    {
        pStrategy = XFndVStrategyById(i + 1);
        if (NULL == pStrategy || pStrategy->status != eXPlotNormal || pStrategy->plotType != eXBondAvg)
        {
            continue;
        }

        // 找到快照位置
        pSnapshot = XFndVSnapshotByKey(pStrategy->market,
                                        pStrategy->securityId);
        if (NULL == pSnapshot)
        {
            continue;
        }
        memcpy(&ticksnap, pSnapshot, sizeof(XSnapshotT));

        // 不在时间范围的策略不运行
        if ((pMonitorMd->updateTime < pStrategy->beginTime && 0 != pStrategy->beginTime) || (pMonitorMd->updateTime > pStrategy->endTime && 0 != pStrategy->endTime))
        {
            continue;
        }

        //TODO 收盘清仓
        
        pStock = XFndVStockByKey(pStrategy->market, pStrategy->securityId);
        if (NULL == pStock)
        {
            continue;
        }

        //1、启动点，如果有持仓起点不买，如果无持仓到达启动时间则买入对应数量；
        //2、如果行情趋势往上，在低节点有成交持仓，达到下一节点则卖出；
        //3、如果行情趋势往下，在高节点有持仓卖出，达到下一节点无成交买入则买入；在高节点有持仓买入，达到下一节点一般则卖出；
        //4、根据初始买入点及当前行情点，确定档位
        
    }
}

int main(int argc, char *argv[])
{
    XMonitorT *pMonitor = NULL;
    XMonitorMdT *pMonitorMd = NULL;

    XManShmLoad();
    xslog_init(XSHM_SDB_FILE, "xbondavg");
    slog_info(3, "xbondavg 启动......");

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