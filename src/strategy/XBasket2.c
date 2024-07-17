
#include "XBasket.h"
#include "XLog.h"
#include "XTimes.h"
#include "XBus.h"

static void
Trade (XMonitorT *pMonitor, XMonitorMdT *pMonitorMd)
{
  XIdx i;
  XRSnapshotT *pSnapshot = NULL, ticksnap = { 0 };
  XStrategyT *pStrategy = NULL;
  XStockT *pStock = NULL;
  

  for (i = 0; i < pMonitor->iTPlot; i++)
    {
      pStrategy = XFndVStrategyById (i + 1);
      if (NULL == pStrategy || pStrategy->status != eXPlotNormal
          || pStrategy->plot.plotType != eXMidRobPlot)
        {
          continue;
        }

      // 找到快照位置
      pSnapshot = XFndVRSnapshotByKey (pStrategy->setting.market,
                                       pStrategy->setting.securityId);
      if (NULL == pSnapshot)
        {
          continue;
        }
      
      memcpy (&ticksnap, pSnapshot, XRSNAPSHOT_SIZE);

      pStock = XFndVStockByKey (pStrategy->setting.market,
                                pStrategy->setting.securityId);
      if (NULL == pStock || eXSecPause == pStock->secStatus || eXSecTmpPause == pStock->secStatus)
        {
          continue;
        }

      BasketTrade(&ticksnap, pStrategy, pStock);
    }
}

int
main (int argc, char *argv[])
{
  XMonitorT *pMonitor = NULL;
  XMonitorMdT *pMonitorMd = NULL;

  xslog_init (XSHM_SDB_FILE, "xbsket");
  XManShmLoad ();

  slog_debug (0, "xbasket版本[%s]", __XMAN_VERSION__);

  pMonitor = XFndVMonitor ();
  if (NULL == pMonitor)
    {
      return (-1);
    }
  pMonitorMd = XFndVMdMonitor (eXExchSec);

  XPubVAppByName("xbasket");
  slog_info (0, "xbasket启动中......");
  for (;;)
    {
      if (NULL == pMonitorMd)
        {
          continue;
        }
    
      Trade (pMonitor, pMonitorMd);
    }

  slog_info (3, "xbasket 关闭");

  return (0);
}
