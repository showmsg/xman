/*
 * @file OesMkt.c
 * @brief 柜台行情转换接口
 * @version 2.0.0
 * @date 2022-12-16
 *
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */

#include <getopt.h>
#include <sys/sysinfo.h>

#include <ctype.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "OesMkt.h"
#include "XCom.h"
#include "XLog.h"
#include "XTimes.h"
#include "XUtils.h"
#include "mds_api/mds_api.h"
#include "sutil/time/spk_times.h"

static XMonitorMdT *l_MonitorMd = NULL;

static XNum isBindCpu = 0;

// 行情中证券代码的长度
#define MKT_SECURITYID_LEN 9

typedef struct
{
	MdsApiSessionInfoT session;
	XBool level;
} SessionWithLevelT;

static int32 _OnOrder(const MdsL2OrderT *pMdsL2Order)
{
	XL2LT l2lData;
	XTickOrderT order;

	memset(&l2lData, 0, XL2L_SIZE);
	memset(&order, 0, XTICKORDER_SIZE);
	l2lData.head.type = eMTickOrder;
	l2lData.head.dataLen = XL2L_SIZE;

	order.market = market_from_mds(pMdsL2Order->exchId);

	memcpy(order.securityId, pMdsL2Order->SecurityID, MKT_SECURITYID_LEN);
	order.traday = pMdsL2Order->tradeDate;
	order.updateTime = pMdsL2Order->TransactTime;
	order.channel = pMdsL2Order->ChannelNo;

	order.ordSeq = pMdsL2Order->ApplSeqNum;

	order.bsType = bs_from_mds(pMdsL2Order->Side);
	order.isCancel = cancel_from_order(pMdsL2Order->exchId, pMdsL2Order->OrderType);
	order.ordType = ordtype_from_mds(pMdsL2Order->OrderType);

	order.ordPx = pMdsL2Order->Price;
	order.ordQty = pMdsL2Order->OrderQty;

#ifdef __LATENCY__
  order._recvTime = XGetClockTime ();
#endif

	l2lData.head.market = order.market;
	if (order.market == eXMarketSza)
	{
		order.bizIndex = pMdsL2Order->ApplSeqNum;
		order.seqno = pMdsL2Order->ApplSeqNum;
		l2lData.order = order;
		// XPushCache (XSHMKEYCONECT (mktCacheSz), &l2lData);
	}
	else
	{
		order.bizIndex = pMdsL2Order->SseBizIndex;
		order.seqno = pMdsL2Order->SseOrderNo;
		l2lData.order = order;

	}
	XPushCache(XSHMKEYCONECT(mktCache), &l2lData);
	return (0);
}

static int32 _OnTrade(const MdsL2TradeT *pMdsL2Trade)
{
	XL2LT l2lData;
	XTickTradeT trade;

	memset(&l2lData, 0, XL2L_SIZE);
	memset(&trade, 0, XTICKTRADE_SIZE);

	l2lData.head.type = eMTickTrade;
	l2lData.head.dataLen = XL2L_SIZE;

	trade.market = market_from_mds(pMdsL2Trade->exchId);
	memcpy(trade.securityId, pMdsL2Trade->SecurityID, MKT_SECURITYID_LEN);
	trade.traday = pMdsL2Trade->tradeDate;
	trade.updateTime = pMdsL2Trade->TransactTime;
	trade.channel = pMdsL2Trade->ChannelNo;

	switch (pMdsL2Trade->TradeBSFlag)
	{
	case MDS_L2_TRADE_BSFLAG_BUY:
		trade.trdType = eXL2DriveBid;
		break;
	case MDS_L2_TRADE_BSFLAG_SELL:
		trade.trdType = eXL2DriveAsk;
		break;
	default:
		trade.trdType = eXL2DriveUnknown;
		break;
	}

	trade.tradeSeq = pMdsL2Trade->ApplSeqNum;

	trade.bidSeq = pMdsL2Trade->BidApplSeqNum;
	trade.askSeq = pMdsL2Trade->OfferApplSeqNum;

	trade.isCancel = cancel_from_trade(pMdsL2Trade->ExecType);

	trade.tradePx = pMdsL2Trade->TradePrice;
	trade.tradeMoney = pMdsL2Trade->TradeMoney;
	trade.tradeQty = pMdsL2Trade->TradeQty;
#ifdef __LATENCY__
  trade._recvTime = XGetClockTime ();
#endif

	l2lData.head.market = trade.market;
	if (trade.market == eXMarketSza)
	{
		trade.bizIndex = pMdsL2Trade->ApplSeqNum;
		l2lData.trade = trade;

//      XPushCache (XSHMKEYCONECT (mktCacheSz), &l2lData);
	}
	else
	{
		trade.bizIndex = pMdsL2Trade->SseBizIndex;
		l2lData.trade = trade;

	}
	XPushCache(XSHMKEYCONECT(mktCache), &l2lData);
	return (0);
}

