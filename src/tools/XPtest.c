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

#define FEE_RATIO       (0.00012)

static FILE *forderall = NULL;

typedef struct BackTestParam {
	XInt market; /**< 市场 */
	XSecurityId securityId; /**< 证券代码 */
	XPrice price; /**< 证券价格 < 多少时买入 */
	XPrice buyGapPrice; /**< 盘口价差>控制量时买入 */
	XPrice sellGapPrice; /**< 盘口价差 >控制量时卖出 */
	XInt buySoir; /**< 买SOIR */
	XInt sellSoir; /**< 卖SOIR */
	XInt openZdf; /**< 开盘涨跌幅，买入控制 */
	XInt zdf; /**< 当前涨跌幅要大于设置买入 */
	XInt zfRatio; /**< 振幅波动与开盘涨幅比,卖出控制 */
	XInt buyQtyRatio; /**< 买盘口数量与买卖盘口数量总量比，买控制 */
	XInt sellQtyRatio; /**< 卖盘口数量与买卖盘口数量总量比,卖控制 */
	XSumQty buyQtyLimit; /**< 买一盘口数量大于控制量,买控制 */
	XInt latencyShTradeTime; /**< 延迟成交时间 */
	XInt latencySzTradeTime; /**< 延迟成交时间 */
} XBackTestParamT;

