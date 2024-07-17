/*
 * @file Store.c
 * @brief 行情存储
 * @version 2.0.0
 * @date 2022-12-16
 *
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */

#include<fcntl.h>
#include "XStore.h"
#include "XExport.h"
#include "XLog.h"
#include "XTimes.h"
#include "XUtils.h"
#include "XSignals.h"

XVoid XStoreShMkt()
{
	XL2LT *l2l = NULL;
	XMonitorMdT *pMonitorMd = NULL;
	XInt readId = -1;
	int fd = -1;

	//存储交易所原始行情,追加模式
	fd = open(XMAN_DATA_MKTSTOREBIN, O_WRONLY | O_APPEND | O_CREAT, 0600);

	pMonitorMd = XFndVMdMonitor(eXExchSec);

	readId = XGetReadCache(XSHMKEYCONECT(mktCache));

	while (NULL != pMonitorMd && !isMktClosedTime(pMonitorMd->updateTime) && !XIsNullCache(XSHMKEYCONECT(mktCache)))
	{
		l2l = (XL2LT*) XPopCache(XSHMKEYCONECT(mktCache), readId);
		if (NULL == l2l)
		{
			continue;
		}
		write(fd, l2l, sizeof(XL2LT));

	}
	close(fd);

	exit(0);
}

XVoid XStoreTick(XVoid *params)
{
	XBindParamT *pBind = NULL;
	XInt iret;

	pthread_t sh;

	slog_info(0, "XSTick启动[存储交易所原始行情]......");

	pBind = (XBindParamT*) params;
	if (NULL != pBind)
	{
		iret = XBindCPU(pBind->cpuid);
		if (iret)
		{
			slog_warn(0, "绑核失败[%d]", pBind->cpuid);
		}
	}

	pthread_create(&sh, NULL, (void* (*)(void*)) &XStoreShMkt, NULL);
//  pthread_create (&sz, NULL, (void *(*)(void *)) & XStoreSzMkt, NULL);
	pthread_join(sh, NULL);
//  pthread_join (sz, NULL);

}

static int comp_fn_zdf(const void *p1, const void *p2)
{
	return (((XBlockT*) p1)->zdf > ((XBlockT*) p2)->zdf ? 0 : 1);
}

