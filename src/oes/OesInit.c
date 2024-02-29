/*
 * @file OesInit.c
 * @brief 柜台初始化程序
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */
#include <getopt.h>

#include    "OesInit.h"
#include 	"XCom.h"
#include    "oes_api/oes_api.h"
#include    "sutil/time/spk_times.h"
#include    "XLog.h"

static char *g_pUser = NULL;

static XMonitorTdT *pMonitorTd = NULL;

FILE* fpstatic = NULL;

/**
 * 对现货产品查询返回的产品信息进行处理的回调函数
 *
 * @param   pSessionInfo    会话信息
 * @param   pMsgHead        消息头
 * @param   pMsgBody        消息体数据 @see OesStockItemT
 * @param   pCallbackParams 外部传入的参数
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static int32 _OesApiSample_OnQryStockCallback(OesApiSessionInfoT *pSessionInfo,
		SMsgHeadT *pMsgHead, void *pMsgBody, OesQryCursorT *pQryCursor,
		void *pCallbackParams) {
	OesStockItemT *pStockItem = (OesStockItemT*) pMsgBody;

	XStockT stock, *pStock = NULL;
	int iMarket = -1;
	int iret = -1;

	memset(&stock, 0, sizeof(XStockT));

	iMarket = market_from_oes(pStockItem->mktId);

	pStock = XFndVStockByKey(iMarket, pStockItem->securityId);

	if (NULL == pStock) {
		stock.market = iMarket;
		memcpy(stock.securityId, pStockItem->securityId,
				sizeof(pStockItem->securityId));

		memcpy(stock.securityName, pStockItem->securityName,
				sizeof(pStockItem->securityName));
		memcpy(stock.baseSecurityId, pStockItem->underlyingSecurityId,
				sizeof(pStockItem->underlyingSecurityId));
		pMonitorTd->iStockInfo++;
	} else {

		memcpy(&stock, pStock, sizeof(XStockT));
	}

	
	
	stock.secType = sectype_from_oes(pStockItem->securityType);
	stock.subSecType = subsecttype_from_oes(pStockItem->subSecurityType);

	stock.prdType = producttype_from_oes(pStockItem->productType,
			pStockItem->subSecurityType);

	stock.secStatus = secstatus_from_oes(pStockItem->securityStatus,
			pStockItem->suspFlag, pStockItem->temporarySuspFlag);

	stock.preClose = pStockItem->prevClose;
	stock.priceTick = pStockItem->priceTick;

	stock.openUpperPrice = pStockItem->priceLimit[OES_TRD_SESS_TYPE_O].upperLimitPrice; //涨跌停
	stock.openLowerPrice = pStockItem->priceLimit[OES_TRD_SESS_TYPE_O].lowerLimitPrice;  //涨跌停价

	if(stock.market == eXMarketSha && stock.subSecType != eXSubSecKSH && stock.subSecType != eXSubSecKCDR)
	{
		stock.lmtBuyMinQty = pStockItem->buyQtyUnit;
		stock.lmtSellMinQty = pStockItem->lmtSellMinQty;
		stock.lmtBuyMaxQty = pStockItem->lmtBuyMaxQty;
		stock.lmtSellMaxQty = pStockItem->lmtSellMaxQty;
		stock.buyUnit = pStockItem->lmtBuyMinQty;
		stock.sellUnit = pStockItem->sellQtyUnit;
	}
	else
	{
		stock.lmtBuyMinQty = pStockItem->lmtBuyMinQty;
		stock.lmtSellMinQty = pStockItem->lmtSellMinQty;
		stock.lmtBuyMaxQty = pStockItem->lmtBuyMaxQty;
		stock.lmtSellMaxQty = pStockItem->lmtSellMaxQty;
		stock.buyUnit = pStockItem->buyQtyUnit;
		stock.sellUnit = pStockItem->sellQtyUnit;
	}
	

	stock.upperPrice =
			pStockItem->priceLimit[OES_TRD_SESS_TYPE_T].upperLimitPrice; //涨跌停
	stock.lowerPrice =
			pStockItem->priceLimit[OES_TRD_SESS_TYPE_T].lowerLimitPrice;  //涨跌停价
	stock.outstandingShare = pStockItem->outstandingShare;           //总股本
	stock.publicfloatShare = pStockItem->publicFloatShare;           //流通股本
	stock.isDayTrading = pStockItem->isDayTrading;
	stock.maturityDate = pStockItem->maturityDate;

	iret = XPutOrUpdVStockByKey(&stock);
	if (iret) {
		slog_error(0, "静态数据:[%d-%s],idx[%lld]", stock.market, stock.securityId,
				stock.idx);
	}
	if ((pStockItem->securityStatus & OES_SECURITY_STATUS_FIRST_LISTING)
			== OES_SECURITY_STATUS_FIRST_LISTING
			|| (pStockItem->securityStatus
					& OES_SECURITY_STATUS_RESUME_FIRST_LISTING)
					== OES_SECURITY_STATUS_RESUME_FIRST_LISTING
			|| (pStockItem->securityStatus & OES_SECURITY_STATUS_NEW_LISTING)
					== OES_SECURITY_STATUS_NEW_LISTING) {
		slog_debug(0,
				"[%d-%s-%d] 新上市[%s], 昨收价[%.3f], 涨停价[%.3f-%.3f], 跌停价[%.3f-%.3f], 状态信息[%u-%u]，上市日期[%d]",
				stock.market, stock.securityId, stock.secType,
				stock.securityName, pStockItem->prevClose * XPRICE_DIV,
				stock.openUpperPrice * XPRICE_DIV,
				stock.upperPrice * XPRICE_DIV,
				stock.openLowerPrice * XPRICE_DIV,
				stock.lowerPrice * XPRICE_DIV, pStockItem->securityStatus,
				stock.secStatus, pStockItem->listDate);
	}

	int type = eDStock;
	fwrite(&type, sizeof(int), 1, fpstatic);
	fwrite(&stock, sizeof(XStockT), 1, fpstatic);

	if (pQryCursor->isEnd) {
		slog_info(1, "***** 当日交易证券数量[%d] *****\n", pMonitorTd->iStockInfo);
	}

	return 0;
}

/**
 * 查询现货产品信息
 *
 * @param   pQryChannel     查询通道的会话信息
 * @param   pSecurityId     产品代码
 * @param   mktId           市场代码
 * @param   securityType    证券类别
 * @param   subSecurityType 证券子类别
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static inline int32 _QueryStock(OesApiSessionInfoT *pQryChannel,
		const char *pSecurityId, uint8 mktId, uint8 securityType,
		uint8 subSecurityType) {
	OesQryStockFilterT qryFilter = { NULLOBJ_OES_QRY_STOCK_FILTER };
	int32 ret = 0;

	if (pSecurityId) {
		memcpy(qryFilter.securityId, pSecurityId, sizeof(qryFilter.securityId));
	}

	slog_info(1, "************** 查询证券基本信息 **************************");

	qryFilter.mktId = mktId;
	qryFilter.securityType = securityType;
	qryFilter.subSecurityType = subSecurityType;

	ret = OesApi_QueryStock(pQryChannel, &qryFilter,
			_OesApiSample_OnQryStockCallback, NULL);
	if (ret < 0) {
		slog_error(0,
				"Query stock failure! ret[%d], pSecurityId[%s], " "mktId[%u], " "securityType[%u], " "subSecurityType[%u]",
				ret, pSecurityId ? pSecurityId : "NULL", mktId, securityType,
				subSecurityType);
		return ret;
	} else {
		slog_info(1, "Query stock success! total count: [%d]", ret);
	}

	return 0;
}

/**
 * 对现货产品查询返回的产品信息进行处理的回调函数
 *
 * @param   pSessionInfo    会话信息
 * @param   pMsgHead        消息头
 * @param   pMsgBody        消息体数据 @see OesStockItemT
 * @param   pCallbackParams 外部传入的参数
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static int32 _OesApiSample_OnQryIssueCallback(OesApiSessionInfoT *pSessionInfo,
		SMsgHeadT *pMsgHead, void *pMsgBody, OesQryCursorT *pQryCursor,
		void *pCallbackParams) {
	OesIssueItemT *pStockItem = (OesIssueItemT*) pMsgBody;
	XIssueT issue, *pIssue = NULL;
	int iMarket;

	memset(&issue, 0, sizeof(XIssueT));

	iMarket = market_from_oes(pStockItem->mktId);

	pIssue = XFndVIssueByKey(iMarket, pStockItem->securityId);
	if (NULL == pIssue) {
		issue.market = iMarket;
		memcpy(issue.securityId, pStockItem->securityId,
				sizeof(pStockItem->securityId));

		memcpy(issue.securityName, pStockItem->securityName,
				sizeof(pStockItem->securityName));
		pMonitorTd->iIssue++;
	} else {
		memcpy(&issue, pIssue, sizeof(XIssueT));
	}

	issue.prdType = producttype_from_oes(pStockItem->productType, -1);
	issue.secType = sectype_from_oes(pStockItem->securityType);
	issue.subSecType = subsecttype_from_oes(pStockItem->subSecurityType);
	issue.issuePrice = pStockItem->issuePrice;
	issue.issueType = issuetype_from_oes(pStockItem->productType,
			pStockItem->issueType);
	issue.minQty = pStockItem->ordMinQty;
	issue.maxQty = pStockItem->ordMaxQty;
	issue.qtyUnit = pStockItem->qtyUnit;

	XPutOrUpdVIssueByKey(&issue);

	int type = eDIssue;
	fwrite(&type, sizeof(int), 1, fpstatic);
	fwrite(&issue, sizeof(XIssueT), 1, fpstatic);

	slog_debug(0, "证券代码:[%d-%s], 发行方式:[%u], 发行价:[%.3f]", issue.market,
			issue.securityId, issue.issueType, issue.issuePrice * XPRICE_DIV);

	if (pQryCursor->isEnd) {
		slog_info(1, "**** 发行数量[%d] *****\n", pMonitorTd->iIssue);
	}

	return 0;
}

/**
 * 查询现货产品信息
 *
 * @param   pQryChannel     查询通道的会话信息
 * @param   pSecurityId     产品代码
 * @param   mktId           市场代码
 * @param   securityType    证券类别
 * @param   subSecurityType 证券子类别
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static inline int32 _QueryIssue(OesApiSessionInfoT *pQryChannel,
		const char *pSecurityId, uint8 mktId, uint8 productType) {
	OesQryIssueFilterT qryFilter = { NULLOBJ_OES_QRY_ISSUE_FILTER };
	int32 ret = 0;

	if (pSecurityId) {
		memcpy(qryFilter.securityId, pSecurityId, sizeof(qryFilter.securityId));
	}

	slog_info(1,
			"******************** 查询证券发行 *********************************");

	qryFilter.mktId = mktId;
	qryFilter.productType = productType;

	ret = OesApi_QueryIssue(pQryChannel, &qryFilter,
			_OesApiSample_OnQryIssueCallback, NULL);
	if (ret < 0) {
		slog_error(0,
				"Query stock failure! ret[%d], pSecurityId[%s], " "mktId[%u], " "productType[%u], ",
				ret, pSecurityId ? pSecurityId : "NULL", mktId, productType);
		return ret;
	} else {
		slog_debug(0, "Query stock success! total count: [%d]", ret);
	}

	return 0;
}

/**
 * 对资金查询返回的资金信息进行处理的回调函数
 *
 * @param   pSessionInfo    会话信息
 * @param   pMsgHead        消息头
 * @param   pMsgBody        消息体数据 @see OesCashAssetItemT
 * @param   pCallbackParams 外部传入的参数
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static int32 _OesApiSample_OnQryCashAssetCallback(
		OesApiSessionInfoT *pSessionInfo, SMsgHeadT *pMsgHead, void *pMsgBody,
		OesQryCursorT *pQryCursor, void *pCallbackParams) {
	OesCashAssetItemT *pCashAssetItem = (OesCashAssetItemT*) pMsgBody;
	XCashT cash, *pCash = NULL;

	memset(&cash, 0, sizeof(XCashT));

	cash.acctType = accttype_from_oes(pCashAssetItem->cashType);

	pCash = XFndVCashByKey(pCashAssetItem->custId, cash.acctType);
	if (NULL == pCash) {
		if (g_pUser != NULL && strlen(g_pUser) != 0) {
			memcpy(cash.customerId, g_pUser, strlen(g_pUser));
		} else {
			memcpy(cash.customerId, pCashAssetItem->custId,
					sizeof(pCashAssetItem->custId));
		}
		memcpy(cash.accountId, pCashAssetItem->cashAcctId,
				sizeof(pCashAssetItem->cashAcctId));
		
		pMonitorTd->iCashAsset++;
	} else {
		memcpy(&cash, pCash, sizeof(XCashT));
	}

	cash.beginBalance = pCashAssetItem->beginningBal;
	cash.beginAvailable = pCashAssetItem->beginningAvailableBal;
	cash.beginDrawable = pCashAssetItem->beginningDrawableBal;

	cash.frozenAmt = pCashAssetItem->buyFrzAmt;
	cash.curAvailable = pCashAssetItem->currentAvailableBal;
	cash.countAvailable = pCashAssetItem->currentAvailableBal;
	cash.balance = cash.countAvailable + cash.frozenAmt;
	cash.totalBuy = pCashAssetItem->totalBuyAmt;
	cash.totalSell = pCashAssetItem->totalSellAmt;
	cash.totalFee = pCashAssetItem->totalFeeAmt;
	cash.locFrz = 0;
	XPutOrUpdVCashByKey(&cash);

	int type = eDCash;
	fwrite(&type, sizeof(int), 1, fpstatic);
	fwrite(&cash, sizeof(XCashT), 1, fpstatic);

	slog_debug(0, "资金账号[%s], 账户类型[%u],可用余额[%.2f]", cash.accountId, cash.acctType,
			cash.curAvailable * XPRICE_DIV);
	if (pQryCursor->isEnd) {
		slog_info(1, "**** 资金账户数量[%d] *****\n", pMonitorTd->iCashAsset);
		return 0;
	}

	return 0;
}

/**
 * 查询资金
 *
 * @param   pQryChannel     查询通道的会话信息
 * @param   pCashAcctId     资金账户代码
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static inline int32 _QueryCashAsset(OesApiSessionInfoT *pQryChannel,
		const char *pCashAcctId) {
	OesQryCashAssetFilterT qryFilter = { NULLOBJ_OES_QRY_CASH_ASSET_FILTER };
	int32 ret = 0;

	if (pCashAcctId) {
		memcpy(qryFilter.cashAcctId, pCashAcctId, sizeof(qryFilter.cashAcctId));
	}

	slog_info(1, "************* 查询资金 *****************");
	ret = OesApi_QueryCashAsset(pQryChannel, &qryFilter,
			_OesApiSample_OnQryCashAssetCallback, NULL);
	if (ret < 0) {
		slog_error(0, "Query cash asset failure! " "ret[%d], pCashAcctId[%s]",
				ret, pCashAcctId ? pCashAcctId : "NULL");
		return ret;
	} else {
		slog_debug(0, "Query cash asset success! total count: [%d]", ret);
	}

	return 0;
}

/**
 * 对资金查询返回的资金信息进行处理的回调函数
 *
 * @param   pSessionInfo    会话信息
 * @param   pMsgHead        消息头
 * @param   pMsgBody        消息体数据 @see OesInvAcctItemT
 * @param   pCallbackParams 外部传入的参数
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static int32 _OesApiSample_OnQryInvAcctCallback(
		OesApiSessionInfoT *pSessionInfo, SMsgHeadT *pMsgHead, void *pMsgBody,
		OesQryCursorT *pQryCursor, void *pCallbackParams) {
	OesInvAcctItemT *pInvAcctItem = (OesInvAcctItemT*) pMsgBody;
	XInvestT invest, *pInvest = NULL;
	int market;

	memset(&invest, 0, sizeof(XInvestT));
	market = market_from_oes(pInvAcctItem->mktId);

	pInvest = XFndVInvestByKey(pInvAcctItem->custId, market,
			pInvAcctItem->invAcctId);
	if (NULL == pInvest) {
		invest.market = market;
		if (g_pUser != NULL && strlen(g_pUser) != 0) {
			memcpy(invest.customerId, g_pUser, strlen(g_pUser));
		} else {
			memcpy(invest.customerId, pInvAcctItem->custId,
					sizeof(pInvAcctItem->custId));
		}
		memcpy(invest.investId, pInvAcctItem->invAcctId,
				sizeof(pInvAcctItem->invAcctId));
		pMonitorTd->iInvest++;
	} else {
		memcpy(&invest, pInvest, sizeof(XInvestT));
	}

	invest.acctType = accttype_from_oes(pInvAcctItem->acctType);
	invest.mainQuota = pInvAcctItem->subscriptionQuota;
	invest.kcQuota = pInvAcctItem->kcSubscriptionQuota;

	XPutOrUpdVInvestByKey(&invest);
	slog_debug(0, "客户号:[%s], 股东帐户:[%s-%d], 市场:[%d], 申购额度:[%d]",
			invest.customerId, invest.investId, invest.acctType, invest.market,
			invest.mainQuota);

	int type = eDInvest;
	fwrite(&type, sizeof(int), 1, fpstatic);
	fwrite(&invest, sizeof(XInvestT), 1, fpstatic);

	if (pQryCursor->isEnd) {
		slog_info(1, "**** 投资者账户数量[%d] *****\n", pMonitorTd->iInvest);
		return 0;
	}

	return 0;
}

/**
 * 查询股东帐户
 *
 * @param   pQryChannel     查询通道的会话信息
 * @param   pCashAcctId     资金账户代码
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static inline int32 _QueryInvAcct(OesApiSessionInfoT *pQryChannel,
		const char *pInvAcctId) {
	OesQryInvAcctFilterT qryFilter = { NULLOBJ_OES_QRY_INV_ACCT_FILTER };
	int32 ret = 0;

	if (pInvAcctId) {
		memcpy(qryFilter.invAcctId, pInvAcctId, sizeof(qryFilter.invAcctId));
	}
	slog_info(1, "************** 查询投资者账户 **********************");
	ret = OesApi_QueryInvAcct(pQryChannel, &qryFilter,
			_OesApiSample_OnQryInvAcctCallback, NULL);
	if (ret < 0) {
		slog_error(0, "Query inv acct failure! " "ret[%d], pInvAcctId[%s]", ret,
				pInvAcctId ? pInvAcctId : "NULL");
		return ret;
	} else {
		slog_debug(0, "Query inv acct success! total count: [%d]", ret);
	}

	return 0;
}

/**
 * 对股票持仓查询返回的持仓信息进行处理的回调函数
 *
 * @param   pSessionInfo    会话信息
 * @param   pMsgHead        消息头
 * @param   pMsgBody        消息体数据 @see OesStkHoldingItemT
 * @param   pCallbackParams 外部传入的参数
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static int32 _OesApiSample_OnQryStkHoldingCallback(
		OesApiSessionInfoT *pSessionInfo, SMsgHeadT *pMsgHead, void *pMsgBody,
		OesQryCursorT *pQryCursor, void *pCallbackParams) {
	OesStkHoldingItemT *pHoldingItem = (OesStkHoldingItemT*) pMsgBody;
	XHoldT hold, *pHold = NULL;
	int iMarket = -1;

	memset(&hold, 0, sizeof(XHoldT));

	iMarket = market_from_oes(pHoldingItem->mktId);

	pHold = XFndVHoldByKey(g_pUser, pHoldingItem->invAcctId, iMarket,
			pHoldingItem->securityId);
	if (NULL == pHold) {
		if (g_pUser != NULL && strlen(g_pUser) != 0) {
			memcpy(hold.customerId, g_pUser, strlen(g_pUser));
		}
		hold.market = iMarket;
		memcpy(hold.securityId, pHoldingItem->securityId,
				sizeof(pHoldingItem->securityId));
		memcpy(hold.investId, pHoldingItem->invAcctId,
				sizeof(pHoldingItem->invAcctId));
		pMonitorTd->iHolding++;
	} else {
		memcpy(&hold, pHold, sizeof(XHoldT));
	}

	hold.locFrz = 0;
	hold.countSellAvlHld = pHoldingItem->sellAvlHld;
	hold.sellAvlHld = hold.countSellAvlHld + hold.locFrz;
	hold.orgHld = pHoldingItem->originalHld;
	hold.orgAvlHld = pHoldingItem->originalAvlHld;
	hold.orgCostAmt = pHoldingItem->originalCostAmt;
	hold.totalBuyHld = pHoldingItem->totalBuyHld;
	hold.totalSellHld = pHoldingItem->totalSellHld;
	hold.totalBuyAmt = pHoldingItem->totalBuyAmt;
	hold.totalSellAmt = pHoldingItem->totalSellAmt;
	hold.sellFrz = pHoldingItem->sellFrzHld;
	hold.etfAvlHld = pHoldingItem->trsfOutAvlHld;
	hold.costPrice = pHoldingItem->costPrice;
	hold.sumHld = pHoldingItem->sumHld;

	XPutOrUpdVHoldByKey(&hold);
	slog_debug(0, "invAcctId[%s], mktId[%u], securityId[%s], sellAvlHld[%d]}",
			pHoldingItem->invAcctId, pHoldingItem->mktId,
			pHoldingItem->securityId, pHoldingItem->sellAvlHld);

	int type = eDHold;
	fwrite(&type, sizeof(int), 1, fpstatic);
	fwrite(&hold, sizeof(XHoldT), 1, fpstatic);

	if (pQryCursor->isEnd) {
		slog_info(3, "***** 当日证券持仓条数[%d] *****\n", pMonitorTd->iHolding);
	}

	return 0;
}

/**
 * 查询股票持仓
 *
 * @param   pQryChannel     查询通道的会话信息
 * @param   mktId           市场代码 @see eOesMarketIdT
 * @param   pSecurityId     股票代码 (char[6]/char[8])
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static inline int32 _QueryStkHolding(OesApiSessionInfoT *pQryChannel,
		uint8 mktId, const char *pSecurityId) {
	OesQryStkHoldingFilterT qryFilter = { NULLOBJ_OES_QRY_STK_HOLDING_FILTER };
	int32 ret = 0;

	qryFilter.mktId = mktId;
	if (pSecurityId) {
		memcpy(qryFilter.securityId, pSecurityId, sizeof(qryFilter.securityId));
	}

	slog_info(1, "******************* 查询持仓 ****************************");
	ret = OesApi_QueryStkHolding(pQryChannel, &qryFilter,
			_OesApiSample_OnQryStkHoldingCallback, NULL);
	if (ret < 0) {
		slog_error(0,
				"Query stock holding failure! " "ret[%d], mktId[%u], pSecurityId[%s]",
				ret, mktId, pSecurityId ? pSecurityId : "NULL");
		return ret;
	} else {
		slog_debug(0, "Query stock holding success! total count: [%d]", ret);
	}

	return 0;
}

/**
 * 对市场状态查询返回的市场状态信息进行处理的回调函数
 *
 * @param   pSessionInfo    会话信息
 * @param   pMsgHead        消息头
 * @param   pMsgBody        消息体数据 @see OesMarketStateItemT
 * @param   pCallbackParams 外部传入的参数
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static int32 _OesApiSample_OnQryMarketStateCallback(
		OesApiSessionInfoT *pSessionInfo, SMsgHeadT *pMsgHead, void *pMsgBody,
		OesQryCursorT *pQryCursor, void *pCallbackParams) {
	OesMarketStateItemT *pMktStateItem = (OesMarketStateItemT*) pMsgBody;

	printf(">>> Recv QryMktStatusRsp: {index[%d], isEnd[%c], "
			"exchId[%u], platformId[%u], "
			"mktId[%u], mktState[%u]}\n", pQryCursor->seqNo,
			pQryCursor->isEnd ? 'Y' : 'N', pMktStateItem->exchId,
			pMktStateItem->platformId, pMktStateItem->mktId,
			pMktStateItem->mktState);

	return 0;
}

/**
 * 查询市场状态
 *
 * @param   pQryChannel     查询通道的会话信息
 * @param   exchId          交易所代码 @see eOesExchangeIdT
 * @param   platformId      交易平台类型 @see eOesPlatformIdT
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
static inline int32 _OesApiSample_QueryMarketStatus(
		OesApiSessionInfoT *pQryChannel, uint8 exchId, uint8 platformId) {
	OesQryMarketStateFilterT qryFilter = { NULLOBJ_OES_QRY_MARKET_STATE_FILTER };
	int32 ret = 0;

	qryFilter.exchId = exchId;
	qryFilter.platformId = platformId;

	ret = OesApi_QueryMarketState(pQryChannel, &qryFilter,
			_OesApiSample_OnQryMarketStateCallback, NULL);
	if (ret < 0) {
		slog_error(0,
				"Query market state failure! " "ret[%d], exchId[%u], " "platformId[%u]",
				ret, exchId, platformId);
		return ret;
	} else {
		slog_debug(0, "Query market state success! total count: [%d]", ret);
	}

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

	assert(pRptChannel && pMsgHead && pRspMsg);

	switch (pMsgHead->msgId) {
	case OESMSG_RPT_ORDER_INSERT: /* OES委托已生成 (已通过风控检查) @see OesOrdCnfmT */

		break;

	case OESMSG_RPT_BUSINESS_REJECT: /* OES业务拒绝 (未通过风控检查等) @see OesOrdRejectT */

		break;

	case OESMSG_RPT_ORDER_REPORT: /* 交易所委托回报 (包括交易所委托拒绝、委托确认和撤单完成通知) @see OesOrdCnfmT */

		break;

	case OESMSG_RPT_TRADE_REPORT: /* 交易所成交回报 @see OesTrdCnfmT */

		break;

	case OESMSG_RPT_CASH_ASSET_VARIATION: /* 资金变动信息 @see OesCashAssetItemT */

		break;

	case OESMSG_RPT_STOCK_HOLDING_VARIATION: /* 持仓变动信息 (股票) @see OesStkHoldingItemT */

		break;

	case OESMSG_RPT_OPTION_HOLDING_VARIATION: /* 持仓变动信息 (期权) @see OesOptHoldingItemT */

		break;

	case OESMSG_RPT_FUND_TRSF_REJECT: /* 出入金委托响应-业务拒绝 @see OesFundTrsfRejectT */

		break;

	case OESMSG_RPT_FUND_TRSF_REPORT: /* 出入金委托执行报告 @see OesFundTrsfReportT */

		break;

	case OESMSG_RPT_REPORT_SYNCHRONIZATION: /* 回报同步响应 @see OesReportSynchronizationRspT */

		break;

	case OESMSG_RPT_MARKET_STATE: /* 市场状态信息 @see OesMarketStateInfoT */

		break;

	case OESMSG_SESS_HEARTBEAT:

		break;

	default:

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
					"会话已超时, 将主动断开与服务器[%s:%d]的连接! " "lastRecvTime: [%d], " "lastSendTime: [%d], " "heartBtInt: [%d], recvInterval: [%d]",
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
static inline void*
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