static void Sell(XRSnapshotT *snapshot, XStrategyT *pStrategy,
		XBackTestParamT *param) {
	XMoney amt = 0;
	XPrice p = 0;
	XInt SOIR, SOIR1, SOIR2, SOIR3, SOIR4, SOIR5;
	XSumQty bidqty, askqty;
	XKLineT *k1 = NULL, *k5 = NULL;
	XNum kcursor1 = -1, kcursor5 = -1;
	XPrice avgPx = 0;

	bidqty = snapshot->bidqty[0] + snapshot->bidqty[1] + snapshot->bidqty[2]
			+ snapshot->bidqty[3] + snapshot->bidqty[4];
	askqty = snapshot->askqty[0] + snapshot->askqty[1] + snapshot->askqty[2]
			+ snapshot->askqty[3] + snapshot->askqty[4];

	//价格预测
	p = (snapshot->bid[0] + snapshot->ask[0]) / 2
			+ (snapshot->ask[0] - snapshot->bid[0]) * snapshot->bidqty[0]
					/ ((snapshot->bidqty[0] + snapshot->askqty[0]) * 2);

	//订单失衡SOIR
	//SOIR = (Bvi - Avi)/(Bvi + Avi)
	//权重 w = 1 - (i - 1)/5
	//加强 SOIR = ((Bv1 - Av1)/(Bv1 + Av1) + 4/5(Bv2 - Av2)/(Bv2 + Av2) + 3/5(Bv3 - Av3)/(Bv3 + Av3) + 2/5(B4 - A4)/(B4 + A4) + 1/5(B5 - A5)/(B5 + A5)) / ( 1 + 4/5 + 3/5 + 2/5 + 1/5)
	//SOIR为正说明市场买压大于卖压，未来价格趋向上涨，且SOIR的值越大，上涨的概率越高，反之亦然

	//中间价变化率MPC,MPC为正说明股票未来短期价格趋向上涨，且MPC的值越大，其上涨的概率越高，反之亦然
	//MPC = (A + B) - (A1 + B1) / (A1 + B1)

	SOIR1 = (snapshot->bidqty[0] - snapshot->askqty[0]) * 100
			/ (snapshot->bidqty[0] + snapshot->askqty[0]);
	SOIR2 = (snapshot->bidqty[1] - snapshot->askqty[1]) * 100
			/ (snapshot->bidqty[1] + snapshot->askqty[1]);
	SOIR3 = (snapshot->bidqty[2] - snapshot->askqty[2]) * 100
			/ (snapshot->bidqty[2] + snapshot->askqty[2]);
	SOIR4 = (snapshot->bidqty[3] - snapshot->askqty[3]) * 100
			/ (snapshot->bidqty[3] + snapshot->askqty[3]);
	SOIR5 = (snapshot->bidqty[4] - snapshot->askqty[4]) * 100
			/ (snapshot->bidqty[4] + snapshot->askqty[4]);

	SOIR = (5 * SOIR1 + 4 * SOIR2 + 3 * SOIR3 + 2 * SOIR4 + SOIR5)
			/ (5 + 4 + 3 + 2 + 1);

	/**
	 slog_debug(0, "[%d-%s] [%d] 当前最新价[%d] 预测股价[%d], SOIR[%d]", snapshot->market, snapshot->securityId,
	 snapshot->updateTime, snapshot->tradePx, p, SOIR);
	 */

	pStrategy->_folwPx = snapshot->tradePx;

	kcursor1 = (kcursor1 - 2) & (SNAPSHOT_K1_CNT - 1);
	k1 = GetKlinesByBlock(snapshot->idx, 0);

	kcursor5 = (kcursor5 - 2) & (SNAPSHOT_K5_CNT - 1);
	k5 = GetKlinesByBlock(snapshot->idx, 1);

	if(k1[kcursor1].high > k5[kcursor5].high)
	{
		return;
	}

	avgPx = snapshot->amountTrade / snapshot->volumeTrade;
	//必须受到成交后再触发卖单，此处以时间控制
	//严进松出,在买入时价格在最高+最低的一半以下
	if (!pStrategy->_signal && p < snapshot->tradePx
			&& snapshot->ask[0] - snapshot->bid[0] > param->sellGapPrice
			&& SOIR < param->sellSoir
			&& askqty > param->sellQtyRatio * 0.0001 * (askqty + bidqty)) {
		pStrategy->_signal = 1;
		pStrategy->_sellLocTime = snapshot->updateTime;
		pStrategy->_lastSellPx = snapshot->bid[0];
		//发送委托
		slog_debug(0, "<<<<<< 卖委托[%d-%s],[%d-%d], SOIR[%d], 价差[%d], 买卖势能[%.2f]",
				snapshot->market, snapshot->securityId, pStrategy->_sellLocTime,
				pStrategy->_lastSellPx, SOIR,
				snapshot->ask[0] - snapshot->bid[0],
				askqty * 100.0 / (askqty + bidqty));
	}

	//如果后续价格大于买的价格,成交
	if (pStrategy->_signal
			&& ((snapshot->market == eXMarketSha&& XMsC2S(snapshot->updateTime)
					> XMsC2S(pStrategy->_sellLocTime)) || (snapshot->market == eXMarketSza && XMsC2S(snapshot->updateTime)
	> XMsC2S(pStrategy->_sellLocTime)))
							+ param->latencySzTradeTime) {
		if (pStrategy->_lastSellPx <= snapshot->tradePx) {
			pStrategy->status = eXBuy;
			pStrategy->_signal = 0;
			pStrategy->__sellTrdAmt = snapshot->tradePx * 10 * (1 - FEE_RATIO); // 扣除手续费
			pStrategy->_buyCtrlMoney += pStrategy->__sellTrdAmt;
			pStrategy->_slipSellTimes++;
			slog_debug(0, "@@@@@@ 卖成交[%d-%s],成交时间[%d]-成交价格[%.3f],收益[%.3f]",
					snapshot->market, snapshot->securityId,
					snapshot->updateTime, snapshot->tradePx * 0.0001,
					pStrategy->_buyCtrlMoney * 0.0001);
			fprintf(forderall, "%s,%lld,%d,%d,%.3f,%d,%.3f,%.2f,%d,%d,%.2f\n",
					snapshot->securityId, pStrategy->_sellLocTime,
					snapshot->updateTime, 2, pStrategy->_lastSellPx * 0.0001,
					10, snapshot->tradePx * 0.0001, snapshot->tradePx * 10 * (1 - FEE_RATIO) * 0.0001,
					XMsC2S(pStrategy->_sellLocTime) - XMsC2S(pStrategy->_buyLocTime),
					XMsC2S(snapshot->updateTime) - XMsC2S(pStrategy->_sellLocTime), (pStrategy->__sellTrdAmt - pStrategy->__buyTrdAmt) * 0.0001);
		}
	}

}

