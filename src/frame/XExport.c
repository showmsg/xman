/*
 * XExport.c
 *
 *  Created on: 2022年6月9日
 *      Author: DELL
 */

#include "XExport.h"
#include "XTimes.h"
#include "XLog.h"
#include "XCSV.h"
#include "XINI.h"
#ifdef __USED_NCURSES__
#include <ncurses.h>
#include <locale.h>
#include <signal.h>
#endif

#define CLR_NORMAL   "\x1B[0m"
#define CLR_RED      "\x1B[31m"
#define CLR_GREEN    "\x1B[32m"
#define CLR_YELLOW   "\x1B[33m"
#define CLR_BLUE     "\x1B[34m"
#define CLR_NAGENTA  "\x1B[35m"
#define CLR_CYAN     "\x1B[36m"
#define CLR_WHITE    "\x1B[37m"
#define CLR_RESET    "\033[0m"


#define OES_COUNTER_SEC  "OES"
#define XTP_COUNTER_SEC  "XTP"
#define CTP_COUNTER_SEC  "CTP"

XVoid XExpPrint() {
	XMonitorT *pMonitor = NULL;
	XMonitorMdT *pMonitorMd = NULL;
	XInt i = 0;
	XProcInfoT *proc = NULL;
	XIdx idx = -1;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	pMonitorMd = XFndVMdMonitor(eXExchSec);
	if (NULL != pMonitorMd) {
		printf("#交易日[%8d],交易所时间[%09d],敲门时间[%09d-%09d]\n", pMonitorMd->traday,
				pMonitorMd->updateTime, pMonitorMd->_mktFirstTime,
				XNsTime2I(pMonitorMd->_locFirstTime));
		printf("%s涨[%4d]%s %s平[%4d]%s %s跌[%4d]%s\n", CLR_RED,
				pMonitorMd->uppCnt, CLR_RESET, CLR_NORMAL, pMonitorMd->eqlCnt,
				CLR_RESET, CLR_GREEN, pMonitorMd->lowCnt, CLR_RESET);
	}

	printf("委托总数        :\t %lld\n", pMonitor->iTOrder);
	printf("成交总数        :\t %lld\n", pMonitor->iTTrade);
	printf("持仓总数        :\t %lld\n", pMonitor->iTHold);
	printf("证券总数        :\t %lld\n", pMonitor->iTStock);
	printf("发行总数        :\t %lld\n", pMonitor->iTIssue);
	printf("策略总数        :\t %lld\n", pMonitor->iTPlot);
	printf("交易所快照数    :\t %lld\n", pMonitor->iTSnapshot);
	printf("重构后行情数    :\t %s %lld %s\n", CLR_BLUE, pMonitor->iTOrderBook,
	CLR_RESET);
	if (NULL != pMonitorMd) {
		printf(
				"上海逐笔        :\t biz[%s%4d-%10lld%s] order[%s%10lld%s] trade[%s%10lld%s]\n",
				CLR_BLUE, pMonitorMd->shChannel, pMonitorMd->shBiz, CLR_RESET,
				CLR_BLUE, pMonitorMd->totalShOrders, CLR_RESET, CLR_BLUE,
				pMonitorMd->totalShTrades, CLR_RESET);
		printf(
				"深圳逐笔        :\t biz[%s%4d-%10lld%s] order[%s%10lld%s] trade[%s%10lld%s]\n",
				CLR_BLUE, pMonitorMd->szChannel, pMonitorMd->szBiz, CLR_RESET,
				CLR_BLUE, pMonitorMd->totalSzOrders, CLR_RESET, CLR_BLUE,
				pMonitorMd->totalSzTrades, CLR_RESET);

	}
	idx = XGetAppCnt();
	printf("# 进程 [%2lld]\n", idx);

	for (i = 0; i < idx; i++) {
		proc = XFndVAppById(i + 1);
		/** 判断进程数据是否存在 */
		if (NULL == proc) {
			continue;
		}
		if (!XCheckPid(proc->pid)) {
			printf("%d. [ %20s ] - <%d>\n", i + 1, proc->processName,
					proc->pid);
		} else {
			printf("%d. [ %20s ] - <%sexit%s>\n", i + 1, proc->processName,
			CLR_RED, CLR_RESET);
		}
	}

}

XVoid XMonitorPrint() {

#ifdef __USED_NCURSES__	
	XMonitorT *pMonitor = NULL;
	XMonitorMdT *pMonitorMd = NULL;
	XInt i = 0, iline = 0;
	XProcInfoT *proc = NULL;
	XIdx idx = -1;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	setlocale(LC_ALL, "");
	setlocale(LC_CTYPE, "");
	raw();
	// 初始化ncurses
    initscr();
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	init_pair(2, COLOR_BLUE, COLOR_WHITE);
	init_pair(3, COLOR_RED, COLOR_WHITE);
    //cbreak();
    //noecho();
    keypad(stdscr, TRUE);
	curs_set(0);
    	
	clear();
	while(1)
	{
		iline = 2;
		box(stdscr, ACS_VLINE, ACS_HLINE);
		attron(COLOR_PAIR(2));
		mvprintw(iline, 2, "                   量赢策略交易平台-Monitor (%s)                   ", __XMAN_VERSION__);
		attroff(COLOR_PAIR(2));

		mvprintw(++iline, 2, "-----------------------------------------------------------------------");
		pMonitorMd = XFndVMdMonitor(eXExchSec);
		if (NULL != pMonitorMd) {
			mvprintw(++iline, 2,  "交易日[%8d],交易所时间[%09d],敲门时间[%09d-%09d]",
			pMonitorMd->traday, pMonitorMd->updateTime, pMonitorMd->_mktFirstTime, XNsTime2I(pMonitorMd->_locFirstTime));

			mvprintw(++iline, 2, "涨[%4d] 平[%4d] 跌[%4d]\n",  pMonitorMd->uppCnt,   pMonitorMd->eqlCnt, pMonitorMd->lowCnt);

		}

		mvprintw(++iline, 2,"委托总数    :\t %lld", pMonitor->iTOrder);
		mvprintw(++iline, 2,"成交总数    :\t %lld", pMonitor->iTTrade);
		mvprintw(++iline, 2,"持仓总数    :\t %lld", pMonitor->iTHold);
		mvprintw(++iline, 2,"证券总数    :\t %lld", pMonitor->iTStock);
		mvprintw(++iline, 2,"发行总数    :\t %lld", pMonitor->iTIssue);
		mvprintw(++iline, 2,"策略总数    :\t %lld", pMonitor->iTPlot);
		mvprintw(++iline, 2,"交易所快照数:\t %lld", pMonitor->iTSnapshot);
		mvprintw(++iline, 2,"重构后行情数:\t %lld",  pMonitor->iTOrderBook);
		if(NULL != pMonitorMd)
		{
			mvprintw(++iline, 2,"上海逐笔    :\t biz[%4d-%10lld] order[%10lld] trade[%10lld]",pMonitorMd->shChannel, pMonitorMd->shBiz,  
			pMonitorMd->totalShOrders,   pMonitorMd->totalShTrades);
			mvprintw(++iline, 2,"深圳逐笔    :\t biz[%4d-%10lld] order[%10lld] trade[%10lld]", pMonitorMd->szChannel, pMonitorMd->szBiz, 
			pMonitorMd->totalSzOrders,   pMonitorMd->totalSzTrades);


		}
		mvprintw(++iline, 2, "-----------------------------------------------------------------------");
		idx = XGetAppCnt();
		mvprintw(++iline, 2,"进程信息 [%2lld]",idx);

		for (i = 0; i < idx; i++) {
			proc = XFndVAppById(i + 1);
			// 判断进程数据是否存在
			if (NULL == proc)
			{
				continue;
			}
			if(!XCheckPid(proc->pid))
			{
				mvprintw(++iline, 2,"%d. [ %20s ] - <%d>", i + 1, proc->processName, proc->pid);
			}
			else
			{
				attron(A_BLINK);
				attron(COLOR_PAIR(3));
				mvprintw(++iline, 2,"%d. [ %20s ] - <exit>", i + 1, proc->processName);
				attroff(COLOR_PAIR(3));
				attroff(A_BLINK);
			}
		}
		//TODO 统计行情中涨停的证券，跌停的证券；涨的股票数量，平的股票数量，跌的股票数量；一字板数量,封板数量，炸板数量
		//TODO 涨幅靠前的10个板块及龙一龙二龙三票

		mvprintw(++iline, 2, "-----------------------------------------------------------------------");
		mvprintw(++iline, 2, "                                        CTRL+C退出         ");
		refresh();
		sleep(1);	
	}
	// 刷新窗口并等待用户按下任意键
    refresh();
	endwin();
#endif

}

