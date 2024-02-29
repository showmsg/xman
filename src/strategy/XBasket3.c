/*
 * @file XBasket2.c
 * @brief 盘中抢单程序
 * @author kangyq
 * @version 2.0.0
 * @date 2022-12-16
 * @version 20230525  调整均线计算,不包括当前未完结的最新行情
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */

#include "XBasket.h"
#include "XBus.h"
#include "XCom.h"
#include "XLog.h"
#include "XTimes.h"

static XMonitorT *pMonitor = NULL;
static XMonitorMdT *pMonitorMd = NULL;

static void
Trade (XSnapshotNotifyT *notify)
{
  XIdx i;
  XStrategyT *pStrategy = NULL;
  XStockT *pStock = NULL;
  XRSnapshotT *snapshot = NULL;

  for (i = 0; i < pMonitor->iTPlot; i++)
    {
      pStrategy = XFndVStrategyById (i + 1);
      if (NULL == pStrategy || pStrategy->status != eXPlotNormal || pStrategy->plot.plotType != eXMidRobPlot)
	{
	  continue;
	}

      if (pStrategy->setting.market != notify->market || 0 != strcmp (pStrategy->setting.securityId, notify->securityId))
	{
	  continue;
	}

      // 找到最新的行情
      snapshot = XFndVRSnapshotById (notify->idx);
      if (NULL == snapshot)
	{
	  continue;
	}
      pStock = XFndVStockByKey (pStrategy->setting.market, pStrategy->setting.securityId);
      if (NULL == pStock)
	{
	  continue;
	}
      BasketTrade (snapshot, pStrategy, pStock);
    }
}

int
main (int argc, char *argv[])
{

  xslog_init (XSHM_SDB_FILE, "xbsket3");
  XManShmLoad ();

  slog_debug(0, "xrob版本[%s]", __XMAN_VERSION__);

  pMonitor = XFndVMonitor ();
  if (NULL == pMonitor)
    {
      return (-1);
    }
  pMonitorMd = XFndVMdMonitor (eXExchSec);

  slog_info(0, "xrob 启动中......");

  SetCallback ("xrob", Trade);

  slog_info(3, "xrob 关闭");

  return (0);
}