static void Buy(XRSnapshotT *snapshot, XStrategyT *pStrategy,
		XBackTestParamT *param) {
	XMoney amt = 0;
	XPrice p = 0;
	XInt SOIR, SOIR1, SOIR2, SOIR3, SOIR4, SOIR5;
	XSumQty bidqty, askqty;
	XKLineT *k1 = NULL, *k5 = NULL;
	XNum kcursor1 = -1, kcursor5 = -1;
	XInt i;
	XSumQty sum, div;
	XInt zdf = 0, openzdf;

	//高价股不做
	if(snapshot->tradePx > param->price)
	{
		return;
	}

	// 如果14:55不再开仓
	if (snapshot->updateTime > 145000000) {
		return;
	}

	//盘口订单失衡
	for (i = 0; i < 5; i++) {
		sum += (5 - i) * (snapshot->bid[i] - snapshot->askqty[i]);
		div += (5 - i) * (snapshot->bid[i] + snapshot->askqty[i]);
	}
	//      pt = sum / div;
	//订单失衡
	//与上一根K线的比较

	//买单量大于卖单量

	bidqty = snapshot->bidqty[0] + snapshot->bidqty[1] + snapshot->bidqty[2]
			+ snapshot->bidqty[3] + snapshot->bidqty[4];
	askqty = snapshot->askqty[0] + snapshot->askqty[1] + snapshot->askqty[2]
			+ snapshot->askqty[3] + snapshot->askqty[4];

	//价格预测
	p = (snapshot->bid[0] + snapshot->ask[0]) / 2
			+ (snapshot->ask[0] - snapshot->bid[0]) * snapshot->bidqty[0]
					/ ((snapshot->bidqty[0] + snapshot->askqty[0]) * 2);

	//订单失衡SOIR
	//SOIR = (Bvi - Avi)/(Bvi + Avi)
	//权重 w = 1 - (i - 1)/5
	//加强 SOIR = ((Bv1 - Av1)/(Bv1 + Av1) + 4/5(Bv2 - Av2)/(Bv2 + Av2) + 3/5(Bv3 - Av3)/(Bv3 + Av3) + 2/5(B4 - A4)/(B4 + A4) + 1/5(B5 - A5)/(B5 + A5)) / ( 1 + 4/5 + 3/5 + 2/5 + 1/5)
	//SOIR为正说明市场买压大于卖压，未来价格趋向上涨，且SOIR的值越大，上涨的概率越高，反之亦然

	//中间价变化率MPC,MPC为正说明股票未来短期价格趋向上涨，且MPC的值越大，其上涨的概率越高，反之亦然
	//MPC = (A + B) - (A1 + B1) / (A1 + B1)

	SOIR1 = (snapshot->bidqty[0] - snapshot->askqty[0]) * 100
			/ (snapshot->bidqty[0] + snapshot->askqty[0]);
	SOIR2 = (snapshot->bidqty[1] - snapshot->askqty[1]) * 100
			/ (snapshot->bidqty[1] + snapshot->askqty[1]);
	SOIR3 = (snapshot->bidqty[2] - snapshot->askqty[2]) * 100
			/ (snapshot->bidqty[2] + snapshot->askqty[2]);
	SOIR4 = (snapshot->bidqty[3] - snapshot->askqty[3]) * 100
			/ (snapshot->bidqty[3] + snapshot->askqty[3]);
	SOIR5 = (snapshot->bidqty[4] - snapshot->askqty[4]) * 100
			/ (snapshot->bidqty[4] + snapshot->askqty[4]);

	SOIR = (5 * SOIR1 + 4 * SOIR2 + 3 * SOIR3 + 2 * SOIR4 + SOIR5)
			/ (5 + 4 + 3 + 2 + 1);

	/**
	 slog_debug(0, "[%d-%s] [%d] 当前最新价[%d] 预测股价[%d], SOIR[%d],买一量[%lld]", snapshot->market, snapshot->securityId,
	 snapshot->updateTime, snapshot->tradePx, p, SOIR, snapshot->bidqty[0]);
	 */
	pStrategy->_folwPx = snapshot->tradePx;

	kcursor1 = (kcursor1 - 2) & (SNAPSHOT_K1_CNT - 1);
	k1 = GetKlinesByBlock(snapshot->idx, 0);

	kcursor5 = (kcursor5 - 2) & (SNAPSHOT_K5_CNT - 1);
	k5 = GetKlinesByBlock(snapshot->idx, 1);

	zdf = (snapshot->tradePx - snapshot->preClosePx) * 10000
			/ snapshot->preClosePx;

	openzdf = (snapshot->openPx - snapshot->preClosePx) * 10000
			/ snapshot->preClosePx;
	//如果短周期不在长周期上不开仓
	//必须受到成交后再触发卖单，此处以时间控制
	//严进松出,在买入时价格在最高+最低的一半以下
	//量比
	//上一交易日振幅
	//K线位置

	if(k1[kcursor1].low < k5[kcursor5].low)
	{
		return;
	}

	//当前股票价格
	if (!pStrategy->_signal && openzdf >=param->openZdf && zdf > param->zdf
			&& abs(openzdf - zdf) < param->zfRatio && p > snapshot->tradePx
			&& ((snapshot->market == eXMarketSha && snapshot->bidqty[0] > param->buyQtyLimit) || (snapshot->market == eXMarketSza && snapshot->bidqty[0] > param->buyQtyLimit * 10)) /** 买一量要大于100 */
			&& (snapshot->ask[0] - snapshot->bid[0]) > param->buyGapPrice
			&& SOIR > param->buySoir
			&& bidqty > param->buyQtyRatio * 0.0001 * (askqty + bidqty)) {
		pStrategy->_signal = 1;
		pStrategy->_buyLocTime = snapshot->updateTime;
		pStrategy->_lastBuyPx = snapshot->ask[0];
		//发送委托
		slog_debug(0,
				">>>>>> 买委托[%d-%s],[%d-%d], SOIR[%d], 价差[%d], 买卖势能[%.2f],均价线[5.3f]",
				snapshot->market, snapshot->securityId, pStrategy->_buyLocTime,
				pStrategy->_lastBuyPx, SOIR,
				snapshot->ask[0] - snapshot->bid[0],
				bidqty * 100.0 / (askqty + bidqty),
				snapshot->amountTrade * 0.0001 / snapshot->volumeTrade);
	}

	//如果后续价格大于买的价格,成交
	if (pStrategy->_signal
			&& ((snapshot->market == eXMarketSha && XMsC2S(snapshot->updateTime)
					> XMsC2S(pStrategy->_buyLocTime) + param->latencyShTradeTime) ||
			(snapshot->market == eXMarketSza && XMsC2S(snapshot->updateTime)
	        > XMsC2S(pStrategy->_buyLocTime) + param->latencySzTradeTime))) {
		if (pStrategy->_lastBuyPx >= snapshot->tradePx) {
			pStrategy->status = eXSell;
			pStrategy->_signal = 0;

			pStrategy->__buyTrdAmt = snapshot->tradePx * 10 * (1 + FEE_RATIO);

			pStrategy->_buyCtrlMoney -= pStrategy->__buyTrdAmt;

			pStrategy->_slipBuyTimes++;
			slog_debug(0, "@@@@@@ 买成交[%d-%s],成交时间[%d]-成交价格[%.3f], 收益[%.3f]",
					snapshot->market, snapshot->securityId,
					snapshot->updateTime, snapshot->tradePx * 0.0001, 0.0);
			fprintf(forderall, "%s,%lld,%d,%d,%.3f,%d,%.3f,-%.2f,0,%d,0\n",
					snapshot->securityId, pStrategy->_buyLocTime,
					snapshot->updateTime, 1, pStrategy->_lastBuyPx * 0.0001, 10,
					snapshot->tradePx * 0.0001,
					snapshot->tradePx * 10 * (1 + FEE_RATIO) * 0.0001,
					XMsC2S(snapshot->updateTime) - XMsC2S(pStrategy->_buyLocTime)
					);
		}
	}
}

