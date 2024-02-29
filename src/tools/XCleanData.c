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

static void Merge(XInt market, XChar *securityId, XChar *inputFile,
		XChar *outputFile) {
	FILE *fp = NULL, *fpK1 = NULL, *fpK5 = NULL;
	XRSnapshotT snapshot, *pSnapshot = NULL;
	XKLineT *k1 = NULL, *k5 = NULL;
	XNum kcursor1 = -1, kcursor5 = -1, lc1 = -1, lc5 = -1, lc1_5 = -1, lc5_5 =
			-1;
	XIdx idx = -1;
	XChar buf[1024];
	XMoney totalProfit = 0;
	FILE *fpMerge = NULL;

	memset(buf, 0, sizeof(buf));
	xslog_init(NULL, "cdata");

	XManShmInit();
	XManShmLoad();

	// 拆分到每个证券代码
	fp = fopen(inputFile, "rb");
	if (NULL == fp) {
		fprintf(stderr, "文件[%s]找不到\n", XMAN_DATA_TSNAPBIN);
		return;
	}
	fpK1 = fopen("k1.csv", "w");
	fpK5 = fopen("k5.csv", "w");
	fpMerge = fopen(outputFile, "w");
	fprintf(fpMerge,
			"tscode,time,preClose,open,high,low,close,amount,volume,"
					"ask5,ask4,ask3,ask2,ask1,askqty5,askqty4,askqty3,askqty2,askqty1,"
					"bid5,bid4,bid3,bid2,bid1,bidqty5,bidqty4,bidqty3,bidqty2,bidqty1,"
					"numTrades,totalBidQty,totalOfferQty,totalBidCnt,totalOfferCnt,totalBidAmt,"
					"toatalOfferAmt,outsideAmt,insideAmt, upperLimitPx,lowerLimitPx,pchg,side,"
					"qs1,qs5,numTrades1,numTrades5,time1,time5\n");
	fprintf(fpK1, "tscode, time,open,high,low,close,vloume,amount,numTrades\n");
	fprintf(fpK5, "tscode, time,open,high,low,close,vloume,amount,numTrades\n");
	while (!feof(fp)) {

		fread(&snapshot, sizeof(XRSnapshotT), 1, fp);

		if (NULL == snapshot.securityId || 0 == snapshot.market) {
			continue;
		}

		if (NULL != securityId && strlen(securityId) > 0
				&& strncmp(snapshot.securityId, securityId, strlen(securityId))
						!= 0) {
			continue;
		}

		//逆回购不做回测
		if (!strncmp(snapshot.securityId, "204", strlen("204"))
				|| !strncmp(snapshot.securityId, "1318", strlen("1318"))) {
			continue;
		}

		idx = XFndOrderBook(snapshot.market, snapshot.securityId);
		if (idx < 1) {
			idx = XPutOrderBookHash(snapshot.market, snapshot.securityId);
		}

		snapshot.idx = idx;
		pSnapshot = XFndVRSnapshotById(idx);
		if (NULL == pSnapshot) {
			snapshot.kcursor1 = 0;
			snapshot.kcursor5 = 0;
			XPutOrUpdVRSnapshot(&snapshot);
			pSnapshot = XFndVRSnapshotByKey(snapshot.market,
					snapshot.securityId);
		} else {
			kcursor1 = pSnapshot->kcursor1;
			kcursor5 = pSnapshot->kcursor5;
			memcpy(pSnapshot, &snapshot, sizeof(XRSnapshotT));
			pSnapshot->kcursor1 = kcursor1;
			pSnapshot->kcursor5 = kcursor5;
		}

		if (NULL == pSnapshot) {
			continue;
		}

		//当前1刻的K线
		kcursor1 = pSnapshot->kcursor1 > 0 ? pSnapshot->kcursor1 : 1;
		kcursor1 = (SNAPSHOT_K1_CNT + kcursor1 - 1) & (SNAPSHOT_K1_CNT - 1);
		kcursor5 = pSnapshot->kcursor5 > 0 ? pSnapshot->kcursor5 : 1;
		kcursor5 = (SNAPSHOT_K1_CNT + kcursor5 - 1) & (SNAPSHOT_K5_CNT - 1);
		k1 = GetKlinesByBlock(pSnapshot->idx, 0);
		k5 = GetKlinesByBlock(pSnapshot->idx, 1);

		//94011234
		if (snapshot.updateTime / 20000 == k1[kcursor1].updateTime / 20000) {
			k1[kcursor1].close = snapshot.tradePx;
			k1[kcursor1].updateTime = snapshot.updateTime;
			k1[kcursor1].low =
					k1[kcursor1].low > snapshot.tradePx ?
							snapshot.tradePx : k1[kcursor1].low;
			k1[kcursor1].high =
					k1[kcursor1].high < snapshot.tradePx ?
							snapshot.tradePx : k1[kcursor1].high;
		} else {

			//输出K线数据
			if (pSnapshot->kcursor1 > 0) {
				fprintf(fpK1, "%s.%s,%d%09d,%.3f,%.3f,%.3f,%.3f,%lld,%.2f,%d\n",
						snapshot.securityId,
						snapshot.market == eXMarketSha ? "SH" : "SZ",
						snapshot.traday, k1[kcursor1].updateTime / 20000 * 20000,
						k1[kcursor1].open * 0.0001, k1[kcursor1].high * 0.0001,
						k1[kcursor1].low * 0.0001, k1[kcursor1].close * 0.0001,
						snapshot.volumeTrade - k1[kcursor1].qty,
						(snapshot.amountTrade - k1[kcursor1].amt) * 0.0001,
						snapshot.numTrades - k1[kcursor1].numTrades);
			}
			pSnapshot->kcursor1++;
			kcursor1 = (pSnapshot->kcursor1 - 1) & (SNAPSHOT_K1_CNT - 1);
			memset(&k1[kcursor1], 0, sizeof(XKLineT));
			k1[kcursor1].open = snapshot.tradePx;
			k1[kcursor1].close = snapshot.tradePx;
			k1[kcursor1].updateTime = snapshot.updateTime;
			k1[kcursor1].low = snapshot.tradePx;
			k1[kcursor1].high = snapshot.tradePx;
			k1[kcursor1].qty = snapshot.volumeTrade;
			k1[kcursor1].amt = snapshot.amountTrade;
			k1[kcursor1].numTrades = snapshot.numTrades;
		}

		if (snapshot.updateTime / 100000 == k5[kcursor5].updateTime / 100000) {
			k5[kcursor5].close = snapshot.tradePx;
			k5[kcursor5].updateTime = snapshot.updateTime;
			k5[kcursor5].low =
					k5[kcursor5].low > snapshot.tradePx ?
							snapshot.tradePx : k5[kcursor5].low;
			k5[kcursor5].high =
					k5[kcursor5].high < snapshot.tradePx ?
							snapshot.tradePx : k5[kcursor5].high;
		} else {
			if (pSnapshot->kcursor5 > 0) {
				fprintf(fpK5, "%s.%s,%d%09d,%.3f,%.3f,%.3f,%.3f,%lld,%.2f,%d\n",
						snapshot.securityId,
						snapshot.market == eXMarketSha ? "SH" : "SZ",
						snapshot.traday,
						k5[kcursor5].updateTime / 100000 * 100000,
						k5[kcursor5].open * 0.0001, k5[kcursor5].high * 0.0001,
						k5[kcursor5].low * 0.0001, k5[kcursor5].close * 0.0001,
						snapshot.volumeTrade - k5[kcursor5].qty,
						(snapshot.amountTrade - k5[kcursor5].amt) * 0.0001,
						snapshot.numTrades - k5[kcursor5].numTrades);
			}
			pSnapshot->kcursor5++;
			kcursor5 = (pSnapshot->kcursor5 - 1) & (SNAPSHOT_K5_CNT - 1);
			memset(&k5[kcursor5], 0, sizeof(XKLineT));
			k5[kcursor5].open = snapshot.tradePx;
			k5[kcursor5].close = snapshot.tradePx;
			k5[kcursor5].updateTime = snapshot.updateTime;
			k5[kcursor5].low = snapshot.tradePx;
			k5[kcursor5].high = snapshot.tradePx;
			k5[kcursor5].qty = snapshot.volumeTrade;
			k5[kcursor5].amt = snapshot.amountTrade;
			k5[kcursor5].numTrades = snapshot.numTrades;
		}

		///上一刻的K线
		kcursor1 = pSnapshot->kcursor1 > 0 ? pSnapshot->kcursor1 : 1;

		lc1 = (SNAPSHOT_K1_CNT + kcursor1 - 3) & (SNAPSHOT_K1_CNT - 1);
		lc1_5 = (SNAPSHOT_K1_CNT + kcursor1 - 7) & (SNAPSHOT_K1_CNT - 1);
		kcursor1 = (SNAPSHOT_K1_CNT + kcursor1 - 2) & (SNAPSHOT_K1_CNT - 1);

		kcursor5 = pSnapshot->kcursor5 > 0 ? pSnapshot->kcursor5 : 1;
		lc5 = (SNAPSHOT_K1_CNT + kcursor5 - 3) & (SNAPSHOT_K5_CNT - 1);
		lc5_5 = (SNAPSHOT_K1_CNT + kcursor5 - 7) & (SNAPSHOT_K5_CNT - 1);
		kcursor5 = (SNAPSHOT_K1_CNT + kcursor5 - 2) & (SNAPSHOT_K5_CNT - 1);

		//趋势指标
		fprintf(fpMerge, "%s.%s,%d%09d, %.3f,%.3f,%.3f,%.3f,%.3f,%.2f,%lld,"
				"%.3f,%.3f,%.3f,%.3f,%.3f,"
				"%lld,%lld,%lld,%lld,%lld,"
				"%.3f,%.3f,%.3f,%.3f,%.3f,"
				"%lld,%lld,%lld,%lld,%lld,"
				"%d,%lld,%lld,%d,%d,%.2f,%.2f,%.2f,%.2f,%.3f,%.3f,%.2f,%c",
				snapshot.securityId,
				snapshot.market == eXMarketSha ? "SH" : "SZ", snapshot.traday,
				snapshot.updateTime, snapshot.preClosePx * 0.0001,
				snapshot.openPx * 0.0001, snapshot.highPx * 0.0001,
				snapshot.lowPx * 0.0001, snapshot.tradePx * 0.0001,
				snapshot.amountTrade * 0.0001, snapshot.volumeTrade,
				snapshot.ask[0] * 0.0001, snapshot.ask[1] * 0.0001,
				snapshot.ask[2] * 0.0001, snapshot.ask[3] * 0.0001,
				snapshot.ask[4] * 0.0001, snapshot.askqty[0],
				snapshot.askqty[1], snapshot.askqty[2], snapshot.askqty[3],
				snapshot.askqty[4], snapshot.bid[0] * 0.0001,
				snapshot.bid[1] * 0.0001, snapshot.bid[2] * 0.0001,
				snapshot.bid[3] * 0.0001, snapshot.bid[4] * 0.0001,
				snapshot.bidqty[0], snapshot.bidqty[1], snapshot.bidqty[2],
				snapshot.bidqty[3], snapshot.bidqty[4], snapshot.numTrades,
				snapshot.totalBuyOrdQty, snapshot.totalSellOrdQty,
				snapshot.totalBuyOrdCnt, snapshot.totalSellOrdCnt,
				snapshot.totalBuyOrdAmt * 0.0001,
				snapshot.totalSellOrdAmt * 0.0001,
				snapshot.outsideTrdAmt * 0.0001, snapshot.insideTrdAmt * 0.0001,
				snapshot.upperPx * 0.0001, snapshot.lowerPx * 0.0001,
				(snapshot.tradePx - snapshot.preClosePx) * 10000
						/ snapshot.preClosePx / 100.0,
				snapshot.side == eXBuy ? 'B' : 'S');

		//前5分钟的趋势比较,量放量，有溢价买卖
		fprintf(fpMerge, ",%d,%d,%d,%d,%d%09d,%d\n",
				k1[lc1_5].close == 0 ? 0 : k1[lc1].close - k1[lc1_5].close,
				k5[lc5_5].close == 0 ? 0 : k5[lc5].close - k5[lc5_5].close,
				k1[lc1].numTrades, k5[lc5].numTrades, snapshot.traday, k1[lc1].updateTime / 20000 * 20000,
				k5[lc5].updateTime / 100000 * 100000);
	}

	fclose(fp);
	fclose(fpMerge);
	fclose(fpK1);
	fclose(fpK5);
	XManShmDelete();

}

static void usage() {
	printf("#            重构后数据清洗\n");
	printf("-m[market] -s[security] -f[resnap] -o[outputFile]\n");
	printf("-s 如果为空则清洗全部数据\n");
	printf("-f 不填默认文件为../data/store/resnap.bin\n");
	printf("-o 不填默认文件为merge.csv\n");
}
int main(int argc, char *argv[]) {

	XInt i, j;
	int opt;
	int option_index = 0;
	const char *option = "hm:s:f:o:";
	const struct option long_options[] = { { "help", 0, NULL, 'h' }, { "market",
			1, NULL, 'm' }, { "securityId", 1, NULL, 's' }, { "Inputfile", 1,
			NULL, 'f' }, { "Outputfile", 1, NULL, '0' }, { NULL, 0, NULL, 0 } };
	XChar *securityId = NULL;
	XInt market = 0;
	XChar *reSnapFile = XMAN_DATA_TSNAPBIN;
	XChar *outputFile = "snapshot.csv";

	while ((opt = getopt_long(argc, argv, option, long_options, &option_index))
			!= -1) {
		switch (opt) {
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