static int32 _OnL2Snap(const MdsMktDataSnapshotHeadT *pHead, const MdsL2StockSnapshotBodyT *pSnapshotBody)
{
	XL2LT l2lData;
	XSnapshotBaseT snapshot;
	XInt j;

	memset(&snapshot, 0, XSNAPSHOT_BASE_SIZE);
	memset(&l2lData, 0, XL2L_SIZE);
	l2lData.head.type = eMSnapshot;
	l2lData.head.dataLen = XL2L_SIZE;

	// 接收时间
#ifdef __LATENCY__
  snapshot._recvTime = XGetClockTime ();
#endif
	snapshot.market = market_from_mds(pHead->exchId);

	memcpy(snapshot.securityId, pSnapshotBody->SecurityID, MKT_SECURITYID_LEN);

	snapshot.updateTime = pHead->updateTime;
	snapshot.traday = pHead->tradeDate;
	snapshot.openPx = pSnapshotBody->OpenPx;
	snapshot.highPx = pSnapshotBody->HighPx;
	snapshot.lowPx = pSnapshotBody->LowPx;
	snapshot.preClosePx = pSnapshotBody->PrevClosePx;

	snapshot.tradePx = pSnapshotBody->TradePx;
	snapshot.volumeTrade = pSnapshotBody->TotalVolumeTraded;
	snapshot.amountTrade = pSnapshotBody->TotalValueTraded;
	snapshot.numTrades = pSnapshotBody->NumTrades;
	snapshot.IOPV = pSnapshotBody->IOPV;
	// 卖价
	for (j = 9; j >= 0; j--)
	{
		snapshot.ask[j] = pSnapshotBody->OfferLevels[j].Price;
		snapshot.bid[j] = pSnapshotBody->BidLevels[j].Price;
		snapshot.askqty[j] = pSnapshotBody->OfferLevels[j].OrderQty;
		snapshot.bidqty[j] = pSnapshotBody->BidLevels[j].OrderQty;
	}

	snapshot.secStatus = secstatus_from_mds(snapshot.market, pSnapshotBody->TradingPhaseCode);
	snapshot.mrkStatus = mktstatus_from_mds(snapshot.market, pSnapshotBody->TradingPhaseCode);

	l2lData.snapshot = snapshot;
	l2lData.head.market = snapshot.market;

	XPushCache(XSHMKEYCONECT(mktCache), &l2lData);
	return (0);
}

