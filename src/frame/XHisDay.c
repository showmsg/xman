/*
 *
 * @file XHisDay.c
 * @brief 日内存储的二进制快照数据的读取及转换
 * @author kangyq
 * @version 2.0.0
 * @date 2022-12-16
 *
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */
#include "XCom.h"
#include "XLog.h"
#include "XTimes.h"
#include "XBus.h"
#include "XUtils.h"
#include <getopt.h>

static void trans_rsnap_bin2csv(XInt market, XChar *securityId) {
	FILE *fp = NULL, *fpsnapcsv = NULL;
	XRSnapshotT snapshot;

	fp = fopen(XMAN_DATA_TSNAPBIN, "rb");
	if (NULL == fp) {
		return;
	}

	fpsnapcsv = fopen(XMAN_DATA_TSNAPCSV, "w");
	if (NULL == fpsnapcsv) {
		return;
	}
	fprintf(fpsnapcsv,
			"tradeDate,securityId,pre_close,open,"
					"high,low,close,numTrades,volume,amount,"
					"times,locTime(ns),ask5,askqty5,askcqty5,ask4,askqty4,"
					"askcqty4,ask3,askqty3,askcqty3,ask2,askqty2,askcqty2,ask1,askqty1,"
					"askcqty1,bid1,bidqty1,bidcqty1,bid2,bidqty2,bidcqty2,bid3,bidqty3,"
					"bidcqty3,bid4,bidqty4,bidcqty4,bid5,bidqty5,bidcqty5,"
					"gapTimes(ns),_channel,version,bizIndex,driveAskPx,driveBidPx,"
					"bigBuyOrdCnt,bigSellOrdCnt,bigBuyOrdAmt,bigSellOrdAmt,bigBuyOrdQty,bigSellOrdQty,"
					"bigBuyTrdAmt,bigSellTrdAmt,totalBuyOrdCnt,totalBuyOrdQty,totalBuyOrdAmt,"
					"totalSellOrdCnt,totalSellOrdQty,totalSellOrdAmt,upperPx,lowerPx\n");
	while (!feof(fp)) {
		fread(&snapshot, sizeof(XRSnapshotT), 1, fp);
		if (NULL == securityId
				|| (market == snapshot.market
						&& 0
								== strncmp(snapshot.securityId, securityId,
										strlen(securityId)))) {
			fprintf(fpsnapcsv,
					"%d,%s.%s,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%lld,%.2f,%d,%f,"
							"%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%."
							"3f,"
							"%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%lld,%lld,%.3f,%"
							"lld,"
							"%lld,%.3f,%lld,%lld,%lld,%d,%lld,%lld,%.3f,%.3f,%d,%d,%.2f,%.2f,%lld,%lld,%.2f,%.2f,"
							"%d,%lld,%.2f,%d,%lld,%.2f,%.3f,%.3f\n",
					snapshot.traday, snapshot.securityId,
					snapshot.market == eXMarketSza ? "SZ" : "SH",
					snapshot.preClosePx * XPRICE_DIV,
					snapshot.openPx * XPRICE_DIV, snapshot.highPx * XPRICE_DIV,
					snapshot.lowPx * XPRICE_DIV, snapshot.tradePx * XPRICE_DIV,
					snapshot.numTrades, snapshot.volumeTrade,
					snapshot.amountTrade * XPRICE_DIV, snapshot.updateTime,
					XNsTime2D(snapshot._genTime), snapshot.ask[4] * XPRICE_DIV,
					snapshot.askqty[4], snapshot.askcqty[4],
					snapshot.ask[3] * XPRICE_DIV, snapshot.askqty[3],
					snapshot.askcqty[3], snapshot.ask[2] * XPRICE_DIV,
					snapshot.askqty[2], snapshot.askcqty[2],
					snapshot.ask[1] * XPRICE_DIV, snapshot.askqty[1],
					snapshot.askcqty[1], snapshot.ask[0] * XPRICE_DIV,
					snapshot.askqty[0], snapshot.askcqty[0],
					snapshot.bid[0] * XPRICE_DIV, snapshot.bidqty[0],
					snapshot.bidcqty[0], snapshot.bid[1] * XPRICE_DIV,
					snapshot.bidqty[1], snapshot.bidcqty[1],
					snapshot.bid[2] * XPRICE_DIV, snapshot.bidqty[2],
					snapshot.bidcqty[2], snapshot.bid[3] * XPRICE_DIV,
					snapshot.bidqty[3], snapshot.bidcqty[3],
					snapshot.bid[4] * XPRICE_DIV, snapshot.bidqty[4],
					snapshot.bidcqty[4], snapshot._genTime - snapshot._recvTime,
					snapshot._channel, snapshot.version, snapshot._bizIndex,
					snapshot.driveAskPx * XPRICE_DIV,
					snapshot.driveBidPx * XPRICE_DIV, snapshot.bigBuyOrdCnt,
					snapshot.bigSellOrdCnt, snapshot.bigBuyOrdAmt * XPRICE_DIV,
					snapshot.bigSellOrdAmt * XPRICE_DIV, snapshot.bigBuyOrdQty,
					snapshot.bigSellOrdQty, snapshot.bigBuyTrdAmt * XPRICE_DIV,
					snapshot.bigSellTrdAmt * XPRICE_DIV,
					snapshot.totalBuyOrdCnt, snapshot.totalBuyOrdQty,
					snapshot.totalBuyOrdAmt * XPRICE_DIV,
					snapshot.totalSellOrdCnt, snapshot.totalSellOrdQty,
					snapshot.totalSellOrdAmt * XPRICE_DIV,
					snapshot.upperPx * XPRICE_DIV,
					snapshot.lowerPx * XPRICE_DIV);
		}
	}

	fflush(fpsnapcsv);
	fclose(fpsnapcsv);
	fclose(fp);
}