XVoid XExpOrder(XChar *expFile) {
	XIdx i;
	XOrderT *pOrder = NULL;
	FILE *fp = NULL;
	XRSnapshotT *pSnapshot = NULL;
	XMonitorT *pMonitor = NULL;
	XPrice tradePx = 0;
	XPrice avgTdPx = 0;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}
	fprintf(fp,
			"id,reqid, plotid, localId,clocalid,ordid,customerId,market,securityId,investid,envno,bsType,isCancel,ordType,ordQty,"
					"ordPrice,orgEnvno,orgLocalId,orgOrdid,mktPx, mktTime,trdQty,trdMoney,ordStatus,sendLocTime,cnfLocTime,"
					"gapCnterRsp(ns), gapExchRsp(ns),gapMktRsp(ns),sendTime,cnfTime, gapCntExch(ms), errorId,errorMsg,"
					"locFrzQty,locFrzMoney,bizIndex,exeStatus, closePx, avgTdPx\n");
	for (i = 0; i < pMonitor->iTOrder; i++) {
		pOrder = XFndVOrderById(i + 1);
		if (NULL == pOrder) {
			continue;
		}
		tradePx = 0;
		avgTdPx = 0;
		pSnapshot = XFndVRSnapshotByKey(pOrder->request.market,
				pOrder->request.securityId);
		if (NULL != pSnapshot) {
			tradePx = pSnapshot->tradePx;
		}
		if (pOrder->trdQty != 0) {
			avgTdPx = pOrder->trdMoney / pOrder->trdQty;
		}

		fprintf(fp,
				"%lld,%lld,%lld,%d,%d,%lld,%s,%d,%s,%s,%d,%d,%d,%d,%d,%.3f,%d,%d,%lld,%.3f,%d,%d,%.2f,%d,%f,%f,%lld,%lld,"
						"%lld,%lld,%lld,%d,%d,%s,%d,%.2f,%lld,%d,%.3f,%.3f\n",
				pOrder->idx, pOrder->request.reqId, pOrder->request.plotid,
				pOrder->request.localId, pOrder->request.clocalId,
				pOrder->ordid, pOrder->request.customerId,
				pOrder->request.market, pOrder->request.securityId,
				pOrder->request.investId, pOrder->envno, pOrder->request.bsType,
				pOrder->request.isCancel, pOrder->request.ordType,
				pOrder->request.ordQty, pOrder->request.ordPrice * XPRICE_DIV,
				pOrder->request.orgEnvno, pOrder->request.orgLocalId,
				pOrder->request.orgOrdId, pOrder->request._lastPx * XPRICE_DIV,
				pOrder->request._lastTime, pOrder->trdQty,
				eXBuy == pOrder->request.bsType ?
						-pOrder->trdMoney * XPRICE_DIV :
						pOrder->trdMoney * XPRICE_DIV, pOrder->ordStatus,
				XNsTime2D(pOrder->_sendLocTime), XNsTime2D(pOrder->_cnfLocTime),
				pOrder->_cnfLocTime - pOrder->_sendLocTime, /**< 本地柜台确认 - 本地委托发送 */
				pOrder->_cnfExTime - pOrder->_sendLocTime, /**< 交易所本地确认时间  - 本地发送 */
				pOrder->_cnfExTime - pOrder->request._mktTime, /**< 交易所本地确认  - 策略出发行情时间 */
				pOrder->_sendTime, pOrder->_cnfTime,
				XMsC2S(pOrder->_cnfTime) - XMsC2S(pOrder->_sendTime), /**< 交易所确认 - 柜台发送 */
				pOrder->errorno, pOrder->errmsg, pOrder->locFrzHold,
				pOrder->locFrzMoney * XPRICE_DIV, pOrder->request._bizIndex,
				pOrder->exeStatus, tradePx * XPRICE_DIV, avgTdPx * XPRICE_DIV);

	}

	fclose(fp);
}

XVoid XExpTrade(XChar *expFile) {
	XIdx i;
	XTradeT *pTrade = NULL;
	FILE *fp = NULL;

	XMonitorT *pMonitor = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}
	fprintf(fp,
			"id,trdId,customerId,market,securityId,investid,trdSide,trdTime,trdQty,trdPrice,trdAmt,ordid\n");
	for (i = 0; i < pMonitor->iTTrade; i++) {
		pTrade = XFndVTradeById(i + 1);
		if (NULL == pTrade) {
			continue;
		}
		fprintf(fp, "%lld,%lld,%s,%d,%s,%s,%d,%d,%d,%.3f,%.2f,%lld\n",
				pTrade->idx, pTrade->trdId, pTrade->customerId, pTrade->market,
				pTrade->securityId, pTrade->investId, pTrade->trdSide,
				pTrade->trdTime, pTrade->trdQty,
				eXBuy == pTrade->trdSide ?
						-pTrade->trdPrice * XPRICE_DIV :
						pTrade->trdPrice * XPRICE_DIV,
				pTrade->trdAmt * XPRICE_DIV, pTrade->ordid);

	}

	fclose(fp);
}

XVoid XExpHold(XChar *expFile) {
	XIdx i;
	XHoldT *pHold = NULL;
	FILE *fp = NULL;
	XRSnapshotT *pSnapshot = NULL;
	XMonitorT *pMonitor = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}
	fprintf(fp,
			"id,customerId,investid,market,securityId,orgHld,orgAvlHld,orgCostAmt,totalBuyHld,totalSellHld,"
					"sumHld,sellAvlHld,countSellAvlHld,etfAvlHld,costPrice,locFrz,tradePx,sellFrzHold,totalBuyAmt,totalSellAmt\n");
	for (i = 0; i < pMonitor->iTHold; i++) {
		pHold = XFndVHoldById(i + 1);
		if (NULL == pHold) {
			continue;
		}

		pSnapshot = XFndVRSnapshotByKey(pHold->market, pHold->securityId);
		if (NULL != pSnapshot) {
			fprintf(fp,
					"%lld,%s,%s,%d,%s,%lld,%lld,%.2f,%lld,%lld,%lld,%lld,%lld,%lld,%.3f,%lld,%.3f,%lld,%.2f,%.2f\n",
					pHold->idx, pHold->customerId, pHold->investId,
					pHold->market, pHold->securityId, pHold->orgHld,
					pHold->orgAvlHld, pHold->orgCostAmt * XPRICE_DIV,
					pHold->totalBuyHld, pHold->totalSellHld, pHold->sumHld,
					pHold->sellAvlHld, pHold->countSellAvlHld, pHold->etfAvlHld,
					pHold->costPrice * XPRICE_DIV, pHold->locFrz,
					pSnapshot->tradePx * XPRICE_DIV, pHold->sellFrz,
					pHold->totalBuyAmt * XPRICE_DIV,
					pHold->totalSellAmt * XPRICE_DIV);
		} else {
			fprintf(fp,
					"%lld,%s,%s,%d,%s,%lld,%lld,%.2f,%lld,%lld,%lld,%lld,%lld,%lld,%.3f,%lld,0,%lld,%.2f,%.2f\n",
					pHold->idx, pHold->customerId, pHold->investId,
					pHold->market, pHold->securityId, pHold->orgHld,
					pHold->orgAvlHld, pHold->orgCostAmt * XPRICE_DIV,
					pHold->totalBuyHld, pHold->totalSellHld, pHold->sumHld,
					pHold->sellAvlHld, pHold->countSellAvlHld, pHold->etfAvlHld,
					pHold->costPrice * XPRICE_DIV, pHold->locFrz,
					pHold->sellFrz, pHold->totalBuyAmt * XPRICE_DIV,
					pHold->totalSellAmt * XPRICE_DIV);
		}

	}

	fclose(fp);
}

XVoid XExpSellHold(XChar *expFile) {
	XIdx i;
	XHoldT *pHold = NULL;
	FILE *fp = NULL;
	XMonitorT *pMonitor = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}
	fprintf(fp,
			"#市场(上海:1 深圳:2),证券代码,买卖方向(买:1 卖:2),时间批次1(HHMMSSsss),时间批次2(HHMMSSsss),委托数量类型(0:数量 1:资金),委托金额/委托数量,涨速(拉升过快卖出 -1不限制),卖一未成交量(-1表示不控制),当前成交金额或最近2分钟成交金额(万元 0时涨速同时无意义),买一封单金额不足撤单(万元 -1不控制),是否可以交易(0允许交易 1不允许),类型标识(0普通卖出 6盈利卖出),开盘涨幅,涨停卖倍量,涨停卖倍量2\n");
	for (i = 0; i < pMonitor->iTHold; i++) {
		pHold = XFndVHoldById(i + 1);
		if (NULL == pHold || pHold->sumHld <= 0) {
			continue;
		}

		//去掉逆回购代码 TODO

		fprintf(fp, "%d,%s,2,%d,%d,0,%lld,%d,-1, 0, 0,0,%d,0,0,0\n",
				pHold->market, pHold->securityId, 94000000, 100000000,
				pHold->sumHld, 180, 6);
	}

	fclose(fp);
}

XVoid XExpInvest(XChar *expFile) {
	XInvestT *pInvest = NULL;
	FILE *fp = NULL;
	XMonitorT *pMonitor = NULL;
	XIdx i = 0;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}

	fprintf(fp, "customerId, acctType, market, investId,quota\n");
	for (i = 0; i < pMonitor->iTInvest; i++) {
		pInvest = XFndVInvestById(i + 1);
		if (NULL == pInvest) {
			continue;
		}
		fprintf(fp, "%s,%d,%d,%s,%d\n", pInvest->customerId, pInvest->acctType,
				pInvest->market, pInvest->investId, pInvest->mainQuota);
	}

	fclose(fp);
}

XVoid XExpStock(XChar *expFile) {
	XIdx i;
	XStockT *pStock = NULL;
	FILE *fp = NULL;

	XMonitorT *pMonitor = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}
	fprintf(fp,
			"id,market,securityId,securityName,secStatus,secType,subSecType,baseSecurityId,"
					"prdType,isPriceLimit, isDayTrading,buyUnit,sellUnit,preClose,priceTick,HighPrice,LowPrice,"
					"outstandingShare,publicfloatShare,maturityDate,lmtBuyMinQty,lmtBuyMaxQty,convPx\n");
	for (i = 0; i < pMonitor->iTStock; i++) {
		pStock = XFndVStockById(i + 1);
		if (NULL == pStock) {
			continue;
		}
		fprintf(fp,
				"%lld,%d,%s,%s,%d,%d,%d,%s,%d,%d,%d,%d,%d,%.3f,%.3f,%.3f,%.3f,%lld,%lld,%d,%d,%d,%.3f\n",
				pStock->idx, pStock->market, pStock->securityId,
				pStock->securityName, pStock->secStatus, pStock->secType,
				pStock->subSecType, pStock->baseSecurityId, pStock->prdType,
				pStock->isPriceLimit, pStock->isDayTrading, pStock->buyUnit,
				pStock->sellUnit, pStock->preClose * XPRICE_DIV,
				pStock->priceTick * XPRICE_DIV, pStock->upperPrice * XPRICE_DIV,
				pStock->lowerPrice * XPRICE_DIV, pStock->outstandingShare,
				pStock->publicfloatShare, pStock->maturityDate,
				pStock->lmtBuyMinQty, pStock->lmtBuyMaxQty,
				pStock->convPx * XPRICE_DIV);
	}

	fclose(fp);
}