static int32 _OnL1Snap(const MdsMktDataSnapshotHeadT *pHead, const MdsStockSnapshotBodyT *pSnapshotBody)
{
	XL2LT l2lData;
	XSnapshotBaseT snapshot;
	XInt j;

	memset(&snapshot, 0, XSNAPSHOT_BASE_SIZE);
	memset(&l2lData, 0, XL2L_SIZE);
	l2lData.head.type = eMSnapshot;
	l2lData.head.dataLen = XL2L_SIZE;

	// 接收时间
#ifdef __LATENCY__
  snapshot._recvTime = XGetClockTime ();
#endif

	snapshot.market = market_from_mds(pHead->exchId);

	memcpy(snapshot.securityId, pSnapshotBody->SecurityID, MKT_SECURITYID_LEN);

	snapshot.updateTime = pHead->updateTime;
	snapshot.traday = pHead->tradeDate;
	snapshot.openPx = pSnapshotBody->OpenPx;
	snapshot.highPx = pSnapshotBody->HighPx;
	snapshot.lowPx = pSnapshotBody->LowPx;
	snapshot.preClosePx = pSnapshotBody->PrevClosePx;

	snapshot.tradePx = pSnapshotBody->TradePx;
	snapshot.volumeTrade = pSnapshotBody->TotalVolumeTraded;
	snapshot.amountTrade = pSnapshotBody->TotalValueTraded;
	snapshot.numTrades = pSnapshotBody->NumTrades;
	snapshot.IOPV = pSnapshotBody->IOPV;
	// 卖价
	for (j = 4; j >= 0; j--)
	{
		snapshot.ask[j] = pSnapshotBody->OfferLevels[j].Price;
		snapshot.bid[j] = pSnapshotBody->BidLevels[j].Price;
		snapshot.askqty[j] = pSnapshotBody->OfferLevels[j].OrderQty;
		snapshot.bidqty[j] = pSnapshotBody->BidLevels[j].OrderQty;
	}

	snapshot.secStatus = secstatus_from_mds(snapshot.market, pSnapshotBody->TradingPhaseCode);
	snapshot.mrkStatus = mktstatus_from_mds(snapshot.market, pSnapshotBody->TradingPhaseCode);

	l2lData.snapshot = snapshot;
	l2lData.head.market = snapshot.market;

	XPushCache(XSHMKEYCONECT(mktCache), &l2lData);

	return (0);
}

static int32 _OnIndex(const MdsMktDataSnapshotHeadT *pHead, const MdsIndexSnapshotBodyT *pSnapshotBody)
{
	XL2LT l2lData;
	XSnapshotBaseT snapshot;

	memset(&snapshot, 0, XSNAPSHOT_BASE_SIZE);
	memset(&l2lData, 0, XL2L_SIZE);
	l2lData.head.type = eMSnapshot;
	l2lData.head.dataLen = XL2L_SIZE;

	// 接收时间
#ifdef __LATENCY__
  snapshot._recvTime = XGetClockTime ();
#endif

	snapshot.market = market_from_mds(pHead->exchId);

	memcpy(snapshot.securityId, pSnapshotBody->SecurityID, MKT_SECURITYID_LEN);

	snapshot.updateTime = pHead->updateTime;
	snapshot.traday = pHead->tradeDate;
	snapshot.openPx = pSnapshotBody->OpenIdx;
	snapshot.highPx = pSnapshotBody->HighIdx;
	snapshot.lowPx = pSnapshotBody->LowIdx;
	snapshot.preClosePx = pSnapshotBody->PrevCloseIdx;
	snapshot.tradePx = pSnapshotBody->LastIdx;
	snapshot.volumeTrade = pSnapshotBody->TotalVolumeTraded;
	snapshot.amountTrade = pSnapshotBody->TotalValueTraded;
	snapshot.numTrades = pSnapshotBody->NumTrades;

	l2lData.snapshot = snapshot;
	l2lData.head.market = snapshot.market;

	XPushCache(XSHMKEYCONECT(mktCache), &l2lData);

	// slog_debug(0, "[%d-%s]指数行情[%d]", snapshot.market, snapshot.securityId, snapshot.updateTime);
	return (0);
}
/**
 * 进行消息处理的回调函数
 *
 * @param   pSessionInfo    会话信息
 * @param   pMsgHead        消息头
 * @param   pMsgBody        消息体数据
 * @param   pCallbackParams 外部传入的参数
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static int32 MdsApiSample_HandleMsg(MdsApiSessionInfoT *pSessionInfo, SMsgHeadT *pMsgHead, void *pMsgBody, void *pCallbackParams)
{
	MdsMktRspMsgBodyT *pRspMsg = (MdsMktRspMsgBodyT*) pMsgBody;

	/*
	 * 根据消息类型对行情消息进行处理
	 */
	switch (pMsgHead->msgId)
	{
	case MDS_MSGTYPE_L2_ORDER:
	case MDS_MSGTYPE_L2_SSE_ORDER:

		_OnOrder(&(pRspMsg->order));
		/* 处理Level2逐笔成交消息 */
		break;

	case MDS_MSGTYPE_L2_TRADE:
		_OnTrade(&(pRspMsg->trade));

		break;

	case MDS_MSGTYPE_L2_BEST_ORDERS_SNAPSHOT:
	case MDS_MSGTYPE_L2_MARKET_DATA_INCREMENTAL:
	case MDS_MSGTYPE_L2_BEST_ORDERS_INCREMENTAL:
	case MDS_MSGTYPE_L2_MARKET_OVERVIEW:
		/* 处理Level2快照行情消息 */

		break;

	case MDS_MSGTYPE_OPTION_SNAPSHOT_FULL_REFRESH:
		/* 处理Level1快照行情消息 */

		break;
	case MDS_MSGTYPE_MARKET_DATA_SNAPSHOT_FULL_REFRESH:
		/* 处理Level1快照行情消息 */
		_OnL1Snap(&(pRspMsg->mktDataSnapshot.head), &(pRspMsg->mktDataSnapshot.stock));
		break;
	case MDS_MSGTYPE_L2_MARKET_DATA_SNAPSHOT:

		_OnL2Snap(&(pRspMsg->mktDataSnapshot.head), &(pRspMsg->mktDataSnapshot.l2Stock));
		break;

	case MDS_MSGTYPE_INDEX_SNAPSHOT_FULL_REFRESH:
		_OnIndex(&(pRspMsg->mktDataSnapshot.head), &(pRspMsg->mktDataSnapshot.index));
		break;
	case MDS_MSGTYPE_SECURITY_STATUS:
		/* 处理(深圳)证券状态消息 */

		break;

	case MDS_MSGTYPE_TRADING_SESSION_STATUS:

		break;

	case MDS_MSGTYPE_MARKET_DATA_REQUEST:
		/* 处理行情订阅请求的应答消息 */
		if (pMsgHead->status != 0)
		{
			printf("... 接收到行情订阅请求应答, 行情订阅失败! "
					"errCode[%02u%02u]\n", pMsgHead->status, pMsgHead->detailStatus);
		}
		break;

	case MDS_MSGTYPE_TEST_REQUEST:
		/* 处理测试请求的应答消息 */
		break;

	case MDS_MSGTYPE_HEARTBEAT:
		/* 直接忽略心跳消息即可 */
		// printf("... 接收到心跳消息\n");
		break;
	case MDS_MSGTYPE_COMPRESSED_PACKETS:
		break;
	default:
		return EFTYPE;
	}

	return 0;
}

