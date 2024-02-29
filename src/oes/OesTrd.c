/*
 * @file OesTrd.c
 * @brief 柜台交易接口
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */
#define _GNU_SOURCE
#define __USE_GNU

#include    <getopt.h>

#include <sys/types.h>

#include    "oes_api/oes_api.h"
#include    "sutil/time/spk_times.h"
#include    "XLog.h"
#include    "XCom.h"
#include    "OesTrd.h"
#include    "XUtils.h"
#include    "XTimes.h"

static char *g_pUser = NULL;
static XMonitorTdT *l_pMonitorTd = NULL;
static XMonitorMdT *l_pMonitorMd = NULL;
static XMonitorT *l_pMonitor = NULL;

//static XIdx l_Counter = 0;

/**
 * 发送委托请求
 *
 * 提示:
 * - 可以通过 OesApi_GetClEnvId() 方法获得到当前通道所使用的客户端环境号(clEnvId), 如:
 *   <code>int8 clEnvId = OesApi_GetClEnvId(pOrdChannel);</code>
 *
 * @param   pOrdChannel     委托通道的会话信息
 * @param   mktId           市场代码 (必填) @see eOesMarketIdT
 * @param   pSecurityId     股票代码 (必填)
 * @param   pInvAcctId      股东账户代码 (可不填)
 * @param   ordType         委托类型 (必填) @see eOesOrdTypeT, eOesOrdTypeShT, eOesOrdTypeSzT
 * @param   bsType          买卖类型 (必填) @see eOesBuySellTypeT
 * @param   ordQty          委托数量 (必填, 单位为股/张)
 * @param   ordPrice        委托价格 (必填, 单位精确到元后四位，即1元 = 10000)
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static inline int32 _OesApiSample_SendOrderReq(OesApiSessionInfoT *pOrdChannel,
		XOrderT *pOrder) {
	OesOrdReqT ordReq = { NULLOBJ_OES_ORD_REQ };
	XOrderReqT request;

	int iret = eXExecFailure;

	request = pOrder->request;
	//自己维护流水号
	ordReq.clSeqNo = pOrder->request.localId;

	ordReq.mktId = market_to_oes(request.acctType, request.market);

	ordReq.ordType = ordtype_to_oes(request.market, request.ordType);

	ordReq.bsType = bstype_to_oes(request.acctType, pOrder->productType,
			request.bsType);

	memcpy(ordReq.securityId, request.securityId, SECURITYID_LEN);

	if (request.investId != NULL && strlen(request.investId) != 0) {
		/* 股东账户可不填 */
		memcpy(ordReq.invAcctId, request.investId, INVESTID_LEN);
	}

	ordReq.ordQty = request.ordQty;
	ordReq.ordPrice = request.ordPrice;

	//策略号
	ordReq.userInfo.i64 = pOrder->request.plotid;

	pOrder->envno = OesApi_GetClEnvId(pOrdChannel);
	//委托时间
	pOrder->_sendLocTime = XGetClockTime();

	pOrder->ordStatus = eXOrdStatusDefalut;
	//本地委托放入Hash，如果Rsp有则更新该委托

	iret = OesApi_SendOrderReq(pOrdChannel, &ordReq);
	if (iret == 0) {
		pOrder->exeStatus = eXExec;
	} else {
		pOrder->errorno = iret;
		pOrder->exeStatus = eXExecFailure;
		slog_error(0, "发送订单失败[%d]", ordReq.clSeqNo);
	}

	iret = pOrder->exeStatus;
	XPutOrderHashByLoc(pOrder);
	slog_info(0,
			"<<<<<< ReqOrder-idx[%lld-%s],产品类型[%u]--[%u-%s],策略单号[%lld]-账户[%s],本地单号[%d-%d],账户类型[%d], 买卖[%u], 订单类型[%d], 价格[%d->%.3f], 数量[%d],本地发送时间[%lld]",
			pOrder->idx, request.customerId, pOrder->productType, ordReq.mktId,
			ordReq.securityId, ordReq.userInfo.i64, ordReq.invAcctId,
			pOrder->envno, ordReq.clSeqNo, request.acctType, ordReq.bsType,
			ordReq.ordType, ordReq.ordPrice, ordReq.ordPrice * XPRICE_DIV,
			ordReq.ordQty, pOrder->_sendLocTime);

	return iret;
}

