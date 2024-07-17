/*
 * @file XBackTest.c
 * @brief 高频回测框架
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */
#include <getopt.h>
#include "XCom.h"
#include "XBus.h"
#include "XTimes.h"
#include "XLog.h"
#include "XStore.h"
#ifdef __XMAN_FAST_REBUILD__
#include "XShFastRebuild.h"
#include "XSzFastRebuild.h"
#else
#include "XShRebuild.h"
#include "XSzRebuild.h"
#endif
#include "XINI.h"
#include "XOrdeng.h"
#include "XExport.h"

typedef struct _XMktSimParm {
	XInt market; /**< 市场 */
	XInt speed; /**< 多少笔 */
	XInt ms; /**< 毫秒 */
	XNum channel; /**< 频道 */
	XSecurityId securityId; /**< 证券代码 */
	XInt cpu; /**< 绑定的CPU */
	XChar bckFile[256];
	XSeqNum breakBiz; /**< 断点位置 */
} XMktSimParmT;

static XIdx g_OrderIdx = 1;

static void read_static(const char *staticfile) {
	FILE *fp = NULL;
	XCashT cash;
	XInvestT invest;
	XHoldT hold;
	XIssueT issue;
	XStockT stock;
	XInt i = 0;
	XMonitorT *pMonitor = NULL;
	XCustT *pCust = NULL;

	int type = -1;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(staticfile, "rb+");
	if (NULL == fp) {
		slog_error(0, "文件[%s]找不到", staticfile);
		exit(-1);
	}

	while (!feof(fp)) {
		fread(&type, sizeof(int), 1, fp);

		switch (type) {
		case eDCash:
			memset(&cash, 0, sizeof(XCashT));
			fread(&cash, sizeof(XCashT), 1, fp);
			for (i = 0; i < pMonitor->iTCust; i++) {
				pCust = XFndVCustomerById(i + 1);
				if (NULL == pCust || pCust->counter == eXUserMarket
						|| !strlen(cash.accountId)) {
					continue;
				}
				memcpy(cash.customerId, pCust->customerId, CUSTOMERID_LEN);
				cash.idx = 0;
				/** 更改资金 */
				cash.beginBalance = 1000000000000;
				cash.beginAvailable = 1000000000000;
				cash.beginDrawable = 1000000000000;
				cash.frozenAmt = 0;
				cash.curAvailable = 1000000000000;
				cash.countAvailable = 1000000000000;
				cash.balance = cash.countAvailable + cash.frozenAmt;

				slog_info(3, "资金：[%s-%d]可用资金[%.2f]", cash.customerId,
						cash.acctType, cash.curAvailable / 10000.0);
				XPutOrUpdVCashByKey(&cash);
				cash.idx = 1;		//for backtest
				XPutOrUpdVBCashByKey(&cash);
			}
			break;
		case eDInvest:
			memset(&invest, 0, sizeof(XInvestT));
			fread(&invest, sizeof(XInvestT), 1, fp);
			for (i = 0; i < pMonitor->iTCust; i++) {
				pCust = XFndVCustomerById(i + 1);
				if (NULL == pCust || pCust->counter == eXUserMarket
						|| !invest.market) {
					continue;
				}
				memcpy(invest.customerId, pCust->customerId,
				CUSTOMERID_LEN);
				invest.idx = 0;
				slog_info(3, "股东账户：[%s-%s]", invest.customerId, invest.investId);
				XPutOrUpdVInvestByKey(&invest);
			}
			break;
		case eDHold:
			memset(&hold, 0, sizeof(XHoldT));
			fread(&hold, sizeof(XHoldT), 1, fp);
			for (i = 0; i < pMonitor->iTCust; i++) {
				pCust = XFndVCustomerById(i + 1);
				if (NULL == pCust || pCust->counter == eXUserMarket
						|| !hold.market) {
					continue;
				}
				hold.idx = 0;
				memcpy(hold.customerId, pCust->customerId, CUSTOMERID_LEN);
				slog_info(3, "持仓：[%s-%s],[%d-%s],持仓总量[%lld]", hold.customerId,
						hold.investId, hold.market, hold.securityId,
						hold.sumHld);
				XPutOrUpdVHoldByKey(&hold);

				XPutOrUpdBHold(&hold); // TODO Idx
			}
			break;
		case eDIssue:
			memset(&issue, 0, sizeof(XIssueT));
			fread(&issue, sizeof(XIssueT), 1, fp);
			issue.idx = 0;
			if (strlen(issue.securityId)) {
				XPutOrUpdVIssueByKey(&issue);
			}
			break;
		case eDStock:
			memset(&stock, 0, sizeof(XStockT));
			fread(&stock, sizeof(XStockT), 1, fp);
			stock.idx = 0;
			if (strlen(stock.securityId)) {
				XPutOrUpdVStockByKey(&stock);
			}
			break;
		default:
			break;
		}

	}

	fclose(fp);
}

static void SimMkt(XVoid *param) {
	FILE *fp = NULL;
	XMonitorMdT *pMonitorMd = NULL;
	XL2LT l2l;
	XMktSimParmT *mktSimParm = NULL;
	XInt iCount = 0;

	mktSimParm = (XMktSimParmT*) param;

	if (NULL == mktSimParm) {
		return;
	}

	pMonitorMd = XFndVMdMonitor(eXExchSec);

	if (NULL == pMonitorMd) {
		slog_error(0, "获取行情信息错误");
		return;
	}

	fp = fopen(mktSimParm->bckFile, "rb");
	if (NULL == fp) {
		slog_error(0, "文件[%s]找不到", mktSimParm->bckFile);
		exit(-1);
	}
	slog_debug(0, "开始回放行情......");
	while (!feof(fp)) {
		if (pMonitorMd->isRunning != true) {
			usleep(100);
			continue;
		}

		iCount++;
		if (mktSimParm->speed && iCount % mktSimParm->speed == 0) {
			usleep(mktSimParm->ms);
			iCount = 0;
		}

		fread(&l2l, sizeof(XL2LT), 1, fp);

		switch (l2l.head.type) {
		case eMSnapshot:
			l2l.snapshot._recvTime = XGetClockTime();
			l2l.head.market = l2l.snapshot.market;

			XPushCache(XSHMKEYCONECT(mktCache), &l2l);

			break;
		}

	}

	fclose(fp);
	slog_debug(0, "!!! 回放行情结束 !!!");
	exit(0);
}
static XInt _HandleInsert(XOrderT *pOrder) {
	XTradeCache cache = { 0 };
	//已经收到交易所响应的订单
	XOrderT order;

	pOrder->_sendTime = XGetComMSec();
	pOrder->_cnfTime = XGetComMSec();

	pOrder->_cnfLocTime = XGetClockTime();
	//根据订单进行资金冻结
//	pOrder->frzAmt = pOrder->request.ordPrice * pOrder->request.ordQty;
	pOrder->frzFee = 0;
	pOrder->errorno = 0;

	memcpy(&order, pOrder, sizeof(XOrderT));
	order.idx = 0;
	cache.head.type = eDOrder;
	cache.head.dataLen = XORDER_SIZE;
	cache.ordrsp = order;

	XPushCache(XSHMKEYCONECT(tradeCache), &cache);

	return 0;
}

