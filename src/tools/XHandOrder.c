/**
 * @file XHandOrder.c
 * @brief 手工交易
 * @author kangyq
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */
#include    <getopt.h>
#include "XCom.h"
#include "XBus.h"
#include "XLog.h"
#include "XPlot.h"

static void UpdateHold(XChar* customer, XInt market, XChar* securityId, XPrice price, XQty qty)
{
	XCustT* pCust = NULL;
	XHoldT hold, *pFndHold = NULL;
	XInvestT *pInvest = NULL;
	XStockT *pStock = NULL;

	pCust = XFndVCustomerByKey(customer);
	if(NULL == pCust)
	{
		slog_error(0, "未找到客户号[%s]", customer);
		return;
	}

	pInvest = XFndVInvestByAcctType(customer, market, eXInvSpot);
	if(NULL == pInvest)
	{
		slog_error(0, "未找到股东账户[%s]", customer);
		return;
	}

	pStock = XFndVStockByKey(market, securityId);
	if(NULL == pStock)
	{
		slog_error(0, "未找到证券信息[%d-%s]", market, securityId);
		return;
	}

	pFndHold = XFndVHoldByKey(customer, pInvest->investId, market, securityId);
	if(NULL == pFndHold)
	{
		memset(&hold, 0, sizeof(XHoldT));
		memcpy(hold.customerId, customer, strlen(customer));
		memcpy(hold.investId, pInvest->investId, INVESTID_LEN);
		memcpy(hold.securityId, securityId, strlen(securityId));
		hold.orgAvlHld = qty;
		hold.orgHld = qty;
		hold.sumHld = qty;
		hold.sellAvlHld = qty;
		hold.countSellAvlHld = qty;
		hold.costPrice = price;
		hold.orgCostAmt = price * qty;
		hold.market = market;

		XPutOrUpdVHoldByKey(&hold);
		slog_info(0, "[%s-%s-%d-%s],总持仓[%lld],可用持仓[%lld]", customer, hold.investId, market, securityId, hold.sumHld, hold.sellAvlHld);

	}
	else
	{
		pFndHold->orgAvlHld += qty;
		pFndHold->orgHld += qty;
		pFndHold->sumHld += qty;
		pFndHold->orgCostAmt += price * qty; 
		pFndHold->costPrice = price;
		pFndHold->sellAvlHld += qty;
		pFndHold->countSellAvlHld += qty;
		slog_info(0, "[%s-%d-%s],总持仓[%lld],可用持仓[%lld]", customer, market, securityId, pFndHold->sumHld, pFndHold->sellAvlHld);
	}
}

static void UpdateCash(XChar* customer, XPrice money)
{
	XCashT* pCash = NULL;

	pCash = XFndVCashByKey(customer, eXInvSpot);
	if(NULL == pCash)
	{
		slog_error(0, "未找到资金账户[%s]", customer);
		return;
	}
	pCash->balance += money * 10000;
	pCash->curAvailable += money * 10000;

	slog_info(0, "[%s],资金更新后，资金余额[%.2f], 可用资金[%.2f]", customer, pCash->balance * 0.0001, pCash->curAvailable * 0.0001);

}


static void usage() {
	printf("###############################################################\n");
	printf("#     手动交易 1.0.0 \n");
	printf("# -t [业务模式 0:交易,1:查询行情,2:查资金,3:查持仓,4:更新资金(模拟使用),5:更新持仓()\n");
	printf("# -c [客户号]\n");
	printf("# -m [市场 1:上海,2:深圳(默认)] \n");
	printf("# -s [证券代码 6位] \n");
	printf("# -b [买卖 1: 买入,2:卖出(默认),3:逆回购卖出, 4:撤单,5:申购,6:赎回]\n");
	printf("# -p [价格为实际价格*10000的整数]\n");
	printf("# -q [买卖数量,超过最大数量进行拆单，逆回购数量为0使用所有资金进行逆回购操作]\n");
	printf("# -l [撤单编号]\n");
	printf("# 例如:\n");
	printf("# 1. 以最新利息卖出所有逆回购\n");
	printf("# ./xhorder -t0 -ccustomer -m1 -s204001 -b3 -p0 -q0\n");
	printf("# 2. 加100万模拟资金测试\n");
	printf("# ./xhorder -t4 -ccustomer -p1000000\n");
	printf("# 3. 加100张可转债持仓模拟测试,成本价108.3\n");
	printf("# ./xhorder -t5 -ccustomer -m1 -s110058 -p1083000 -q100\n");
	printf("###############################################################\n");
}