static void trans_tick_bin2csv(XChar *mktInput, XInt market, XChar *securityId,
		XInt channel) {
	FILE *fp = NULL, *fptickcsv = NULL;
	XL2LT l2l;
	XTickTradeT trade = { 0 };
	XTickOrderT order = { 0 };
	XSnapshotBaseT snapshot = { };

	fp = fopen(mktInput, "rb");
	if (NULL == fp) {
		printf("未找到数据文件");
		return;
	}

	fptickcsv = fopen(XMAN_DATA_MKTSTORECSV, "w");
	if (NULL == fptickcsv) {
		return;
	}
	fprintf(fptickcsv,
			"#order=1,market,securityId,updateTime,channel,bizIndex,ordSeq,"
					"seqno,bsType,isCancel,ordType,ordPx,ordQty,locTime\r\n");
	fprintf(fptickcsv,
			"#trade=2,market,securityId,updateTime,channel,bizIndex,tradeSeq,"
					"askSeq,bidSeq,isCancel,tradeMoney,tradePx,tradeQty,locTime\r\n");
	fprintf(fptickcsv,
			"#snapshot=3,market,securityId,updateTime,ask,askqty,close,bid,"
					"bidqty,volumeTrade,amtTrade,high,low,locTime,open\r\n");

	while (!feof(fp)) {
		fread(&l2l, sizeof(XL2LT), 1, fp);
		switch (l2l.head.type) {
		case eMTickOrder:
			order = l2l.order;

			if (NULL == securityId
					|| (market == order.market
							&& 0
									== strncmp(order.securityId, securityId,
											strlen(securityId)))) {
				if (channel == 0 || channel == order.channel) {
					fprintf(fptickcsv,
							"1,%d,%s,%d,%d,%lld,%lld,%lld,%d,%d,%d,%.3f,%d,%d\r\n",
							order.market, order.securityId, order.updateTime,
							order.channel, order.bizIndex, order.ordSeq,
							order.seqno, order.bsType, order.isCancel,
							order.ordType, order.ordPx * XPRICE_DIV,
							order.ordQty, XNsTime2I(order._recvTime));
				}
			}
			break;
		case eMTickTrade:
			trade = l2l.trade;

			if (NULL == securityId
					|| (market == trade.market
							&& 0
									== strncmp(trade.securityId, securityId,
											strlen(securityId)))) {
				if (channel == 0 || channel == trade.channel) {
					fprintf(fptickcsv,
							"2,%d,%s,%d,%d,%lld,%lld,%lld,%lld,%d,%.2f, "
									"%.3f,%d,%d\r\n", trade.market,
							trade.securityId, trade.updateTime, trade.channel,
							trade.bizIndex, trade.tradeSeq, trade.askSeq,
							trade.bidSeq, trade.isCancel,
							trade.tradeMoney * XPRICE_DIV,
							trade.tradePx * XPRICE_DIV, trade.tradeQty,
							XNsTime2I(trade._recvTime));
				}
			}
			break;
		case eMSnapshot:
			snapshot = l2l.snapshot;
			if (NULL == securityId
					|| (market == snapshot.market
							&& 0
									== strncmp(snapshot.securityId, securityId,
											strlen(securityId)))) {
				if (channel == 0) {
					fprintf(fptickcsv,
							"3,%d,%s,%d,%d,%lld,%d,%d,%lld, %lld,%lld,%d,%d,%d\r\n",
							snapshot.market, snapshot.securityId,
							snapshot.updateTime, snapshot.ask[0],
							snapshot.askqty[0], snapshot.tradePx,
							snapshot.bid[0], snapshot.bidqty[0],
							snapshot.volumeTrade, snapshot.amountTrade,
							snapshot.highPx, snapshot.lowPx,
							XNsTime2I(snapshot._recvTime));
				}
			}
			break;
		default:
			break;
		}
	}
	fflush(fptickcsv);
	fclose(fptickcsv);
	fclose(fp);
}