/**
 * 发送撤单请求
 *
 * @param   pOrdChannel     委托通道的会话信息
 * @param   mktId           被撤委托的市场代码 (必填) @see eOesMarketIdT
 * @param   pSecurityId     被撤委托的股票代码 (选填, 若不为空则校验待撤订单是否匹配)
 * @param   pInvAcctId      被撤委托的股东账户代码 (选填, 若不为空则校验待撤订单是否匹配)
 * @param   origClSeqNo     被撤委托的流水号 (若使用 origClOrdId, 则不必填充该字段)
 * @param   origClEnvId     被撤委托的客户端环境号 (小于等于0, 则使用当前会话的 clEnvId)
 * @param   origClOrdId     被撤委托的客户订单编号 (若使用 origClSeqNo, 则不必填充该字段)
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static inline int32 _OesApiSample_SendOrderCancelReq(
		OesApiSessionInfoT *pOrdChannel, XOrderT *pOrder) {
	OesOrdCancelReqT cancelReq = { NULLOBJ_OES_ORD_CANCEL_REQ };
	int iret = 0;
	XOrderReqT request = pOrder->request;

	cancelReq.clSeqNo = pOrder->request.localId;
	cancelReq.mktId = market_to_oes(request.acctType, request.market);

	if (request.securityId) {
		/* 撤单时被撤委托的股票代码可不填 */
		memcpy(cancelReq.securityId, request.securityId,
		SECURITYID_LEN);
	}

	if (request.investId) {
		/* 撤单时被撤委托的股东账户可不填 */
		memcpy(cancelReq.invAcctId, request.investId, INVESTID_LEN);
	}

	cancelReq.origClSeqNo = request.orgLocalId;
	cancelReq.origClEnvId = request.orgEnvno;
	cancelReq.origClOrdId = request.orgOrdId;
	//策略号
	cancelReq.userInfo.i64 = pOrder->request.plotid;

	//委托时间
	pOrder->_sendLocTime = XGetClockTime();
	pOrder->envno = OesApi_GetClEnvId(pOrdChannel);
	pOrder->ordStatus = eXOrdStatusDefalut;

	iret = OesApi_SendOrderCancelReq(pOrdChannel, &cancelReq);
	if (iret == 0) {
		pOrder->exeStatus = eXExec;
		//更新统计值
//		pMonitorTd->iDealOrder += 1;
	} else {
		pOrder->errorno = iret;
		pOrder->exeStatus = eXExecFailure;
	}
	iret = pOrder->exeStatus;

	XPutOrderHashByLoc(pOrder);

	slog_info(0,
			"<<<<<< ReqCancel-[%u-%s],序号[%lld]-策略编号[%lld]-账户[%s-%s]: 撤单编号:[%d-%d],原编号:[%d-%lld],发单时间[%lld]",
			cancelReq.mktId, cancelReq.securityId, pOrder->request.reqId,
			pOrder->request.plotid, request.customerId, cancelReq.invAcctId,
			pOrder->envno, cancelReq.clSeqNo, cancelReq.origClSeqNo,
			cancelReq.origClOrdId, pOrder->_sendLocTime);

	return iret;
}

/**
 * 发送出入金请求
 *
 * 提示:
 * - 可以通过 OesApi_GetClEnvId() 方法获得到当前通道所使用的客户端环境号(clEnvId), 如:
 *   <code>int8 clEnvId = OesApi_GetClEnvId(pOrdChannel);</code>
 *
 * @param   pOrdChannel     委托通道的会话信息
 * @param   direct          划转方向 (必填) @see eOesFundTrsfDirectT
 * @param   fundTrsfType    出入金转账类型 (必填) @see eOesFundTrsfTypeT
 * @param   pCashAcctId     资金账户代码 (可不填)
 * @param   pTrdPasswd      交易密码 (沪深OES之间内部资金划拨时无需填写该字段, 其它场景该字段必填)
 * @param   trsfPasswd      转账密码 (银行转证券时为银行密码; 证券转银行时为主柜资金密码; \
 *                                  OES和主柜之间划拨资金时为主柜资金密码; 沪深OES之间划拨时，无需填写)
 * @return  大于等于0, 成功；小于0, 失败（错误号）
 */
static inline int32
_OesStkSample_SendFundTransferReq(OesApiSessionInfoT *pOrdChannel, XOrderT *pOrder) {
    OesFundTrsfReqT     fundTrsfReq = {NULLOBJ_OES_FUND_TRSF_REQ};

    fundTrsfReq.clSeqNo = pOrder->request.localId;
    fundTrsfReq.direct = OES_FUND_TRSF_DIRECT_IN;
    fundTrsfReq.fundTrsfType = OES_FUND_TRSF_TYPE_OES_TO_OES;
    fundTrsfReq.occurAmt = pOrder->request.orgOrdId;

    /* 资金账号可不填 */
    if (pOrder->request.investId) {
        strncpy(fundTrsfReq.cashAcctId, pOrder->request.investId,
                strlen(pOrder->request.investId));
    }

    return OesApi_SendFundTransferReq(pOrdChannel, &fundTrsfReq);
}