static XInt _HandleReject(XOrderT *pOrder) {

	XTradeCache cache = { 0 };
	XOrderT order;

	pOrder->_cnfTime = XGetComMSec();
	pOrder->_cnfLocTime = XGetClockTime();

	memcpy(&order, pOrder, sizeof(XOrderT));
	order.idx = 0;

	cache.head.type = eDOrder;
	cache.head.dataLen = XORDER_SIZE;
	cache.ordrsp = order;

	XPushCache(XSHMKEYCONECT(tradeCache), &cache);

	return 0;
}

static XInt _HandleCnfm(XOrderT *pOrder) {
	XTradeCache cache = { 0 };
	XOrderT order;

	pOrder->_cnfExTime = XGetClockTime();

	memcpy(&order, pOrder, sizeof(XOrderT));
	order.idx = 0;

	cache.head.type = eDOrder;
	cache.head.dataLen = XORDER_SIZE;
	cache.ordrsp = order;

	XPushCache(XSHMKEYCONECT(tradeCache), &cache);

	slog_debug(0,
			"OnRtnOrder:[%d-%s] 委托序号[%d],买卖方向[%d],委托数量[%d],成交数量[%d],成交金额[%.2f]",
			pOrder->request.market, pOrder->request.securityId,
			pOrder->request.localId, pOrder->request.bsType,
			pOrder->request.ordQty, pOrder->trdQty, pOrder->trdMoney * 0.0001);
	return 0;
}

static inline XInt _HandleTrade(XOrderT *pOrder, XQty trdQty, XPrice trdPrice) {
	XTradeT trade = { 0 };
	XTradeCache cache = { 0 };

	trade.ordid = pOrder->ordid;
	trade.market = pOrder->request.market;
	memcpy(trade.securityId, pOrder->request.securityId, SECURITYID_LEN);
	memcpy(trade.investId, pOrder->request.investId, INVESTID_LEN);
	trade.trdId = XGetIdByType(eSBlkSec);

	memcpy(trade.customerId, pOrder->request.customerId,
			strlen(pOrder->request.customerId));

	trade.trdQty = trdQty;
	trade.trdAmt = trdQty * trdPrice;
	trade.trdPrice = trdPrice;
	trade.trdTime = XGetComMSec();

	trade.cumQty = pOrder->trdQty;
	trade.cumAmt = pOrder->trdMoney;

	trade.trdSide = pOrder->request.bsType;

	trade.counter = eXCounterOes;

	trade.idx = 0;

	slog_debug(0,
			"OnRtnTrade:[%d-%s] 委托序号[%d],买卖方向[%d],委托数量[%d],成交数量[%d],成交价格[%.3f]",
			trade.market, trade.securityId, pOrder->request.localId,
			pOrder->request.bsType, pOrder->request.ordQty, trade.trdQty,
			trade.trdPrice * 0.0001);
	cache.head.type = eDTrade;
	cache.head.dataLen = XTRADE_SIZE;
	cache.trade = trade;

	XPushCache(XSHMKEYCONECT(tradeCache), &cache);

	return 0;
}

//持仓或资金应该是模拟交易所的TODO
static XInt _HandleHolding(XOrderT *pOrder, XInt isTrade, XQty trdQty,
		XPrice trdPrice) {
	XHoldT hold = { 0 }, *pHold = NULL;
	XTradeCache cache = { 0 };
	XStockT *pStock = NULL;

	pStock = XFndVStockByKey(pOrder->request.market,
			pOrder->request.securityId);
	if (NULL == pStock) {
		return (-1);
	}

	pHold = XFndVBHoldByKey(pOrder->request.customerId,
			pOrder->request.investId, pOrder->request.market,
			pOrder->request.securityId);

	//找到原始持仓，如果没有新添加持仓
	switch (pOrder->request.bsType) {
	case eXBuy:
		if (NULL == pHold) {
			memcpy(hold.customerId, pOrder->request.customerId,
					strlen(pOrder->request.customerId));
			memcpy(hold.investId, pOrder->request.investId, INVESTID_LEN);
			hold.market = pOrder->request.market;
			memcpy(hold.securityId, pOrder->request.securityId, SECURITYID_LEN);

			hold.countSellAvlHld = 0;
			hold.sellAvlHld = 0;
			hold.orgHld = 0;
			hold.orgAvlHld = 0;
			hold.orgCostAmt = 0;
			hold.totalBuyHld = trdQty;
			hold.totalSellHld = 0;
			hold.totalBuyAmt = trdQty * trdPrice;
			hold.totalSellAmt = 0;
			hold.sellFrz = 0;
			hold.etfAvlHld = 0;
			hold.costPrice = trdPrice;
			hold.sumHld = trdQty;
			if (pStock->secType == eXBond) {
				hold.sellAvlHld = trdQty;
				hold.countSellAvlHld = trdQty;
			}

			XPutOrUpdBHold(&hold);

			pHold = XFndVBHoldById(hold.idx);

		} else {
			pHold->totalBuyHld += trdQty;
			pHold->totalBuyAmt += trdQty * trdPrice;
			pHold->sumHld += trdQty;
			if (pStock->secType == eXBond) {
				pHold->sellAvlHld += trdQty;
				pHold->countSellAvlHld += trdQty;
			}
			if (pHold->sumHld != 0) {
				pHold->costPrice = (pHold->totalBuyAmt - pHold->totalSellAmt)
						/ pHold->sumHld;
			}
		}

		break;
	case eXSell:

		if (NULL != pHold) {
			//成交
			if (isTrade == 1) {
				pHold->totalSellHld += trdQty;
				pHold->totalSellAmt += trdQty * trdPrice;
				pHold->sumHld -= trdQty;
				pHold->sellFrz -= trdQty;
				if (pHold->sumHld != 0) {
					pHold->costPrice =
							(pHold->totalBuyAmt - pHold->totalSellAmt)
									/ pHold->sumHld;
				}
			}
			//撤单
			else if (isTrade == 2) {
				pHold->sellFrz -= trdQty;
				pHold->countSellAvlHld += trdQty;
				pHold->sellAvlHld += trdQty;
			}
			//委托
			else {
				pHold->countSellAvlHld -= trdQty;
				pHold->sellAvlHld -= trdQty;
				pHold->sellFrz += trdQty;
			}

		}
		break;
	default:
		break;
	}

	if (NULL != pHold) {
		memcpy(&hold, pHold, sizeof(XHoldT));
		hold.idx = 0;
		cache.head.type = eDHold;
		cache.head.dataLen = XHOLD_SIZE;
		cache.hold = hold;

		slog_debug(0,
				"OnRtnHold:[%d-%s],总持仓[%lld], 可用持仓[%lld],买入数量[%d], 冻结持仓[%lld]",
				hold.market, hold.securityId, hold.sumHld, hold.sellAvlHld,
				trdQty, hold.sellFrz);

		XPushCache(XSHMKEYCONECT(tradeCache), &cache);
	}

	return 0;
}