static void find_tick_by_biz(XChar *mktInput, XSeqNum biz) {
	FILE *fp = NULL;
	XL2LT l2l;
	XTickTradeT trade = { 0 };
	XTickOrderT order = { 0 };

	fp = fopen(mktInput, "rb");
	if (NULL == fp) {
		slog_error(3, "未找到数据文件[%s]", mktInput);
		return;
	}

	while (!feof(fp)) {
		fread(&l2l, sizeof(XL2LT), 1, fp);
		switch (l2l.head.type) {
		case eMTickOrder:
			order = l2l.order;

			if (biz == order.bizIndex) {
				printf(
						"#order=1,market,securityId,updateTime,channel,bizIndex,ordSeq,"
								"seqno,bsType,isCancel,ordType,ordPx,ordQty,locTime\n");
				printf(

				"1,%d,%s,%d,%d,%lld,%lld,%lld,%d,%d,%d,%.3f,%d,%d\n",
						order.market, order.securityId, order.updateTime,
						order.channel, order.bizIndex, order.ordSeq,
						order.seqno, order.bsType, order.isCancel,
						order.ordType, order.ordPx * XPRICE_DIV, order.ordQty,
						XNsTime2I(order._recvTime));

			}
			break;
		case eMTickTrade:
			trade = l2l.trade;

			if (biz == trade.bizIndex) {

				printf(
						"#trade=2,market,securityId,updateTime,channel,bizIndex,tradeSeq,"
								"askSeq,bidSeq,isCancel,tradeMoney,tradePx,tradeQty,locTime\n");

				printf("2,%d,%s,%d,%d,%lld,%lld,%lld,%lld,%d,%.2f, "
						"%.3f,%d,%d\n", trade.market, trade.securityId,
						trade.updateTime, trade.channel, trade.bizIndex,
						trade.tradeSeq, trade.askSeq, trade.bidSeq,
						trade.isCancel, trade.tradeMoney * XPRICE_DIV,
						trade.tradePx * XPRICE_DIV, trade.tradeQty,
						XNsTime2I(trade._recvTime));

			}
			break;
		default:
			break;
		}
	}
	fclose(fp);
}