static int32 _HandleInsert(OesOrdCnfmT pOrdCnfm) {
	XOrderT order = { 0 };
	const char *pTmpStr = NULL;
	XTradeCache cache = { 0 };
	//已经收到交易所响应的订单

//	slog_debug(0, ">>>>>>_HandleInsert");

	order.request.market = market_from_oes(pOrdCnfm.mktId);
	memcpy(order.request.securityId, pOrdCnfm.securityId, SECURITYID_LEN);

	memcpy(order.request.investId, pOrdCnfm.invAcctId, INVESTID_LEN);

	if (g_pUser != NULL && strlen(g_pUser) != 0) {
		memcpy(order.request.customerId, g_pUser, strlen(g_pUser));
	}
	order._cnfLocTime = XGetClockTime();
	order._sendTime = pOrdCnfm.ordTime;
	order._cnfTime = pOrdCnfm.ordCnfmTime;
	order.ordStatus = ordstatus_from_oes(pOrdCnfm.ordStatus);

	memcpy(order.exchordId, pOrdCnfm.exchOrdId, EXCHORDID_LEN);
	order.frzAmt = pOrdCnfm.frzAmt;
	order.frzFee = pOrdCnfm.frzFee;

	order.errorno = pOrdCnfm.ordRejReason;
	pTmpStr = OesApi_GetErrorMsg(order.errorno);
	if (pTmpStr) {
		memcpy(order.errmsg, pTmpStr, strlen(pTmpStr));
	}

	order.request.ordType = ordtype_from_oes(pOrdCnfm.ordType);
	order.request.ordQty = pOrdCnfm.ordQty;
	order.request.bsType = bstype_from_oes(pOrdCnfm.bsType);
	order.request.isCancel = iscancel_from_oes(pOrdCnfm.bsType);

	order.request.localId = pOrdCnfm.clSeqNo;
	order.ordid = pOrdCnfm.clOrdId;
	order.counter = eXCounterOes;
	order.request.ordPrice = pOrdCnfm.ordPrice;
	order.request.orgOrdId = pOrdCnfm.origClOrdId;
	order.request.orgLocalId = pOrdCnfm.origClSeqNo;
	order.request.orgEnvno = pOrdCnfm.origClEnvId;
	order.envno = pOrdCnfm.clEnvId;
	order.request.plotid = pOrdCnfm.userInfo.i64;
	order.exeStatus = eXExec;

	cache.head.type = eDOrder;
//	cache.head.num = ++l_Counter;
	cache.head.dataLen = XORDER_SIZE;
	cache.ordrsp = order;

	/**
	 slog_debug(0, "<<<<<<消息序号[%lld], 响应[%d-%s]订单号[%d]", cache.head.num,
	 order.request.market, order.request.securityId,
	 order.request.localId);
	 */
	XPushCache(XSHMKEYCONECT(tradeCache), &cache);

	return 0;
}

static int32 _HandleReject(OesOrdRejectT pOrdReject) {

	XOrderT order = { 0 };
	const char *pTmpStr = NULL;
	XTradeCache cache = { 0 };

//	slog_debug(0, ">>>>>>_HandleReject");
	order.request.market = market_from_oes(pOrdReject.mktId);
	memcpy(order.request.securityId, pOrdReject.securityId,
	SECURITYID_LEN);

	memcpy(order.request.investId, pOrdReject.invAcctId,
	INVESTID_LEN);

	order.ordStatus = eXOrdStatusInvalid;
	if (g_pUser != NULL && strlen(g_pUser) != 0) {
		memcpy(order.request.customerId, g_pUser, strlen(g_pUser));
	}

	order._sendTime = pOrdReject.ordTime;
	order._cnfLocTime = XGetClockTime();
	order.request.orgOrdId = pOrdReject.origClOrdId;
	order.request.orgLocalId = pOrdReject.origClSeqNo;
	order.request.orgEnvno = pOrdReject.origClEnvId;
	order.errorno = pOrdReject.ordRejReason;
	order.counter = eXCounterOes;
	pTmpStr = OesApi_GetErrorMsg(order.errorno);
	if (pTmpStr) {
		memcpy(order.errmsg, pTmpStr, strlen(pTmpStr));
	}

	order.request.bsType = bstype_from_oes(pOrdReject.bsType); //需要从OES转为内部
	order.request.isCancel = iscancel_from_oes(pOrdReject.bsType); //需要从OES转为内部

	order.request.localId = pOrdReject.clSeqNo;

	order.request.ordType = ordtype_from_oes(pOrdReject.ordType); //需要从OES转为内部
	order.request.ordQty = pOrdReject.ordQty;
	order.request.ordPrice = pOrdReject.ordPrice;
	order.envno = pOrdReject.clEnvId;
	order.request.plotid = pOrdReject.userInfo.i64;
	order.exeStatus = eXExec;

	cache.head.type = eDOrder;
//	cache.head.num = ++l_Counter;
	cache.head.dataLen = XORDER_SIZE;
	cache.ordrsp = order;

	/**
	 slog_debug(0, "<<<<<<%lld 拒单[%d-%s]订单号[%d],错误信息[%d-%s]", cache.head.num,
	 order.request.market, order.request.securityId,
	 order.request.localId, order.errorno, order.errmsg);
	 */

	XPushCache(XSHMKEYCONECT(tradeCache), &cache);

	return 0;
}