XVoid XExpCash(XChar *expFile) {
	XCashT *pCash = NULL;
	XIdx i;
	FILE *fp = NULL;

	XMonitorT *pMonitor = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}
	fprintf(fp,
			"id,customerId,acctType,accountId,beginBalance,beginAvailable,beginDrawable,curAvailable,totalBuy,totalSell,locFrz\n");
	for (i = 0; i < pMonitor->iTCash; i++) {
		pCash = XFndVCashById(i + 1);
		if (NULL == pCash) {
			continue;
		}

		fprintf(fp, "%lld,%s,%d,%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
				pCash->idx, pCash->customerId, pCash->acctType,
				pCash->accountId, pCash->beginBalance * XPRICE_DIV,
				pCash->beginAvailable * XPRICE_DIV,
				pCash->beginDrawable * XPRICE_DIV,
				pCash->curAvailable * XPRICE_DIV, pCash->totalBuy * XPRICE_DIV,
				pCash->totalSell * XPRICE_DIV, pCash->locFrz * XPRICE_DIV);
	}

	fclose(fp);
}

XVoid XExpSnapshot(XChar *expFile) {
	XIdx i;
	XSnapshotT *pSnapshot = NULL;
	FILE *fp = NULL;

	XMonitorT *pMonitor = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}
	fprintf(fp,
			"id,tradeDate,market,securityId,secStatus,preClosePx,openPx,highPx,lowPx,tradePx,numTrades,"
					"volumeTrade,amountTrade,updateTime,locTime,ask5,askqty5,ask4,askqty4,ask3,askqty3,ask2,askqty2,"
					"ask1,askqty1,bid1,bidqty1,bid2,bidqty2,bid3,bidqty3,bid4,bidqty4,bid5,bidqty5,gapTime(ns)\n");
	for (i = 0; i < pMonitor->iTSnapshot; i++) {

		pSnapshot = XFndVSnapshotById(i + 1);
		if (NULL == pSnapshot) {
			continue;
		}
		fprintf(fp,
				"%lld,%d,%d,%s,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%lld,%.2f,%d,%lld,%.3f,%lld,%.3f,%lld,%.3f,"
						"%lld,%.3f,%lld,%.3f,%lld,%.3f,%lld,%.3f,%lld,%.3f,%lld,%.3f,%lld,%.3f,%lld,%lld\n",
				pSnapshot->idx, pSnapshot->traday, pSnapshot->market,
				pSnapshot->securityId, pSnapshot->secStatus,
				pSnapshot->preClosePx * XPRICE_DIV,
				pSnapshot->openPx * XPRICE_DIV, pSnapshot->highPx * XPRICE_DIV,
				pSnapshot->lowPx * XPRICE_DIV, pSnapshot->tradePx * XPRICE_DIV,
				pSnapshot->numTrades, pSnapshot->volumeTrade,
				pSnapshot->amountTrade * XPRICE_DIV, pSnapshot->updateTime,
				pSnapshot->_genTime, pSnapshot->ask[4] * XPRICE_DIV,
				pSnapshot->askqty[4], pSnapshot->ask[3] * XPRICE_DIV,
				pSnapshot->askqty[3], pSnapshot->ask[2] * XPRICE_DIV,
				pSnapshot->askqty[2], pSnapshot->ask[1] * XPRICE_DIV,
				pSnapshot->askqty[1], pSnapshot->ask[0] * XPRICE_DIV,
				pSnapshot->askqty[0], pSnapshot->bid[0] * XPRICE_DIV,
				pSnapshot->bidqty[0], pSnapshot->bid[1] * XPRICE_DIV,
				pSnapshot->bidqty[1], pSnapshot->bid[2] * XPRICE_DIV,
				pSnapshot->bidqty[2], pSnapshot->bid[3] * XPRICE_DIV,
				pSnapshot->bidqty[3], pSnapshot->bid[4] * XPRICE_DIV,
				pSnapshot->bidqty[4],
				pSnapshot->_genTime - pSnapshot->_recvTime);

	}

	fclose(fp);
}

XVoid XExpKSnap(XChar *expFile) {
	XIdx i;
	XInt j = 0;
	XSnapshotT *pSnapshot = NULL;
	FILE *fp = NULL;

	XMonitorT *pMonitor = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}
	fprintf(fp,
			"id,tradeDate,updateTime,market,securityId,open,high,low,close,qty\n");
	for (i = 0; i < pMonitor->iTStock; i++) {

		pSnapshot = XFndVSnapshotById(i + 1);
		if (NULL == pSnapshot) {
			continue;
		}

		for (j = 0; j < SNAPSHOT_K1_CNT; j++) {
			/**
			 fprintf(fp,
			 "%lld,%d,%d,%d,%s,%d,%d,%d,%d,%lld\n",
			 pSnapshot->idx, pSnapshot->traday, pSnapshot->kline1[j].updateTime, pSnapshot->market,
			 pSnapshot->securityId, pSnapshot->kline1[j].open, pSnapshot->kline1[j].high,
			 pSnapshot->kline1[j].low,pSnapshot->kline1[j].close,
			 pSnapshot->kline1[j].qty);
			 */
		}

	}

	fclose(fp);
}

XVoid XExpK1(XChar *expFile) {
	FILE *fp = NULL;
	XMonitorT *pMonitor = NULL;
	XIdx cursor = -1, i;
	XRSnapshotT *pRSnapshot = NULL;
	XInt icnt = 0;
	XKLineT *klines = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");

	fprintf(fp, "market,security,time1,close1,time2,close2,time3,close3,time4,close4,time5,close5\n");
	for (i = 0; i < pMonitor->iTOrderBook; i++) {
		pRSnapshot = XFndVRSnapshotById(i + 1);
		if (NULL == pRSnapshot) {
			continue;
		}

		icnt = 0;
		fprintf(fp, "%d,%s", pRSnapshot->market, pRSnapshot->securityId);
		klines = GetKlinesByBlock(pRSnapshot->idx, 0);
		//遍历行情,s
		cursor = (SNAPSHOT_K1_CNT + pRSnapshot->kcursor1 - 5 + icnt)
				% SNAPSHOT_K1_CNT;
		while (cursor >= 0) {
			if (icnt >= 5) {
				break;
			}

			fprintf(fp, ",%d,%d", klines[cursor].updateTime,
					klines[cursor].close);
			icnt++;

			cursor = (SNAPSHOT_K1_CNT + pRSnapshot->kcursor1 - 5 + icnt)
					% SNAPSHOT_K1_CNT;
		}
		fprintf(fp, "\n");
	}

	fclose(fp);
}

XVoid XExpK5(XChar *expFile) {
	FILE *fp = NULL;
	XMonitorT *pMonitor = NULL;
	XIdx cursor = -1, i;
	XRSnapshotT *pRSnapshot = NULL;
	XInt icnt = 0;
	XKLineT *klines = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	fprintf(fp, "market,security");
	icnt = 0;
	for(icnt = 0; icnt < 60; icnt++)
	{
		fprintf(fp, ",time%d,close%d", icnt + 1, icnt + 1);
	}
	fprintf(fp, "\n");
	for (i = 0; i < pMonitor->iTOrderBook; i++) {
		pRSnapshot = XFndVRSnapshotById(i + 1);
		if (NULL == pRSnapshot) {
			continue;
		}

		icnt = 0;
		fprintf(fp, "%d,%s", pRSnapshot->market, pRSnapshot->securityId);
		klines = GetKlinesByBlock(pRSnapshot->idx, 1);
		//遍历行情,从最远的开始写
		cursor = (SNAPSHOT_K5_CNT + pRSnapshot->kcursor5 - 60 + icnt)
				% SNAPSHOT_K5_CNT;
		while (cursor >= 0) {
			if (icnt >= 60) {
				break;
			}
			fprintf(fp, ",%d,%d", klines[cursor].updateTime,
					klines[cursor].close);

			icnt++;
			cursor = (SNAPSHOT_K5_CNT + pRSnapshot->kcursor5 - 60 + icnt)
					% SNAPSHOT_K5_CNT;
		}
		fprintf(fp, "\n");
	}

	fclose(fp);
}