static void find_tick_frm_px_to_biz(XChar *mktInput, XInt market,
		XChar *securityId, XPrice price, XSeqNum biz) {
	FILE *fp = NULL, *fpResult = NULL;
	XL2LT l2l;
	XTickTradeT trade = { 0 };
	XTickOrderT order = { 0 };
	XSeqNum beginSeq = 0;
	XBool bContinue = true;

	fp = fopen(mktInput, "rb");
	if (NULL == fp) {
		slog_error(3, "未找到数据文件[%s]", mktInput);
		return;
	}

	fpResult = fopen("result.csv", "w");
	if (NULL == fpResult) {
		return;
	}

	fprintf(fpResult,
							"#order=1,market,securityId,updateTime,channel,bizIndex,ordSeq,"
									"seqno,bsType,isCancel,ordType,ordPx,ordQty,locTime\n");

	fprintf(fpResult,
							"#trade=2,market,securityId,updateTime,channel,bizIndex,tradeSeq,"
									"askSeq,bidSeq,isCancel,tradeMoney,tradePx,tradeQty,locTime\n");

	while (!feof(fp) && bContinue) {
		fread(&l2l, sizeof(XL2LT), 1, fp);
		switch (l2l.head.type) {
		case eMTickOrder:
			order = l2l.order;

			if(beginSeq && market == order.market && !strncmp(order.securityId, securityId, strlen(securityId)))
			{
				fprintf(fpResult,

								"1,%d,%s,%d,%d,%lld,%lld,%lld,%d,%d,%d,%.3f,%d,%d\n",
										order.market, order.securityId, order.updateTime,
										order.channel, order.bizIndex, order.ordSeq,
										order.seqno, order.bsType, order.isCancel,
										order.ordType, order.ordPx * XPRICE_DIV, order.ordQty,
										XNsTime2I(order._recvTime));
				if(biz == order.bizIndex)
				{
					bContinue = false;
				}
			}

			break;
		case eMTickTrade:
			trade = l2l.trade;

			if(market == trade.market && !strncmp(trade.securityId, securityId, strlen(securityId)))
			{
				//首次成交,开始输出数据
				if(price == trade.tradePx)
				{
					beginSeq = trade.bizIndex;
				}

				if(beginSeq)
				{
					fprintf(fpResult, "2,%d,%s,%d,%d,%lld,%lld,%lld,%lld,%d,%.2f, "
															"%.3f,%d,%d\n", trade.market, trade.securityId,
															trade.updateTime, trade.channel, trade.bizIndex,
															trade.tradeSeq, trade.askSeq, trade.bidSeq,
															trade.isCancel, trade.tradeMoney * XPRICE_DIV,
															trade.tradePx * XPRICE_DIV, trade.tradeQty,
															XNsTime2I(trade._recvTime));
				}
				if(biz == order.bizIndex)
				{
					bContinue = false;
				}
			}
			break;
		default:
			break;
		}
	}
	fclose(fp);
	fclose(fpResult);
}

static void trans_snap_bin2csv(XInt market, XChar *securityId, XChar *inputfile,
		XChar *outputfile) {
	FILE *fp = NULL, *fptickcsv = NULL;
	XL2LT l2l;
	XSnapshotBaseT snapshot = { };
	int i;
	fp = fopen(inputfile, "rb");
	if (NULL == fp) {
		return;
	}

	fptickcsv = fopen(outputfile, "w");
	if (NULL == fptickcsv) {
		return;
	}
	fprintf(fptickcsv,
			"securityId,updateTime,open,high,low,close,volume,amount,"
					"bid5,"
					"bidqty5,bid4,bidqty4,bid3,bidqty3,bid2,bidqty2,bid1,bidqty1,ask5,"
					"askqty5,ask4,askqty4,ask3,askqty3,ask2,askqty2,ask1,askqty1\n");

	while (!feof(fp)) {
		fread(&l2l, sizeof(XL2LT), 1, fp);
		switch (l2l.head.type) {
		case eMSnapshot:
			snapshot = l2l.snapshot;

			if (NULL == securityId
					|| (market == snapshot.market
							&& 0
									== strncmp(snapshot.securityId, securityId,
											strlen(securityId)))) {

				fprintf(fptickcsv, "%s.%s,%d,%.3f,%.3f,%.3f,%.3f,%lld,%.2f",
						snapshot.securityId,
						snapshot.market == eXMarketSha ? "SH" : "SZ",
						snapshot.updateTime, snapshot.openPx * 0.0001,
						snapshot.highPx * 0.0001, snapshot.lowPx * 0.0001,
						snapshot.tradePx * 0.0001, snapshot.volumeTrade,
						snapshot.amountTrade * 0.0001);

				for (i = 4; i >= 0; i--) {
					fprintf(fptickcsv, ",%.3f,%lld", snapshot.bid[i] * 0.0001,
							snapshot.bidqty[i]);
				}
				for (i = 4; i >= 0; i--) {
					fprintf(fptickcsv, ",%.3f,%lld", snapshot.ask[i] * 0.0001,
							snapshot.askqty[i]);
				}
				fprintf(fptickcsv, "\n");
				fflush(fptickcsv);
			}
			break;
		default:
			break;
		}
	}
	fflush(fptickcsv);
	fclose(fptickcsv);
	fclose(fp);
}