static int32 _HandleCnfm(OesOrdCnfmT pOrdCnfm) {
	XOrderT order = { 0 };
	const char *pTmpStr = NULL;
	XTradeCache cache = { 0 };

	order.request.market = market_from_oes(pOrdCnfm.mktId);

	if (pOrdCnfm.securityId != NULL && strlen(pOrdCnfm.securityId) != 0) {
		memcpy(order.request.securityId, pOrdCnfm.securityId,
		SECURITYID_LEN);
	}
	memcpy(order.request.investId, pOrdCnfm.invAcctId,
	INVESTID_LEN);
	order.ordid = pOrdCnfm.clOrdId;
	order.ordStatus = ordstatus_from_oes(pOrdCnfm.ordStatus);
	if (g_pUser != NULL && strlen(g_pUser) != 0) {
		memcpy(order.request.customerId, g_pUser, strlen(g_pUser));
	}

	order._sendTime = pOrdCnfm.ordTime;
	order._cnfTime = pOrdCnfm.ordCnfmTime;
	order._cnfExTime = XGetClockTime();
	order.ordStatus = ordstatus_from_oes(pOrdCnfm.ordStatus);
	memcpy(order.exchordId, pOrdCnfm.exchOrdId, EXCHORDID_LEN);

	order.frzAmt = pOrdCnfm.frzAmt;
	order.frzFee = pOrdCnfm.frzFee;
	order.counter = eXCounterOes;
	order.errorno = pOrdCnfm.ordRejReason;
	pTmpStr = OesApi_GetErrorMsg(order.errorno);
	order.request.plotid = pOrdCnfm.userInfo.i64;
	if (pTmpStr) {
		memcpy(order.errmsg, pTmpStr, strlen(pTmpStr));
	}

	order.request.ordType = ordtype_from_oes(pOrdCnfm.ordType);
	order.request.ordQty = pOrdCnfm.ordQty;
	order.trdQty = pOrdCnfm.cumQty;
	order.trdMoney = pOrdCnfm.cumAmt;
	order.request.bsType = bstype_from_oes(pOrdCnfm.bsType);

	//被撤订单的原始订单仍然为对应的买卖标志即不为撤单
	order.request.isCancel = iscancel_from_oes(pOrdCnfm.bsType);

	order.request.localId = pOrdCnfm.clSeqNo;


	order.request.ordPrice = pOrdCnfm.ordPrice;
	order.request.orgOrdId = pOrdCnfm.origClOrdId;
	order.request.orgLocalId = pOrdCnfm.origClSeqNo;
	order.request.orgEnvno = pOrdCnfm.origClEnvId;
	order.envno = pOrdCnfm.clEnvId;
	order.exeStatus = eXExec;

	cache.head.type = eDOrder;
//	cache.head.num = ++l_Counter;
	cache.head.dataLen = XORDER_SIZE;
	cache.ordrsp = order;

	/**
	 slog_debug(0, "<<<<<<%d 确认[%d-%s]订单号[%d]", cache.head.num,
	 order.request.market, order.request.securityId,
	 order.request.localId);
	 */

	XPushCache(XSHMKEYCONECT(tradeCache), &cache);

	return 0;
}

static int32 _HandleTrade(OesTrdCnfmT pTrdCnfm) {
	XTradeT trade = { 0 };
	XTradeCache cache = { 0 };

//	slog_debug(0, ">>>>>>开始收成交回报");
	trade.ordid = pTrdCnfm.clOrdId;
	trade.market = market_from_oes(pTrdCnfm.mktId);
	memcpy(trade.securityId, pTrdCnfm.securityId, SECURITYID_LEN);
	memcpy(trade.investId, pTrdCnfm.invAcctId, INVESTID_LEN);
	trade.trdId = pTrdCnfm.exchTrdNum;

	if (g_pUser != NULL && strlen(g_pUser) != 0) {
		memcpy(trade.customerId, g_pUser, strlen(g_pUser));
	}

	trade.trdQty = pTrdCnfm.trdQty;
	trade.trdAmt = pTrdCnfm.trdAmt;
	trade.trdPrice = pTrdCnfm.trdPrice;
	trade.trdTime = pTrdCnfm.trdTime;

	trade.cumQty = pTrdCnfm.cumQty;
	trade.cumAmt = pTrdCnfm.cumAmt;

	trade.trdSide = bstype_from_oes(pTrdCnfm.trdSide);

	trade.counter = eXCounterOes;

	cache.head.type = eDTrade;
//	cache.head.num = ++l_Counter;
	cache.head.dataLen = XTRADE_SIZE;
	cache.trade = trade;

	/**
	 slog_debug(0, "<<<<<<%lld 成交[%c-%s]订单号[%d]", cache.head.num, trade.market,
	 trade.securityId, trade.ordid);
	 */
	XPushCache(XSHMKEYCONECT(tradeCache), &cache);

	return 0;
}