XVoid XExpDifSnapshot(XChar *expFile) {

	XIdx i;
	XSnapshotT *pSnapshot = NULL;
	XRSnapshotT *pRSnapshot = NULL;
	FILE *fp = NULL;

	XMonitorT *pMonitor = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}
	fprintf(fp,
			"id,tradeDate,market,securityId,type,secStatus,preClosePx,openPx,highPx,lowPx,tradePx,numTrades,volumeTrade,amountTrade,"
					"updateTime(ms),locTime(ns),ask5,askqty5,askcqty5,ask4,askqty4,askcqty4,ask3,askqty3,askcqty3,ask2,askqty2,askcqty2,ask1,askqty1,"
					"askcqty1,bid1,bidqty1,bidcqty1,bid2,bidqty2,bidcqty2,bid3,bidqty3,bidcqty3,bid4,bidqty4,bidcqty4,bid5,bidqty5,bidcqty5,"
					"gapTimes(ns),_channel,version,upperPx,lowerPx,driveAskPx,driveBidPx,"
					"totalBidQty,totalBidCQty,curUpBidQty,curUpBidCQty\n");
	for (i = 0; i < pMonitor->iTOrderBook; i++) {

		pRSnapshot = XFndVRSnapshotById(i + 1);
		if (NULL == pRSnapshot) {
			continue;
		}

		fprintf(fp,
				"%lld,%d,%d,%s,0,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%lld,%.2f,%d,%f,"
						"%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,"
						"%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,"
						"%lld,%.3f,%lld,%lld,%lld, %d,%lld,%.3f,%.3f,%.3f,%.3f,%lld,%lld,%lld,%lld\n",
				pRSnapshot->idx, pRSnapshot->traday, pRSnapshot->market,
				pRSnapshot->securityId, pRSnapshot->secStatus,
				pRSnapshot->preClosePx * XPRICE_DIV,
				pRSnapshot->openPx * XPRICE_DIV,
				pRSnapshot->highPx * XPRICE_DIV, pRSnapshot->lowPx * XPRICE_DIV,
				pRSnapshot->tradePx * XPRICE_DIV, pRSnapshot->numTrades,
				pRSnapshot->volumeTrade, pRSnapshot->amountTrade * XPRICE_DIV,
				pRSnapshot->updateTime, XNsTime2D(pRSnapshot->_genTime),
				pRSnapshot->ask[4] * XPRICE_DIV, pRSnapshot->askqty[4],
				pRSnapshot->askcqty[4], pRSnapshot->ask[3] * XPRICE_DIV,
				pRSnapshot->askqty[3], pRSnapshot->askcqty[3],
				pRSnapshot->ask[2] * XPRICE_DIV, pRSnapshot->askqty[2],
				pRSnapshot->askcqty[2], pRSnapshot->ask[1] * XPRICE_DIV,
				pRSnapshot->askqty[1], pRSnapshot->askcqty[1],
				pRSnapshot->ask[0] * XPRICE_DIV, pRSnapshot->askqty[0],
				pRSnapshot->askcqty[0], pRSnapshot->bid[0] * XPRICE_DIV,
				pRSnapshot->bidqty[0], pRSnapshot->bidcqty[0],
				pRSnapshot->bid[1] * XPRICE_DIV, pRSnapshot->bidqty[1],
				pRSnapshot->bidcqty[1], pRSnapshot->bid[2] * XPRICE_DIV,
				pRSnapshot->bidqty[2], pRSnapshot->bidcqty[2],
				pRSnapshot->bid[3] * XPRICE_DIV, pRSnapshot->bidqty[3],
				pRSnapshot->bidcqty[3], pRSnapshot->bid[4] * XPRICE_DIV,
				pRSnapshot->bidqty[4], pRSnapshot->bidcqty[4],
				pRSnapshot->_genTime - pRSnapshot->_recvTime,
				pRSnapshot->_channel, pRSnapshot->version,
				pRSnapshot->upperPx * XPRICE_DIV,
				pRSnapshot->lowerPx * XPRICE_DIV,
				pRSnapshot->driveAskPx * XPRICE_DIV,
				pRSnapshot->driveBidPx * XPRICE_DIV, pRSnapshot->totalBuyOrdQty,
				pRSnapshot->totalSellOrdQty, pRSnapshot->_catchUpBidQty,
				pRSnapshot->_catchUpBidCQty);

		pSnapshot = XFndVSnapshotByKey(pRSnapshot->market,
				pRSnapshot->securityId);
		if (NULL == pSnapshot) {
			continue;
		}
		fprintf(fp,
				"%lld,%d,%d,%s,1,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%lld,%.2f,%d,%f,"
						"%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,"
						"%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,"
						"%lld,%.3f,%lld,%lld,%lld,%d,%lld,%.3f,%.3f,0,0,0,0,0,0\n",
				pSnapshot->idx, pSnapshot->traday, pSnapshot->market,
				pSnapshot->securityId, pSnapshot->secStatus,
				pSnapshot->preClosePx * XPRICE_DIV,
				pSnapshot->openPx * XPRICE_DIV, pSnapshot->highPx * XPRICE_DIV,
				pSnapshot->lowPx * XPRICE_DIV, pSnapshot->tradePx * XPRICE_DIV,
				pSnapshot->numTrades, pSnapshot->volumeTrade,
				pSnapshot->amountTrade * XPRICE_DIV, pSnapshot->updateTime,
				XNsTime2D(pSnapshot->_genTime), pSnapshot->ask[4] * XPRICE_DIV,
				pSnapshot->askqty[4], pSnapshot->askcqty[4],
				pSnapshot->ask[3] * XPRICE_DIV, pSnapshot->askqty[3],
				pSnapshot->askcqty[3], pSnapshot->ask[2] * XPRICE_DIV,
				pSnapshot->askqty[2], pSnapshot->askcqty[2],
				pSnapshot->ask[1] * XPRICE_DIV, pSnapshot->askqty[1],
				pSnapshot->askcqty[1], pSnapshot->ask[0] * XPRICE_DIV,
				pSnapshot->askqty[0], pSnapshot->askcqty[0],
				pSnapshot->bid[0] * XPRICE_DIV, pSnapshot->bidqty[0],
				pSnapshot->bidcqty[0], pSnapshot->bid[1] * XPRICE_DIV,
				pSnapshot->bidqty[1], pSnapshot->bidcqty[1],
				pSnapshot->bid[2] * XPRICE_DIV, pSnapshot->bidqty[2],
				pSnapshot->bidcqty[2], pSnapshot->bid[3] * XPRICE_DIV,
				pSnapshot->bidqty[3], pSnapshot->bidcqty[3],
				pSnapshot->bid[4] * XPRICE_DIV, pSnapshot->bidqty[4],
				pSnapshot->bidcqty[4],
				pSnapshot->_genTime - pSnapshot->_recvTime, pSnapshot->_channel,
				pSnapshot->version, pSnapshot->upperPx * XPRICE_DIV,
				pSnapshot->lowerPx * XPRICE_DIV);

	}

	fclose(fp);

}

XVoid XExpRSnapshot(XChar *expFile) {

	XIdx i;
	XRSnapshotT *pRSnapshot = NULL;
	FILE *fp = NULL;
	XMonitorT *pMonitor = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}
	fprintf(fp,
			"id,tradeDate,market,securityId,secStatus,preClosePx,openPx,highPx,lowPx,tradePx,numTrades,volumeTrade,amountTrade,"
					"updateTime(ms),recvTime(ns),genTime(ns),ask5,askqty5,askcqty5,ask4,askqty4,askcqty4,ask3,askqty3,askcqty3,ask2,askqty2,askcqty2,ask1,askqty1,"
					"askcqty1,bid1,bidqty1,bidcqty1,bid2,bidqty2,bidcqty2,bid3,bidqty3,bidcqty3,bid4,bidqty4,bidcqty4,bid5,bidqty5,bidcqty5,"
					"gapTimes(ns),_channel,version,upperPx,lowerPx,driveAskPx,driveBidPx,"
					"totalBidQty,totalAskQty,curUpBidQty,curUpBidCQty,bigBuyOrdAmt,bigSellOrdAmt,bigBuyOrdQty,bigSellOrdQty,bigBuyTrdAmt,bigSellTrdAmt,"
					"driveBuyAmt,driverSellTrdAmt,totalBuyOrdCnt,totalSellOrdCnt,sealTime,pchg,upperOfferOrdQty,upperOfferOrdCnt,bigBuyOrdCnt,bigSellOrdCnt\n");
	for (i = 0; i < pMonitor->iTOrderBook; i++) {

		pRSnapshot = XFndVRSnapshotById(i + 1);
		if (NULL == pRSnapshot) {
			continue;
		}

		fprintf(fp,
				"%lld,%d,%d,%s,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%lld,%.2f,%d,%f,%f,"
						"%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,"
						"%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,"
						"%lld,%.3f,%lld,%lld,%lld, %d,%lld,%.3f,%.3f,%.3f,%.3f,%lld,%lld,%lld,%lld,%.2f,%.2f,%lld,%lld,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%.2f,%lld,%d,%d,%d\n",
				pRSnapshot->idx, pRSnapshot->traday, pRSnapshot->market,
				pRSnapshot->securityId, pRSnapshot->secStatus,
				pRSnapshot->preClosePx * XPRICE_DIV,
				pRSnapshot->openPx * XPRICE_DIV,
				pRSnapshot->highPx * XPRICE_DIV, pRSnapshot->lowPx * XPRICE_DIV,
				pRSnapshot->tradePx * XPRICE_DIV, pRSnapshot->numTrades,
				pRSnapshot->volumeTrade, pRSnapshot->amountTrade * XPRICE_DIV,
				pRSnapshot->updateTime, XNsTime2D(pRSnapshot->_recvTime),
				XNsTime2D(pRSnapshot->_genTime),
				pRSnapshot->ask[4] * XPRICE_DIV, pRSnapshot->askqty[4],
				pRSnapshot->askcqty[4], pRSnapshot->ask[3] * XPRICE_DIV,
				pRSnapshot->askqty[3], pRSnapshot->askcqty[3],
				pRSnapshot->ask[2] * XPRICE_DIV, pRSnapshot->askqty[2],
				pRSnapshot->askcqty[2], pRSnapshot->ask[1] * XPRICE_DIV,
				pRSnapshot->askqty[1], pRSnapshot->askcqty[1],
				pRSnapshot->ask[0] * XPRICE_DIV, pRSnapshot->askqty[0],
				pRSnapshot->askcqty[0], pRSnapshot->bid[0] * XPRICE_DIV,
				pRSnapshot->bidqty[0], pRSnapshot->bidcqty[0],
				pRSnapshot->bid[1] * XPRICE_DIV, pRSnapshot->bidqty[1],
				pRSnapshot->bidcqty[1], pRSnapshot->bid[2] * XPRICE_DIV,
				pRSnapshot->bidqty[2], pRSnapshot->bidcqty[2],
				pRSnapshot->bid[3] * XPRICE_DIV, pRSnapshot->bidqty[3],
				pRSnapshot->bidcqty[3], pRSnapshot->bid[4] * XPRICE_DIV,
				pRSnapshot->bidqty[4], pRSnapshot->bidcqty[4],
				pRSnapshot->_genTime - pRSnapshot->_recvTime,
				pRSnapshot->_channel, pRSnapshot->version,
				pRSnapshot->upperPx * XPRICE_DIV,
				pRSnapshot->lowerPx * XPRICE_DIV,
				pRSnapshot->driveAskPx * XPRICE_DIV,
				pRSnapshot->driveBidPx * XPRICE_DIV, pRSnapshot->totalBuyOrdQty,
				pRSnapshot->totalSellOrdQty, pRSnapshot->_catchUpBidQty,
				pRSnapshot->_catchUpBidCQty,
				pRSnapshot->bigBuyOrdAmt * XPRICE_DIV,
				pRSnapshot->bigSellOrdAmt * XPRICE_DIV,
				pRSnapshot->bigBuyOrdQty, pRSnapshot->bigSellOrdQty,
				pRSnapshot->bigBuyTrdAmt * XPRICE_DIV,
				pRSnapshot->bigSellTrdAmt * XPRICE_DIV,
				pRSnapshot->outsideTrdAmt * XPRICE_DIV,
				pRSnapshot->insideTrdAmt * XPRICE_DIV,
				pRSnapshot->totalBuyOrdCnt, pRSnapshot->totalSellOrdCnt,
				pRSnapshot->_sealTime,
				(pRSnapshot->tradePx - pRSnapshot->preClosePx) * 100.0
						/ pRSnapshot->preClosePx, pRSnapshot->upperOfferOrdQty,
				pRSnapshot->upperOfferOrdCnt, pRSnapshot->bigBuyOrdCnt,
				pRSnapshot->bigSellOrdCnt);

	}

	fclose(fp);

}