static void trans_tick_filter(XChar *fileInput, XChar *fileFilter,
		XChar *channel[], XInt icnt) {
	FILE *fp = NULL, *fpFilter = NULL;
	XL2LT l2l;
	XTickTradeT trade = { 0 };
	XTickOrderT order = { 0 };
	XInt i = 0;

	fp = fopen(fileInput, "rb");
	if (NULL == fp) {
		return;
	}

	fpFilter = fopen(fileFilter, "wb");
	if (NULL == fpFilter) {
		return;
	}

	while (!feof(fp)) {
		fread(&l2l, sizeof(XL2LT), 1, fp);
		switch (l2l.head.type) {
		case eMTickOrder:
			order = l2l.order;

			for (i = 0; i < icnt; i++) {
				if (atoi(channel[i]) == order.channel) {
					fwrite(&l2l, sizeof(XL2LT), 1, fpFilter);
					break;
				}
			}

			break;
		case eMTickTrade:
			trade = l2l.trade;

			for (i = 0; i < icnt; i++) {
				if (atoi(channel[i]) == trade.channel) {
					fwrite(&l2l, sizeof(XL2LT), 1, fpFilter);
					break;
				}
			}

			break;
		default:
			break;
		}
	}
	fflush(fpFilter);
	fclose(fpFilter);
	fclose(fp);
}

static void trans_snapshot_filter(XChar *fileInput, XChar *fileFilter,
		XChar *securityId) {
	FILE *fp = NULL, *fpFilter = NULL;
	XL2LT l2l;
	XSnapshotBaseT snapshot = { 0 };

	fp = fopen(fileInput, "rb");
	if (NULL == fp) {
		return;
	}

	fpFilter = fopen(fileFilter, "wb");
	if (NULL == fpFilter) {
		return;
	}

	while (!feof(fp)) {
		fread(&l2l, sizeof(XL2LT), 1, fp);
		switch (l2l.head.type) {

		// 快照先不落地
		case eMSnapshot:
			snapshot = l2l.snapshot;
			if (NULL == securityId
					|| 0
							== strncmp(securityId, snapshot.securityId,
									strlen(securityId))) {
				fwrite(&l2l, sizeof(XL2LT), 1, fpFilter);
			}

			break;
		default:
			break;
		}
	}
	fflush(fpFilter);
	fclose(fpFilter);
	fclose(fp);
}