static int32 _HandleHolding(OesStkHoldingItemT pHoldingItem) {
	XHoldT hold = { 0 };
	XTradeCache cache = { 0 };

//	slog_debug(0, ">>>>>>开始收持仓回报");
	if (g_pUser != NULL && strlen(g_pUser) != 0) {
		memcpy(hold.customerId, g_pUser, strlen(g_pUser));
	}
		memcpy(hold.investId, pHoldingItem.invAcctId, INVESTID_LEN);

	hold.market = market_from_oes(pHoldingItem.mktId);

	memcpy(hold.securityId, pHoldingItem.securityId, SECURITYID_LEN);


	hold.countSellAvlHld = pHoldingItem.sellAvlHld;
	hold.sellAvlHld = pHoldingItem.sellAvlHld;
	hold.orgHld = pHoldingItem.originalHld;
	hold.orgAvlHld = pHoldingItem.originalAvlHld;
	hold.orgCostAmt = pHoldingItem.originalCostAmt;
	hold.totalBuyHld = pHoldingItem.totalBuyHld;
	hold.totalSellHld = pHoldingItem.totalSellHld;
	hold.totalBuyAmt = pHoldingItem.totalBuyAmt;
	hold.totalSellAmt = pHoldingItem.totalSellAmt;
	hold.sellFrz = pHoldingItem.sellFrzHld;
	hold.etfAvlHld = pHoldingItem.trsfOutAvlHld;
	hold.costPrice = pHoldingItem.costPrice;
	hold.sumHld = pHoldingItem.sumHld;

	cache.head.type = eDHold;
//	cache.head.num = ++l_Counter;
	cache.head.dataLen = XHOLD_SIZE;
	cache.hold = hold;
	XPushCache(XSHMKEYCONECT(tradeCache), &cache);

	/**
	 slog_debug(0, "<<<<<<%lld 持仓发送完毕", cache.head.num);
	 */
	return 0;
}