static XInt HOrder(XChar* customer, XInt market, XChar* securityId, XQty qty, XInt bstype, XPrice price, XInt localid)
{
	XInt i = 0, iret = -1;
	XQty realqty = -1;
	XOrderReqT order = { 0 };
	XCustT *pCust = NULL;
	XSnapshotT *pSnapshot = NULL;
	XStockT *pStock = NULL;
	XCashT *pCash = NULL;

	pCust = XFndVCustomerByKey(customer);
	if (NULL == pCust) {
		slog_error(0, "未找到客户号");
		return (iret);
	}

	memset(&order, 0, sizeof(XOrderReqT));

	order.reqId = i + 1;
	order.sessionId = 1;
	order.frontId = i + 1;
	memcpy(order.customerId, pCust->customerId, strlen(pCust->customerId));
	order.acctType = eXInvSpot;

	order.isCancel = false;

	switch(bstype)
	{
		case 1:
			order.bsType = eXBuy;
			break;
		case 2:
			order.bsType = eXSell;
			break;
		case 3:
			order.bsType = eXCSell;
			pCash = XFndVCashByKey(customer, eXInvSpot);

			if(qty == 0 && NULL != pCash)
			{
				qty = pCash->curAvailable / 10000000 * 10;
			}
			break;
		case 4:
			order.bsType = eXSell;
			order.isCancel = true;
			break;
		case 5:
			order.bsType = eXDeem;
			break;
		case 6:
			order.bsType = eXRedeem;

			break;
		default:
		break;
	}

	order.market = market;
	
	memcpy(order.securityId, securityId, strlen(securityId));
	order.ordType = eXOrdLimit;

	if(bstype != 4)
	{
		while(qty > 0)
		{
			if(price == 0)
			{
				// 找到快照位置
				pSnapshot = XFndVSnapshotByKey(order.market, order.securityId);
				if(NULL == pSnapshot)
				{
					slog_error(0, "未找到对应的快照[%d-%s]", order.market, order.securityId);
					return (iret);
				}
				price = pSnapshot->tradePx;
			}

			/** 计算可卖数量 */
			realqty = qty;
			if(bstype != 3)
			{
				pStock = XFndVStockByKey(order.market, order.securityId);
				if(NULL == pStock)
				{
					return (iret);
				}
				if(qty > pStock->lmtBuyMaxQty)
				{
					realqty = pStock->lmtBuyMaxQty;
				}
			}
			else
			{
				if(qty > 10000)
				{
					realqty = 10000;
				}
			}
			order.ordPrice = price;
			order.ordQty = realqty;
			
			slog_info(0, ">>>>>> 发出订单[%s],证券代码[%d-%s],证券账户[%s],委托类型[%d],委托价格[%d],委托数量[%d],是否撤单[%d],原委托[%d],可委托数量[%d]",
					order.customerId, order.market, order.securityId, order.investId, order.bsType,
					order.ordPrice, order.ordQty, order.isCancel, order.orgLocalId, qty);

			iret = XPutOrderReq(&order);
			qty -= realqty  + 1; //0.0005手续费
		}
	}
	else
	{
		order.orgLocalId = localid;

		slog_info(0, ">>>>>> 发出订单[%s],证券代码[%d-%s],证券账户[%s],委托类型[%d],委托价格[%d],委托数量[%d],是否撤单[%d],原委托[%d],可委托数量[%d]",
				order.customerId, order.market, order.securityId, order.investId, order.bsType,
				order.ordPrice, order.ordQty, order.isCancel, order.orgLocalId, qty);

		iret = XPutOrderReq(&order);
	}
	return (0);
}

static XInt HMarket(XInt market, XChar* securityId)
{
	XInt iret = -1;
	XSnapshotT *pSnapshot = NULL;
	XStockT *pStock = NULL;
	XPrice ma1_5 = -1, ma5_5 = -1;
	XKLineT *kline1 = NULL, *kline5 = NULL;

	pStock = XFndVStockByKey(market, securityId);
	if(NULL == pStock)
	{
		slog_error(0, "[%d-%s] 未找到证券信息", market, securityId);
		return (iret);
	}

	slog_info(0, "[%d-%s-%s], 涨停价[%d],跌停价[%d]", pStock->market, pStock->securityId, pStock->securityName, pStock->upperPrice, pStock->lowerPrice);

	pSnapshot = XFndVRSnapshotByKey(market, securityId);
	if(NULL == pSnapshot)
	{
		return (iret);
	}
	kline1 = GetKlinesByBlock(pSnapshot->idx, 0);
	kline5 = GetKlinesByBlock(pSnapshot->idx, 1);

	ma1_5 = MA(kline1, SNAPSHOT_K1_CNT, pSnapshot->kcursor1, 5);
	ma5_5 = MA(kline5, SNAPSHOT_K5_CNT, pSnapshot->kcursor5, 5);

	slog_info(0, "[%d-%s],频道号[%d],最新价[%d],最新时间[%d], 买一[%d-%lld], 卖一[%d-%lld],ma1_5[%d-%d], ma5_5[%d-%d]", pSnapshot->market, pSnapshot->securityId, 
	pSnapshot->_channel, pSnapshot->tradePx,
	  pSnapshot->updateTime, pSnapshot->bid[0], pSnapshot->bidqty[0], pSnapshot->ask[0], pSnapshot->askqty[0], pSnapshot->kcursor1, ma1_5, pSnapshot->kcursor5, ma5_5);

	return (0);
}