static void tick_check(XInt channel, XChar *file) {
	FILE *fp = NULL;
	XL2LT l2l;
	XTickTradeT trade = { 0 };
	XTickOrderT order = { 0 };
	XSeqNum orderseq = 0;
	XSeqNum tradeseq = 0;
	XSeqNum bizIndex = 0;

	fp = fopen(file, "rb");
	if (NULL == fp) {
		return;
	}

	while (!feof(fp)) {
		fread(&l2l, sizeof(XL2LT), 1, fp);
		switch (l2l.head.type) {
		case eMTickOrder:
			order = l2l.order;
			if (order.channel != channel) {
				break;
			}
			if (order.market == eXMarketSha) {
				if (order.ordSeq != orderseq + 1) {
					printf("[%d]频道委托丢失,当前[%lld],上次[%lld]\n", channel,
							order.ordSeq, orderseq);
				}
				orderseq = order.ordSeq;
			} else {
				if (order.bizIndex != bizIndex + 1) {
					printf("[%d]频道丢失,当前[%lld],上次[%lld]\n", channel,
							order.bizIndex, bizIndex);
				}
				bizIndex = order.bizIndex;
			}
			break;
		case eMTickTrade:
			trade = l2l.trade;
			if (trade.channel != channel) {
				break;
			}
			if (trade.market == eXMarketSha) {
				if (trade.tradeSeq != tradeseq + 1) {
					printf("[%d]频道成交丢失,当前[%lld],上次[%lld]\n", channel,
							trade.tradeSeq, tradeseq);
				}
				tradeseq = trade.tradeSeq;
			} else {
				if (trade.bizIndex != bizIndex + 1) {
					printf("[%d]频道丢失,当前[%lld],上次[%lld]\n", channel,
							trade.bizIndex, bizIndex);
				}
				bizIndex = trade.bizIndex;
			}
			break;
		default:
			break;
		}
	}

	fclose(fp);
}
static void trans_trade_bin2csv() {
	FILE *fp = NULL, *fptradecsv = NULL;
	XTradeCache tradecache;
	XOrderReqT ordReq;
	XOrderT order;
	XTradeT trade;

	fp = fopen(XMAN_DATA_TRADEBIN, "rb");
	if (NULL == fp) {
		return;
	}

	fptradecsv = fopen(XMAN_DATA_TRADECSV, "w");
	if (NULL == fptradecsv) {
		return;
	}
	fprintf(fptradecsv,
			"订单请求=本地报单编号,撤单编号,客户号,股东帐户,市场,证券代码,买卖类型,是否撤单,订单类型,数量,原始订单编号,原始柜台订单编号\n");
	fprintf(fptradecsv,
			"订单响应=本地报单编号,撤单编号,客户号,股东帐户,市场,证券代码,买卖类型,是否撤单,订单类型,数量,原始订单编号,原始柜台订单编号,状态,订单编号,成交数量,错误码,发送时间,确认时间\n");
	fprintf(fptradecsv, "成交=订单编号,股东账户,市场,证券代码,买卖方向,成交数量\n");

	while (!feof(fp)) {
		fread(&tradecache, sizeof(XTradeCache), 1, fp);
		switch (tradecache.head.type) {
		case eDOrderReq:
			memcpy(&ordReq, &(tradecache.ordreq), sizeof(XOrderReqT));
			// 本地报单编号,撤单编号,客户号,股东帐户,市场,证券代码,买卖类型,是否撤单,订单类型,数量,原始订单编号,原始柜台订单编号
			fprintf(fptradecsv, "%d,%d,%d,%s,%s,%d,%s,%d,%d,%d,%d,%d,%lld\n",
					eDOrderReq, ordReq.localId, ordReq.clocalId,
					ordReq.customerId, ordReq.investId, ordReq.market,
					ordReq.securityId, ordReq.bsType, ordReq.isCancel,
					ordReq.ordType, ordReq.ordQty, ordReq.orgLocalId,
					ordReq.orgOrdId);
			break;
		case eDOrder:
			memcpy(&order, &(tradecache.ordrsp), sizeof(XOrderT));
			// 本地报单编号,撤单编号,客户号,股东帐户,市场,证券代码,买卖类型,是否撤单,订单类型,数量,原始订单编号,原始柜台订单编号,状态,订单编号,成交数量,错误码,确认时间
			fprintf(fptradecsv,
					"%d,%d,%d,%s,%s,%d,%s,%d,%d,%d,%d,%d,%lld,%d,%lld,%d,%d,%lld,%lld\n",
					eDOrder, order.request.localId, order.request.clocalId,
					order.request.customerId, order.request.investId,
					order.request.market, order.request.securityId,
					order.request.bsType, order.request.isCancel,
					order.request.ordType, order.request.ordQty,
					order.request.orgLocalId, order.request.orgOrdId,
					order.ordStatus, order.ordid, order.trdQty, order.errorno,
					order._sendTime, order._cnfTime);
			break;
		case eDTrade:
			// 订单编号,股东账户,市场,证券代码,买卖方向,成交数量
			memcpy(&trade, &(tradecache.trade), sizeof(XTradeT));
			fprintf(fptradecsv, "%d,%lld,%s,%d,%s,%d,%d\n", eDTrade,
					trade.ordid, trade.investId, trade.market, trade.securityId,
					trade.trdSide, trade.trdQty);

			break;
		case eDCash:
			break;
		case eDHold:
			break;
		default:
			break;
		}
	}

	fflush(fptradecsv);
	fclose(fptradecsv);
	fclose(fp);
}

static void usage() {
	printf("###############################################################\n");
	printf("#    量赢策略交易平台(%s) \n", __XMAN_VERSION__);
	printf("# -h [help] 帮助\n");
	printf("# -t "
			"[2:tsnap2csv(转重构后的快照),3:trade2csv(转交易),4:tick2csv("
			"转逐笔和快照),5:tick2filter(转逐笔数据到频道,需-i[InputFile], "
			"-f[FilterFile]),6:snap2filter(转交易所快照数据,需-i[InputFile], "
			"-f[FilterFile])]\n");
	printf("# -t [6:检查频道是否丢单]\n");
	printf("# -m [market] 市场\n");
	printf("# -s [securityid] 证券代码\n");
	printf("# -c [channel] 频道,多个频道以,分割\n");
	printf("# -i [mktInputBin] 输入行情\n");
	printf("# -f [FilterMktBin] 输出行情\n");
	printf("例子:\n");
	printf("1. 转换重构快照为csv\n");
	printf("./xhisday -t2 -m1 -s600837\n");
	printf("2. 转换交易回报数据为csv\n");
	printf("./xhisday -t3\n");
	printf("3. 转换落地行情数据为csv\n");
	printf("./xhisday -t4\n");
	printf("4. 以频道拆分当天行情文件\n");
	printf(
			"./xhisday -t5 -i../data/store/mktstore.bin -fsave.bin -c2014,801\n");
	printf("5. 过滤落地快照数据为bin\n");
	printf("./xhisday -t6 -i../data/mktstore.bin -fsave.bin\n");
	printf("6. 检查频道是否丢单\n");
	printf("./xhisday -t7 -c2 -imktstore.bin\n");
	printf("7. 转换交易所快照为csv\n");
	printf("./xhisday -t8 -isnapshot.bin -fsnapshot.csv\n");
	printf("8. 查找bizIndex\n");
	printf("./xhisday -t9 -imktstore.bin -b6798655\n");
	printf("9. 查找涨停价开始到bizIndex之间所有数据\n");
	printf("./xhisday -t10 -imktsotre.bin -m1 -s600837 -c103400 -b6798655\n");

	printf("###############################################################\n");
}

