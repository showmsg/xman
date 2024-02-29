#include "XTimes.h"
#include "XUtils.h"
#include "XLog.h"
#include "XBus.h"
#include "FastRebuild.h"

static XMonitorMdT *l_pMonitor = NULL;

XVoid XSnapshot(XVoid* params)
{
    XL2LT *l2l = NULL;
	XTickTradeT trade;
	XTickOrderT order;
	XSnapshotBaseT snapshot;
	XInt readId = -1;
	XBindParamT *pBind = NULL;
	XInt iret = 0;
	
	slog_info(0, "XSnapshot启动......");
	pBind = (XBindParamT *)params;
	if (NULL != pBind)
	{
		iret = XBindCPU(pBind->cpuid);
		if(iret)
		{
			slog_warn(0, "绑核失败[%d]", pBind->cpuid);
		}
	}

	l_pMonitor = XFndVMdMonitor(eXExchSec);

	if (NULL == l_pMonitor)
	{
		slog_error(0, "获取行情信息错误");
		return;
	}

	if (XIsNullCache(XSHMKEYCONECT(mktCache)))
	{
		slog_error(0, "获取行情缓存失败");
		return;
	}

	readId = XGetReadCache(XSHMKEYCONECT(mktCache));
	slog_info(3, "开始重构订单薄行情[%d]......", readId);

	for (;;)
	{
		l2l = (XL2LT *)XPopCache(XSHMKEYCONECT(mktCache), readId);
		if (NULL == l2l)
		{
			continue;
		}

		switch (l2l->head.type)
		{
		case eMSnapshot:
			snapshot = l2l->snapshot;

			if (snapshot.updateTime > __atomic_load_n(&l_pMonitor->updateTime, __ATOMIC_RELAXED))
			{
				__atomic_store_n(&l_pMonitor->updateTime, snapshot.updateTime, __ATOMIC_RELAXED);
				
				if(__atomic_load_n(&l_pMonitor->traday, __ATOMIC_RELAXED) == 0)
				{
					__atomic_store_n(&l_pMonitor->traday, order.traday, __ATOMIC_RELAXED);
				}
			}
			
			if(__atomic_load_n(&l_pMonitor->_locFirstTime, __ATOMIC_RELAXED) == 0)
			{
				__atomic_store_n(&l_pMonitor->_locFirstTime, XGetClockTime(), __ATOMIC_RELAXED);
				__atomic_store_n(&l_pMonitor->_mktFirstTime, snapshot.updateTime, __ATOMIC_RELAXED);
			}

			OnOrgSnapshot(&snapshot);
			break;
		default:
			break;
		}
	}
	XReleaseCache(XSHMKEYCONECT(mktCache), readId);
	exit(0);
}