XVoid XCalcBlock(XVoid *params)
{

	// 根据行情每1秒计算一次盘口板块数据
	XMonitorMdT *pMonitorMd = NULL;
	XShortTime curTime = 0;
	XMonitorT *pMonitor = NULL;
	XIdx i = 0, m = 0;
	XNum j = 0, count = 0;
	XBlockT *pBlock = NULL;
	XRatio zdf = 0, zdfSum = 0;
	XBlockInfoT *pBlockInfo = NULL;
	XRSnapshotT *pSnapshot = NULL, snapshot;
	XIdx idx = -1;

	XBlockT blocks[1024];

	pMonitorMd = XFndVMdMonitor(eXExchSec);
	pMonitor = XFndVMonitorS(__FILE__, __LINE__);

	slog_info(0, "XCalcIdx 开始计算板块涨跌......");
	while (NULL != pMonitorMd && !isMktClosedTime(pMonitorMd->updateTime))
	{
		if (XMsC2S(pMonitorMd->updateTime) < XMsC2S(curTime) + 10000)
		{
			continue;
		}

		curTime = pMonitorMd->updateTime;

		memset(&blocks, 0, sizeof(blocks));

		for (i = 0; i < pMonitor->iTBlock; i++)
		{
			pBlock = XFndVBlockById(i + 1);
			if (NULL == pBlock)
			{
				continue;
			}
			zdf = -500;
			count = 0;
			zdfSum = 0;
			idx = -1;
			for (j = 0; j < pBlock->count; j++)
			{
				pBlockInfo = XFndVBlockInfoById(pBlock->beginIdx + j);
				if (NULL == pBlockInfo || strcmp(pBlock->blockNo, pBlockInfo->blockNo) != 0)
				{
					continue;
				}

				pSnapshot = XFndVRSnapshotByKey(pBlockInfo->market, pBlockInfo->securityId);
				if (NULL == pSnapshot || !pSnapshot->tradePx || !pSnapshot->preClosePx)
				{
					continue;
				}
				memcpy(&snapshot, pSnapshot, sizeof(XRSnapshotT));

				pBlockInfo->zdf = (snapshot.tradePx - snapshot.preClosePx) * 10000 / snapshot.preClosePx;
				pBlockInfo->updateTime = snapshot.updateTime;
				// 计算涨跌幅
				zdfSum += pBlockInfo->zdf;
				if (pBlockInfo->zdf > zdf)
				{
					zdf = pBlockInfo->zdf;
					idx = snapshot.idx;
				}

				count++;
			}

			//找到排名前5的股票

			if (count)
			{
				pBlock->zdf = zdfSum / count;
				pBlock->secIdx = idx;
				memcpy(&blocks[i], pBlock, sizeof(XBlockT));
			}
		}
		qsort(blocks, i, sizeof(XBlockT), comp_fn_zdf);
		// 取排名前几的板块，板块里有1个涨停，且板块涨跌幅>1%，买入板块涨幅排名靠前且未涨停的股票

		for (m = 0; m < 10; m++)
		{
			pSnapshot = XFndVRSnapshotById(blocks[m].secIdx);
			if (NULL == pSnapshot)
			{
				continue;
			}

			zdf = (pSnapshot->tradePx - pSnapshot->preClosePx) * 10000 / pSnapshot->preClosePx;
			//把板块计算靠前的放到MonitorMd中
			slog_debug(0, "%d,时间:%d, [%s-%s], 涨跌幅[%.2f], 领涨股票[%s.%s],涨跌幅[%.2f]", m + 1, pMonitorMd->updateTime, blocks[m].blockNo, blocks[m].blockName, blocks[m].zdf * 0.01, pSnapshot->securityId, (
					pSnapshot->market == eXMarketSha ? "SH" : "SZ"), zdf * 0.01);
			pMonitorMd->blockTopIdx[m] = blocks[m].idx;
		}
		slog_debug(0, "---------------------------------------------------------------\n");

		sched_yield();
	}
}

XVoid XStore(XVoid *params)
{

	XInt fd;
	XInt readId = -1;
	XMonitorMdT *pMonitorMd = NULL;
	XRSnapshotT *pSnapshot = NULL, snapshot;
//	XBindParamT *pBind = NULL;
#ifndef __XMAN_FAST_REBUILD__
	XRSnapshotT *pFndSnap = NULL;
	XInt iCount;
	XBool bUpdate = false;
#endif

	slog_info(0, "XStoreRSnap启动......");

	/**
	 pBind = (XBindParamT*) params;
	 if (NULL != pBind) {
	 iret = XBindCPU(pBind->cpuid);
	 if (iret) {
	 slog_warn(0, "绑核失败[%d]", pBind->cpuid);
	 }
	 }
	 */
	fd = open(XMAN_DATA_TSNAPBIN, O_WRONLY | O_APPEND | O_CREAT, 0600);

	pMonitorMd = XFndVMdMonitor(eXExchSec);

	readId = XGetReadCache(XSHMKEYCONECT(reSnapCache));

	while (NULL != pMonitorMd && !isMktClosedTime(pMonitorMd->updateTime) && !XIsNullCache(XSHMKEYCONECT(reSnapCache)))
	{

		pSnapshot = (XRSnapshotT*) XPopCache(XSHMKEYCONECT(reSnapCache), readId);
		if (NULL == pSnapshot)
		{
			continue;
		}

		memcpy(&snapshot, pSnapshot, XRSNAPSHOT_SIZE);
#ifndef __XMAN_FAST_REBUILD__
		bUpdate = false;
		pFndSnap = XFndVRSnapshotById(snapshot.idx);

		//首笔或者收盘后的行情都保存
		if (NULL == pFndSnap)
		{
			bUpdate = true;
		}
		//缓存如果不够大,放入数据大于写会造成重复写
		else if (snapshot.version < pFndSnap->version)
		{
			bUpdate = false;
		}

		// 如果成交笔数变化
		else if (snapshot.volumeTrade != pFndSnap->volumeTrade)
		{
			bUpdate = true;
		}
		/**
		 //没有成交变化的时候,1秒生成1笔快照
		 else if (XMsC2S(snapshot.updateTime)
		 < XMsC2S(pFndSnap->updateTime) + 1000) {
		 bUpdate = false;
		 }
		 */
		// 如果5档价格或数量变化
		else
		{
			for (iCount = 0; iCount < 10; iCount++)
			{
				if (snapshot.bid[iCount] != pFndSnap->bid[iCount] || snapshot.ask[iCount] != pFndSnap->ask[iCount])
				{
					bUpdate = true;
					break;
				}
				if (snapshot.bidqty[iCount] != pFndSnap->bidqty[iCount] || snapshot.askqty[iCount] != pFndSnap->askqty[iCount])
				{
					bUpdate = true;
					break;
				}
			}
		}
		if (bUpdate)
		{
			XPutOrUpdVRSnapshot(&snapshot);
#ifdef __STORE_RESNAP__
          write (fd, &snapshot, XRSNAPSHOT_SIZE);
#endif
		}
#else
      write (fd, &snapshot, XRSNAPSHOT_SIZE);
#endif
		sched_yield();
	}
	close(fd);
}