static XInt _HandleCashAsset(XOrderT *pOrder, XCashT *pCash, XInt isTrade,
		XQty qty, XPrice price) {
	XTradeCache cache = { 0 };
	XCashT cash = { 0 };

	switch (pOrder->request.bsType) {
	case eXBuy:
		//成交
		if (isTrade) {
			pCash->curAvailable -= qty * price;
			pCash->countAvailable -= qty * price;
			pCash->totalBuy += qty * price;
		}
		//委托,冻结可用TODO
		//撤单
		else {
			pCash->curAvailable -= qty * price;
			pCash->countAvailable -= qty * price;
		}
		slog_debug(0, "OnRtnCash:[%s],当前可用[%.2f],交易资金[%.2f]", pCash->accountId,
				pCash->curAvailable * 0.0001, qty * price * 0.0001)
		;
		break;
	case eXSell:
		if (isTrade) {
			pCash->curAvailable += qty * price;
			pCash->countAvailable += qty * price;
			pCash->totalSell += qty * price;
		}
		slog_debug(0, "OnRtnCash:[%s],当前可用[%.2f],交易资金[%.2f]", pCash->accountId,
				pCash->curAvailable * 0.0001, qty * price * 0.0001)
		;
		break;
	default:
		break;

	}

	memcpy(&cash, pCash, sizeof(XCashT));
	cash.idx = 0;

	cache.head.type = eDCash;
	cache.head.dataLen = XCASH_SIZE;
	cache.cash = cash;
	XPushCache(XSHMKEYCONECT(tradeCache), &cache);

	return 0;
}