XVoid XExpHisSnapshot(XChar *expFile) {

	XIdx i;
	XRSnapshotT *pRSnapshot = NULL;
	FILE *fp = NULL;
	XStockT *pStock = NULL;
	XMonitorT *pMonitor = NULL;
	XInt upperTimes = 0;
	XBool multVol = 0;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}
	fprintf(fp,
			"tradeDate,market,securityId,preClosePx,openPx,highPx,lowPx,tradePx,numTrades,volumeTrade,amountTrade, preVolume, multVol,upperTimes,upperBidOrdQty,upperBidOrdCnt,lowerOfferOrdQty,lowerOfferOrdCnt,upperOfferOrdQty,upperOfferOrdCnt,lowerBidOrdQty,lowerBidOrdCnt,yesUpperOfferOrdQty,yesUpperOfferOrdCnt\n");
	for (i = 0; i < pMonitor->iTOrderBook; i++) {

		pRSnapshot = XFndVRSnapshotById(i + 1);
		if (NULL == pRSnapshot) {
			continue;
		}
		pStock = XFndVStockByKey(pRSnapshot->market, pRSnapshot->securityId);
		if (NULL == pStock) {
			continue;
		}
		if (pRSnapshot->tradePx == pStock->upperPrice) {
			upperTimes = ++pStock->upperTimes;
			slog_debug(0, "[%d-%s],当天收盘涨停[%d-%d]", pRSnapshot->market,
					pRSnapshot->securityId, pRSnapshot->tradePx,
					pRSnapshot->upperPx);
		} else {
			upperTimes = 0;
		}
		if (pStock->ysVolumeTrade
				&& pRSnapshot->volumeTrade > 2 * pStock->ysVolumeTrade) {
			multVol = 1;
		} else {
			multVol = 0;
		}

		fprintf(fp,
				"%d,%d,%s,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%lld,%.2f,%lld,%d,%d,%lld,%d,%lld,%d,%lld,%d,%lld,%d,%lld,%d\n",
				pRSnapshot->traday, pRSnapshot->market, pRSnapshot->securityId,
				pRSnapshot->preClosePx * XPRICE_DIV,
				pRSnapshot->openPx * XPRICE_DIV,
				pRSnapshot->highPx * XPRICE_DIV, pRSnapshot->lowPx * XPRICE_DIV,
				pRSnapshot->tradePx * XPRICE_DIV, pRSnapshot->numTrades,
				pRSnapshot->volumeTrade, pRSnapshot->amountTrade * XPRICE_DIV,
				pStock->ysVolumeTrade, multVol, upperTimes,
				pRSnapshot->upperBidOrdQty, pRSnapshot->upperBidOrdCnt,
				pRSnapshot->lowerOfferOrdQty, pRSnapshot->lowerOfferOrdCnt,
				pRSnapshot->upperOfferOrdQty, pRSnapshot->upperOfferOrdCnt,
				pRSnapshot->lowerBidOrdQty, pRSnapshot->lowerBidOrdCnt,
				pStock->upperOfferOrdQty, pStock->upperOfferOrdCnt);
	}

	fclose(fp);

}

XVoid XExpStrategy(XChar *expFile) {
	XIdx i;
	XStrategyT *pStrategy = NULL;
	FILE *fp = NULL;

	XMonitorT *pMonitor = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}
	fprintf(fp,
			"id,plotid, frontid, market, securityId, bs, status, investId, ordPx, confPx, qtyType, ordQty,money, askQty, buyMoney, cdl, buyCtrlMoney, ordGapTime,ctrGapTime,sign,beginTime, endTime, isUpperStop, isCtrlStop, isAutoCtrl, upperQtyMulty, upperQtyMultyMin,nxtCtrlMoney,followCtrlMoney,ctrlUpRatio\r\n");
	for (i = 0; i < pMonitor->iTPlot; i++) {
		pStrategy = XFndVStrategyById(i + 1);
		if (NULL == pStrategy) {
			continue;
		}
		fprintf(fp,
				"%lld,%lld, %d, %d, %s, %d, %d, %s, %d, %d, %d,%d,%.2f,%lld,%.2f,%d,%.2f, %lld,%lld, %d, %d,%d, %d,%d,%d,%d,%d,%.2f,%.2f,%d\r\n",
				pStrategy->idx, pStrategy->plotid, pStrategy->plot.frontId,
				pStrategy->setting.market, pStrategy->setting.securityId,
				pStrategy->setting.bsType, pStrategy->status,
				pStrategy->investId, pStrategy->setting.ordPx,
				pStrategy->setting.conPx, pStrategy->setting.qtyType,
				pStrategy->setting.ordQty, pStrategy->setting.money * 1.0,
				pStrategy->setting.askQty, pStrategy->setting.buyMoney * 1.0,
				pStrategy->setting.cdl, pStrategy->setting.buyCtrlMoney * 1.0,
				pStrategy->plot.ordGapTime, pStrategy->plot.ctrGapTime,
				pStrategy->setting.sign, pStrategy->plot.beginTime,
				pStrategy->plot.endTime, pStrategy->plot.isUpperStop,
				pStrategy->plot.isCtrlStop, pStrategy->plot.isAutoCtrl,
				pStrategy->setting.upperQtyMulty,
				pStrategy->setting.upperQtyMultyMin,
				pStrategy->setting.nxtCtrlMoney * 1.0,
				pStrategy->setting.followCtrlMoney * 1.0,
				pStrategy->plot.ctrlUpRatio);
	}

	fclose(fp);
}

XVoid XExpSnapLevel(XChar *expFile) {
	XIdx i;
	XPriceLevelT *pPriceLevel = NULL;
	XOrderBookT *pOrderBook = NULL;
	FILE *fp = NULL;
	XMonitorT *pMonitor = NULL;
	XNum level = 0;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}

	fprintf(fp, "market,security,tradepx,level,bs,price,qty\n");
	for (i = 0; i < pMonitor->iTOrderBook; i++) {
		pOrderBook = XFndVOrderBookById(i + 1);
		if (NULL == pOrderBook) {
			continue;
		}

		level = 0;
		pPriceLevel = XFndVPriceLevelById(pOrderBook->sellLevelIdx,
				pOrderBook->snapshot.market);
		while (NULL != pPriceLevel) {
			fprintf(fp, "%d,%s,%d,%d,%d,%d,%lld\n", pOrderBook->snapshot.market,
					pOrderBook->snapshot.securityId,
					pOrderBook->snapshot.tradePx, level, eXSell,
					pPriceLevel->entry.price, pPriceLevel->entry.qty);

			//档位价格大于最新价的时候，输出
			if (pOrderBook->snapshot.tradePx > pPriceLevel->entry.price) {
				break;
			}
			level++;
			pPriceLevel = XFndVPriceLevelById(pPriceLevel->next,
					pOrderBook->snapshot.market);
		}

		level = 0;
		pPriceLevel = XFndVPriceLevelById(pOrderBook->buyLevelIdx,
				pOrderBook->snapshot.market);
		while (NULL != pPriceLevel) {

			fprintf(fp, "%d,%s,%d,%d,%d,%d,%lld\n", pOrderBook->snapshot.market,
					pOrderBook->snapshot.securityId,
					pOrderBook->snapshot.tradePx, level, eXBuy,
					pPriceLevel->entry.price, pPriceLevel->entry.qty);

			//档位价格小于最新的时候，输出
			if (pPriceLevel->entry.price < pOrderBook->snapshot.tradePx) {
				break;
			}
			level++;
			pPriceLevel = XFndVPriceLevelById(pPriceLevel->next,
					pOrderBook->snapshot.market);
		}

	}

	fclose(fp);

}

XVoid XExpBlock(XChar *expFile) {
	XIdx i;
	XBlockT *pBlock = NULL;
	FILE *fp = NULL;
	XMonitorT *pMonitor = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}

	fprintf(fp, "blockno,blockname,count,zdf\n");
	for (i = 0; i < pMonitor->iTBlock; i++) {
		pBlock = XFndVBlockById(i + 1);
		if (NULL == pBlock) {
			continue;
		}

		fprintf(fp, "%s,%s,%d,%.2f\n", pBlock->blockNo, pBlock->blockName,
				pBlock->count, pBlock->zdf * 0.01);

	}

	fclose(fp);

}

XVoid XExpBlockInfo(XChar *expFile) {
	XIdx i;
	XBlockInfoT *pBlock = NULL;
	FILE *fp = NULL;
	XMonitorT *pMonitor = NULL;
	XStockT *pStock = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return;
	}

	fp = fopen(expFile, "w");
	if (NULL == fp) {
		return;
	}

	fprintf(fp, "blockno,securityid,market,zdf,securityName\n");
	for (i = 0; i < pMonitor->iTBlockInfo; i++) {
		pBlock = XFndVBlockInfoById(i + 1);
		if (NULL == pBlock) {
			continue;
		}
		pStock = XFndVStockByKey(pBlock->market, pBlock->securityId);
		if (NULL == pStock) {
			continue;
		}
		fprintf(fp, "%s,%s,%d,%.2f,%s\n", pBlock->blockNo, pBlock->securityId,
				pBlock->market, pBlock->zdf * 0.01, pStock->securityName);

	}

	fclose(fp);

}