static int32 _HandleCashAsset(OesCashAssetItemT pCashAssetItem) {
	XCashT account = { 0 };
	XTradeCache cache = { 0 };

//	slog_debug(0, ">>>>>>开始收资金回报");
	if (g_pUser != NULL && strlen(g_pUser) != 0) {
		memcpy(account.customerId, g_pUser, strlen(g_pUser));
	}

		memcpy(account.accountId, pCashAssetItem.cashAcctId, ACCOUNTID_LEN);

	account.acctType = accttype_from_oes(pCashAssetItem.cashType);

	account.frozenAmt = pCashAssetItem.buyFrzAmt;
	account.curAvailable = pCashAssetItem.currentAvailableBal;
	account.countAvailable = pCashAssetItem.currentAvailableBal;
	account.totalBuy = pCashAssetItem.totalBuyAmt;
	account.totalSell = pCashAssetItem.totalSellAmt;
	account.totalFee = pCashAssetItem.totalFeeAmt;

	cache.head.type = eDCash;
//	cache.head.num = ++l_Counter;
	cache.head.dataLen = XCASH_SIZE;
	cache.cash = account;
	XPushCache(XSHMKEYCONECT(tradeCache), &cache);
	/**
	 slog_debug(0, "<<<<<<%lld 资金发送完毕", cache.head.num);
	 */
	return 0;
}
/**
 * 对执行报告消息进行处理的回调函数
 *
 * @param   pRptChannel     回报通道的会话信息
 * @param   pMsgHead        消息头
 * @param   pMsgBody        消息体数据
 * @param   pCallbackParams 外部传入的参数
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static inline int32 _OesApiSample_HandleReportMsg(
		OesApiSessionInfoT *pRptChannel, SMsgHeadT *pMsgHead, void *pMsgBody,
		void *pCallbackParams) {
	OesRspMsgBodyT *pRspMsg = (OesRspMsgBodyT*) pMsgBody;
	OesRptMsgT *pRptMsg = &pRspMsg->rptMsg;

	assert(pRptChannel && pMsgHead && pRspMsg);

	switch (pMsgHead->msgId) {
	case OESMSG_RPT_ORDER_INSERT: /* OES委托已生成 (已通过风控检查) @see OesOrdCnfmT */
		_HandleInsert(pRptMsg->rptBody.ordInsertRsp);
		break;

	case OESMSG_RPT_BUSINESS_REJECT: /* OES业务拒绝 (未通过风控检查等) @see OesOrdRejectT */

		_HandleReject(pRptMsg->rptBody.ordRejectRsp);
		break;

	case OESMSG_RPT_ORDER_REPORT: /* 交易所委托回报 (包括交易所委托拒绝、委托确认和撤单完成通知) @see OesOrdCnfmT */
		_HandleCnfm(pRptMsg->rptBody.ordCnfm);
		break;

	case OESMSG_RPT_TRADE_REPORT: /* 交易所成交回报 @see OesTrdCnfmT */

		_HandleTrade(pRptMsg->rptBody.trdCnfm);
		break;

	case OESMSG_RPT_CASH_ASSET_VARIATION: /* 资金变动信息 @see OesCashAssetItemT */
		_HandleCashAsset(pRptMsg->rptBody.cashAssetRpt);
		break;

	case OESMSG_RPT_STOCK_HOLDING_VARIATION: /* 持仓变动信息 (股票) @see OesStkHoldingItemT */
		_HandleHolding(pRptMsg->rptBody.stkHoldingRpt);
		break;

	case OESMSG_RPT_FUND_TRSF_REJECT: /* 出入金委托响应-业务拒绝 @see OesFundTrsfRejectT */
		slog_info(3,
				">>>>>> Recv FundTrsfReject: {cashAcctId: %s, rejReason: %d}",
				pRptMsg->rptBody.fundTrsfRejectRsp.cashAcctId,
				pRptMsg->rptBody.fundTrsfRejectRsp.rejReason)
		;
		break;

	case OESMSG_RPT_FUND_TRSF_REPORT: /* 出入金委托执行报告 @see OesFundTrsfReportT */
		slog_info(3,
				">>>>>> Recv FundTrsfReport: {cashAcctId: %s,trsfStatus: %u}\n",
				pRptMsg->rptBody.fundTrsfCnfm.cashAcctId,
				pRptMsg->rptBody.fundTrsfCnfm.trsfStatus)
		;
		break;

	case OESMSG_RPT_REPORT_SYNCHRONIZATION: /* 回报同步响应 @see OesReportSynchronizationRspT */
		slog_info(3,
				">>>>>> Recv report synchronization: {subscribeEnvId: %d, subscribeRptTypes: %d, lastRptSeqNum: %d}",
				pRspMsg->reportSynchronizationRsp.subscribeEnvId,
				pRspMsg->reportSynchronizationRsp.subscribeRptTypes,
				pRspMsg->reportSynchronizationRsp.lastRptSeqNum)
		;
		break;

	case OESMSG_RPT_MARKET_STATE: /* 市场状态信息 @see OesMarketStateInfoT */
		slog_info(3,
				">>>>>> Recv MktStatusReport: {exchId: %u, platformId: %u, mktId: %u, mktState: %u}",
				pRspMsg->mktStateRpt.exchId, pRspMsg->mktStateRpt.platformId,
				pRspMsg->mktStateRpt.mktId, pRspMsg->mktStateRpt.mktState)
		;
		if (pRspMsg->mktStateRpt.mktId == OES_MKT_SZ_ASHARE
				&& pRspMsg->mktStateRpt.platformId
						== OES_PLATFORM_CASH_AUCTION) {
			l_pMonitorTd->status = pRspMsg->mktStateRpt.mktState;
		}
		break;

	case OESMSG_SESS_HEARTBEAT:
		break;

	default:
		fprintf(stderr, "Invalid message type! msgId[0x%02X]\n",
				pMsgHead->msgId);
		break;
	}

	return 0;
}

/**
 * 超时检查处理
 *
 * @param   pRptChannel     回报通道的会话信息
 * @return  等于0，运行正常，未超时；大于0，已超时，需要重建连接；小于0，失败（错误号）
 */
static inline int32 _OesApiSample_OnTimeout(OesApiClientEnvT *pClientEnv) {
	OesApiSessionInfoT *pRptChannel = &pClientEnv->rptChannel;
	int64 recvInterval = 0;

	if (pRptChannel->heartBtInt > 0) {
		recvInterval = time((time_t*) NULL)
				- OesApi_GetLastRecvTime(pRptChannel);
		if (recvInterval > pRptChannel->heartBtInt * 2) {
			slog_error(0,
					"会话已超时, 将主动断开与服务器[%s:%d]的连接! lastRecvTime: [%d], lastSendTime: [%d], heartBtInt: [%d], recvInterval: [%d]",
					pRptChannel->channel.remoteAddr,
					pRptChannel->channel.remotePort,
					(int64 ) pRptChannel->lastRecvTime.tv_sec,
					(int64 ) pRptChannel->lastSendTime.tv_sec,
					pRptChannel->heartBtInt, recvInterval);
			return ETIMEDOUT;
		}
	}

	return 0;
}

/**
 * 回报采集处理 (可以做为线程的主函数运行)
 *
 * @param   pRptChannel     回报通道的会话信息
 * @return  TRUE 处理成功; FALSE 处理失败
 */