XVoid XCalcMkt(XVoid *params)
{
	XMonitorT *pMonitor = NULL;
	XIdx i = 0;
	XRSnapshotT *pSnapshot = NULL;
	XNum iUpCnt = 0, iLowCnt = 0, iEqualCnt = 0, iUpperCnt = 0, iLowerCnt = 0, iUpOpenCnt = 0, iLowOpenCnt = 0;
	XStockT *pStock = NULL;
	XMonitorMdT *pMonitorMd = NULL;

	pMonitor = XFndVMonitorS(__FILE__, __LINE__);
	if (NULL == pMonitor)
	{
		return;
	}
	pMonitorMd = XFndVMdMonitor(eXExchSec);
	slog_info(0, "计算大盘涨跌......");

	while (1)
	{

		iUpCnt = 0;
		iLowCnt = 0;
		iEqualCnt = 0;
		iUpperCnt = 0;
		iLowerCnt = 0;
		iUpOpenCnt = 0;
		iLowOpenCnt = 0;
		for (i = 0; i < pMonitor->iTOrderBook; i++)
		{
			pSnapshot = XFndVRSnapshotById(i + 1);
			if (NULL == pSnapshot)
			{
				continue;
			}

			//找股票数据
			pStock = XFndVStockByKey(pSnapshot->market, pSnapshot->securityId);
			if (NULL == pStock || eXStock != pStock->secType)
			{
				continue;
			}
			if (pSnapshot->tradePx > pSnapshot->preClosePx)
			{
				iUpCnt++;
			}
			else if (0 != pSnapshot->tradePx && pSnapshot->tradePx < pSnapshot->preClosePx)
			{
				iLowCnt++;
			}
			else
			{
				iEqualCnt++;
			}

			if (pSnapshot->lowPx == pSnapshot->upperPx)
			{
				iUpOpenCnt++;
			}

			//涨停
			if (pSnapshot->tradePx == pSnapshot->upperPx)
			{
				iUpperCnt++;

			}
			//跌停
			else if (pSnapshot->tradePx == pSnapshot->lowerPx)
			{
				iLowerCnt++;
			}

			if (pSnapshot->highPx == pSnapshot->lowerPx)
			{
				iLowOpenCnt++;
			}
		}

		//更新大盘情况
		pMonitorMd->uppCnt = iUpCnt;
		pMonitorMd->lowCnt = iLowCnt;
		pMonitorMd->eqlCnt = iEqualCnt;

		pMonitorMd->upperCnt = iUpperCnt;
		pMonitorMd->lowerCnt = iLowerCnt;
		pMonitorMd->uperOpenCnt = iUpOpenCnt;
		pMonitorMd->lowerOpenCnt = iLowOpenCnt;

		sleep(1);
		sched_yield();
	}
}

