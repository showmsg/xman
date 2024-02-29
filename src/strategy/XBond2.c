/*
 * XBond2.c
 *
 *  Created on: 2024年1月5日
 *      Author: Administrator
 */

#include "XBond.h"
#include "XLog.h"
#include "XTimes.h"
#include "XBus.h"

static XBondTradeParamT g_bondTradeParam;

static void Trade(XIdx totalPlot) {
	XIdx i;
	XRSnapshotT *pSnapshot = NULL, ticksnap = { 0 };
	XStrategyT *pStrategy = NULL;
//	XStockT *pStock = NULL;

	for (i = 0; i < totalPlot; i++) {
		pStrategy = XFndVStrategyById(i + 1);
		if (NULL == pStrategy || pStrategy->status != eXPlotNormal
				|| pStrategy->plot.plotType != eXBondT0Plot) {
			continue;
		}

		// 找到快照位置
		pSnapshot = XFndVRSnapshotByKey(pStrategy->setting.market,
				pStrategy->setting.securityId);
		if (NULL == pSnapshot) {
			continue;
		}

		// 跳过9:30分委托
		if (pSnapshot->updateTime < 93100000) {
			continue;
		}

		memcpy(&ticksnap, pSnapshot, XRSNAPSHOT_SIZE);

		BondTrade(&ticksnap, pStrategy, &g_bondTradeParam);
	}
}

int main(int argc, char *argv[]) {
	XMonitorT *pMonitor = NULL;
	XMonitorMdT *pMonitorMd = NULL;

	memset(&g_bondTradeParam, 0, sizeof(XBondTradeParamT));
	g_bondTradeParam.buyGapPrice = 300;
	g_bondTradeParam.sellGapPrice = 700;
	g_bondTradeParam.buyQtyLimit = 100;
	g_bondTradeParam.buyQtyRatio = 7000;
	g_bondTradeParam.latencyShTradeTime = 30;
	g_bondTradeParam.latencySzTradeTime = 3;
	g_bondTradeParam.openZdf = 0;
	g_bondTradeParam.price = 2500000;
	g_bondTradeParam.sellQtyRatio = 7000;
	g_bondTradeParam.zdf = -250;
	g_bondTradeParam.zfRatio = 300;
	g_bondTradeParam.buySoir = 28;
	g_bondTradeParam.sellSoir = -20;

	xslog_init(XSHM_SDB_FILE, "xbond");
	XManShmLoad();

	slog_debug(0, "xbond版本[%s]", __XMAN_VERSION__);

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return (-1);
	}
	pMonitorMd = XFndVMdMonitor(eXExchSec);

	slog_info(0, "xbond启动中......");
	for (;;) {
		if (NULL == pMonitorMd) {
			continue;
		}

		Trade(pMonitor->iTPlot);
	}

	slog_info(3, "xbond 关闭");

	return (0);
}