static XInt HCash(XChar* customer)
{
	XCashT *pCash = NULL;

	pCash = XFndVCashByKey(customer, eXInvSpot);
	if(NULL == pCash)
	{
		return (-1);
	}
	slog_info(0, "[%s-%s], 初始资金[%.2f], 可用资金[%.2f]", pCash->customerId, pCash->accountId, pCash->beginAvailable * 0.0001, pCash->curAvailable * 0.0001);
	return (0);
}

static XInt HHold(XChar* customer, XInt market, XChar* securityId)
{
	XMonitorT *pMonitor = NULL;
	XHoldT *pHold = NULL;
	XIdx i = -1;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) 
	{
		return (-1);
	}

	for(i = 0; i < pMonitor->iTHold; i++)
	{
		pHold = XFndVHoldById(i + 1);
		if(NULL == pHold)
		{
			continue;
		}
		if(NULL != customer && strlen(customer) > 0 && strncmp(pHold->customerId, customer, strlen(customer)))
		{
			continue;
		}
		if(market != -1 && market != pHold->market)
		{
			continue;
		}

		if(NULL != securityId && strlen(securityId) > 0 && strncmp(pHold->securityId, securityId, strlen(securityId)))
		{
			continue;
		}
		slog_info(0, "[%s-%s-%d-%s], 总持仓[%lld], 可用持仓[%lld]", pHold->customerId, pHold->investId, pHold->market, pHold->securityId, pHold->sumHld, pHold->sellAvlHld);

	}

	return (0);
}

int main(int argc, char *argv[]) {
	XInt iret = -1;
	XChar* customer = NULL;
	XInt market = -1;
	XChar* securityId = NULL;
	XPrice price = -1;
	XQty qty = -1;
	XInt bstype = -1;
	XInt localid = 0;
	XInt type = 0;

	int opt;
	int option_index = 0;
	const char *option = "ht:m:s:b:p:q:l:c:";
	const struct option long_options[] = { { "help", 0, NULL, 'h' }, 
	{"type", 1, NULL, 't'}, {"customer", 1, NULL, 'c'},
	{ "market",1, NULL, 'm' }, { "securityId", 1, NULL, 's' }, { "bstype", 1, NULL,
			'b' }, { "price", 1, NULL, 'p' }, { "qty", 1, NULL, 'q' },{ "local", 1, NULL, 'l' }, { NULL,
			0, NULL, 0 } };

		
        while ((opt = getopt_long (argc, argv, option, long_options,
                                   &option_index))
               != -1)
          {
            switch (opt)
              {
              case 'h':
              case '?':
                usage ();
                exit (0);
                break;
              case 't':
                type = atoi (optarg);
                break;
              case 'c':
                customer = optarg;
                break;
              case 'm':
                market = atoi (optarg);
                break;
              case 's':
                securityId = optarg;
                break;
              case 'b':
                bstype = atoi (optarg);
                break;
              case 'p':
                price = atoi (optarg);
                break;
              case 'q':
                qty = atoi (optarg);
                break;
              case 'l':
                localid = atoi (optarg);
                break;
              default:
                break;
              }
          }

        xslog_init(NULL, "xhorder");
	slog_debug(0, "xman版本[%s]", __XMAN_VERSION__);
	XManShmLoad();

	switch(type)
	{
		case 0:
			if (-1 == market || NULL == customer || NULL == securityId || -1 == bstype || -1 == price
			|| -1 == qty) {
				usage();
				break;
			}
			HOrder(customer, market, securityId, qty, bstype, price, localid);
			break;
		case 1:
			if(-1 == market || NULL == securityId)
			{
				usage();
				break;
			}
			HMarket(market, securityId);
			break;
		case 2:
			if(NULL == customer)
			{
				usage();
				break;
			}
			HCash(customer);
			break;
		case 3:
			if(NULL == customer || NULL == securityId)
			{
				usage();
				break;
			}
			HHold(customer, market, securityId);
			break;
		case 4:
			if(NULL == customer)
			{
				usage();
				break;
			}
			UpdateCash(customer, price);
			break;
		case 5:
			if(NULL == customer || NULL == securityId || qty <= 0)
			{
				usage();
				break;
			}
			UpdateHold(customer, market, securityId, price, qty);
			break;
		
		default:
			break;

	}

	return (iret);
}