static void Trade(XRSnapshotT *snapshot, XBackTestParamT *param) {
	XStrategyT *pStrategy = NULL, strategy;

	pStrategy = XFndVStrategyByFrontId("customer", 0, 0, snapshot->securityId,
			eXBuy);
	if (NULL == pStrategy) {
		memset(&strategy, 0, sizeof(XStrategyT));
		strategy.setting.market = snapshot->market;
		strategy.idx = XGetIdByType(eSPlot);
		memcpy(strategy.setting.securityId, snapshot->securityId,
		SECURITYID_LEN);
		strategy.setting.bsType = eXBuy;
		memcpy(strategy.plot.customerId, "customer", strlen("customer"));
		strategy.plotid = XGenPlotId(1, strategy.idx);
		XPutVStrategy(&strategy);

		pStrategy = XFndVStrategyByFrontId("customer", 0, 0,
				snapshot->securityId, eXBuy);
	}

	if (pStrategy->status != eXSell) {
		Buy(snapshot, pStrategy, param);
	} else {
		Sell(snapshot, pStrategy, param);
	}

}
static XMoney Result(XBackTestParamT *param) {
	XIdx i = 0;
	XStrategyT *pStrategy = NULL;
	FILE *fp = NULL, *fpall = NULL;
	XMonitorT *pMonitor = NULL;
	XInt kpzf = -1, zf = -1, zfr, dfr;
	XRSnapshotT *snapshot = NULL;
	XChar buf[1024];
	XMoney totalProfit = 0;
	XChar *profitMode = "";
	XNum zj = 0,dj = 0,yj = 0,kj = 0;

	memset(buf, 0, sizeof(buf));

	fpall = fopen("backtest.csv", "a+");

	//落地文件名组成：证券代码+ 开盘涨幅 + 买价差 + 卖价差 + 买数量控制 + 买盘口比例 + SOIR(买） +　SOIR(卖)
	if (!strlen(param->securityId)) {
		sprintf(buf, "result/profit_all_%d_%d_%d_%lld_%d_%d_%d.csv",
				param->zdf, param->buyGapPrice, param->sellGapPrice,
				param->buyQtyLimit, param->buyQtyRatio, param->buySoir,
				param->sellSoir);
	} else {
		sprintf(buf, "result/profit_%s_%d_%d_%d_%lld_%d_%d_%d.csv",
				param->securityId, param->zdf, param->buyGapPrice,
				param->sellGapPrice, param->buyQtyLimit, param->buyQtyRatio,
				param->buySoir, param->sellSoir);
	}
	fp = fopen(buf, "w");

	//输出当前参数列表
	fprintf(fp, "回测参数列表：\n");
	fprintf(fp, "市场%d,证券代码：%s\n", param->market, param->securityId);
	fprintf(fp, "证券价格<:%.3f\n", param->price * 0.0001);
	fprintf(fp, "买盘口价差>:%.3f\n", param->buyGapPrice * 0.0001);
	fprintf(fp, "卖盘口价差>:%.3f\n", param->sellGapPrice * 0.0001);
	fprintf(fp, "买SOIR>:%d\n", param->buySoir);
	fprintf(fp, "卖SOIR<:%d\n", param->sellSoir);
	fprintf(fp, "开盘涨跌幅>:%d\n", param->openZdf);
	fprintf(fp, "当前涨跌幅>:%d\n", param->zdf);
	fprintf(fp, "振幅<:%d\n", param->zfRatio);
	fprintf(fp, "买盘口数量与买卖盘口数量比例:%.3f\n", param->buyQtyRatio * 0.0001);
	fprintf(fp, "卖盘口数量与买卖盘口数量比例：%.3f\n", param->sellQtyRatio * 0.0001);
	fprintf(fp, "买一盘口数量>:%lld\n", param->buyQtyLimit);
	fprintf(fp, "上海模拟成交距委托的最小时间:%dms\n", param->latencyShTradeTime);
	fprintf(fp, "深圳模拟成交距委托的最小时间:%dms\n", param->latencySzTradeTime);

	fprintf(fp, "证券代码,收益额,买次数,卖次数,开盘涨幅,收盘涨幅,最大涨幅,最大跌幅,收盘价,盈利模式,平均每次盈亏\%\n");
	pMonitor = XFndVMonitor();
	//最后输出收益统计
	for (i = 0; i < pMonitor->iTPlot; i++) {
		pStrategy = XFndVStrategyById(i + 1);
		if (NULL == pStrategy) {
			continue;
		}
		if (pStrategy->_slipBuyTimes > 0) {
			if (pStrategy->_slipBuyTimes != pStrategy->_slipSellTimes) {
				pStrategy->_buyCtrlMoney += pStrategy->_folwPx * 10;
			}
			profitMode = "";

			snapshot = XFndVRSnapshotByKey(pStrategy->setting.market,
					pStrategy->setting.securityId);
			kpzf = (snapshot->openPx - snapshot->preClosePx) * 10000
					/ snapshot->preClosePx;
			zf = (snapshot->tradePx - snapshot->preClosePx) * 10000
					/ snapshot->preClosePx;
			zfr = (snapshot->highPx - snapshot->preClosePx) * 10000 / snapshot->preClosePx;
			dfr = (snapshot->lowPx - snapshot->preClosePx) * 10000 / snapshot->preClosePx;
			//上涨盈利
			if (zf > kpzf && pStrategy->_buyCtrlMoney > 0) {
				profitMode = "上涨盈利";
			}
			//上涨亏损
			else if (zf > kpzf && pStrategy->_buyCtrlMoney < 0) {
				profitMode = "上涨亏损";
			}
			//下跌盈利
			else if (zf < kpzf && pStrategy->_buyCtrlMoney > 0) {
				profitMode = "下跌盈利";
			}
			//下跌亏损
			else if (zf < kpzf && pStrategy->_buyCtrlMoney < 0) {
				profitMode = "下跌亏损";
			}

			if(zf >= kpzf)
			{
				zj++;
			}
			else
			{
				dj++;
			}
			if(pStrategy->_buyCtrlMoney >= 0)
			{
				yj++;
			}
			else
			{
				kj++;
			}

			fprintf(fp, "%s,%.3f,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%s,%.2f\n",
					pStrategy->setting.securityId,
					pStrategy->_buyCtrlMoney * 0.0001, pStrategy->_slipBuyTimes,
					pStrategy->_slipSellTimes, kpzf * 0.01, zf * 0.01, zfr * 0.01, dfr *0.01,
					snapshot->tradePx * 0.0001, profitMode, pStrategy->_buyCtrlMoney * 10.0 /snapshot->tradePx / pStrategy->_slipBuyTimes);

			totalProfit += pStrategy->_buyCtrlMoney;
		}
	}
	fprintf(fp, "总收益:%.2f,上涨家数%d,下跌家数:%d,盈利家数:%d,亏损家数:%d\n", totalProfit * 0.0001, zj, dj, yj, kj);

	fprintf(fpall, "%s, 总收益:%.2f,上涨家数%d,下跌家数:%d,盈利家数:%d,亏损家数:%d\n", buf, totalProfit * 0.0001, zj, dj, yj, kj);

	fclose(fp);
	fclose(fpall);
	return (totalProfit);
}
static void usage() {

}