int main(int argc, char *argv[]) {

	int opt;
	int option_index = 0;
	const char *option = "ht:m:s:c:i:f:b:";
	const struct option long_options[] = { { "help", no_argument, NULL, 'h' }, {
			"type", no_argument, NULL, 't' }, { "market", required_argument,
			NULL, 'm' }, { "security", required_argument, NULL, 's' }, {
			"channel", required_argument, NULL, 'c' }, { "mktInput",
			required_argument, NULL, 'i' }, { "mktFilter", required_argument,
			NULL, 'f' }, { "bizIndex", required_argument, NULL, 'b' }, { NULL,
			0, NULL, 0 } };
	char *securityId = NULL;
	XInt market = 0;
	XInt type = 0;
	XChar *channel = NULL;
	char *filterFile = NULL;
	char *mktInput = XMAN_DATA_MKTSTOREBIN;
	XChar *temp[100];
	XInt icnt = 0;
	XSeqNum biz = 0;

	xslog_init(NULL, "xhisday");

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

			/** 市场 */
		case 'm':
			if (optarg) {
				market = atoi(optarg);
			}
			break;
			/** 证券代码 */
		case 's':
			if (optarg) {
				securityId = optarg;
			}
			break;
		case 'c':
			channel = optarg;
			break;
		case 'i':
			mktInput = optarg;
			break;
		case 'f':
			filterFile = optarg;
			break;
		case 'b':
			biz = atoll(optarg);
			break;
		default:
			break;
		}
	}

	xslog_init(NULL, "xhisday");

	switch (type) {

	case 1:

		break;
		/** 重构快照数据转为csv */
	case 2:
		trans_rsnap_bin2csv(market, securityId);
		break;
		/** 交易日志 */
	case 3:
		trans_trade_bin2csv();
		break;
		/** 逐笔转csv */
	case 4:
		if (NULL != channel) {
			trans_tick_bin2csv(mktInput, market, securityId, atoi(channel));
		} else {
			trans_tick_bin2csv(mktInput, market, securityId, 0);
		}
		break;

		/** 分数据 */
	case 5:
		if (NULL == filterFile || 0 == strcmp(mktInput, filterFile)) {
			usage();
			break;
		}
		if (NULL != channel) {
			XSplit(channel, ",", strlen(channel), temp, &icnt);
			trans_tick_filter(mktInput, filterFile, temp, icnt);
		}
		break;
		/** 转换交易所快照数据 */
	case 6:
		if (NULL == filterFile || 0 == strcmp(mktInput, filterFile)) {
			usage();
			break;
		}
		trans_snapshot_filter(mktInput, filterFile, securityId);
		break;

		// 检查频道数据有无丢失
	case 7:

		if (channel == 0) {
			break;
		}
		if (NULL != channel) {
			tick_check(atoi(channel), mktInput);
		} else {
			tick_check(0, mktInput);
		}
		break;
	case 8:
		if (NULL == filterFile || 0 == strcmp(mktInput, filterFile)) {
			usage();
			break;
		}
		trans_snap_bin2csv(market, securityId, mktInput, filterFile);
		break;
		//找到对应biz数据并打印
	case 9:
		if(0 == biz)
		{
			usage();
			break;
		}
		find_tick_by_biz(mktInput, biz);
		break;
	case 10:
		if(NULL == securityId || NULL == channel)
		{
			usage();
			break;
		}
		find_tick_frm_px_to_biz(mktInput, market, securityId, atoi(channel), biz);
		break;
	default:
		break;
	}

	return (0);
}