XVoid XExpPriceLevel(XChar market, XChar *securityId, XInt speed) {
	XOrderBookT *pOrderBook = NULL;
	XPriceLevelT *pBuyPrice = NULL, *pSellPrice = NULL;
	XInt iCount = 0;
	XIdx buyidx, sellidx;

	pOrderBook = XFndVOrderBook(market, securityId);
	if (NULL == pOrderBook) {
		return;
	}

	pBuyPrice = XFndVPriceLevelById(pOrderBook->buyLevelIdx,
			pOrderBook->snapshot.market);
	pSellPrice = XFndVPriceLevelById(pOrderBook->sellLevelIdx,
			pOrderBook->snapshot.market);

	buyidx = pBuyPrice->idx;
	sellidx = pSellPrice->idx;

	while (buyidx > 0) {
		pBuyPrice = XFndVPriceLevelById(buyidx, pOrderBook->snapshot.market);
		if (NULL == pBuyPrice) {
			break;
		}
		iCount++;
		slog_info(0, "买[%4d] 价格[%4.3f],数量[%lld],位置[%lld],前[%lld],后[%lld]",
				iCount, pBuyPrice->entry.price * 0.0001, pBuyPrice->entry.qty,
				buyidx, pBuyPrice->prev, pBuyPrice->next);
		if (iCount >= speed) {
			break;
		}
		buyidx = pBuyPrice->next;
	}
	slog_info(0, "########## [%d-%s] 最新价[%d],更新时间[%d]##########",
			pOrderBook->snapshot.market, pOrderBook->snapshot.securityId,
			pOrderBook->snapshot.tradePx * 0.0001,
			pOrderBook->snapshot.updateTime);
	iCount = 0;
	while (sellidx > 0) {

		pSellPrice = XFndVPriceLevelById(sellidx, pOrderBook->snapshot.market);
		if (NULL == pSellPrice) {
			break;
		}

		iCount++;
		slog_info(0, "卖[%4d] 价格[%4.3f],数量[%lld],位置[%lld], 前[%lld],后[%lld]",
				iCount, pSellPrice->entry.price * 0.0001, pSellPrice->entry.qty,
				sellidx, pSellPrice->prev, pSellPrice->next);
		if (iCount >= speed) {
			break;
		}

		sellidx = pSellPrice->next;
	}
}

XVoid XPLevelPrint(XChar market, XChar *securityId, XInt speed) {
#ifdef __USED_NCURSES__	

	XOrderBookT* pOrderBook = NULL;
	XPriceLevelT *pBuyPrice = NULL, *pSellPrice = NULL;
	XInt iCount = 0;
	XIdx buyidx = 0, sellidx = 0;
	XInt iline, icol = 0;
	XChar buf[100];
	XRSnapshotT snapshot;

	setlocale(LC_ALL, "");
	setlocale(LC_CTYPE, "");
	raw();
	// 初始化ncurses
    initscr();
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_RED, COLOR_WHITE);
	init_pair(4, COLOR_GREEN, COLOR_WHITE);
    //cbreak();
    //noecho();
    keypad(stdscr, TRUE);
	curs_set(0);
    	
	clear();
	while(1)
	{
		clear();

		pOrderBook = XFndVOrderBook(market, securityId);
		if(NULL == pOrderBook)
		{
			continue;
		}

		snapshot = pOrderBook->snapshot;
		iline = 1;
		box(stdscr, ACS_VLINE, ACS_HLINE);
		
		pBuyPrice = XFndVPriceLevelById(pOrderBook->buyLevelIdx, snapshot.market);
		pSellPrice = XFndVPriceLevelById(pOrderBook->sellLevelIdx, snapshot.market);

		if(NULL != pBuyPrice)
		{
			buyidx = pBuyPrice->idx;
		}
		
		if(NULL != pSellPrice)
		{
			sellidx = pSellPrice->idx;
		}
		
		attron(COLOR_PAIR(3));		
		iCount = 0;
		icol = 0;
		while(sellidx > 0)
		{
			
			pSellPrice = XFndVPriceLevelById(sellidx, snapshot.market);
			if(NULL == pSellPrice)
			{
				break;
			}
			if(pSellPrice->entry.qty > 0)
			{
				iCount++;

				if(icol > 6)
				{
					icol = 0;
					iline++;
				}
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "[%4d] %4.3f %8lld | ", iCount, pSellPrice->entry.price * 0.0001, pSellPrice->entry.qty);
				mvprintw(iline, 2 + icol * strlen(buf), "%s", buf);
				icol++;
				if(iCount >= 105)
				{
					break;
				}
			}
			sellidx = pSellPrice->next;
		}
		attroff(COLOR_PAIR(3));

		attron(COLOR_PAIR(2));
		mvprintw(++iline, 2, "[%-d-%d-%s] == %4.3f  %d,涨跌幅[%2.2f\%]主买[%4.3f],主卖[%4.3f],成交笔数[%8d],成交数量[%lld],净流入[%8.2f],累计委买金额[%8.2f],累计委卖金额[%8.2f]", 
		snapshot._channel, snapshot.market, snapshot.securityId, snapshot.tradePx * 0.0001, snapshot.updateTime,  (snapshot.tradePx - snapshot.preClosePx) * 100.0 / snapshot.preClosePx,
		snapshot.driveBidPx * 0.0001, snapshot.driveAskPx * 0.0001, snapshot.numTrades, snapshot.volumeTrade,
		(snapshot.outsideTrdAmt - snapshot.insideTrdAmt) * 0.0001, snapshot.totalBuyOrdAmt * 0.0001, snapshot.totalSellOrdAmt * 0.0001);
		attroff(COLOR_PAIR(2));

		++iline;
		iCount = 0;
		icol = 0;
		attron(COLOR_PAIR(4));	
		while(buyidx > 0)
		{
			pBuyPrice = XFndVPriceLevelById(buyidx, pOrderBook->snapshot.market);
			if(NULL == pBuyPrice)
			{
				break;
			}
			if(pBuyPrice->entry.qty > 0)
			{
				iCount++;
				memset(buf, 0, sizeof(buf));
				if(icol > 6)
				{
					icol = 0;
					iline++;
				}

				sprintf(buf, "[%4d] %4.3f %8lld | ", iCount, pBuyPrice->entry.price * 0.0001, pBuyPrice->entry.qty);
				mvprintw(iline, 2 + icol * strlen(buf), "%s", buf);
				icol++;
				if(iCount >= 105)
				{
					break;
				}
			}
			buyidx = pBuyPrice->next;
		}
		attroff(COLOR_PAIR(4));

		
		mvprintw(++iline, 2, "-----------------------------------------------------------------------");
		mvprintw(++iline, 2, "                                        CTRL+C退出                     ");
		refresh();
		sleep(1);	
	}
	// 刷新窗口并等待用户按下任意键
    refresh();
	endwin();
#endif
}

XVoid XRSnapPrint(XChar market, XChar *securityId) {
#ifdef __USED_NCURSES__	
	XRSnapshotT *pSnapshot = NULL, *pBaseSnapshot = NULL;
	XInt i, iline;
	XStockT* pStock = NULL, *pBaseStock = NULL;
	XChar buf[1024];

	setlocale(LC_ALL, "");
	setlocale(LC_CTYPE, "");
	raw();
	// 初始化ncurses
    initscr();
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_RED, COLOR_WHITE);
	init_pair(4, COLOR_GREEN, COLOR_WHITE);
    //cbreak();
    //noecho();
    keypad(stdscr, TRUE);
	curs_set(0);
    	
	clear();
	while(1)
	{
		clear();

		pStock = XFndVStockByKey(market, securityId);
		if(NULL == pStock)
		{
			continue;
		}

		iline = 1;
		box(stdscr, ACS_VLINE, ACS_HLINE);

		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%s %s  u[%.3f] l[%.3f] ", pStock->securityId, pStock->securityName, pStock->upperPrice * XPRICE_DIV, pStock->lowerPrice * XPRICE_DIV);
		mvprintw(iline, 2, "%s", buf);
		if(strlen(pStock->baseSecurityId))
		{
			pBaseStock = XFndVStockByKey(pStock->market, pStock->baseSecurityId);
			if(NULL != pBaseStock)
			{
				mvprintw(iline, 2 + strlen(buf), "=>%s %s", pBaseStock->securityId, pBaseStock->securityName);
			}
		}
		
		iline++;


		pSnapshot = XFndVRSnapshotByKey(market, securityId);
		if(NULL == pSnapshot)
		{
			continue;
		}

		attron(COLOR_PAIR(3));		
		
		for(i = 9; i >= 0;i--)
		{
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%5.3f %9lld %9lld", pSnapshot->ask[i] * XPRICE_DIV, pSnapshot->askqty[i],  pSnapshot->askcqty[i]);
			mvprintw(iline, 2, "%s", buf);
			
			iline++;
		}		
		attroff(COLOR_PAIR(3));

		attron(COLOR_PAIR(2));
		mvprintw(iline, 2, "%5.3f %9d    [%2.2f\%]", pSnapshot->tradePx * XPRICE_DIV, pSnapshot->updateTime, (pSnapshot->tradePx - pSnapshot->preClosePx) * 100.0 / pSnapshot->preClosePx);
		if(strlen(pStock->baseSecurityId))
		{
			pBaseSnapshot = XFndVRSnapshotByKey(market, pStock->baseSecurityId);
			if(NULL != pBaseSnapshot)
			{
				++iline;
				mvprintw(iline, 2, "%5.3f %9d    [%2.2f\%]", pBaseSnapshot->tradePx * XPRICE_DIV, pBaseSnapshot->updateTime, (pBaseSnapshot->tradePx - pBaseSnapshot->preClosePx) * 100.0 / pBaseSnapshot->preClosePx);
			}
		}

		attroff(COLOR_PAIR(2));

		++iline;

		attron(COLOR_PAIR(4));
		for(i = 0; i <= 9;i++)
		{
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%5.3f %9lld %9lld", pSnapshot->bid[i] * XPRICE_DIV, pSnapshot->bidqty[i],  pSnapshot->bidcqty[i]);
			mvprintw(iline, 2, "%s", buf);
			iline++;
		}		
		attroff(COLOR_PAIR(4));

		
		mvprintw(++iline, 2, "-----------------------------------------------------------------------");
		mvprintw(++iline, 2, "                                        CTRL+C退出                     ");
		refresh();
		sleep(1);	
	}
	// 刷新窗口并等待用户按下任意键
    refresh();
	endwin();