/**
 * 用于处理证券静态信息查询结果的回调函数
 *
 * @param   pSessionInfo        会话信息
 * @param   pMsgHead            消息头
 * @param   pMsgBody            消息体数据
 * @param   pQryCursor          指示查询进度的游标
 * @param   pCallbackParams     外部传入的参数
 * @return  大于等于0，成功；小于0，失败（错误号）
 *
 * @see     eMdsMsgTypeT
 * @see     MdsStockStaticInfoT
 */
static inline int32 _MdsQuerySample_OnQueryStockStaticInfo(MdsApiSessionInfoT *pSessionInfo, SMsgHeadT *pMsgHead, void *pMsgBody, MdsQryCursorT *pQryCursor, void *pCallbackParams)
{
	//	MdsStockStaticInfoT *pItem = (MdsStockStaticInfoT*) pCallbackParams;

	return 0;
}

/**
 * 查询证券静态信息
 *
 * @param   pQryChannel         查询通道的会话信息
 * @param   pSecurityListStr    以逗号或空格分隔的证券代码列表字符串
 * (证券代码的最大数量限制为 200)
 *                              - 证券代码支持以 .SH 或 .SZ
 * 为后缀来指定其所属的交易所
 *                              - 空字符串 "" 或 NULL, 表示查询所有证券
 * (不包括指数和期权)
 *                              -
 * 证券代码列表的分隔符可以是逗号、分号、竖线或空格 (e.g. ",;| \t")
 * @param   pQryFilter          查询过滤条件
 *                              - 传空指针或者将过滤条件初始化为0, 代表无需过滤
 * @retval  >=0                 成功查询到的记录数
 * @retval  <0                  失败 (负的错误号)
 *
 * @see     MdsStockStaticInfoT
 */
