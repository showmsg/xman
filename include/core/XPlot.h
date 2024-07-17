/**
 * @file XBus.h
 *
 *  Created on: 2022年8月22日
 *      Author: DELL
 */

#ifndef INCLUDE_CORE_XPLOT_H_
#define INCLUDE_CORE_XPLOT_H_
#include "XDataStruct.h"
#include "XStrategy.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 均线计算
 * 
 * @param data 
 * @param len 
 * @param curIdx   最新存储均线为主
 * @param calCnt   计算跨度,如MA5,calCnt = 5
 * @return XPrice 
 */
extern XPrice MA(XKLineT data[], int len, int curIdx, int calCnt);

/**
 * 标准差
 */
extern XPrice SD(XKLineT data[], int len, int curIdx, int calCnt, XPrice maPx);

/**
 * @brief 能量潮指标（On Balance Volume，OBV）MA
 * 
 * @param data 
 * @param len 
 * @param curIdx 
 * @param calCnt 
 * @return XSumQty 
 */
extern XSumQty OBV_MA(XKLineT data[],int len, int curIdx, int calCnt);

/**
 * @brief 能量潮指标（On Balance Volume，OBV）
 * 
 * @param data 
 * @param len 
 * @param curIdx 
 * @return XSumQty 
 */
extern XSumQty OBV(XKLineT data[],int len, int curIdx);
/**
 * @brief 均幅指标（ATR）是取一定时间周期内的股价波动幅度的移动平均值，主要用于研判买卖时机
 * 较高的ATR值常发生在市场底部，并伴随恐慌性抛盘。当其值较低时，则往往发生在合并以后的市场顶部。
 * 根据这个指标来进行预测的原则可以表达为：该指标价值越高，趋势改变的可能性就越高；该指标的价值越低，趋势的移动性就越弱
 * 短期ATR一般大于长期的ATR,我们可以通过短期和长期的波动性的差值,并根据相应的价格方向入场,同时在趋势比较稳定的时候平仓出局
 * 海龟交易法则按照价格高于初始价格0.5ATR进行加仓操作,按照价格低于建仓价2ATR进行止损操作
 * @param data 
 * @param len 
 * @param curIdx 
 * @return XPrice
 * @note 均幅指标一般不单独使用，应结合其他指标综合研判 
 */
extern XPrice ATR(XKLineT data[], int len, int curIdx);

/**
 * @brief 简易波动指标
 * 量价合成指标，帮助我们判别市场情绪，掌控股价变动的技术分析工具，通过将价格与成交量的变化结合，计算出一个波动指标来反映股价或指数的变动状况
 * 如果较少的成交量能推动股价上涨，则EMV值升高；如果股价下跌时，仅伴随有较少的成交量，则EMV降低。如果股价不涨不跌或者股价上涨和下跌的时候都伴随
 * 着较大的成交量，则EMV的值趋近于0
 * @param data 
 * @param len 
 * @param curIdx 
 * @return XDouble 
 */
extern XDouble EMV(XKLineT data[], int len, int curIdx);

extern XDouble MAEMV(XKLineT data[], int len, int curIdx, int calCnt);

/**
 * @brief 涨跌幅
 * 
 * @param tradePx 
 * @param openPx 
 * @return XRatio 
 */
extern XRatio ChangeRatio(XPrice tradePx, XPrice openPx);

/**
 * @brief 涨跌
 * 
 * @param tradePx 
 * @param openPx 
 * @return XPrice 
 */
extern XPrice Change(XPrice tradePx, XPrice openPx);

/**
 * @brief RSI
 * 
 * @param data 
 * @param len 
 * @param openPx 
 * @return XRatio 
 */
extern XRatio RSI(XKLineT data[], int len, int curIdx, int calCnt);

/**
 * @brief 日内趋势指标，以(最新价-开盘) / (最高-最低)
 * *  计算潮汐指数
 *  潮汐指数（CMI）用于判断当前市场为震荡市/趋势市。
 *  潮汐指数（CMI） = abs(30 天的移动窗口中最后 1 天的收盘价 - 第 1 天的收盘价之差) ÷ 移动窗口的日内最高价的最高者与日内最低价的最低者之差 * 100。
 *  潮汐指数的取值范围在（0,100）之间。
 * 
 * @param tradePx 
 * @param openPx 
 * @param highPx 
 * @param lowPx 
 * @return XRatio 
 */
extern XRatio CMI(XPrice tradePx, XPrice openPx, XPrice highPx, XPrice lowPx);

/**
 * @brief 成交比
 * 
 * @param qty 
 * @param floatQty 
 * @return XRatio 
 */
extern XRatio TradeRatio(XSumQty qty, XSumQty floatQty);

/**
 * @brief 量比
 * 
 * @param qty 
 * @param kline 
 * @param len 
 * @param curIdx 
 * @param calCnt 
 * @return XRatio 
 */
extern XRatio QtyRatio(XSumQty qty, XKLineT kline[], XInt len, XInt curIdx, XInt calCnt);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_CORE_XPLOT_H_ */