#endif
}

XVoid XPRLevelPrint(XChar market, XChar *securityId, XInt speed) {

#ifdef __USED_NCURSES__	
	XOrderBookT* pOrderBook = NULL;
	XPriceLevelT *pBuyPrice = NULL, *pSellPrice = NULL;
	XInt iCount = 0;
	XIdx buyidx, sellidx;
	XInt iline, icol = 0;
	XChar buf[100];
	XRSnapshotT snapshot;

	setlocale(LC_ALL, "");
	setlocale(LC_CTYPE, "");
	raw();
	// 初始化ncurses
    initscr();
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_RED, COLOR_WHITE);
	init_pair(4, COLOR_GREEN, COLOR_WHITE);
    //cbreak();
    //noecho();
    keypad(stdscr, TRUE);
	curs_set(0);
    	
	clear();
	while(1)
	{
		clear();

		pOrderBook = XFndVOrderBook(market, securityId);
		if(NULL == pOrderBook)
		{
			continue;
		}
		snapshot = pOrderBook->snapshot;
		iline = 1;
		box(stdscr, ACS_VLINE, ACS_HLINE);
		
		pBuyPrice = XFndVPriceLevelById(pOrderBook->buyLevelIdx, snapshot.market);
		pSellPrice = XFndVPriceLevelById(pOrderBook->sellLevelIdx, snapshot.market);

		buyidx = pBuyPrice->idx;
		sellidx = pSellPrice->idx;

		attron(COLOR_PAIR(3));		
		
		iCount = 0;
		icol = 0;
		while(sellidx > 0)
		{
			
			pSellPrice = XFndVPriceLevelById(sellidx, snapshot.market);
			if(NULL == pSellPrice)
			{
				break;
			}
			if(pSellPrice->entry.qty > 0 && pSellPrice->entry.price >= snapshot.tradePx)
			{
				iCount++;

				if(icol > 6)
				{
					icol = 0;
					iline++;
				}
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "[%4d] %4.3f %8lld | ", iCount, pSellPrice->entry.price * 0.0001, pSellPrice->entry.qty);
				mvprintw(iline, 2 + icol * strlen(buf), "%s", buf);
				icol++;
				if(iCount >= 105)
				{
					break;
				}
			}
			sellidx = pSellPrice->next;
		}
		attroff(COLOR_PAIR(3));

		attron(COLOR_PAIR(2));

		mvprintw(++iline, 2, "[%-d-%d-%s] == %4.3f  %d,涨跌幅[%2.2f\%],主买[%4.3f],主卖[%4.3f],成交笔数[%8d],成交数量[%lld],净流入[%8.2f],累计委托买入金额[%8.2f],累计委托卖出金额[%8.2f]", 
		snapshot._channel, snapshot.market, snapshot.securityId, snapshot.tradePx * 0.0001, snapshot.updateTime, (snapshot.tradePx - snapshot.preClosePx) * 100.0 / snapshot.preClosePx,
		snapshot.driveBidPx * 0.0001, snapshot.driveAskPx * 0.0001, snapshot.numTrades, snapshot.volumeTrade,
		(snapshot.outsideTrdAmt - snapshot.insideTrdAmt) * 0.0001, snapshot.totalBuyOrdAmt * 0.0001, snapshot.totalSellOrdAmt * 0.0001);
		attroff(COLOR_PAIR(2));

		++iline;
		iCount = 0;
		icol = 0;
		attron(COLOR_PAIR(4));	
		while(buyidx > 0)
		{
			pBuyPrice = XFndVPriceLevelById(buyidx, snapshot.market);
			if(NULL == pBuyPrice)
			{
				break;
			}
			if(pBuyPrice->entry.qty > 0 && pBuyPrice->entry.price <= snapshot.tradePx)
			{
				iCount++;
				memset(buf, 0, sizeof(buf));
				if(icol > 6)
				{
					icol = 0;
					iline++;
				}

				sprintf(buf, "[%4d] %4.3f %8lld | ", iCount, pBuyPrice->entry.price * 0.0001, pBuyPrice->entry.qty);
				mvprintw(iline, 2 + icol * strlen(buf), "%s", buf);
				icol++;
				if(iCount >= 105)
				{
					break;
				}
			}
			buyidx = pBuyPrice->next;
		}
		attroff(COLOR_PAIR(4));

		mvprintw(++iline, 2, "-----------------------------------------------------------------------");
		mvprintw(++iline, 2, "                                        CTRL+C退出                     ");
		refresh();
		sleep(1);	
	}
	// 刷新窗口并等待用户按下任意键
    refresh();
	endwin();
#endif
}

int read_kline(const char *trade_file, XInt knum) {
	const char *col = NULL;
	XCsvHandleT handle;
	XInt iret = -1, i = 0;
	XStockT *pStock = NULL;
	XInt market = 0;
	XChar *securityId = NULL;
	XRSnapshotT *pSnapshot, snapshot;
	XIdx idx = -1;
	XKLineT *kline = NULL;

	iret = XCsvOpen(&handle, trade_file);
	if (iret) {
		slog_error(0, "文件不存在[%s]", trade_file);
		return (0);
	}

	while ((!XCsvReadLine(&handle))) {
		if (handle.colSize < 2) {
			slog_error(0, "列数错误");
			break;
		}
		col = handle.GetFieldByCol(&handle, 0);
		if (col) {
			market = atoi(col);
		}
		securityId = handle.GetFieldByCol(&handle, 1);
		if (NULL == securityId) {
			continue;
		}
		pStock = XFndVStockByKey(market, securityId);
		if (NULL == pStock) {
			continue;
		}

		idx = XFndOrderBook(market, securityId);
		if (idx < 1) {
			idx = XPutOrderBookHash(market, securityId);
		}

		memset(&snapshot, 0, sizeof(XRSnapshotT));
		// 存储涨跌停价
		pSnapshot = XFndVRSnapshotById(idx);
		if (NULL != pSnapshot) {
			memcpy(&snapshot, pSnapshot, XRSNAPSHOT_SIZE);
		} else {
			snapshot.idx = idx;
			snapshot.market = market;
			memcpy(snapshot.securityId, pStock->securityId, SECURITYID_LEN);

			snapshot.upperPx = pStock->upperPrice;
			snapshot.lowerPx = pStock->lowerPrice;
			snapshot.preClosePx = pStock->preClose;
			snapshot.secStatus = pStock->secStatus;

		}

		if (knum == 5) {
			kline = GetKlinesByBlock(snapshot.idx, 0);
			snapshot.kcursor1 = 0;
			for (i = 2; i < 2 * knum + 2; i++) {
				col = handle.GetFieldByCol(&handle, i);
				if (NULL == col) {
					continue;
				}
				kline[snapshot.kcursor1].updateTime = atoi(col);

				col = handle.GetFieldByCol(&handle, ++i);
				if (NULL == col) {
					continue;
				}
				kline[snapshot.kcursor1].close = atoi(col);
				snapshot.kcursor1++;
			}
		} else if (knum == 60) {
			snapshot.kcursor5 = 0;
			kline = GetKlinesByBlock(snapshot.idx, 1);
			for (i = 2; i < 2 * knum + 2; i++) {
				col = handle.GetFieldByCol(&handle, i);
				if (NULL == col) {
					continue;
				}
				kline[snapshot.kcursor5].updateTime = atoi(col);

				col = handle.GetFieldByCol(&handle, ++i);
				if (NULL == col) {
					continue;
				}
				kline[snapshot.kcursor5].close = atoi(col);
				snapshot.kcursor5++;
			}
		}

		XPutOrUpdVRSnapshot(&snapshot);
	}
	XCsvClose(&handle);

	return (iret);
}

int read_hissnapshot(const char *trade_file) {
	const char *col = NULL;
	XCsvHandleT handle;
	XInt iret = -1;
	XStockT *pStock = NULL;
	XInt market = 0;
	XChar *securityId = NULL;
	XSumQty qty = 0;
	XInt upperTimes = 0;
	XBool multVol = 0;

	iret = XCsvOpen(&handle, trade_file);
	if (iret) {
		slog_error(0, "文件不存在[%s]", trade_file);
		return (0);
	}

	while ((!XCsvReadLine(&handle))) {
		if (handle.colSize < 2) {
			slog_error(0, "列数错误");
			break;
		}
		col = handle.GetFieldByCol(&handle, 1);
		if (col) {
			market = atoi(col);
		}
		securityId = handle.GetFieldByCol(&handle, 2);
		if (NULL == securityId) {
			continue;
		}
		col = handle.GetFieldByCol(&handle, 9);
		if (col) {
			qty = atol(col);
		}

		col = handle.GetFieldByCol(&handle, 12);
		if (col) {
			multVol = atoi(col);
		}

		col = handle.GetFieldByCol(&handle, 13);
		if (col) {
			upperTimes = atoi(col);
		}

		pStock = XFndVStockByKey(market, securityId);
		if (NULL == pStock) {
			continue;
		}
		pStock->ysVolumeTrade = qty;
		pStock->ysMultiple = multVol;
		pStock->upperTimes = upperTimes;

		col = handle.GetFieldByCol(&handle, 14);
		if (col) {
			pStock->upperBidOrdQty = atol(col);
		}

		col = handle.GetFieldByCol(&handle, 15);
		if (col) {
			pStock->upperBidOrdCnt = atoi(col);
		}

		col = handle.GetFieldByCol(&handle, 16);
		if (col) {
			pStock->lowerOfferOrdQty = atol(col);
		}

		col = handle.GetFieldByCol(&handle, 17);
		if (col) {
			pStock->lowerOfferOrdCnt = atoi(col);
		}

		col = handle.GetFieldByCol(&handle, 18);
		if (col) {
			pStock->upperOfferOrdQty = atol(col);
		}

		col = handle.GetFieldByCol(&handle, 19);
		if (col) {
			pStock->upperOfferOrdCnt = atoi(col);
		}

		col = handle.GetFieldByCol(&handle, 20);
		if (col) {
			pStock->lowerBidOrdQty = atol(col);
		}

		col = handle.GetFieldByCol(&handle, 21);
		if (col) {
			pStock->lowerBidOrdCnt = atoi(col);
		}

	}
	XCsvClose(&handle);

	return (iret);
}