static inline int32 _MdsQuerySample_QueryStockStaticInfoList(MdsApiSessionInfoT *pQryChannel, const char *pSecurityListStr, const MdsQryStockStaticInfoListFilterT *pQryFilter)
{
	int32 ret = 0;
	static MdsStockStaticInfoT items[100000];
	SLOG_ASSERT(pQryChannel);

	if (__spk_unlikely(!pQryChannel))
	{
		SLOG_ERROR("无效的参数! pQryChannel[%p]", pQryChannel);
		return SPK_NEG(EINVAL);
	}

	memset(items, 0, 100000 * sizeof(MdsStockStaticInfoT));

	ret = MdsApi_QueryStockStaticInfoList(pQryChannel, pSecurityListStr, (char*) NULL, pQryFilter, _MdsQuerySample_OnQueryStockStaticInfo, items);
	if (__spk_unlikely(ret < 0))
	{
		SLOG_ERROR("查询证券静态信息失败 (或回调函数返回负值)! ret[%d]", ret);
		return ret;
	}
	else if (__spk_unlikely(ret == 0))
	{
		SLOG_WARN("未查询到证券静态信息! ret[%d]", ret);
		return 0;
	}
	SLOG_INFO("静态数据数量[%d]", ret);

	/* 查询到 ret 条证券静态信息 */
	return ret;
}

static int Init(const char *configfile)
{
	int32 ret = 0;
	MdsApiClientEnvT cliEnv =
	{ NULLOBJ_MDSAPI_CLIENT_ENV };

	MdsQryStockStaticInfoListFilterT qryFilter =
	{ NULLOBJ_MDS_QRY_STOCK_STATIC_INFO_LIST_FILTER };

	memset(&qryFilter, 0, sizeof(MdsQryStockStaticInfoListFilterT));

	slog_debug(0, "查询盘前静态数据......");
	MdsApi_InitLogger(configfile, "log");

	if (!MdsApi_InitQryChannel(&cliEnv.qryChannel, configfile, "mds_client", "qryServer"))
	{
		slog_debug(0, "连接柜台查询通道失败");
		return -1;
	}

	ret = _MdsQuerySample_QueryStockStaticInfoList(&cliEnv.qryChannel, (char*) NULL, &qryFilter);

	MdsApi_Destory(&cliEnv.qryChannel);

	return ret;
}

