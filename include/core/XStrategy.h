
/**  
 * @file: XStrategy.h
 * @Description: TODO(描述)
 * @author kangyq
 * @date 2022-07-11 10:45:46 
 */  
#ifndef INCLUDE_CORE_XSTRATEGY_H_
#define INCLUDE_CORE_XSTRATEGY_H_

#include "XDataType.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief 策略类型
 * 
 */
typedef enum 
{
	eXMidRobPlot = 1,				/**< 盘中抢板 */
	eXHalfRobPlot = 2,              /**< 盘中半路 */
	eXT0Plot = 3,                   /**< T0策略 */
	eXGridPlot = 4,                  /**< 网格 */
	eXAuctionRobPlot = 5,          /**< 集合竞价抢板 */
	eXSellPlot = 6,			   		   /**< 卖出策略  */
	eXBondPlot = 7,                  /**< 可转债策略 */
	eXTest     = 9,                 /**< 测试策略 */
	eXEtfPlot = 10                  /**< ETF */

}XStrategyType;

/**
 * @brief 个股参数设置信息
 * 
 */
typedef struct 
{
	XSecurityId securityId; 								// 16

	XChar market;											// 4
	XChar bsType;											// 1
	XChar isnontrade;										// 1
	XChar _field;												
	XPrice conPx;											// 条件价格，达到条件价格买入

	XNum sign;                                             /**< 对策略的不同处理方式 */
	XRatio kpzf;											/**< 开盘涨幅 小于-2000 不控制 */
	XPrice ordPx;											/**< 委托价格 */
	XNum qtyType;											/** 委托数量类型 0数量1资金 */

	XNum buySlip;											/**< 买滑点 */
	XNum sellSlip;											/**< 卖滑点 */

	XMoney money;											/** 资金,qtyType为1资金时，值填入该字段 */

	XQty ordQty;											/**< 委托数量,实际委托数量,qtyType为1资金时,后端计算实际数量填入该字段 */
	XRatio cdl;												/**< 买一撤单率 */

	XSumQty askQty;											/**< 卖一未成交量 */

	XMoney buyMoney;										/**< 封单金额万元 */
	XMoney buyCtrlMoney;									/**< 买一金额不足多少万元时撤单 */

	XNum batchSellTimes;										/**< 卖出时时间批次 */

	XNum bigCtrlNums;										/**< 大单撤单且金额不足买入要求时 TODO */
	
	XPrice highPx;										/**< 买入当天最高价 */
	XPrice lowPx;										/**< 买入当天最低价 */

	//在涨停前以涨停价买入大单金额累计超过3000万,在涨停时直接买入
	//封单金额不足最大封单金额的0.1撤单
	//启动前已经涨停的不买
	//撤单率小于设置值且封单金额小于多少撤单
	//封涨停后下一分钟成交量大于1000万撤单
	//封涨停后下一个连续2分钟成交量大于800万撤单
	//沪市下单多少ms后封单金额不足撤单,深市下单多少ms后封单金额不足撤单
	//撤单后不开板封单金额增加多少
	//排撤次数

	XRatio upperQtyMulty;									/**< 涨停价卖出挂单量是昨天的倍数 */
	XRatio upperQtyMultyMin;								/**< 涨停价卖出挂单量比前15-60分钟挂单量增加的倍数 */

	XMoney nxtCtrlMoney;									/**< 封板后下一分钟成交金额 */
	XMoney followCtrlMoney;                                 /**< 封板后任意连续2分钟成交金额 */
	XPlotId plotid;											/**< 策略编号,修改时填入 */										
}XSettingT,*pXSettingT;

/**
 * @brief 篮子参数设置信息
 * 
 */
typedef struct 
{
	XCustomer customerId;                   				/**< 客户号 */
	
	XNum frontId;											/**< 前端请求编号,每次添加或修改必须自增 */
	XNum isForbid;											/**< 0:执行,1:停止 */
	
	XNum plotType;											/**< 策略类型 */

	XNum isCtrlStop;										/**< 撤单后是否停止报单 */
	
	XNum isUpperStop;										/**< 涨停打开后不买 */
	
	XNum isOpenStop;										/**< 开盘涨停继续买 */

	XSumQty allowHoldQty;									/**< 允许最大持仓 */
	
	XLongTime ordGapTime;									/**< 委托间隔 */
	XLongTime ctrGapTime;									/**< 撤单间隔时间 */
	
	XShortTime beginTime;									/**< 委托开始时间HHMMSSsss */
	XShortTime endTime;										/**< 结束交易时间HHMMSSsss */

	//TODO 增加滑点价格

	XNum slipBuyTimes;										/**< 买允许的滑点次数,默认0 */
	XNum slipSellTimes;										/**< 卖允许的滑点次数,默认0 */
	XNum userInfo;											/**< 用户自定义字段 */
	XChar baskfile[128];									/**< 篮子文件地址 */
	XNum isAutoCtrl;										/**< 是否允许撤单 */
	XRatio ctrlUpRatio;										/**< 每次撤单后封单金额增长比例,万分比 */
}XPlotT;