static XMoney BackTest(XBackTestParamT *param) {
	FILE *fp = NULL;
	XRSnapshotT snapshot, *pSnapshot = NULL;
	XKLineT *k1 = NULL, *k5 = NULL;
	XNum kcursor1 = -1, kcursor5 = -1;
	XIdx idx = -1;
	XChar buf[1024];
	XMoney totalProfit = 0;

	memset(buf, 0, sizeof(buf));
	xslog_init(NULL, "xptest");

	XManShmInit();
	XManShmLoad();

	// 拆分到每个证券代码
	fp = fopen(XMAN_DATA_TSNAPBIN, "rb");
	if (NULL == fp) {
		fprintf(stderr, "文件[%s]找不到\n", XMAN_DATA_TSNAPBIN);
		return (totalProfit);
	}

	if (!strlen(param->securityId)) {
		sprintf(buf, "result/order_all_%d_%d_%d_%lld_%d_%d_%d.csv",
				param->zdf, param->buyGapPrice, param->sellGapPrice,
				param->buyQtyLimit, param->buyQtyRatio, param->buySoir,
				param->sellSoir);
	} else {
		sprintf(buf, "result/order_%s_%d_%d_%d_%lld_%d_%d_%d.csv",
				param->securityId, param->zdf, param->buyGapPrice,
				param->sellGapPrice, param->buyQtyLimit, param->buyQtyRatio,
				param->buySoir, param->sellSoir);
	}

	forderall = fopen(buf, "w");
	fprintf(forderall, "证券代码,委托时间,成交时间,买卖方向,委托价格,委托数量,成交价格,成交金额,持仓时间(ms),委托成交间隔(ms),单笔盈亏\n");

	while (!feof(fp)) {

		fread(&snapshot, sizeof(XRSnapshotT), 1, fp);

		if (NULL == snapshot.securityId || 0 == snapshot.market) {
			continue;
		}

		if (strlen(param->securityId) > 0
				&& strncmp(snapshot.securityId, param->securityId,
						strlen(param->securityId)) != 0) {
			continue;
		}

		idx = XFndOrderBook(snapshot.market, snapshot.securityId);
		if (idx < 1) {
			idx = XPutOrderBookHash(snapshot.market, snapshot.securityId);
		}

		snapshot.idx = idx;
		pSnapshot = XFndVRSnapshotById(idx);
		if (NULL == pSnapshot) {
			XPutOrUpdVRSnapshot(&snapshot);
			pSnapshot = XFndVRSnapshotByKey(snapshot.market,
					snapshot.securityId);
		} else {
			memcpy(pSnapshot, &snapshot, sizeof(XRSnapshotT));
		}

		if (NULL == pSnapshot) {
			continue;
		}

		kcursor1 = pSnapshot->kcursor1 > 0 ? pSnapshot->kcursor1 : 1;
		kcursor1 = (kcursor1 - 1) & (SNAPSHOT_K1_CNT - 1);
		kcursor5 = pSnapshot->kcursor5 > 0 ? pSnapshot->kcursor5 : 1;
		kcursor5 = (kcursor5 - 1) & (SNAPSHOT_K5_CNT - 1);
		k1 = GetKlinesByBlock(pSnapshot->idx, 0);
		k5 = GetKlinesByBlock(pSnapshot->idx, 1);

		//94011234
		if (pSnapshot->updateTime / 5000
				== k1[kcursor1].updateTime / 5000) {
			k1[kcursor1].close = pSnapshot->tradePx;
			k1[kcursor1].updateTime = pSnapshot->updateTime;
			k1[kcursor1].low =
					k1[kcursor1].low > pSnapshot->tradePx ?
							pSnapshot->tradePx : k1[kcursor1].low;
			k1[kcursor1].high =
					k1[kcursor1].high < pSnapshot->tradePx ?
							pSnapshot->tradePx : k1[kcursor1].high;
			k1[kcursor1].qty += pSnapshot->volumeTrade;
			k1[kcursor1].amt += pSnapshot->amountTrade;
		} else {
			pSnapshot->kcursor1++;
			kcursor1 = (pSnapshot->kcursor1 - 1) & (SNAPSHOT_K1_CNT - 1);
			memset(&k1[kcursor1], 0, sizeof(XKLineT));
			k1[kcursor1].open = pSnapshot->tradePx;
			k1[kcursor1].close = pSnapshot->tradePx;
			k1[kcursor1].updateTime = pSnapshot->updateTime;
			k1[kcursor1].low = pSnapshot->tradePx;
			k1[kcursor1].high = pSnapshot->tradePx;
			k1[kcursor1].qty = pSnapshot->volumeTrade;
			k1[kcursor1].amt = pSnapshot->amountTrade;
		}

		if (pSnapshot->updateTime / 100000
				== k5[kcursor5].updateTime / 100000) {
			k5[kcursor5].close = pSnapshot->tradePx;
			k5[kcursor5].updateTime = pSnapshot->updateTime;
			k5[kcursor5].low =
					k5[kcursor5].low > pSnapshot->tradePx ?
							pSnapshot->tradePx : k5[kcursor5].low;
			k5[kcursor5].high =
					k5[kcursor5].high < pSnapshot->tradePx ?
							pSnapshot->tradePx : k5[kcursor5].high;
			k5[kcursor5].qty += pSnapshot->volumeTrade;
			k5[kcursor5].amt += pSnapshot->amountTrade;
		} else {
			pSnapshot->kcursor5++;
			kcursor5 = (pSnapshot->kcursor5 - 1) & (SNAPSHOT_K5_CNT - 1);
			memset(&k5[kcursor5], 0, sizeof(XKLineT));
			k5[kcursor5].open = pSnapshot->tradePx;
			k5[kcursor5].close = pSnapshot->tradePx;
			k5[kcursor5].updateTime = pSnapshot->updateTime;
			k5[kcursor5].low = pSnapshot->tradePx;
			k5[kcursor5].high = pSnapshot->tradePx;
			k5[kcursor5].qty = pSnapshot->volumeTrade;
			k5[kcursor5].amt = pSnapshot->amountTrade;
		}

		// 跳过9:30分委托
		if (snapshot.updateTime < 93100000) {
			continue;
		}

		//逆回购不做回测
		if(!strncmp(snapshot.securityId, "204", strlen("204")) || !strncmp(snapshot.securityId, "1318", strlen("1318")))
		{
			continue;
		}

		Trade(&snapshot, param);
	}

	fclose(fp);

	fclose(forderall);

	totalProfit = Result(param);

	XManShmDelete();

	return (totalProfit);
}