static XVoid SimRecvOrd(XMonitorT *pMonitor) {
	XOrderT *pOrder = NULL, exhOrder = { 0 };
	XIdx i = 0;
	XOrderT *pOrgOrder = NULL, *pExchOrder = NULL;
	XCashT *pCash = NULL;
	XHoldT *pHold = NULL;
	XMonitorTdT *pMonitorTd = NULL;
//	XRSnapshotT *pSnapshot = NULL;
	XMonitorMdT *pMonitorMd = NULL;

	for (i = 0; i < pMonitor->iTOrder; i++) {
		pOrder = XFndVOrderById(i + 1);
		if (NULL == pOrder || pOrder->ordStatus != eXOrdStatusDefalut) {
			continue;
		}
		//找本地订单,以策略端idx作为交易所ordid
		pExchOrder = XFndVBOrderByOrdid(pOrder->request.customerId,
				pOrder->request.market, pOrder->idx);
		if (NULL != pExchOrder) {
			continue;
		}

		pMonitorTd = XFndVTdMonitor(pOrder->request.customerId);
		if (NULL != pMonitorTd) {
			//设置环境号
			pOrder->envno = pMonitorTd->envno;
		}

		pOrder->exeStatus = eXExecRisk;
		pOrder->exeStatus = eXExec;
		pOrder->_sendLocTime = XGetClockTime();
		//如果是回测，需要把本地单号放到内存，收响应时才能正常找到
		XPutOrderHashByLoc(pOrder);

		memcpy(&exhOrder, pOrder, sizeof(XOrderT));
		exhOrder.ordid = pOrder->idx;
		exhOrder.idx = g_OrderIdx;
		g_OrderIdx++;

		pMonitorMd = XFndVMdMonitor(eXExchSec);
		//模拟开盘前
		if (NULL != pMonitorMd && !pMonitorMd->isRunning) {
			exhOrder.ordStatus = eXOrdStatusInvalid;
			exhOrder.errorno = XORDER_IS_REJECT;
			_HandleReject(&exhOrder);
			continue;
		}

		if (exhOrder.request.isCancel == false) {
			//需要加上价格笼子控制
			/**
			pSnapshot = XFndVSnapshotByKey(pOrder->request.market,
					pOrder->request.securityId);
			*/
			//买单,看看资金够不够
			if (exhOrder.request.bsType == eXBuy) {
				/**
				 if(NULL != pSnapshot && pSnapshot->tradePx && abs(pOrder->request.ordPrice - pSnapshot->tradePx) > pSnapshot->tradePx * 0.02)
				 {
				 slog_warn(0, "[%s]超过价格笼子,委托价格[%d],最新价[%d]", exhOrder.request.customerId, pOrder->request.ordPrice, pSnapshot->tradePx);
				 exhOrder.ordStatus = eXOrdStatusInvalid;
				 exhOrder.errorno = XORDER_IS_REJECT;
				 _HandleReject(&exhOrder);
				 continue;
				 }
				 */
				pCash = XFndVBCashByKey(exhOrder.request.customerId,
						exhOrder.request.acctType);
				if (pCash != NULL
						&& pCash->curAvailable
								>= exhOrder.request.ordPrice
										* exhOrder.request.ordQty) {
#ifdef  __AUCTION__
					exhOrder.ordStatus = eXOrdStatusInvalid;
										exhOrder.errorno = XORDER_IS_REJECT;
										_HandleReject(&exhOrder);
#else
					exhOrder.ordStatus = eXOrdStatusDeclared;
					_HandleInsert(&exhOrder);
					_HandleCashAsset(&exhOrder, pCash, 0,
							exhOrder.request.ordQty, exhOrder.request.ordPrice);
					_HandleCnfm(&exhOrder);
#endif
				} else {
					slog_warn(0, "[%s]资金不足", exhOrder.request.customerId);
					exhOrder.ordStatus = eXOrdStatusInvalid;
					exhOrder.errorno = XEMONEY_IS_NOT_ENOUGH;
					_HandleReject(&exhOrder);
				}
			} else {

				/**
				 if(NULL != pSnapshot && pSnapshot->tradePx && abs(pOrder->request.ordPrice - pSnapshot->tradePx) > pSnapshot->tradePx * 0.02)
				 {
				 slog_warn(0, "[%s]超过价格笼子,委托价格[%d],最新价[%d]", exhOrder.request.customerId, pOrder->request.ordPrice, pSnapshot->tradePx);
				 exhOrder.ordStatus = eXOrdStatusInvalid;
				 exhOrder.errorno = XORDER_IS_REJECT;
				 _HandleReject(&exhOrder);
				 continue;
				 }
				 */
				slog_debug(0, "[%s-%s-%d-%s]处理卖单[%d],本地单号[%d]",
						exhOrder.request.customerId, exhOrder.request.investId,
						exhOrder.request.market, exhOrder.request.securityId,
						exhOrder.request.ordQty, exhOrder.request.localId);
				pHold = XFndVBHoldByKey(exhOrder.request.customerId,
						exhOrder.request.investId, exhOrder.request.market,
						exhOrder.request.securityId);
				if (pHold != NULL && pHold->sellAvlHld > 0) {
					exhOrder.ordStatus = eXOrdStatusDeclared;
					_HandleInsert(&exhOrder);
					_HandleHolding(&exhOrder, 0, exhOrder.request.ordQty,
							exhOrder.request.ordPrice);
					_HandleCnfm(&exhOrder);
				} else {
					slog_warn(0, "[%s]持仓不足[%d-%s]", exhOrder.request.customerId,
							exhOrder.request.market,
							exhOrder.request.securityId);
					exhOrder.ordStatus = eXOrdStatusInvalid;
					exhOrder.errorno = XHOLD_IS_NOT_ENOUGH;
					_HandleReject(&exhOrder);
				}
			}
		} else {
			//找原始订单,用本地单号代替orderid
			pOrgOrder = XFndVBOrderByOrdid(exhOrder.request.customerId,
					pOrder->request.market, pOrder->request.orgLocalId);
			if (NULL != pOrgOrder) {
				//无成交
				if (pOrgOrder->trdQty == 0) {
					pOrgOrder->ordStatus = eXOrdStatusCanceled;

					exhOrder.ordStatus = pOrgOrder->ordStatus;
					_HandleInsert(&exhOrder);
					_HandleCnfm(&exhOrder);

					//推送原始订单的状态
					_HandleCnfm(pOrgOrder);
				}
				//有成交
				else if (pOrgOrder->request.ordQty - pOrgOrder->trdQty > 0) {
					pOrgOrder->ordStatus = eXOrdStatusPCanceled;

					exhOrder.ordStatus = pOrgOrder->ordStatus;
					_HandleInsert(&exhOrder);
					_HandleCnfm(&exhOrder);

					//推送原始订单的状态
					_HandleCnfm(pOrgOrder);
				} else {
					exhOrder.ordStatus = eXOrdStatusInvalid;
					exhOrder.errorno = 1225; //无效的订单状态
					_HandleInsert(&exhOrder);
					_HandleCnfm(&exhOrder);
				}

			} else {
				exhOrder.ordStatus = eXOrdStatusInvalid;
				exhOrder.errorno = 1210; //未找到委托信息
				_HandleInsert(&exhOrder);
				_HandleCnfm(&exhOrder);
			}
		}

		XPutOrUpdBOrder(&exhOrder);
	}
}

