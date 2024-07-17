/*
 * @file XMan.c
 * @author kangyq
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 * 
 \dot
 digraph XMan {
 charset="UTF-8";
 node [color = Blue, fontcolor = Red, fontsize = 24];
 main[label = "Start",  shape = box];
 init[label = "Init memory"shape = ellipse];
 user[label = "set userinfo"shape = ellipse];
 oesinit[label = "init counter",shape = ellipse];
 oestrd[label = "start trade", shape = ellipse];
 oesmkt[label = "start market", shape = ellipse];
 mktcache[label = "restruct order", shape = ellipse];
 trdcache[label = "order engine", shape = ellipse];
 strategy[label = "startegy", shape = ellipse];
 mainend[label = "exit", shape = box];

 main -> init[label = "init"];
 init -> user[label = "set data"];
 user -> oesinit[label = "init"];
 oesinit -> mktcache;
 oesinit -> trdcache;
 oesinit -> oestrd;
 oesinit -> oesmkt;
 oesinit -> strategy;

 mktcache -> mainend[label = "closed"] ;
 trdcache -> mainend[label = "closed"] ;
 oestrd -> mainend[label = "closed"] ;
 oesmkt -> mainend[label = "closed"] ;
 strategy -> mainend[label = "closed"] ;

 }
 \enddot
 *  Created on: 2022年3月30日
 *      Author: kyle
 */
#include <getopt.h>
#include "XCom.h"
#include "XTimes.h"
#include "OesCom.h"
//#include "CtpCom.h"
#include "XBus.h"
#include "XExport.h"
#include "XCSV.h"
#include "XOrdeng.h"
#ifdef __XMAN_FAST_REBUILD__
#include "XShFastRebuild.h"
#include "XSzFastRebuild.h"
#else
#include "XShRebuild.h"
#include "XSzRebuild.h"
#endif

#include "XStore.h"
#include "XLog.h"
#include "XINI.h"