// 只订阅可转债、逆回购、新股上市第一天行情
static int SubMktData(MdsApiSessionInfoT *pTcpChannel, XInt level, XInt market, XChar secType[])
{
	XIdx i = 0, j = 0;
	XBool bFirst = true;
	char buff[960];
	XIdx icount = 0;
	XStockT *pStock = NULL;
	XMonitorT *pMonitor = NULL;
	XBool bSub = false;
	int32 dataTypes = 0;

	if (!level)
	{
		dataTypes = MDS_SUB_DATA_TYPE_L1_SNAPSHOT | MDS_SUB_DATA_TYPE_INDEX_SNAPSHOT;
		slog_info(0, "只使用L1行情......");
	}
	else
	{
		dataTypes = MDS_SUB_DATA_TYPE_L2_SNAPSHOT | MDS_SUB_DATA_TYPE_L2_ORDER | MDS_SUB_DATA_TYPE_L2_SSE_ORDER | MDS_SUB_DATA_TYPE_L2_TRADE | MDS_SUB_DATA_TYPE_INDEX_SNAPSHOT;
		slog_info(0, "使用L2行情......");
	}

	memset(buff, 0, sizeof(buff));

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor)
	{
		return (-1);
	}

	for (i = 0; i < pMonitor->iTStock; i++)
	{
		bSub = false;
		pStock = XFndVStockById(i + 1);
		if (NULL == pStock)
		{
			continue;
		}
		if (market != exMarketShSz && market != pStock->market)
		{
			continue;
		}
		/**
		 // 订阅逆回购数据
		 if (market != eXMarketSza && !strncmp (pStock->securityId, "204001", 6))
		 {
		 bSub = true;
		 }
		 else if (market != eXMarketSha
		 && !strncmp (pStock->securityId, "131810", 6))
		 {
		 bSub = true;
		 }
		 */
		if (!bSub)
		{
			for (j = 0; j < 5; j++)
			{
				if (secType[j] == pStock->secType)
				{
					if (pStock->secType != eXBond || (pStock->secType == eXBond && pStock->subSecType == eXSubSecCCF))
					{
						bSub = true;
						break;
					}
				}
			}
		}
		if (!bSub)
		{
			continue;
		}

		slog_info(0, "订阅代码:[%d-%s]", pStock->market, pStock->securityId);
		// 支持按类别订阅,判断是否订阅中的类别
		if (pStock->market == eXMarketSha)
		{
			icount++;
			strcat(buff, pStock->securityId);
			strcat(buff, ".SH,");
		}
		else
		{
			icount++;
			strcat(buff, pStock->securityId);
			strcat(buff, ".SZ,");
		}

		if (icount % 90 == 89)
		{
			if (bFirst)
			{
				// 订阅行情
				bFirst = false;
				/* 根据证券代码列表订阅行情 */
				if (!MdsApi_SubscribeByString(pTcpChannel, buff, (char*) NULL, MDS_EXCH_SSE, MDS_MD_PRODUCT_TYPE_STOCK, MDS_SUB_MODE_SET, dataTypes))
				{
					slog_error(0, "根据证券代码列表订阅行情失败!");
					return (-1);
				}
			}
			else
			{
				/* 根据证券代码列表订阅行情 */
				if (!MdsApi_SubscribeByString(pTcpChannel, buff, (char*) NULL, MDS_EXCH_SSE, MDS_MD_PRODUCT_TYPE_STOCK, MDS_SUB_MODE_APPEND, dataTypes))
				{
					slog_error(0, "根据证券代码列表订阅行情失败!");
					return (-1);
				}
			}
			slog_info(3, "*** 当前订阅行情数量[%d]", icount);
			memset(buff, 0, sizeof(buff));
		}
	}

	/* 根据证券代码列表订阅行情 */
	if (icount % 90 != 0 && !MdsApi_SubscribeByString(pTcpChannel, buff, (char*) NULL, MDS_EXCH_SSE, MDS_MD_PRODUCT_TYPE_STOCK, MDS_SUB_MODE_APPEND, dataTypes))
	{
		slog_error(0, "根据证券代码列表订阅行情失败");
		return (-1);
	}

	/* 订阅指数行情 */
	if (!MdsApi_SubscribeByString(pTcpChannel, (char*) NULL, (char*) NULL, MDS_EXCH_SSE, MDS_MD_PRODUCT_TYPE_INDEX, MDS_SUB_MODE_APPEND, dataTypes))
	{
		slog_error(0, "根据证券代码列表订阅行情失败!");
		return (-1);
	}

	/* 订阅指数行情 */
	if (!MdsApi_SubscribeByString(pTcpChannel, (char*) NULL, (char*) NULL, MDS_EXCH_SZSE, MDS_MD_PRODUCT_TYPE_INDEX, MDS_SUB_MODE_APPEND, dataTypes))
	{
		slog_error(0, "根据证券代码列表订阅行情失败!");
		return (-1);
	}

	/** 订阅逆回购 */
	if (!MdsApi_SubscribeByString(pTcpChannel, (char*) "204001", (char*) NULL, MDS_EXCH_SSE, MDS_MD_PRODUCT_TYPE_STOCK, MDS_SUB_MODE_APPEND, dataTypes))
	{
		slog_error(0, "根据证券代码列表订阅行情失败!");
		return (-1);
	}
	if (!MdsApi_SubscribeByString(pTcpChannel, (char*) "131810", (char*) NULL, MDS_EXCH_SZSE, MDS_MD_PRODUCT_TYPE_STOCK, MDS_SUB_MODE_APPEND, dataTypes))
	{
		slog_error(0, "根据证券代码列表订阅行情失败!");
		return (-1);
	}

	slog_info(3, "订阅行情数量[%d],", icount);

	return (0);
}