/**
 * @brief 策略请求
 * 修改策略，只允许修改状态，价格、数量、开始结束时间、委托间隔
 */
typedef struct 
{
	XPlotT plot;
	XSettingT setting;
}XStrategyReqT;


#define MAX_GRID_LEVEL                 (5)


/**
 * @brief 策略信息数据
 * 
 */
typedef struct 
{
	XIdx idx;								/**< 编号 */

	XPlotT    plot;							/**< plot.conf参数 */			
	XSettingT setting;						/**< 篮子参数 */

	XInvestId investId;						/**< 还未确认怎么添加到策略 */

	XPlotId plotid;							/**< 策略编号 */

	XChar acctType;           				/**< 1:现货;2:两融;3期权;4:期货;5:黄金 4 */
	XChar status;							/**< 策略执行状态 @see eXPlotStatus,通过策略请求中的isforbid修改策略状态 */
	XBool _bFirst;                      	/**< 初次买入，后续补再买入 */
	XBool _hasCtrl;							/**< 达到1分钟条件已经撤单过 */
	XNum _signal;							/**< 是否触发报单 */
	

	XNum sessionId;							/**< 会话编号 */
	XNum envno;								/**< 先放着，后续想怎么维护方便 */
	
	XSeqNum version;						/**< 行情版本号 */
	XSeqNum kversion;						/**< K线序列号 */
	
	/** 策略进程修改数据 */
	/** 可发送订单 = 委托总数量 - 已发送数量(sends) + 响应数量(rnt) + 撤单(废单)数量(invalid) - 成交(trades) */

	XLocalId buyList[MAX_GRID_LEVEL];   	/**< 买本地订单号 */
	XLocalId sellList[MAX_GRID_LEVEL];  	/**< 卖本地订单号 */

	XLongTime _buyLocTime;					/**< 最近一次下单时间 */
	XLongTime _sellLocTime;					/**< 最近一次下单时间 */
	XLongTime _ctrLocTime;					/**< 撤单时间，两次撤单的间隔不能太短 */

	XPrice _lastBuyPx;						/**< 最近一次买价格 */
	XPrice _lastSellPx;						/**< 最近一次卖价格 */
	XPrice _folwPx;							/**< 买入之后的最高 */
	XRatio _cdl;								/**< 当前撤单 */
	XMoney _buyCtrlMoney;					/**< 达到涨停条件时最大的买一金额 */

	XSumQty _buySends;						/**< 买已经发送但未给响应的 */
	XSumQty _sellSends;						/**< 卖已经发送但未给响应的 */

	/** 回报处理时更新成交数据 */
	XSumQty _buyTrades;						/**< 买成交 */
	XSumQty _sellTrades;					/**< 卖成交 */
	XSumQty _buyRtn;						/**< 买响应 */
	XSumQty _sellRtn;						/**< 卖响应 */
	XSumQty _buyValid;						/**< 买有效数量 */
	XSumQty _sellValid;						/**< 卖有效数量 */
	
	XNum _slipBuyTimes;						/**< 买允许的滑点次数,默认0 */
	XNum _slipSellTimes;					/**< 卖允许的滑点次数,默认0 */

    XRatio zdf1;                             /**< 9:40涨幅 */
    XRatio zdf2;                            /**< 9:50涨幅 */

    XMoney __buyTrdAmt;						/**< 回测时用 */
    XMoney __sellTrdAmt;                     /**< 回测时用 */
    XNum buyCntIdx;
    XNum sellCntIdx;
    XLongTime cfmTime;                     /**< 确认时间 */
    XLongTime _calTime;
    XNum   errorno;						   /**< 错误码 */
    XNum updateTime;                        /**< 买入时行情的时间 */

    //处理订单发送与撤单
    XNum ordPos;                           /**< 是否已经发出订单 TODO */
    XNum ordCtrPos;						   /**< 是否已经撤单 */
    XNum curPos;						   /**< 当前报单位置 */
    XNum __field1;

    XLong __field2[6];

} XStrategyT;

#define XSTRATEGY_SIZE                               (sizeof(XStrategyT))

typedef struct
{
	XInt train_size;				/**< 训练集数量 */
	XInt epoch;
	XInt factornum;					/**< 因子数量 */
	XDouble rate;					/**< 比例 */
}XSgdHeadT;

typedef struct 
{
	XChar label[24];
	XDouble weight;
}XSgdLabelT;

#define SGD_FACTOR_SIZE          (50)
typedef struct 
{
	XNum size;						/**< 实际训练集数量 */
	XSgdLabelT label[SGD_FACTOR_SIZE];
}XSgdDataHeadT;

#ifdef __cplusplus
}
#endif
#endif /* INCLUDE_CORE_XSTRATEGY_H_ */