static XVoid SimRspTrd(XOrderT *pOrder) {
	XRSnapshotT *pSnapshot = NULL;
	XPrice trdPx;
	XQty trdQty;
	XStockT *pStock = NULL;
	XLongTime gapTime = 0;

	//模拟成交要考虑交易延迟,需考虑回放速度
	gapTime = XGetClockTime() - pOrder->_sendLocTime;
	if ((pOrder->request.market == eXMarketSha && gapTime < 20 * XTIMS_MS4NS)
			|| (pOrder->request.market == eXMarketSza
					&& gapTime < 2 * XTIMS_MS4NS)) {
		return;
	}

	//匹配成交
	trdQty = pOrder->request.ordQty - pOrder->trdQty;

	//获取最新快照,
	//判断能否成交
	pSnapshot = XFndVSnapshotByKey(pOrder->request.market,
			pOrder->request.securityId);
	if (NULL == pSnapshot || pSnapshot->tradePx == 0) {
		return;
	}

	pStock = XFndVStockByKey(pOrder->request.market,
			pOrder->request.securityId);
	if (NULL == pSnapshot) {
		return;
	}

	//限价订单
	if (pOrder->request.ordType == eXOrdLimit) {
		if (pOrder->request.bsType == eXBuy) {
			if (pOrder->request.ordPrice >= pSnapshot->tradePx) {
				trdPx = pSnapshot->tradePx;
				if (pStock->secType == eXBond
						&& pStock->market == eXMarketSha) {
					trdQty =
							trdQty >= pSnapshot->askqty[0] * 10 ?
									pSnapshot->askqty[0] * 10 : trdQty;
				} else {
					trdQty =
							trdQty >= pSnapshot->askqty[0] ?
									pSnapshot->askqty[0] : trdQty;
				}
				//触发成交

				if (trdQty) {
					pOrder->trdQty += trdQty;
					pOrder->trdMoney += trdQty * trdPx;
					if (pOrder->request.ordQty - pOrder->trdQty == 0) {
						pOrder->ordStatus = eXOrdStatusFilled;
					}

					_HandleCnfm(pOrder);
					_HandleTrade(pOrder, trdQty, trdPx);
					_HandleHolding(pOrder, 1, trdQty, trdPx);
				}
			}
		} else {
			if (pOrder->request.ordPrice <= pSnapshot->tradePx) {
				trdPx = pSnapshot->tradePx;
				if (pStock->secType == eXBond
						&& pStock->market == eXMarketSha) {
					trdQty =
							trdQty >= pSnapshot->bidqty[0] * 10 ?
									pSnapshot->bidqty[0] * 10 : trdQty;
				} else {
					trdQty =
							trdQty >= pSnapshot->bidqty[0] ?
									pSnapshot->bidqty[0] : trdQty;
				}

				//触发成交								
				if (trdQty) {
					pOrder->trdQty += trdQty;
					pOrder->trdMoney += trdQty * trdPx;
					if (pOrder->request.ordQty - pOrder->trdQty == 0) {
						pOrder->ordStatus = eXOrdStatusFilled;
					}
					_HandleCnfm(pOrder);
					_HandleTrade(pOrder, trdQty, trdPx);
					_HandleHolding(pOrder, 1, trdQty, trdPx);
//									slog_debug(0, "[%c-%s],限价成交时间[%d],成交价格[%d],成交数量[%d]", pSnapshot->market, pSnapshot->securityId, pSnapshot->updateTime, trdPx, trdQty);
				}
			}
		}
	}
	//本方最优
	else if (eXOrdBestParty == pOrder->request.ordType) {
		if (pOrder->request.bsType == eXBuy) {
			//以本方最有成交
			trdPx = pSnapshot->tradePx;

			if (pStock->secType == eXBond && pStock->market == eXMarketSha) {
				trdQty =
						trdQty >= pSnapshot->askqty[0] * 10 ?
								pSnapshot->askqty[0] * 10 : trdQty;
			} else {
				trdQty =
						trdQty >= pSnapshot->askqty[0] ?
								pSnapshot->askqty[0] : trdQty;
			}

			//触发成交

			if (trdQty) {
				pOrder->trdQty += trdQty;
				pOrder->trdMoney += trdQty * trdPx;
				if (pOrder->request.ordQty - pOrder->trdQty == 0) {
					pOrder->ordStatus = eXOrdStatusFilled;
				}
				_HandleCnfm(pOrder);
				_HandleTrade(pOrder, trdQty, trdPx);
				_HandleHolding(pOrder, 1, trdQty, trdPx);
				slog_debug(0, "[%d-%s],本方最优,成交时间[%d],成交价格[%d],成交数量[%d]",
						pSnapshot->market, pSnapshot->securityId,
						pSnapshot->updateTime, trdPx, trdQty);
			}
		} else {
			//以本方最优成交
			trdPx = pSnapshot->tradePx;

			if (pStock->secType == eXBond && pStock->market == eXMarketSha) {
				trdQty =
						trdQty >= pSnapshot->bidqty[0] * 10 ?
								pSnapshot->bidqty[0] * 10 : trdQty;
			} else {
				trdQty =
						trdQty >= pSnapshot->bidqty[0] ?
								pSnapshot->bidqty[0] : trdQty;
			}
			//触发成交
			if (trdQty) {
				pOrder->trdQty += trdQty;
				pOrder->trdMoney += trdQty * trdPx;
				if (pOrder->request.ordQty - pOrder->trdQty == 0) {
					pOrder->ordStatus = eXOrdStatusFilled;
				}
				_HandleCnfm(pOrder);
				_HandleTrade(pOrder, trdQty, trdPx);
				_HandleHolding(pOrder, 1, trdQty, trdPx);
				slog_debug(0, "[%d-%s],本方最优,成交时间[%d],成交价格[%d],成交数量[%d]",
						pSnapshot->market, pSnapshot->securityId,
						pSnapshot->updateTime, trdPx, trdQty);
			}
		}
	} else {
		if (pOrder->request.bsType == eXBuy) {
			//以对手方价成交
			trdPx = pSnapshot->tradePx;
			if (pStock->secType == eXBond && pStock->market == eXMarketSha) {
				trdQty =
						trdQty >= pSnapshot->askqty[0] * 10 ?
								pSnapshot->askqty[0] * 10 : trdQty;
			} else {
				trdQty =
						trdQty >= pSnapshot->askqty[0] ?
								pSnapshot->askqty[0] : trdQty;
			}

			//触发成交							
			if (trdQty) {
				pOrder->trdQty += trdQty;
				pOrder->trdMoney += trdQty * trdPx;
				if (pOrder->request.ordQty - pOrder->trdQty == 0) {
					pOrder->ordStatus = eXOrdStatusFilled;
				}
				_HandleCnfm(pOrder);
				_HandleTrade(pOrder, trdQty, trdPx);
				_HandleHolding(pOrder, 1, trdQty, trdPx);
				slog_debug(0, "[%d-%s],对手方,成交时间[%d],成交价格[%d],成交数量[%d]",
						pSnapshot->market, pSnapshot->securityId,
						pSnapshot->updateTime, trdPx, trdQty);
			}
		} else {
			trdPx = pSnapshot->tradePx;
			if (pStock->secType == eXBond && pStock->market == eXMarketSha) {
				trdQty =
						trdQty >= pSnapshot->bidqty[0] * 10 ?
								pSnapshot->bidqty[0] * 10 : trdQty;
			} else {
				trdQty =
						trdQty >= pSnapshot->bidqty[0] ?
								pSnapshot->bidqty[0] : trdQty;
			}
			//触发成交							
			if (trdQty) {
				pOrder->trdQty += trdQty;
				pOrder->trdMoney += trdQty * trdPx;
				if (pOrder->request.ordQty - pOrder->trdQty == 0) {
					pOrder->ordStatus = eXOrdStatusFilled;
				}
				_HandleCnfm(pOrder);
				_HandleTrade(pOrder, trdQty, trdPx);
				_HandleHolding(pOrder, 1, trdQty, trdPx);
				slog_debug(0, "[%d-%s],对手方,成交时间[%d],成交价格[%d],成交数量[%d]",
						pSnapshot->market, pSnapshot->securityId,
						pSnapshot->updateTime, trdPx, trdQty);
			}
		}
	}

}

