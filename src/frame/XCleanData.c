/**
 * @file XBasket.c
 * @brief 盘中抢单程序
 * @author kangyq
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */
#include <getopt.h>
#include "XCom.h"
#include "XTimes.h"
#include "XBus.h"
#include "XLog.h"
#include "map.h"
#include "XCSV.h"

XInt XReadKLine1(XChar *file)
{
	XInt iret = -1;
	const char *col = NULL;
	XCsvHandleT handle;
//	XChar market = 0;
	XChar *pSecurityId = NULL;
	XIdx idx = -1;
	XKLineT *kline = NULL;
	XInt kcursor1 = -1;

	iret = XCsvOpen(&handle, file);
	if (iret)
	{
		slog_error(0, "文件不存在[%s]", file);
		return (0);
	}
	//日期,时间,市场,证券代码,开,高,低,收,笔数,成交金额,预测价格
	while ((!XCsvReadLine(&handle)))
	{
		if (handle.colSize < 2)
		{
			slog_error(0, "列数错误");
			break;
		}
		col = handle.GetFieldByCol(&handle, 2);
		if (col)
		{
//			market = atoi(col);
		}

		pSecurityId = handle.GetFieldByCol(&handle, 3);

		if (NULL == pSecurityId)
		{
			continue;
		}

		col = handle.GetFieldByCol(&handle, 13);
		if (col)
		{
			idx = atol(col);
		}

		col = handle.GetFieldByCol(&handle, 12);
		if (col)
		{
			kcursor1 = atoi(col);
		}

		kline = GetKlinesByBlock(idx, 0);

		kcursor1 = (SNAPSHOT_K1_CNT + kcursor1 - 1) & (SNAPSHOT_K1_CNT - 1);
		col = handle.GetFieldByCol(&handle, 0);
		kline[kcursor1].traday = atoi(col);

		col = handle.GetFieldByCol(&handle, 1);

		kline[kcursor1].updateTime = atoi(col);

		col = handle.GetFieldByCol(&handle, 4);
		//开盘价
		kline[kcursor1].open = atoi(col);

		col = handle.GetFieldByCol(&handle, 5);
		//最高价
		kline[kcursor1].high = atoi(col);

		col = handle.GetFieldByCol(&handle, 6);
		//最低价
		kline[kcursor1].low = atoi(col);

		col = handle.GetFieldByCol(&handle, 7);
		//收盘价
		kline[kcursor1].close = atoi(col);

		col = handle.GetFieldByCol(&handle, 8);
		//成交数量
		kline[kcursor1].qty = atol(col);
		//成交金额
		col = handle.GetFieldByCol(&handle, 9);
		kline[kcursor1].amt = atol(col);

		//成交笔数
		col = handle.GetFieldByCol(&handle, 10);
		kline[kcursor1].numTrades = atoi(col);

	}
	return (0);
}

XInt XReadKLine5(XChar *file)
{
	XInt iret = -1;
	const char *col = NULL;
	XCsvHandleT handle;
//	XChar market = 0;
	XChar *pSecurityId = NULL;
	XIdx idx = -1;
	XKLineT *kline = NULL;
	XInt kcursor5 = -1;

	iret = XCsvOpen(&handle, file);
	if (iret)
	{
		slog_error(0, "文件不存在[%s]", file);
		return (0);
	}
	//日期,时间,市场,证券代码,开,高,低,收,笔数,成交金额,预测价格
	while ((!XCsvReadLine(&handle)))
	{
		if (handle.colSize < 2)
		{
			slog_error(0, "列数错误");
			break;
		}
		col = handle.GetFieldByCol(&handle, 2);
		if (col)
		{
//			market = atoi(col);
		}

		pSecurityId = handle.GetFieldByCol(&handle, 3);

		if (NULL == pSecurityId)
		{
			continue;
		}

		col = handle.GetFieldByCol(&handle, 13);
		if (col)
		{
			idx = atol(col);
		}

		col = handle.GetFieldByCol(&handle, 12);
		if (col)
		{
			kcursor5 = atoi(col);
		}

		kline = GetKlinesByBlock(idx, 1);

		kcursor5 = (SNAPSHOT_K5_CNT + kcursor5 - 1) & (SNAPSHOT_K5_CNT - 1);
		col = handle.GetFieldByCol(&handle, 0);
		kline[kcursor5].traday = atoi(col);

		col = handle.GetFieldByCol(&handle, 1);

		kline[kcursor5].updateTime = atoi(col);

		col = handle.GetFieldByCol(&handle, 4);
		//开盘价
		kline[kcursor5].open = atoi(col);

		col = handle.GetFieldByCol(&handle, 5);
		//最高价
		kline[kcursor5].high = atoi(col);

		col = handle.GetFieldByCol(&handle, 6);
		//最低价
		kline[kcursor5].low = atoi(col);

		col = handle.GetFieldByCol(&handle, 7);
		//收盘价
		kline[kcursor5].close = atoi(col);

		col = handle.GetFieldByCol(&handle, 8);
		//成交数量
		kline[kcursor5].qty = atol(col);
		//成交金额
		col = handle.GetFieldByCol(&handle, 9);
		kline[kcursor5].amt = atol(col);

		//成交笔数
		col = handle.GetFieldByCol(&handle, 10);
		kline[kcursor5].numTrades = atoi(col);

	}
	return (0);
}