static void*
OesApiSample_ReportThreadMain(OesApiClientEnvT *pClientEnv) {
	static const int32 THE_TIMEOUT_MS = 1000;

	OesApiSessionInfoT *pRptChannel = &pClientEnv->rptChannel;
	volatile int32 *pThreadTerminatedFlag = &pRptChannel->__customFlag;
	int32 ret = 0;

	while (!*pThreadTerminatedFlag) {
		/* 等待回报消息到达, 并通过回调函数对消息进行处理 */
		ret = OesApi_WaitReportMsg(pRptChannel, THE_TIMEOUT_MS,
				_OesApiSample_HandleReportMsg, NULL);
		if (ret < 0) {
			if (SPK_IS_NEG_ETIMEDOUT(ret)) {
				/* 执行超时检查 (检查会话是否已超时) */
				if (_OesApiSample_OnTimeout(pClientEnv) == 0) {
					continue;
				}

				/* 会话已超时 */
				goto ON_ERROR;
			}

			if (SPK_IS_NEG_EPIPE(ret)) {
				/* 连接已断开 */
			}
			goto ON_ERROR;
		}
	}

	*pThreadTerminatedFlag = -1;
	return (void*) TRUE;

	ON_ERROR: *pThreadTerminatedFlag = -1;
	return (void*) FALSE;
}

void usage() {
	printf(
			"###########################################################################\n");
	printf(
			"#                  委托回报模块适配器-证券oes(v1.0)                                \n");
	printf("# -h help\n");
	printf("# -c[customer] 客户号\n");
	printf("# -t[type] 客户类型:1-普通交易;2-两融;3-期权;4-期货;5-黄金,默认普通交易\n");
	printf(
			"###########################################################################\n");
	exit(-1);
}