//模拟撮合
static XVoid SimTrd(XVoid *params) {
	XMonitorT *pMonitor = NULL;
	XIdx i = 0;
	XOrderT *pOrder = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	while (1) {

		//模拟把本地新订单发送到交易所
		SimRecvOrd(pMonitor);
		//遍历的是交易所的订单
		for (i = 0; i < pMonitor->iTOrder; i++) {
			pOrder = XFndVBOrderById(i + 1);
			if (NULL == pOrder || pOrder->ordStatus == eXOrdStatusFilled
					|| pOrder->ordStatus == eXOrdStatusPCanceled
					|| pOrder->ordStatus == eXOrdStatusCanceled
					|| pOrder->ordStatus == eXOrdStatusInvalid) {
				continue;
			}

			//TODO 如果未达到下单后的延迟,等待，模拟实际成交情况
			if (pOrder->exeStatus == eXExec && pOrder->request.isCancel == false) {
				//逐笔撮合
				SimRspTrd(pOrder);
				//快照撮合
			}
		}
	}
	exit(0);
}
static void SimRun() {
	XMonitorMdT *pMonitorMd = NULL;

	xslog_init(XSHM_SDB_FILE, "xbtest");

	XManShmLoad();
	pMonitorMd = XFndVMdMonitor(eXExchSec);

	if (NULL == pMonitorMd) {
		slog_error(0, "获取行情信息错误");
		return;
	}
	pMonitorMd->isRunning = true;
}

static void xlog(XVoid *params) {
	xslog_to_file();
}

static void init(XChar *staticfile) {
	XInt iCnt = 0, i;
	XCustT cust[100] = { 0 }, *pCust = NULL;

	xslog_init(NULL, "xbtest");

	slog_info(0, ">>>>>> 2. 初始化内存");
	XManShmInit();

	XManShmLoad();

	iCnt = XGetUser(XUSER_FILE, cust);
	for (i = 0; i < iCnt; i++) {
		pCust = XFndVCustomerByKey(cust[i].customerId);
		if (NULL != pCust) {
			continue;
		}
		XPutVCustomer(&cust[i]);
		XPutTdMonitor(cust[i].customerId, cust[i].envno);
		XPutMdMonitor(cust[i].exchid);
	}
	slog_info(0, ">>>>>> 3. 读取静态数据[%s]", staticfile);
	read_static(staticfile);

	slog_info(0, ">>>>>> 3.0 加载转股价.....");
	read_stock_convpx(XMAN_IMP_CONVPX);
#ifdef __XMAN_FAST_REBUILD__
	//加载历史数据
	slog_info(0, ">>>>>> 3.1 加载历史K1线数据......");
	read_kline(XMAN_IMP_KSNAPSHOT_1, 5);
	slog_info(0, ">>>>>> 3.2 加载历史K5线数据......");
	read_kline(XMAN_IMP_KSNAPSHOT_5, 60);
	slog_info(0, ">>>>>> 3.3 加载昨日收盘数据......");
	read_hissnapshot(XMAN_IMP_HSNAPSHOT);
	slog_info(0, ">>>>>> 3.4 加载板块数据......");
	read_block(XMAN_IMP_BLOCK);
	read_blockinfo(XMAN_IMP_BLOCKINFO);
#endif
}

static XInt getConf(const char *sysconf, XBindParamT bindparam[]) {
	XInt iret = 0, i = 0;
	XIniT *config = NULL;
	const char *pValue = NULL;
	config = XINILoad(sysconf);

	if (NULL == config) {
		return (-1);
	}
	pValue = XINIGet(config, "system", "xshbld.cpu");
	if (pValue != NULL) {
		bindparam[0].cpuid = atoi(pValue);
	}

	pValue = XINIGet(config, "system", "xszbld.cpu");
	if (pValue != NULL) {
		bindparam[1].cpuid = atoi(pValue);
	}

	pValue = XINIGet(config, "system", "xstore.cpu");
	if (pValue != NULL) {
		bindparam[2].cpuid = atoi(pValue);
	}
	pValue = XINIGet(config, "system", "xrsnap.cpu");
	if (pValue != NULL) {
		bindparam[3].cpuid = atoi(pValue);
	}
	pValue = XINIGet(config, "system", "xorden.cpu");
	if (pValue != NULL) {
		bindparam[4].cpuid = atoi(pValue);
	}

	pValue = XINIGet(config, "system", "oesmkt.cpu");
	if (pValue != NULL) {
		bindparam[5].cpuid = atoi(pValue);
	}

	pValue = XINIGet(config, "system", "oesmkt.level");
	if (pValue != NULL) {
		bindparam[5].level = atoi(pValue);
	} else {
		bindparam[5].level = true;
	}

	pValue = XINIGet(config, "system", "oestrd.cpu");
	if (pValue != NULL) {
		bindparam[6].cpuid = atoi(pValue);
	}

	pValue = XINIGet(config, "system", "xetick.cpu");
	if (pValue != NULL) {
		bindparam[7].cpuid = atoi(pValue);
	}

	pValue = XINIGet(config, "system", "oesmkt.resub");
	if (pValue != NULL) {
		bindparam[5].resub = atoi(pValue);
	}

	pValue = XINIGet(config, "system", "oesmkt.subfile");
	if (pValue != NULL) {
		memcpy(bindparam[5].subFile, pValue, strlen(pValue));
	}

	pValue = XINIGet(config, "system", "oesmkt.market");
	if (pValue != NULL) {
		bindparam[5].market = atoi(pValue);
	}

	pValue = XINIGet(config, "system", "oesmkt.sectype");
	if (pValue != NULL) {
		for (i = 0; i < strlen(pValue); i++) {
			switch (pValue[i]) {
			case '1':
				bindparam[5].secType[i] = eXStock;
				break;
			case '2':
				bindparam[5].secType[i] = eXBond;
				break;
			case '3':
				bindparam[5].secType[i] = eXFund;
				break;
			case '4':
				bindparam[5].secType[i] = eXETF;
				break;
			case '5':
				bindparam[5].secType[i] = eXOpt;
				break;
			default:
				break;
			}
		}
	} else {
		iret = -1;
	}

	XINIFree(config);

	return (iret);
}