static void Merge(XInt market, XChar *securityId, XChar *inputFile, XChar *outputFile)
{
	FILE *fp = NULL;
	XRSnapshotT snapshot;
	XKLineT *k1 = NULL, *k5 = NULL;
	XNum kc1 = -1, kc5 = -1;
	XChar buf[1024];
	FILE *fpMerge = NULL;
	XMonitorT *monitor = NULL;

	memset(buf, 0, sizeof(buf));
	xslog_init(NULL, "cdata");

	XManShmInit();
	XManShmLoad();

	//初始化监控数据
	if (NULL == XFndVMonitor())
	{
		monitor = XGetVMonitor();
		slog_warn(0, "未初始化监控数据");
		monitor->idx = 1;
		XPutVMonitor(monitor);
	}

	// 拆分到每个证券代码
	fp = fopen(inputFile, "rb");
	if (NULL == fp)
	{
		fprintf(stderr, "文件[%s]找不到\n", XMAN_DATA_TSNAPBIN);
		return;
	}

	XReadKLine1(XMAN_IMP_KSNAPSHOT_1);
	XReadKLine5(XMAN_IMP_KSNAPSHOT_5);
	fpMerge = fopen(outputFile, "w");

	fprintf(fpMerge, "tscode,utime,time,preClose,open,high,low,close,amount,volume,"
			"ask5,ask4,ask3,ask2,ask1,askqty5,askqty4,askqty3,askqty2,askqty1,"
			"bid1,bid2,bid3,bid4,bid5,bidqty1,bidqty2,bidqty3,bidqty4,bidqty5,"
			"numTrades,totalBidQty,totalOfferQty,totalBidCnt,totalOfferCnt,totalBidAmt,"
			"toatalOfferAmt,outsideAmt,insideAmt, upperLimitPx,lowerLimitPx,pchg,side,"
			"k1,utime1,k5,utime5,c,c1\n");

	while (!feof(fp))
	{

		fread(&snapshot, sizeof(XRSnapshotT), 1, fp);

		if (NULL == snapshot.securityId || 0 == snapshot.market)
		{
			continue;
		}

		if (NULL != securityId && strlen(securityId) > 0 && strncmp(snapshot.securityId, securityId, strlen(securityId)) != 0)
		{
			continue;
		}

		//1分钟K线
		k1 = GetKlinesByBlock(snapshot.idx, 0);
		k5 = GetKlinesByBlock(snapshot.idx, 1);
		//过去
//		lkc1 = (SNAPSHOT_K1_CNT + snapshot.kcursor1 - 2) & (SNAPSHOT_K1_CNT - 1);
//		lkc5 = (SNAPSHOT_K5_CNT + snapshot.kcursor5 - 2) & (SNAPSHOT_K5_CNT - 1);

		//未来
		kc1 = (SNAPSHOT_K1_CNT + snapshot.kcursor1) & (SNAPSHOT_K1_CNT - 1);
		kc5 = (SNAPSHOT_K5_CNT + snapshot.kcursor5) & (SNAPSHOT_K5_CNT - 1);
		//找到对应的K线数据
		fprintf(fpMerge, "%s.%s,%d%09d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.2f,%lld,"
				"%.3f,%.3f,%.3f,%.3f,%.3f,"
				"%lld,%lld,%lld,%lld,%lld,"
				"%.3f,%.3f,%.3f,%.3f,%.3f,"
				"%lld,%lld,%lld,%lld,%lld,"
				"%d,%lld,%lld,%d,%d,%.2f,%.2f,%.2f,%.2f,%.3f,%.3f,%.2f,%c,%.3f,%d,%.3f,%d,%d,%d\n", snapshot.securityId, snapshot.market == eXMarketSha ? "SH" : "SZ", snapshot.traday, snapshot.updateTime, snapshot.updateTime, snapshot.preClosePx
				* 0.0001, snapshot.openPx * 0.0001, snapshot.highPx * 0.0001, snapshot.lowPx * 0.0001, snapshot.tradePx * 0.0001, snapshot.amountTrade * 0.0001, snapshot.volumeTrade, snapshot.ask[4]
				* 0.0001, snapshot.ask[3] * 0.0001, snapshot.ask[2] * 0.0001, snapshot.ask[1] * 0.0001, snapshot.ask[0] * 0.0001, snapshot.askqty[4], snapshot.askqty[3], snapshot.askqty[2], snapshot.askqty[1], snapshot.askqty[0], snapshot.bid[0]
				* 0.0001, snapshot.bid[1] * 0.0001, snapshot.bid[2] * 0.0001, snapshot.bid[3] * 0.0001, snapshot.bid[4] * 0.0001, snapshot.bidqty[0], snapshot.bidqty[1], snapshot.bidqty[2], snapshot.bidqty[3], snapshot.bidqty[4], snapshot.numTrades, snapshot.totalBuyOrdQty, snapshot.totalSellOrdQty, snapshot.totalBuyOrdCnt, snapshot.totalSellOrdCnt, snapshot.totalBuyOrdAmt
				* 0.0001, snapshot.totalSellOrdAmt * 0.0001, snapshot.outsideTrdAmt * 0.0001, snapshot.insideTrdAmt * 0.0001, snapshot.upperPx * 0.0001, snapshot.lowerPx * 0.0001, (snapshot.tradePx
				- snapshot.preClosePx) * 10000 / snapshot.preClosePx / 100.0, snapshot.side == eXBuy ? 'B' : 'S', k1[kc1].close * 0.0001, k1[kc1].updateTime, k5[kc5].close * 0.0001, k5[kc5].updateTime, snapshot.kcursor1, kc1);

	}

	fclose(fp);
	fclose(fpMerge);

	XManShmDelete();

}