static void initcust() {
	XInt iCnt = 0, i;
	XCustT cust[100] = { 0 }, *pCust = NULL;
	XMonitorTdT tdMonitor;
	XMonitorMdT mdMonitor;

	memset(&mdMonitor, 0, XMONITORMD_SIZE);
	memset(&tdMonitor, 0, XMONITORTD_SIZE);

	XManShmLoad();

	iCnt = XGetUser(XUSER_FILE, cust);
	for (i = 0; i < iCnt; i++) {
		pCust = XFndVCustomerByKey(cust[i].customerId);
		if (NULL != pCust) {
			continue;
		}
		XPutVCustomer(&cust[i]);
		if (cust[i].type != eXUserMarket) {
			XPutTdMonitor(cust[i].customerId, cust[i].envno);
			slog_info(0, "交易用户[%s]", cust[i].customerId);
		}
		if (cust[i].type != eXUserTrade) {
			XPutMdMonitor(cust[i].exchid);
			slog_info(0, "行情用户[%s]", cust[i].customerId);
		}

	}
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
static void init() {
	XIdx i;
	XMonitorT *pMonitor = NULL;
	XCustT *pCust = NULL;
	XMonitorTdT *pMonitorTd = NULL;

	/** 检查内存是否启动，启动不能再启动 */

	if (xslog_check_init(XSHM_SDB_FILE)) {
		slog_error(0, "!!! 系统已经启动 !!!");
		exit(0);
	}

	xslog_init(NULL, "xmanlg");
	slog_debug(0, "xman版本[%s]", __XMAN_VERSION__);

	slog_info(0, ">>>>>> 1. 初始化内存......");

	XManShmInit();

	//初始化监控数据
	if(NULL == XFndVMonitor())
	{
		pMonitor = XGetVMonitor();
		slog_warn(0, "未初始化监控数据");
		pMonitor->idx = 1;
		XPutVMonitor(pMonitor);
	}

	slog_info(0, ">>>>>> 2. 初始化客户信息......");
	initcust();

	slog_info(0, ">>>>>> 3. 初始化柜台......");

	pMonitor = XFndVMonitorS(__FILE__, __LINE__);
	if (NULL == pMonitor) {
		return;
	}
	for (i = pMonitor->iTCust - 1; i >= 0; i--) {
		pCust = XFndVCustomerById(i + 1);
		if (NULL == pCust) {
			continue;
		}
		switch (pCust->counter) {
		case eXCounterOes:
			if (pCust->type != eXUserMarket) {
				//初始化OES
				XOesInit(pCust->customerId);
				pMonitorTd = XFndVTdMonitor(pCust->customerId);
				//检查授权
				if (NULL == pMonitorTd) {
					break;
				}
				CheckAuth(pCust->customerId, pMonitorTd->traday);
			}
			break;
		case eXCounterCtp:
//			if (pCust->type != eXUserMarket) {
//				XCtpInit(pCust->customerId);
//			}
			break;
		default:
			break;
		}
	}

	slog_info(0, ">>>>>> 3.0 加载转股价.....");
	read_stock_convpx(XMAN_IMP_CONVPX);
	/** 快速重构读取历史K线数据 */
	//加载历史数据
	slog_info(0, ">>>>>> 3.1 加载历史K1线数据......");
	XReadKLine1(XMAN_IMP_KSNAPSHOT_1);
	slog_info(0, ">>>>>> 3.2 加载历史K5线数据......");
	XReadKLine5(XMAN_IMP_KSNAPSHOT_5);
	slog_info(0, ">>>>>> 3.3 加载昨日收盘数据......");
	read_hissnapshot(XMAN_IMP_HSNAPSHOT);
	slog_info(0, ">>>>>> 3.4 加载板块数据......");
	read_block(XMAN_IMP_BLOCK);
	read_blockinfo(XMAN_IMP_BLOCKINFO);

}
static XInt read_subfile(XChar *subfile) {
	XMarketSecurityT mktsec = { 0 };
	XInt iret = -1;
	XCsvHandleT handle;
	const char *col = NULL;
//	unsigned rowcount = 0;

	mktsec.type = exReset;
	mktsec.market = eXMarketSha;
	memcpy(mktsec.securityId, "600000", strlen("600000"));
	//增加初始订阅合约的信息
	XPushCache(XSHMKEYCONECT(mktSubscribe), &mktsec);

	mktsec.type = exAppend;
	mktsec.market = eXMarketSza;
	memcpy(mktsec.securityId, "000001", strlen("000001"));
	//增加初始订阅合约的信息
	XPushCache(XSHMKEYCONECT(mktSubscribe), &mktsec);

	iret = XCsvOpen(&handle, subfile);
	if (iret) {
		slog_error(0, "文件不存在[%s]", subfile);
		return (-1);
	}
	while ((!XCsvReadLine(&handle))) {

		memset(&mktsec, 0, sizeof(XMarketSecurityT));
		col = handle.GetFieldByCol(&handle, 0);
		if (col) {
			mktsec.market = atoi(col);
		}
		col = handle.GetFieldByCol(&handle, 1);
		if (col) {
			memcpy(mktsec.securityId, col, strlen(col));
		}
		mktsec.type = exAppend;

		XPushCache(XSHMKEYCONECT(mktSubscribe), &mktsec);
	}
	XCsvClose(&handle);

	return (iret);
}

static void xlog(XVoid *params) {
	xslog_to_file();
}

static void start(XBindParamT bindparam[]) {

	XInt iProc = 0, i = 0;
	XProcInfoT g_proc[1024];
	XMonitorT *pMonitor = NULL;
	XCustT *pCust = NULL;

	memset(&g_proc, 0, 1024 * sizeof(XProcInfoT));

	init();

	pMonitor = XFndVMonitorS(__FILE__, __LINE__);
	if (NULL == pMonitor) {
		return;
	}
	//切换会日志落地文件
	slog_info(0, ">>>>>> 4. 启动柜台服务......");

	if (bindparam[4].resub) {
		read_subfile(bindparam[5].subFile);
	}

	memcpy(g_proc[iProc].processName, "xmanlg", strlen("xmanlg"));
	g_proc[iProc].isRunable = true;
	g_proc[iProc++].callBack = xlog;

	memcpy(g_proc[iProc].processName, "xorden", strlen("xorden"));
	g_proc[iProc].callBack = XOrdeng;
	g_proc[iProc].isRunable = true;
	g_proc[iProc++].params = &bindparam[4];

	for (i = pMonitor->iTCust - 1; i >= 0; i--) {
		pCust = XFndVCustomerById(i + 1);
		if (NULL == pCust || pCust->type == eXUserTrade) {
			continue;
		}
		switch (pCust->counter) {
		case eXCounterOes:
			memcpy(g_proc[iProc].processName, "oesmkt", strlen("oesmkt"));
			g_proc[iProc].callBack = XOesMkt;
			g_proc[iProc].isRunable = true;
			g_proc[iProc++].params = &bindparam[5];
			break;
		case eXCounterCtp:
//			memcpy(g_proc[iProc].processName, "ctpmkt", strlen("ctpmkt"));
//			g_proc[iProc].callBack = XCtpMkt;
//			g_proc[iProc++].params = &bindparam[5];
			break;
		default:
			break;
		}
	}

	for (i = pMonitor->iTCust - 1; i >= 0; i--) {
		pCust = XFndVCustomerById(i + 1);
		if (NULL == pCust || pCust->type == eXUserMarket) {
			continue;
		}
		switch (pCust->counter) {
		case eXCounterOes:

			if (!pCust->cpuid) {
				pCust->cpuid = bindparam[6].cpuid;
			}
			sprintf(g_proc[iProc].processName, "*%s", pCust->customerId);
			g_proc[iProc].callBack = XOesTrd;
			g_proc[iProc].isRunable = true;
			g_proc[iProc++].params = pCust;
			break;
		case eXCounterCtp:
//			if (!pCust->cpuid) {
//				pCust->cpuid = bindparam[6].cpuid;
//			}
//			sprintf(g_proc[iProc].processName, "ctptrd-%s", pCust->customerId);
//			g_proc[iProc].callBack = XCtpTrd;
//			g_proc[iProc++].params = pCust;
			break;
		default:
			break;
		}
	}

	memcpy(g_proc[iProc].processName, "xshbld", strlen("xshbld"));
	g_proc[iProc].callBack = XShRebuild;
	g_proc[iProc].isRunable = true;
	g_proc[iProc++].params = &bindparam[0];

	memcpy(g_proc[iProc].processName, "xszbld", strlen("xszbld"));
	g_proc[iProc].callBack = XSzRebuild;
	g_proc[iProc].isRunable = true;
	g_proc[iProc++].params = &bindparam[1];

#ifdef __XMAN_FAST_REBUILD__

	memcpy(g_proc[iProc].processName, "xstore", strlen("xstore"));
	g_proc[iProc].callBack = XStoreSnap;
	g_proc[iProc].isRunable = true;
	g_proc[iProc++].params = &bindparam[2];

	memcpy(g_proc[iProc].processName, "xetick", strlen("xetick"));
	g_proc[iProc].callBack = XStoreTick;
	g_proc[iProc].isRunable = true;
	g_proc[iProc++].params = &bindparam[7];

#else

	memcpy(g_proc[iProc].processName, "xstore", strlen("xstore"));
	g_proc[iProc].callBack = XStoreSnap;
	g_proc[iProc].isRunable = true;
	g_proc[iProc++].params = &bindparam[2];

	memcpy(g_proc[iProc].processName, "xetick", strlen("xetick"));
	g_proc[iProc].callBack = XStoreTick;
	g_proc[iProc].isRunable = true;
	g_proc[iProc++].params = &bindparam[7];

#endif
	xslog_init(XSHM_SDB_FILE, "xmanlg");
	XAppStart(g_proc, iProc);
	slog_info(0, ">>>>>> 5. !!! 系统启动成功 !!!");
}

static void stop() {
	xslog_init(NULL, "xmanlg");
	slog_info(0, "服务正在关闭中......");
	XAppStop();
	XManShmDelete();
	xslog_release_cache(XSHM_SDB_FILE);
}

static void export() {
//	XMonitorMdT *pMonitorMd = NULL;

//	pMonitorMd = XFndVMdMonitor(eXExchSec);

	XExpPrint();
	XExpStock(XMAN_DATA_STOCK);
	XExpSnapshot(XMAN_DATA_SNAPSHOT);
	XExpDifSnapshot(XMAN_DATA_DIFFSNAP);
	XExpOrder(XMAN_DATA_ORDER);
	XExpTrade(XMAN_DATA_TRADE);
	XExpInvest(XMAN_DATA_INVEST);
	XExpHold(XMAN_DATA_HOLD);
	XExpStrategy(XMAN_DATA_STRATEGY);
	XExpCash(XMAN_DATA_CASH);
	XExpKLine1(XMAN_IMP_KSNAPSHOT_1);
	XExpKLine5(XMAN_IMP_KSNAPSHOT_5);

	XExpBlock(XMAN_DATA_BLOCK);
	XExpBlockInfo(XMAN_DATA_BLOCKINFO);
	XExpSellHold(XMAN_DATA_SELL);
	XExpRSnapshot(XMAN_DATA_RSNAP);
//	if (isMktClosingTime(pMonitorMd->updateTime))
	{
		XExpHisSnapshot(XMAN_IMP_HSNAPSHOT);
	}

}
static void usage() {
	printf("###############################################################\n");
	printf("#    量赢策略交易平台 (%s)  \n", __XMAN_VERSION__);
	printf("# -a [all] 初始化并启动程序\n");
	printf("# -s[stop] 停止程序并清理内存\n");
	printf("#                                                             \n");
	printf("# -t0 初始化数据及建立内存\n");
	printf("# -t1 启动日志记录\n");
	printf("# -t2 启动行情接收,需要先init\n");
	printf("# -t3 上海重建订单薄,需要先init\n");
	printf("# -t4 深圳重建订单薄,需要先init\n");
	printf("# -t5 -c[customer] 启动宽睿柜台接口,需要先init\n");
	printf("# -t6 启动订单处理,需要先init\n");
	printf("# -t7 原始行情存储,需要先init\n");
	printf("# -t8 其它服务,需要先init\n");
	printf("#                                                             \n");
	printf("# -e[export] 导出行情\n");
	printf("# -p[print] 打印当前系统状态\n");
	printf("#                                                             \n");

	printf("###############################################################\n");
}

int main(int argc, char *argv[]) {

	int opt;
	int option_index = 0;
	const char *option = "hasept:c:";
	const struct option long_options[] = { { "help", no_argument, NULL, 'h' }, {
			"all", no_argument, NULL, 'a' }, { "stop", no_argument, NULL, 's' },
			{ "type", required_argument, NULL, 't' }, { "export", no_argument,
					NULL, 'e' }, { "print", no_argument, NULL, 'p' }, {
					"customer", required_argument, NULL, 'c' }, { NULL, 0, NULL,
					0 } };
	XBindParamT bindparam[9];
	XCustT cust;
	XChar *pCustomer = NULL;
	XInt type = -1;

	memset(&cust, 0, sizeof(XCustT));
	memset(bindparam, 0, sizeof(bindparam));

	getConf(XSYSTEM_FILE, bindparam);

	while ((opt = getopt_long(argc, argv, option, long_options, &option_index))
			!= -1) {
		switch (opt) {
		case 'h':
		case '?':
			usage();
			break;
		case 'a':
			start(bindparam);
			break;
		case 's':
			stop();
			break;
		case 'e':
			xslog_init(NULL, "xmanlg");
			XManShmLoad();
			export();
			break;
		case 'p':
			xslog_init(NULL, "xmanlg");
			XManShmLoad();
			XMonitorPrint();
			break;
		case 'c':
			pCustomer = optarg;
			break;
		case 't':
			type = atoi(optarg);
			break;
		default:
			usage();
			break;
		}
	}

	switch (type) {
	//初始化
	case 0:
		init();
		break;
		//落地日志
	case 1:
		xslog_init(XSHM_SDB_FILE, "xmanlg");
		xlog(NULL);
		break;
		//行情收取
	case 2:
		XManShmLoad();
		xslog_init(XSHM_SDB_FILE, "oesmkt");
		XOesMkt(&bindparam[5]);
		break;
		//上交所重构
	case 3:
		xslog_init(XSHM_SDB_FILE, "xshbld");
		XManShmLoad();
		XShRebuild(&bindparam[0]);
		break;
		//深交所重构
	case 4:
		xslog_init(XSHM_SDB_FILE, "xszbld");
		XManShmLoad();
		XSzRebuild(&bindparam[1]);
		break;
		//宽睿柜台接口
	case 5:
		if (NULL == pCustomer) {
			printf("无交易账户\n");
			break;
		}
		xslog_init(XSHM_SDB_FILE, "oestrd");
		XManShmLoad();
		cust.cpuid = bindparam[6].cpuid;
		memcpy(cust.customerId, pCustomer, strlen(pCustomer));
		XOesTrd(&cust);
		break;
		//订单处理
	case 6:
		XManShmLoad();
		xslog_init(XSHM_SDB_FILE, "xorden");
		XOrdeng(&bindparam[4]);
		break;
		//原始行情存储
	case 7:
		XManShmLoad();
		xslog_init(XSHM_SDB_FILE, "xetick");
		XStoreTick(&bindparam[7]);
		break;

		//其他存储
	case 8:
		XManShmLoad();
		xslog_init(XSHM_SDB_FILE, "xstore");
		XStoreSnap(&bindparam[2]);
		break;

	default:
		break;
	}

	return 0;
}