static void backtest(XChar *staticfile, XMktSimParmT *mktsimparm) {

	XProcInfoT g_proc[1024];
	XInt iProc = 0;
	XBindParamT bindparam[9];

	memset(&bindparam, 0, sizeof(bindparam));

	init(staticfile);

	getConf(XSYSTEM_FILE, bindparam);

	slog_info(0, ">>>>>> 4. 启动回测服务");
	memset(&g_proc, 0, sizeof(g_proc));

	memcpy(g_proc[iProc].processName, "xbtest", strlen("xbtest"));
	g_proc[iProc++].callBack = xlog;

	memcpy(g_proc[iProc].processName, "xshbld", strlen("xshbld"));
	g_proc[iProc].callBack = XShRebuild;
	g_proc[iProc++].params = &bindparam[0];

	memcpy(g_proc[iProc].processName, "xszbld", strlen("xszbld"));
	g_proc[iProc].callBack = XSzRebuild;
	g_proc[iProc++].params = &bindparam[1];

	memcpy(g_proc[iProc].processName, "simmkt", strlen("simmkt"));
	g_proc[iProc].callBack = SimMkt;
	mktsimparm->cpu = bindparam[5].cpuid;
	g_proc[iProc++].params = mktsimparm;

	memcpy(g_proc[iProc].processName, "xstore", strlen("xstore"));
	g_proc[iProc++].callBack = XStoreSnap;

	memcpy(g_proc[iProc].processName, "simtrd", strlen("simtrd"));
	g_proc[iProc].callBack = SimTrd;
	g_proc[iProc++].params = &bindparam[6];

	memcpy(g_proc[iProc].processName, "xorden", strlen("xorden"));
	g_proc[iProc].callBack = XOrdeng;
	g_proc[iProc++].params = &bindparam[4];

#ifndef __XMAN_FAST_REBUILD__

#endif 
	xslog_init(XSHM_SDB_FILE, "xmanlg");
	XAppStart(g_proc, iProc);
}

static void stop() {
	xslog_init(NULL, "xbtest");
	XAppStop();
	XManShmDelete();
	xslog_release_cache(XSHM_SDB_FILE);
}

static void UpdateHold(XInt market, XChar *securityId, XSumQty qty) {
	XIdx i = 0;
	XCustT *pCust = NULL;
	XHoldT hold, *pFndHold = NULL;
	XInvestT *pInvest = NULL;
	XMonitorT *pMonitor = NULL;
	XStockT *pStock = NULL;

	xslog_init(XSHM_SDB_FILE, "xbtest");
	XManShmLoad();

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	for (i = 0; i < pMonitor->iTCust; i++) {
		pCust = XFndVCustomerById(i + 1);
		if (NULL == pCust || pCust->counter != eXCounterOes) {
			slog_error(0, "未找到客户号");
			continue;
		}

		pInvest = XFndVInvestByAcctType(pCust->customerId, market, eXInvSpot);
		if (NULL == pInvest) {
			slog_error(0, "未找到股东账户,客户号[%s],市场[%d]", pCust->customerId, market);
			continue;
		}

		pStock = XFndVStockByKey(market, securityId);
		if (NULL == pStock) {
			slog_error(0, "未找到证券信息,证券代码[%d-%s]", market, securityId);
			continue;
		}

		pFndHold = XFndVHoldByKey(pCust->customerId, pInvest->investId, market,
				securityId);

		if (NULL == pFndHold) {
			memset(&hold, 0, sizeof(XHoldT));
			memcpy(hold.customerId, pCust->customerId, CUSTOMERID_LEN);
			memcpy(hold.securityId, securityId, strlen(securityId));
			memcpy(hold.investId, pInvest->investId, INVESTID_LEN);
			hold.orgAvlHld = qty;
			hold.orgHld = qty;
			hold.sumHld = qty;
			hold.sellAvlHld = qty;
			hold.countSellAvlHld = qty;
			hold.costPrice = pStock->preClose;
			hold.orgCostAmt = pStock->preClose * qty;

			hold.market = market;

			slog_info(0, "持仓：[%s-%s],[%d-%s],持仓总量[%lld],可用持仓[%lld]",
					hold.customerId, hold.investId, hold.market,
					hold.securityId, hold.sumHld, hold.sellAvlHld);
			XPutOrUpdVHoldByKey(&hold);
			XPutOrUpdBHold(&hold);
		} else {
			pFndHold->orgAvlHld += qty;
			pFndHold->orgHld += qty;
			pFndHold->sumHld += qty;
			pFndHold->sellAvlHld += qty;
			pFndHold->countSellAvlHld += qty;
			pFndHold->costPrice = pStock->preClose;
			pFndHold->orgCostAmt = pStock->preClose * pFndHold->orgAvlHld;

			XPutOrUpdBHold(pFndHold);
			XPutOrUpdVHoldByKey(pFndHold);
		}
	}
}