static void usage()
{
	printf("#            重构后数据清洗\n");
	printf("-m[market] -s[security] -f[resnap] -o[outputFile]\n");
	printf("-s 如果为空则清洗全部数据\n");
	printf("-f 不填默认文件为../data/store/resnap.bin\n");
	printf("-o 不填默认文件为merge.csv\n");
}
int main(int argc, char *argv[])
{
	int opt;
	int option_index = 0;
	const char *option = "hm:s:f:o:";
	const struct option long_options[] =
	{
	{ "help", 0, NULL, 'h' },
	{ "market", 1, NULL, 'm' },
	{ "securityId", 1, NULL, 's' },
	{ "Inputfile", 1, NULL, 'f' },
	{ "Outputfile", 1, NULL, '0' },
	{ NULL, 0, NULL, 0 } };
	XChar *securityId = NULL;
	XInt market = 0;
	XChar *reSnapFile = XMAN_DATA_TSNAPBIN;
	XChar *outputFile = "snapshot.csv";

	while ((opt = getopt_long(argc, argv, option, long_options, &option_index)) != -1)
	{
		switch (opt)
		{
		case 'h':
		case '?':
			usage();
			exit(0);
			break;
		case 'm':
			market = atoi(optarg);
			break;
		case 's':
			securityId = optarg;
			break;
		case 'f':
			reSnapFile = optarg;
			break;
		case '0':
			outputFile = optarg;
			break;
		default:
			break;
		}
	}

	Merge(market, securityId, reSnapFile, outputFile);

	return (0);
}