XVoid XDealSignal(XVoid *params)
{
	XInt readId;
	XKLineT *kline1 = NULL;
	XSignalT notify;
	XTradeCache webCache;
	XInt cursor = -1;
	XInt zs = -1;          //涨速

	slog_info(0, "信号触发......");
	readId = XGetReadCache(XSHMKEYCONECT(reSnapCache));

	while (1)
	{
		XRSnapshotT *snapshot = (XRSnapshotT*) XPopCache(XSHMKEYCONECT(reSnapCache), readId);
		if (NULL == snapshot)
		{
			continue;
		}

		//拿到K线数据

		kline1 = GetKlinesByBlock(snapshot->idx, 0);
		cursor = (SNAPSHOT_K1_CNT + snapshot->kcursor1 - 2) & (SNAPSHOT_K1_CNT - 1);

		if (snapshot->updateTime >= 93100000 && kline1[cursor].close > 0)
		{
			zs = (snapshot->tradePx - kline1[cursor].close) * 10000 / kline1[cursor].close;
			//涨速大于100
			if (zs > 100)
			{
				memset(&notify, 0, sizeof(XSignalT));
				notify.traday = snapshot->traday;
				notify.market = snapshot->market;
				notify.updateTime = snapshot->updateTime;
				memcpy(notify.securityId, snapshot->securityId, strlen(snapshot->securityId));
				notify.bsType = eXBuy;
				notify.idx = snapshot->idx;
				notify.signalType = eXSgnlPosZs;
				notify.preClosePx = snapshot->preClosePx;
				notify.tradePx = snapshot->tradePx;
				notify.upperPx = snapshot->upperPx;
				notify.lowerPx = snapshot->lowerPx;
				notify.openPx = snapshot->openPx;
				notify.highPx = snapshot->highPx;
				notify.lowPx = snapshot->lowPx;
				notify.kcursor1 = snapshot->kcursor1;
				notify.kcursor5 = snapshot->kcursor5;
				notify._bizIndex = snapshot->_bizIndex;
				notify._recvTime = snapshot->_recvTime;
				notify.upperOfferOrdQty = snapshot->upperOfferOrdQty;
				notify.version = snapshot->version;
				memset(&webCache, 0, sizeof(XTradeCache));
				webCache.head.type = eSignal;
				webCache.head.dataLen = sizeof(XSignalT);
				webCache.signal = notify;
				//推送信号
				XPushCache(XSHMKEYCONECT(rtnCache), &webCache);
				slog_debug(0, "[%d-%s], 时间[%d], 涨速[%d],成交金额[%.2f]", snapshot->market, snapshot->securityId, snapshot->updateTime, zs, kline1[cursor].amt * 0.00000001);
			}
		}
	}
	XReleaseCache(XSHMKEYCONECT(reSnapCache), readId);
}

XVoid XStoreSnap(XVoid *params)
{
	pthread_t calpth;          // 板块计算
#ifndef __XMAN_FAST_REBUILD__
	pthread_t storepth;          // 数据存储
#endif
	pthread_t mktpth;          //大盘数据更新
	pthread_t signalpth;          //大盘数据更新
// 板块计算线程

	pthread_create(&calpth, NULL, (void* (*)(void*)) &XCalcBlock, NULL);
	pthread_create(&mktpth, NULL, (void* (*)(void*)) &XCalcMkt, NULL);
	pthread_create(&signalpth, NULL, (void* (*)(void*)) &XDealSignal, NULL);

#ifndef __XMAN_FAST_REBUILD__
	pthread_create(&storepth, NULL, (void* (*)(void*)) &XStore, NULL);
	pthread_join(storepth, NULL);
#endif
	pthread_join(calpth, NULL);
	pthread_join(mktpth, NULL);
	pthread_join(signalpth, NULL);

}