static void usage() {
	printf("###############################################################\n");
	printf("#    XMan策略回测平台(%s) \n", __XMAN_VERSION__);
	printf("# -h [help] 帮助\n");
	printf(
			"# -t [1:backtest(回测平台启动),2:运行(启动策略后运行回测系统开始回放行情),3:插入持仓,4:停止,12:打印当前盘口信息]\n");
	printf("# -i [staticfile] 静态文件二进制,回测时指定静态文件\n");
	printf("# -q [mktstorefile] 行情二进制文件，回测时指定\n");
	printf("# -m [market] 市场\n");
	printf("# -s [securityid] 证券代码\n");
	printf("# -v [volume] 回放多少条暂停对应的时间|插入可卖持仓的数量|档位数\n");
	printf("# -k [time] 暂停的时间us|0:盘口未处理数据,1:盘口处理后数据\n");
	printf("# -b [breakBiz] 行情回放进程回放到指定的BizIndex \n");
	printf("# -p [print] 打印系统信息\n");
	printf("# 举例:\n");
	printf("1. 回测平台启动\n");
	printf("./xbtest -t1 -istatic.bin -qmktstore.bin -v2000 -k200 -c2014\n");
	printf("2. 运行\n");
	printf("./xbtest -t2\n");
	printf("3. 插入持仓\n");
	printf("./xbtest -t3 -m1 -s600837 -v10000\n");
	printf("4. 停止回测\n");
	printf("./xbtest -t4\n");
	printf("5. 打印未重构的盘口信息\n");
	printf("./xbtest -t12 -m1 -s600837 -v5 -k0\n");
	printf("6. 打印重构后的盘口信息\n");
	printf("./xbtest -t13 -m1 -s600837\n");
	printf("7. 打印交易所原始盘口数据\n");
	printf("./xbtest -t14 -m1 -s600837\n");
	printf("###############################################################\n");
}
//1、加载静态数据;
//2、初始化账户数据;
//3、读取重构的历史行情文件
//4、开始策略回测

int main(int argc, char *argv[]) {

	int opt;
	int option_index = 0;
	const char *option = "ht:m:s:i:q:v:k:pb:c:";
	const struct option long_options[] = { { "help", no_argument, NULL, 'h' }, {
			"type", required_argument, NULL, 't' }, { "static",
			required_argument, NULL, 'i' }, { "mktstore", required_argument,
			NULL, 'q' }, { "market", required_argument, NULL, 'm' }, {
			"security", required_argument, NULL, 's' }, { "volume",
			required_argument, NULL, 'v' }, { "time", required_argument, NULL,
			'k' }, { "bizIndex", required_argument, NULL, 'b' }, { "print",
			no_argument, NULL, 'p' },
			{ "chanel", required_argument, NULL, 'c' }, { NULL, 0, NULL, 0 } };
	XMktSimParmT mktSimParm;
	char *staticfile = NULL;
	XInt type = -1;

	memset(&mktSimParm, 0, sizeof(XMktSimParmT));
	mktSimParm.speed = 10000; /** 默认设置 */
	while ((opt = getopt_long(argc, argv, option, long_options, &option_index))
			!= -1) {
		switch (opt) {
		case 'h':
		case '?':
			usage();
			break;

			/** 操作类型 */
		case 't':
			if (optarg) {
				type = atoi(optarg);
			}
			break;
		case 'i':
			staticfile = optarg;
			break;
		case 'q':
			memcpy(mktSimParm.bckFile, optarg, strlen(optarg));
			break;
			/** 市场 */
		case 'm':
			if (optarg) {
				mktSimParm.market = atoi(optarg);
			}
			break;
			/** 证券代码 */
		case 's':
			if (optarg) {
				memcpy(mktSimParm.securityId, optarg, strlen(optarg));
			}
			break;
		case 'b':
			if (optarg) {
				mktSimParm.breakBiz = atoi(optarg);
			}
			break;
			/** 回放的数量 */
		case 'v':
			mktSimParm.speed = atoi(optarg);
			break;
			/** 暂停的时间 */
		case 'k':
			mktSimParm.ms = atoi(optarg);
			break;
			/** 回放的频道 */
		case 'c':
			mktSimParm.channel = atoi(optarg);
			break;
		case 'p':
			XManShmLoad();
			XExpPrint();
			break;
		default:
			break;
		}
	}

	switch (type) {
	/** 回测 */
	case 1:
		if (staticfile == NULL || strlen(staticfile) == 0
				|| strlen(mktSimParm.bckFile) == 0) {
			break;
		}
		backtest(staticfile, &mktSimParm);
		break;
		//运行
	case 2:
		SimRun();
		break;
		//插入持仓
	case 3:
		if (strlen(mktSimParm.securityId) > 0) {
			UpdateHold(mktSimParm.market, mktSimParm.securityId,
					mktSimParm.speed);
		}
		break;
		//停止
	case 4:
		stop();
		break;
		//初始化
	case 5:
		if (NULL == staticfile || 0 == strlen(staticfile)) {
			usage();
			break;
		}
		init(staticfile);
		break;
		//回放行情
	case 6:
		if (strlen(mktSimParm.bckFile) == 0) {
			usage();
			break;
		}
		XManShmLoad();
		SimMkt(&mktSimParm);
		break;
		//单独启动模拟交易
	case 7:
		XManShmLoad();
		SimTrd(NULL);
		break;
		//单独启动内部交易处理
	case 8:
		XManShmLoad();
		XOrdeng(NULL);
		break;
		//单独启动重构上海行情
	case 9:
		XManShmLoad();
		XShRebuild(NULL);
		break;
		//单独启动重构深圳行情
	case 10:
		XManShmLoad();
		XSzRebuild(NULL);
		break;
		//落地重构行情
	case 11:
		XManShmLoad();
		XStoreSnap(NULL);
		break;
		/** 输出未重构盘口 */
	case 12:
		xslog_init(NULL, "xbtest");
		XManShmLoad();
		if (!mktSimParm.ms) {
			XPLevelPrint(mktSimParm.market, mktSimParm.securityId,
					mktSimParm.speed);
		} else {
			XPRLevelPrint(mktSimParm.market, mktSimParm.securityId,
					mktSimParm.speed);
		}
		break;
		//输出重构后的盘口
	case 13:
		xslog_init(NULL, "xbtest");
		XManShmLoad();
		XRSnapPrint(mktSimParm.market, mktSimParm.securityId);
		break;
	case 14:
		xslog_init(NULL, "xbtest");
		XManShmLoad();
		XSnapPrint(mktSimParm.market, mktSimParm.securityId);
		break;
	default:
		break;
	}

	return (0);
}