static void*
MdsApiSample_ReSubMkt(SessionWithLevelT *sessionWithLevel)
{
	MdsApiSessionInfoT *pTcpChannel = &(sessionWithLevel->session);
	volatile int32 *pThreadTerminatedFlag = &pTcpChannel->__customFlag;
	XMarketSecurityT *security = NULL;
	int32 dataTypes = 0;

	if (!sessionWithLevel->level)
	{
		dataTypes = MDS_SUB_DATA_TYPE_L1_SNAPSHOT;
	}
	else
	{
		dataTypes = MDS_SUB_DATA_TYPE_L2_SNAPSHOT | MDS_SUB_DATA_TYPE_L2_ORDER | MDS_SUB_DATA_TYPE_L2_SSE_ORDER | MDS_SUB_DATA_TYPE_L2_TRADE;
	}
	XInt readId = -1;
	XInt market = -1;

	if (NULL == pTcpChannel)
	{
		return (void*) FALSE;
	}

	readId = XGetReadCache(XSHMKEYCONECT(mktSubscribe));

	while (!*pThreadTerminatedFlag)
	{
		if (XIsNullCache(XSHMKEYCONECT(mktSubscribe)) || (security = XPopCache(XSHMKEYCONECT(mktSubscribe), readId)) == NULL)
		{
			continue;
		}
		market = market_to_oes(eXInvSpot, security->market);
		// 重新订阅
		if (security->type == exReset)
		{
			if (!MdsApi_SubscribeByString(pTcpChannel, security->securityId, (char*) NULL, market, MDS_MD_PRODUCT_TYPE_STOCK, MDS_SUB_MODE_SET, dataTypes))
			{
				slog_error(0, "根据证券代码列表[%s]订阅行情失败!", security->securityId);
				//					return FALSE;
			}
			else
			{
				slog_info(3, "实时订阅行情，重置行情,重新订阅[%s]", security->securityId);
			}
		}

		else if (security->type == exDelete)
		{
			if (!MdsApi_SubscribeByString(pTcpChannel, security->securityId, (char*) NULL, market, MDS_MD_PRODUCT_TYPE_STOCK, MDS_SUB_MODE_DELETE, dataTypes))
			{
				slog_error(0, "根据证券代码列表[%s]订阅行情失败!", security->securityId);
			}
			else
			{
				slog_info(3, "实时订阅行情,删除[%s]", security->securityId);
			}
		}
		else
		{

			slog_info(3, "实时订阅行情[%d-%s]", security->market, security->securityId);
			if (!MdsApi_SubscribeByString(pTcpChannel, security->securityId, (char*) NULL, market, MDS_MD_PRODUCT_TYPE_STOCK, MDS_SUB_MODE_APPEND, dataTypes))
			{
				slog_error(0, "根据证券代码列表[%s]订阅行情失败!", security->securityId);
				//					return FALSE;
			}
			else
			{
				slog_info(3, "实时订阅行情，追加[%s]", security->securityId);
			}
		}
	}

	*pThreadTerminatedFlag = -1;
	return (void*) TRUE;

	*pThreadTerminatedFlag = -1;
	return (void*) FALSE;
}