int main(int argc, char *argv[]) {

	XInt i, j;
	int opt;
	int option_index = 0;
	const char *option = "hm:s:b:a:z:";
	const struct option long_options[] = { { "help", 0, NULL, 'h' },
			{ "market",1, NULL, 'm' },
			{ "securityId", 1, NULL, 's' },
			{ "bid", 1, NULL,'b' },
			{ "ask", 1, NULL, 'a' },
			{ "zdf", 1, NULL, 'z' },
			{ NULL, 0, NULL, 0 } };
	XBackTestParamT backTestParam;
	XMoney totalProfit = 0;

	memset(&backTestParam, 0, sizeof(XBackTestParamT));
	backTestParam.buyGapPrice = 1000;
	backTestParam.sellGapPrice = 1000;
	backTestParam.buyQtyLimit = 100;
	backTestParam.buyQtyRatio = 7000;
	backTestParam.latencyShTradeTime = 70;
	backTestParam.latencySzTradeTime = 3;
	backTestParam.openZdf = 0;
	backTestParam.price = 2000000;
	backTestParam.sellQtyRatio = 7000;
	backTestParam.zdf = -250;
	backTestParam.zfRatio = 300;
	backTestParam.buySoir = 28;
	backTestParam.sellSoir = -20;

	while ((opt = getopt_long(argc, argv, option, long_options, &option_index))
			!= -1) {
		switch (opt) {
		case 'h':
		case '?':
			usage();
			exit(0);
			break;
		case 'm':
			backTestParam.market = atoi(optarg);
			break;
		case 's':
			if (NULL != optarg) {
				memcpy(backTestParam.securityId, optarg, strlen(optarg));
			}
			break;
		case 'b':
			backTestParam.buyGapPrice = atoi(optarg);
			break;
		case 'a':
			backTestParam.sellGapPrice = atoi(optarg);
			break;
		case 'z':
			backTestParam.zdf = atoi(optarg);
			break;
		default:
			break;
		}
	}

	totalProfit = BackTest(&backTestParam);

	return (totalProfit);
}