XInt oestrd(XChar *customer, XNum cpuid) {
	XInt iret = -1;
	XIdx lastIdx = -1;
	OesApiClientEnvT cliEnv = { NULLOBJ_OESAPI_CLIENT_ENV };
	int iRetryTimes = 0;
	int itradingday = 0;
	XCustT *pCustomer = NULL;
	XBool bClosed = false;
	XLongTime beginTime = 0;
	XLongTime lastHeartTime = 0, curHeartTime = 0;

	iret = XBindCPU(cpuid);
	if(iret)
	{
		slog_warn(0, "绑核失败[%d]", cpuid);
	}

	pCustomer = XFndVCustomerByKey(customer);

	l_pMonitor = XFndVMonitor();

	l_pMonitorTd = XFndVTdMonitor(pCustomer->customerId);
	if (NULL == l_pMonitorTd) {
		slog_error(0, "未获取到行情信息[%s]", customer);
		return (-1);
	}

	l_pMonitorMd = XFndVMdMonitor(pCustomer->exchid);

	INIT_OK:
	slog_info(3, "检查柜台是否初始化......");
	if (!l_pMonitorTd->isInitOK) {
		SPK_SLEEP_MS(10000);
		iRetryTimes++;
		slog_info(0, "等待10秒后重试%d......", iRetryTimes);
		if (iRetryTimes > 10) {
			return (-1);
		}
		goto INIT_OK;
	}

	SPK_SLEEP_MS(2000);

	g_pUser = pCustomer->customerId;

	/* 设置当前线程使用的登录用户名 */
	OesApi_SetThreadUsername(pCustomer->customerId);
	/*
	 * 设置当前线程使用的登录密码
	 * @note 如通过API接口设置，则可以不在配置文件中配置;
	 *  - 支持通过前缀指定密码类型, 如 md5:PASSWORD, txt:PASSWORD
	 */
	OesApi_SetThreadPassword(pCustomer->password);
	// OesApi_SetThreadPassword("md5:e10adc3949ba59abbe56e057f20f883e");
	/* 设置客户端本地的设备序列号 */
	OesApi_SetCustomizedDriverId(pCustomer->hd);

	OesApi_SetThreadEnvId(l_pMonitorTd->envno);

	slog_info(0, "设置硬盘序列号[%s],环境号[%d]", pCustomer->hd, l_pMonitorTd->envno);
	/*
	 * 2. 初始化客户端环境
	 *  - 一次性初始化多个通道时, 可通过如下接口完成初始化:
	 */
	{

		START_LOGIN:
		/* 初始化客户端环境 (配置文件参见: oes_client_sample.conf) */
		if (!OesApi_InitAll(&cliEnv, XOES_CONFIG_FILE,
		OESAPI_CFG_DEFAULT_SECTION_LOGGER,
		OESAPI_CFG_DEFAULT_SECTION,
		OESAPI_CFG_DEFAULT_KEY_ORD_ADDR,
		OESAPI_CFG_DEFAULT_KEY_RPT_ADDR,
		OESAPI_CFG_DEFAULT_KEY_QRY_ADDR, 0, (int32*) NULL)) {
			//如果未连接成功,等待10秒重试
			SPK_SLEEP_MS(10000);
			goto START_LOGIN;
		}
	}

	slog_info(0, "检查当前订单薄是否有非当日数据 ......");

	itradingday = OesApi_GetTradingDay(&(cliEnv.qryChannel));
	if (l_pMonitorTd->traday != 0 && l_pMonitorTd->traday != itradingday) {
		slog_info(0, "系统未初始化，请初始化后重试");
		goto ON_ERROR;
	}

	if(!CheckAuth(pCustomer->customerId, itradingday))
	{
		goto ON_ERROR;
	}


	l_pMonitorTd->iMaxLocalId =
			l_pMonitorTd->iMaxLocalId > cliEnv.ordChannel.lastOutMsgSeq ?
					l_pMonitorTd->iMaxLocalId : cliEnv.ordChannel.lastOutMsgSeq;

	slog_info(0, "#* 交易日[%d]当前最大报单序号[%d]", itradingday,
			cliEnv.ordChannel.lastOutMsgSeq);

	/* 3. 创建回报接收进程 */

	{
		pthread_t rptThreadId;
		int32 ret = 0;

		ret = pthread_create(&rptThreadId, NULL, (void*
		(*)(void*)) OesApiSample_ReportThreadMain, &cliEnv);
		if (unlikely(ret != 0)) {
			slog_error(0, "创建回报接收线程失败! error[%d]", ret);
			goto ON_ERROR;
		}
	}

	slog_info(0, "!!!!!! 开始处理委托订单-进度[%d/%d],等待一定时间，确保回报收完...... !!!!!!",
			l_pMonitorTd->iDealPos, l_pMonitor->iTOrder);

	XOrderT *pCurOrder = NULL;
	for (;;) {
		//收市后延迟5分钟关闭
		bClosed = isMktClosedTime(l_pMonitorMd->updateTime);
		if (bClosed) {
			if (!beginTime) {
				beginTime = XGetClockTime();
			}
			if (XGetClockTime() - beginTime > 5 * 60 * XTIMS_S4NS) {
				break;
			}
		}

		//保持心跳
		if ((curHeartTime = XGetClockTime())  > lastHeartTime + 10000000000LL) {
			iret = OesApi_SendHeartbeat(&(cliEnv.ordChannel));
			if (iret < 0) {
				slog_error(0, "链接已断开");
			}
			lastHeartTime = curHeartTime;
		}

		//已经处理的订单
		if (l_pMonitorTd->iDealPos >= l_pMonitor->iTOrder) {
			continue;
		}

		lastHeartTime = XGetClockTime();

		lastIdx = l_pMonitorTd->iDealPos + 1;
		pCurOrder = XFndVOrderById(lastIdx);

		if (NULL == pCurOrder) {
			continue;
		}

		//不是本柜台或本账户数据，跳过
		if (pCurOrder->counter != eXCounterOes
				|| strcmp(pCurOrder->request.customerId, g_pUser) != 0) {
			l_pMonitorTd->iDealPos = pCurOrder->idx;
			continue;
		}
		//未过风控或系统未启动处理的订单
		if (pCurOrder->exeStatus != eXExecRisk) {
			l_pMonitorTd->iDealPos = pCurOrder->idx;
			continue;
		}

		if (pCurOrder->request.isCancel == false) {
			//委托
			switch (pCurOrder->request.bsType) {
			case eXBuy:
			case eXSell:
			case eXDeem:
			case eXCSell:
				iret = _OesApiSample_SendOrderReq(&cliEnv.ordChannel,
						pCurOrder);
				l_pMonitorTd->iDealPos = pCurOrder->idx;
				break;

			default:
				break;
			}
		} else {
			iret = _OesApiSample_SendOrderCancelReq(&cliEnv.ordChannel,
					pCurOrder);
			l_pMonitorTd->iDealPos = pCurOrder->idx;
		}

	}

	/* 7. 通知并等待回报线程退出 (实际场景中请勿参考此部分代码) */
	{
		/* 等待回报消息接收完成 */
		SPK_SLEEP_MS(1000);

		/* 设置回报线程退出标志 */
		*((volatile int32*) &cliEnv.rptChannel.__customFlag) = 1;

		/* 回报线程将标志设置为-1后退出, 父进程再释放资源 */
		while (*((volatile int32*) &cliEnv.rptChannel.__customFlag) != -1) {
			SPK_SLEEP_MS(1000);
		}
	}

	/* 8. 发送注销消息, 并释放会话数据 */
	OesApi_LogoutAll(&cliEnv, TRUE);

	slog_info(0, "**************** 完毕 ************************\n");

	ON_ERROR:
	/* 直接关闭连接, 并释放会话数据 */
	OesApi_DestoryAll(&cliEnv);
	exit(0);
	return (iret);
}

XVoid XOesTrd(XVoid *param) {
	XCustT* pCust = NULL;

	slog_info(0, "XOesTrd启动......");

	pCust = (XCustT*) param;

	if(NULL == pCust)
	{
		return;
	}

	slog_debug(0, "启动[%s] oes交易进程", pCust->customerId);
	//多进程才行
	oestrd(pCust->customerId, pCust->cpuid);
	
	
}