XInt oesmkt(XChar *customer, XBindParamT *pBind)
{
	/* 尝试等待行情消息到达的超时时间 (毫秒) */
	static const int32 THE_TIMEOUT_MS = 1000;
	XInt iret = -1;
	int iRetryTimes = 0;
	MdsApiClientEnvT cliEnv =
	{ NULLOBJ_MDSAPI_CLIENT_ENV };
	XCustT *pCustomer = NULL;
	XBool bClosed = false;
	XLongTime beginTime = 0;
	SessionWithLevelT sessionWithLevel;

	memset(&sessionWithLevel, 0, sizeof(SessionWithLevelT));

	if (!isBindCpu)
	{
		iret = XBindCPU(pBind->cpuid);
		if (iret)
		{
			slog_warn(0, "绑核失败[%d]", pBind->cpuid);
		}
		isBindCpu = 1;
	}

	pCustomer = XFndVCustomerByKey(customer);
	if (NULL == pCustomer)
	{
		slog_error(0, "未找到登录用户[%s]", customer);
		return (-1);
	}

	l_MonitorMd = XFndVMdMonitor(eXExchSec);
	if (NULL == l_MonitorMd)
	{
		slog_error(0, "未获取到行情信息[%s]", customer);
		return (-1);
	}
	slog_debug(0, "账户[%s]-密码[******]", pCustomer->customerId);
	MdsApi_SetThreadUsername(pCustomer->customerId);
	MdsApi_SetThreadPassword(pCustomer->password);

// 查询静态数据
	LOOP_START: if (Init(XMDS_CONFIG_FILE) <= 0)
	{
		SPK_SLEEP_MS(10000);
		goto LOOP_START;
	}

	ON_RECONNECT: MdsApi_SetThreadUsername(pCustomer->customerId);
	MdsApi_SetThreadPassword(pCustomer->password);

	/* 初始化客户端环境 (配置文件参见: mds_client_sample.conf) */
	if (!MdsApi_InitAll(&cliEnv, XMDS_CONFIG_FILE,
	MDSAPI_CFG_DEFAULT_SECTION_LOGGER,
	MDSAPI_CFG_DEFAULT_SECTION, "tcpServer", (char*) NULL, (char*) NULL, (char*) NULL, (char*) NULL, (char*) NULL))
	{
		SPK_SLEEP_MS(10000);
		iRetryTimes++;
		slog_info(3, "等待10秒后重试[%d]......", iRetryTimes);
		if (iRetryTimes > 10)
		{
			goto ON_ERROR;
		}
		goto ON_RECONNECT;
	}

	slog_info(3, "API版本号[%s]", MdsApi_GetApiVersion());

	slog_warn(0, "!!!!!!   开始接收行情数据[%lld]......   !!!!!!", XGetComMSec());

	if (pBind->resub)
	{
		slog_info(3, "开启盘中实时订阅功能(开盘后订阅的证券重构数据会丢失)......");

		pthread_t subThreadId;

		sessionWithLevel.level = pBind->level;
		sessionWithLevel.session = cliEnv.tcpChannel;
		iret = pthread_create(&subThreadId, NULL, (void* (*)(void*)) MdsApiSample_ReSubMkt, &sessionWithLevel);
		if (unlikely(iret != 0))
		{
			slog_error(0, "创建订阅接收线程失败! error[%d]", iret);
			goto ON_ERROR;
		}
	}
	else
	{
		slog_info(3, "系统启动时自动订阅全市场行情......");
		SubMktData(&cliEnv.tcpChannel, pBind->level, pBind->market, pBind->secType);
	}
	for (;;)
	{

		// 收市后延迟5分钟关闭
		bClosed = isMktClosedTime(l_MonitorMd->updateTime);
		if (bClosed)
		{
			if (!beginTime)
			{
				beginTime = XGetClockTime();
			}
			if (XGetClockTime() - beginTime > 5 * 60 * XTIMS_S4NS)
			{
				break;
			}
		}
		/* 等待行情消息到达, 并通过回调函数对消息进行处理 */
		iret = MdsApi_WaitOnMsg(&cliEnv.tcpChannel, THE_TIMEOUT_MS, MdsApiSample_HandleMsg, (void*) (pBind));
		if (iret < 0)
		{
			if (SPK_IS_NEG_ETIMEDOUT(iret))
			{
				/* 执行超时检查 (检查会话是否已超时) */
				continue;
			}

			if (SPK_IS_NEG_EPIPE(iret))
			{
				isBindCpu = 0;
				/* 连接已断开 */
				goto ON_RECONNECT;
			}
			goto ON_ERROR;
		}
	}

	/* 设置回报线程退出标志 */
	*((volatile int32*) &cliEnv.tcpChannel.__customFlag) = 1;

	/* 回报线程将标志设置为-1后退出, 父进程再释放资源 */
	while (*((volatile int32*) &cliEnv.tcpChannel.__customFlag) != -1)
	{
		SPK_SLEEP_MS(1000);
	}

	slog_info(0, "结束时间[%lld]", STime_GetMillisecondsTime());
	MdsApi_LogoutAll(&cliEnv, TRUE);

	ON_ERROR:

	MdsApi_DestoryAll(&cliEnv);

	l_MonitorMd->isRunning = false;
	exit(0);
	return (iret);
}

XVoid XOesMkt(XVoid *param)
{
	XInt i = 0;
	XMonitorT *pMonitor = NULL;
	XCustT *pCust = NULL;
	XBindParamT *pBind = NULL;

	slog_info(0, "XOesMkt启动......");
	pBind = (XBindParamT*) param;
	if (NULL == pBind)
	{
		return;
	}

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor)
	{
		return;
	}

	for (i = 0; i < pMonitor->iTCust; i++)
	{
		pCust = XFndVCustomerById(i + 1);
		if (NULL == pCust || pCust->type == eXUserTrade || pCust->counter != eXCounterOes)
		{
			continue;
		}
		else
		{
			oesmkt(pCust->customerId, pBind);
			break;
		}
	}
}