int read_block(const char *trade_file) {
	XCsvHandleT handle;
	XInt iret = -1;
	XChar *pblockNo = NULL;
	XChar *pblockName = NULL;
	XBlockT block;

	iret = XCsvOpen(&handle, trade_file);
	if (iret) {
		slog_error(0, "文件不存在[%s]", trade_file);
		return (0);
	}

	while ((!XCsvReadLine(&handle))) {
		if (handle.colSize < 2) {
			slog_error(0, "列数错误");
			break;
		}
		pblockNo = handle.GetFieldByCol(&handle, 0);
		pblockName = handle.GetFieldByCol(&handle, 1);
		if (NULL == pblockNo || NULL == pblockName) {
			continue;
		}
		memset(&block, 0, sizeof(XBlockT));
		memcpy(block.blockNo, pblockNo, strlen(pblockNo));
		memcpy(block.blockName, pblockName, strlen(pblockName));
		XPutOrUpdBlock(&block);
	}
	XCsvClose(&handle);

	return (iret);
}

int read_blockinfo(const char *trade_file) {
	XCsvHandleT handle;
	XInt iret = -1;
	XChar *pField = NULL;
	XBlockInfoT blockinfo;
	XBlockT *pBlock = NULL;

	iret = XCsvOpen(&handle, trade_file);
	if (iret) {
		slog_error(0, "文件不存在[%s]", trade_file);
		return (0);
	}

	while ((!XCsvReadLine(&handle))) {
		if (handle.colSize < 2) {
			slog_error(0, "列数错误");
			break;
		}
		memset(&blockinfo, 0, sizeof(XBlockInfoT));
		pField = handle.GetFieldByCol(&handle, 0);
		if (NULL == pField) {
			continue;
		}
		//找到板块对应的信息
		pBlock = XFndVBlockByKey(pField);
		if (NULL == pBlock) {
			continue;
		}
		memcpy(blockinfo.blockNo, pField, strlen(pField));

		pField = handle.GetFieldByCol(&handle, 1);
		if (NULL == pField) {
			continue;
		}
		memcpy(blockinfo.securityId, pField, strlen(pField));

		pField = handle.GetFieldByCol(&handle, 2);
		if (NULL == pField) {
			continue;
		}

		blockinfo.market = atoi(pField);

		if (-1
				!= XFndBlockInfo(blockinfo.blockNo, blockinfo.securityId,
						blockinfo.market)) {
			slog_warn(0, "已存在对应的数据[%s-%s]", blockinfo.blockNo,
					blockinfo.securityId);
			continue;
		}

		//存放为数组
		XPutBlockInfo(&blockinfo);

		//更新板块统计信息及起始位置信息
		pBlock->count++;
		if (!pBlock->beginIdx) {
			pBlock->beginIdx = blockinfo.idx;
		}
		//slog_debug(0, "板块[%s],板块内证券数[%d],起始位置[%lld]", pBlock->blockNo, pBlock->count, pBlock->beginIdx);
	}
	XCsvClose(&handle);

	return (iret);
}

XInt read_stock_convpx(XChar *convFile) {
	XCsvHandleT handle;
	XChar *col = NULL;
	XInt iret = -1;
	XChar market = -1;
	XChar *securityId = NULL;
	XInt convPx = -1;
	XStockT *pStock = NULL;
	XChar *baseSecurityId = NULL;
	XInt idate = -1;

	iret = XCsvOpen(&handle, convFile);
	if (iret) {
		slog_error(0, "文件不存在[%s]", convFile);
		return (0);
	}
	slog_debug(0, "更新转股价");

	while ((!XCsvReadLine(&handle))) {
		if (handle.colSize < 3) {
			continue;
		}
		col = handle.GetFieldByCol(&handle, 0);
		if (col) {
			market = atoi(col);
		}
		securityId = handle.GetFieldByCol(&handle, 1);
		if (NULL == securityId) {
			continue;
		}

		baseSecurityId = handle.GetFieldByCol(&handle, 2);
		if (NULL == baseSecurityId) {
			continue;
		}

		col = handle.GetFieldByCol(&handle, 3);
		if (col) {
			convPx = atoi(col);
		}

		col = handle.GetFieldByCol(&handle, 4);
		if (col) {
			idate = atoi(col);
		}

		pStock = XFndVStockByKey(market, securityId);
		if (NULL != pStock) {
			pStock->convPx = convPx;
			if (!strlen(pStock->baseSecurityId)) {
				memcpy(pStock->baseSecurityId, baseSecurityId,
						strlen(baseSecurityId));
			}
			pStock->maturityDate = idate;
//			slog_debug(0, "更新转股价[%d-%s],[%d]", market, securityId, convPx);
		}

	}

	XCsvClose(&handle);

	return 0;
}

XInt _XGetCounter(XIniT *config, const char *counter, XInt pos, XCustT *cust) {
	XInt iret = -1;
	const char *pValue = NULL;
	char key[200];

	if (NULL != counter && 0 == strcmp(counter, OES_COUNTER_SEC)) {
		cust->counter = eXCounterOes;
		cust->exchid = eXExchSec;
	} else if (NULL != counter && 0 == strcmp(counter, XTP_COUNTER_SEC)) {
		cust->counter = eXCounterXtp;
		cust->exchid = eXExchSec;
	} else if (NULL != counter && 0 == strcmp(counter, CTP_COUNTER_SEC)) {
		cust->counter = eXCounterCtp;
		cust->exchid = eXExchSec;
	} else {
		return (iret);
	}
	memset(&key, 0, sizeof(key));
	sprintf(key, "%s.%d", "name", pos);
	pValue = XINIGet(config, counter, key);
	if (NULL == pValue) {
		return (iret);
	}
	memcpy(cust->customerId, pValue, strlen(pValue));

	memset(&key, 0, sizeof(key));
	sprintf(key, "%s.%d", "password", pos);
	pValue = XINIGet(config, counter, key);
	if (NULL == pValue) {
		return (iret);
	}
	memcpy(cust->password, pValue, strlen(pValue));

	memset(&key, 0, sizeof(key));
	sprintf(key, "%s.%d", "exchange", pos);
	pValue = XINIGet(config, counter, key);
	if (NULL == pValue) {
		return (iret);
	}

	cust->type = atoi(pValue);

	memset(&key, 0, sizeof(key));
	sprintf(key, "%s.%d", "broker", pos);
	pValue = XINIGet(config, counter, key);
	if (NULL != pValue) {
		memcpy(cust->broker, pValue, strlen(pValue));
	}

	memset(&key, 0, sizeof(key));
	sprintf(key, "%s.%d", "remark", pos);
	pValue = XINIGet(config, counter, key);
	if (NULL != pValue) {
		memcpy(cust->remark, pValue, strlen(pValue));
	}

	memset(&key, 0, sizeof(key));
	sprintf(key, "%s.%d", "hd", pos);
	pValue = XINIGet(config, counter, key);
	if (NULL != pValue) {
		memcpy(cust->hd, pValue, strlen(pValue));
	}

	memset(&key, 0, sizeof(key));
	sprintf(key, "%s.%d", "envno", pos);
	pValue = XINIGet(config, counter, key);
	if (NULL != pValue) {
		cust->envno = atoi(pValue);
	}

	memset(&key, 0, sizeof(key));
	sprintf(key, "%s.%d", "cpuid", pos);
	pValue = XINIGet(config, counter, key);
	if (NULL != pValue) {
		cust->cpuid = atoi(pValue);
	}

	memset(&key, 0, sizeof(key));
	sprintf(key, "%s.%d", "ip", pos);
	pValue = XINIGet(config, counter, key);
	if (NULL != pValue) {
		memcpy(cust->ip, pValue, strlen(pValue));
	}

	return (0);
}

XInt XGetUser(const char *userConf, XCustT pCust[]) {
	XInt iret = 0;
	XIniT *config = NULL;
	XCustT cust = { 0 };
	XInt pos = 1;
	XInt iTotal = 0;

	config = XINILoad(userConf);
	if (NULL == config) {
		slog_error(0, "读取配置文件错误[%s]", userConf);
		return (-1);
	}

	pos = 1;
	///读取oes账户
	while (!iret) {
		memset(&cust, 0, sizeof(XCustT));
		iret = _XGetCounter(config, OES_COUNTER_SEC, pos, &cust);
		if (!iret) {
			memcpy(&pCust[iTotal], &cust, sizeof(XCustT));
		} else {
			break;
		}
		pos++;
		iTotal++;
	}

	///读取CTP账户
	pos = 1;
	iret =0;
	///读取oes账户
	while (!iret) {
		memset(&cust, 0, sizeof(XCustT));
		iret = _XGetCounter(config, CTP_COUNTER_SEC, pos, &cust);
		if (!iret) {
			memcpy(&pCust[iTotal], &cust, sizeof(XCustT));
		} else {
			break;
		}
		pos++;
		iTotal++;
	}

	///读取CTP账户
	pos = 1;
	iret = 0;
	///读取oes账户
	while (!iret) {
		memset(&cust, 0, sizeof(XCustT));
		iret = _XGetCounter(config, XTP_COUNTER_SEC, pos, &cust);
		if (!iret) {
			memcpy(&pCust[iTotal], &cust, sizeof(XCustT));
		} else {
			break;
		}
		pos++;
		iTotal++;
	}
	XINIFree(config);

	return (iTotal);
}

XVoid XSnapPrint(XInt market, XChar *securityId) {
	XSnapshotT *pSnapshot = NULL;

	pSnapshot = XFndVSnapshotByKey(market, securityId);
	if (NULL == pSnapshot) {
		return;
	}

	printf("[%d-%s] 最新价[%d],最新时间[%d],买一[%d-%lld],卖一[%d-%lld]\n", market,
			securityId, pSnapshot->tradePx, pSnapshot->updateTime,
			pSnapshot->bid[0], pSnapshot->bidqty[0], pSnapshot->ask[0],
			pSnapshot->askqty[0]);
}