XInt XOesInit(char* customer) {
	XInt iret = -1;
	int itradingday = -1;
	int market = 0;

	OesApiClientEnvT cliEnv = { NULLOBJ_OESAPI_CLIENT_ENV };
	XCustT *pCustomer = NULL;

	xslog_init(XSHM_SDB_FILE, "oesinit");

	XManShmLoad();

	slog_info(1, "# 检查该用户和柜台类型是否匹配......");
	pCustomer = XFndVCustomerByKey(customer);
	if (NULL == pCustomer) {
		slog_error(0, "请确认对应的用户[%s]与使用的柜台系统<>当前适配器[%d]是否匹配?", customer,
				eXCounterOes);
		return (-1);
	}

	pMonitorTd = XFndVTdMonitor(pCustomer->customerId);

	if (NULL == pMonitorTd) {
		slog_error(0, "未找到监控数据,退出");
		return (-1);
	}
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
	OesApi_SetCustomizedDriverId("ABCDEFGHIJKLMN");

	if (!OesApi_InitLogger(XOES_CONFIG_FILE,
	OESAPI_CFG_DEFAULT_SECTION_LOGGER)) {
		slog_error(0, "初始化日志模块失败");
		goto ON_ERROR;
	}

	//开始登录查询通道,如果10秒未联通重试
	START_LOGIN: if (!OesApi_InitQryChannel(&(cliEnv.qryChannel),
	XOES_CONFIG_FILE,
	OESAPI_CFG_DEFAULT_SECTION,
	OESAPI_CFG_DEFAULT_KEY_QRY_ADDR)) {
		SPK_SLEEP_MS(10000);
		goto START_LOGIN;
	}

	slog_info(1, "# 检查交易日是否匹配 ......");
	itradingday = OesApi_GetTradingDay(&(cliEnv.qryChannel));
	slog_info(1, "* 当前交易日为:%d %d", itradingday, pMonitorTd->traday);
	//增加对交易日的检查，如果为当前交易日，不允许直接初始化
	if (pMonitorTd->traday == itradingday) {
		slog_info(0, "已经初始化完毕,请勿重复初始化");
		//系统已经初始化完毕
		goto ON_ERROR;

	} else if (pMonitorTd->traday != 0
			&& pMonitorTd->traday != itradingday) {
		slog_warn(0, "系统未初始化，请初始化后重试");
		goto ON_ERROR;
	}

	pMonitorTd->traday = itradingday;
	slog_info(3, "* 当前交易日为:%d %d", itradingday, pMonitorTd->traday);

	fpstatic = fopen(XMAN_DATA_STATIC, "wb");

	{
		slog_info(1, "# 查询 所有关联资金账户的资金信息 ......");
		_QueryCashAsset(&cliEnv.qryChannel, NULL);

		slog_info(1, "# 查询股东帐户 ......");
		_QueryInvAcct(&cliEnv.qryChannel, NULL);

		slog_info(1, "# 查询证券基本信息 ......");
		if (market == eXMarketSha) {
			_QueryStock(&cliEnv.qryChannel, NULL, OES_MKT_ID_SH_A,
					OES_SECURITY_TYPE_UNDEFINE, OES_SUB_SECURITY_TYPE_UNDEFINE);
		} else if (market == eXMarketSza) {
			_QueryStock(&cliEnv.qryChannel, NULL, OES_MKT_ID_SZ_A,
					OES_SECURITY_TYPE_UNDEFINE, OES_SUB_SECURITY_TYPE_UNDEFINE);
		} else {
			_QueryStock(&cliEnv.qryChannel, NULL, OES_MKT_ID_UNDEFINE,
					OES_SECURITY_TYPE_UNDEFINE, OES_SUB_SECURITY_TYPE_UNDEFINE);
		}
		slog_info(1, "# 查询发信信息 ......");
		_QueryIssue(&cliEnv.qryChannel, NULL, OES_MKT_ID_UNDEFINE,
				OES_PRODUCT_TYPE_UNDEFINE);

		_QueryIssue(&cliEnv.qryChannel, NULL, OES_MKT_ID_UNDEFINE,
				OES_PRODUCT_TYPE_ALLOTMENT);

		slog_info(1, "# 查询 沪深两市 所有股票持仓 ......");

		_QueryStkHolding(&cliEnv.qryChannel, OES_MKT_ID_UNDEFINE,
		NULL);

		pMonitorTd->isInitOK = true;
		slog_info(1, "# 初始化完成后才能进行后续动作[%d][%d] ......", pMonitorTd->traday,
				pMonitorTd->isInitOK);

	}

	fclose(fpstatic);

	/* 发送注销消息, 并释放会话数据 */
	OesApi_LogoutAll(&cliEnv, TRUE);

	slog_info(1,
			"************************************ 完毕 ***************************\n");
	return 0;

	ON_ERROR:

	/* 直接关闭连接, 并释放会话数据 */
	OesApi_DestoryAll(&cliEnv);

	slog_info(0,
			"*********************************** 异常退出 ************************\n");

	return (iret);
}
