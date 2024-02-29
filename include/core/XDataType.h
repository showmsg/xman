/**
 * @file XDataType.h
 * @since 2022/3/15
 */

#ifndef INCLUDE_XMAN_XDATATYPE_H_
#define INCLUDE_XMAN_XDATATYPE_H_

#include "XBase.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * 用户类型
 */
typedef enum {
	eXUserTrade = 1, 				/**< 单交易 */
	eXUserMarket = 2, 				/**< 单行情 */
	eXUserBoth = 3					/**< 交易和行情 */
} eXUserType;

/**
 * 柜台
 */
typedef enum  {
	eXCounterOes = 1, 					/**< OES */
	eXCounterCtp = 2, 					/**< CTP */
	eXCounterXtp = 3					/**< XTP */
} eXCounter;

/**
 * 交易所
 */
typedef enum  {
	eXExchSse = 1, 						/**< 上交所 */
	eXExchSze = 2,						/**< 深交所 */
	eXExchSec = 3,                      /**< 沪深交易所 */
	eXExchCcef = 4,						/**< 中金所 */
} eXExchange;

/**
 * 市场
 */
typedef enum  {
	exMarketShSz = 0,                  /**< 沪深交易所 */
	eXMarketSha = 1,                   /**< 上海A */
	eXMarketSza = 2                    /**< 深圳A */
} eXMarket;

/**
 * 订阅方式，为保持和上游一致
 */
typedef enum 
{
	exReset = 0,						/**< 初始订阅 */
	exAppend = 1, 						/**< 追加订阅 */
	exDelete = 2						/**< 删除订阅 */
}eXSubType;

/**
 * 买卖方向
 */
typedef enum 	{
	eXBuy = 1,						/**< 买 */
	eXSell = 2, 					/**< 卖 */
	eXCBuy = 3,     				/**< 融资买 */
	eXCSell = 4,   					/**< 融券卖 */
	eXDeem = 5,						/**< 申购 */
	eXRedeem = 6,					/**< 赎回 */
	/** 行情专属字段 */
	eXMktBsG = 'G',					/**< 'G' - 借入 */
	eXMktBsF = 'F'                	/**< 'F' - 出借 */
} eXBsType;

/**
 * 组合开平标志
 */
typedef enum {
	eXCombOpen = 0,              	/**< 开仓，对于证券买为开仓 */
	eXCombClose = 1,             	/**< 平仓，对于证券卖为平仓 */
	eXCombCloseToday = 2,		   	/**< 平今仓 */
	eXCombCloseYesterday = 3     	/**< 平昨仓 */
}eXCombOffsetFlag;

/**
 * 投资者账户类型
 */
typedef enum  {
	eXInvSpot = 1,            /**< 现货 */
	eXInvCrd = 2,              /**< 两融 */
	eXInvOpt = 3,             /**< 期权 */
	eXInvFtr = 4,             /**< 期货 */
} eXInvType;

/**
 * 市场状态
 */
typedef enum  {
	eXMktPreOpen = 1,               /**< 开始前 */
	eXMktCallAuct = 2,              /**< 集合竞价 */
	eXMktPause = 3,                 /**< 休市 */
	eXMktConAuct = 4,               /**< 连续竞价 */
	eXMktClosing = 5,               /**< 15收盘 */
	eXMktClosed = 6                /**< 闭市 15:30 */
} eXMktStatus;

/**
 * 证券状态
 */
typedef enum  {
	eXSecDefault = 0, 			 /**< 默认 */
	eXSecTrading = 1,              /**< 交易 */
	eXSecPause = 2,                /**< 全天停牌 */
	eXSecTmpPause = 3,             /**< 临时停牌 */
	eXSecFirstTrading = 4,         /**< 上市首日 */
	eXSecST = 5,                   /**< ST, *ST */
	eXSecDiv = 6,                  /**< 除权除息 */
	eXSecDList = 7,                 /**< 退市 */
	eXSecNewList = 8                /**< 上市前期 */
} eXSecStatus;

/**
 * 产品类型
 */
typedef enum  {
	eXEquity = 0,                  /**< 普通股票、债券、基金、科创板、存托凭证买卖 */
	eXBondPR = 2,                  /**< 债券质押式回购 */
	eXIPO = 3,                     /**< 新股认购 */
	eXAllot = 4,               		/**< 配股 */
	eXOption = 5,                  /**< 期权 */
} eXPrdType;

/**
 * 证券类型
 */
typedef enum  {
	eXStock = 1,                   /**< 股票 */
	eXBond = 2,                    /**< 债券 */
	eXFund = 3,                    /**< 基金(不包括Etf) */
	eXETF = 4,                     /**< ETF */
	eXOpt = 5                      /**< 期权 */
} eXSecType;

/**
 * 证券子类型
 */
typedef enum  {
	eXSubDefault = 0,				/**< 默认类别 */
	eXSubSecASH = 1,             /**< A 股票 */
	eXSubSecSME = 2,             /**<  中小板 */
	eXSubSecGEM = 3,             /**< 创业板 */
	eXSubSecKSH = 4,             /**< 科创板 */
	eXSubSecKCDR = 5,            /**< 科创板CDR */
	eXSubSecCDR = 6,             /**< 存托凭证 */
	eXSubSecHLT = 7,             /**< 沪伦通 */

	eXSubSecGBF = 10,            /**< 国债 */
	eXSubSecCBF = 11,            /**< 企业债 */
	eXSubSecCPF = 12,            /**< 公司债 */
	eXSubSecCCF = 13,            /**< 可转换债 */
	eXSubSecFBF = 14,            /**< 金融机构发行债券 */
	eXSubSecPRP = 15,            /**< 债券质押式回购 */

	eXSubSecSNGLETF = 20,       /**< 单市场ETF */
	eXSubSecCRSETF = 21,             /**< 跨市场ETF */
	eXSubSecBNDETF = 22,             /**< 实物债券ETF */
	eXSubSecCRYETF = 23,             /**< 货币ETF */
	eXSubSecCRFETF = 24,             /**< 跨境ETF */
	eXSubSecGLDETF = 25,             /**< 黄金ETF */
	eXSubSecPRDETF = 26,             /**< 商品ETF */

	eXSubSecLOF = 30,            /**< LOF基金 */
	eXSubSecCEF = 31,                 /**< 封闭式基金 */
	eXSubSecOEF = 32,                 /**< 开放式基金 */
	eXSubSecGRD = 33,                 /**< 分级基金 */

	eXSubSecETFOPT = 40,        /**< ETF期权 */
	eXSubSecSTKOPT = 41             /**< 个股期权 */
} eXSubSecType;

/**
 * 订单类型
 */
typedef enum  {
	eXOrdLimit = 1,              /**< 限价 */
	eXOrdBest5 = 2,              /**< 最优5档即时成交剩余转限价 */
	eXOrdBest = 3,               /**< 对手方最优，仅科创板 */
	eXOrdBestParty = 4,          /**< 本方最优 ，仅科创板 */
	eXOrdBest5FAK = 5,           /**< 最优五档即时成交剩余撤销 */
	eXOrdLmtFOK = 6,             /**< 限价FOK,仅期权 */
	eXOrdMktFOK = 7,             /**< 市价FOK */
	eXOrdFAK = 8,                /**< 即时成交剩余撤销 */

	/*  以下为行情专属字段*/
	eXOrdMkt = 10,               /**< 行情中市价 */
	eXOrdSelf = 11,               /**< 本方最优 */

	eXOrdAdd = 20,                /**< 对于上海行情为添加 */
	eXOrdDel = 21                 /**< 对于上海行情为删除 */
} eXOrdType;

/**
 * 订单状态
 */
typedef enum  {
	eXOrdStatusDefalut = 0, 	  /**< 默认 */
	eXOrdStatusNew = 1,         /**< 新订单 */
	eXOrdStatusDeclared = 2,    /**< 已确认 */
	eXOrdStatusPFilled = 3,     /**< 部分成交 */
	eXOrdStatusFilled = 4,      /**< 全部成交 */
	eXOrdStatusPCanceled = 5,   /**< 部分成交部分撤单 */
	eXOrdStatusCanceled = 6,    /**< 已撤 */
	eXOrdStatusInvalid = 7      /**< 废单 */
} eXOrdStatus;

/**
 * 订单执行状态
 */
typedef enum  {
	eXExecNone = 0,      /**< 未执行 */
	eXExecRisk = 2,      /**< 通过风控检查 */
	eXExec = 3,           /**< 已执行 */
	eXExecFailure = 9         /**< 发送失败 */
} eXExecStatus;

/**
 * 发行方式
 */
typedef enum  {
	eXIssQuota = 1,             /**< 额度 新股 */
	eXIssCash = 2,              /**< 资金 公开增发类 */
	eXIssCredit = 3,            /**< 信用 新债 */
	eXIssAllot = 4              /**< 配股配债,即要检查持仓,又要检查资金 */
} eXIssType;

/**
 * 策略状态
 */
typedef enum  {
	eXPlotInit = 0,          		/**< 未初始化 */
	eXPlotNormal = 1,             	/**< 正常 */
	eXPlotPause = 2,              	/**< 暂停，可以再继续交易 */
	eXPlotStop = 3,              	/**< 停止,该状态在进行策略平仓操作 */
	eXPlotSuspend = 9,             /**< 终止 */
	eXPlotResume = 20,             /**< 重新开始，趋势策略为重新计算均价 */
} eXPlotStatus;

/**
 * 延迟情况
 */
typedef enum  {
	eXDelayNone = 0,      		/**< 不延迟 */
	eXDelayBuy = (1 << 0),       /**< 卖 */
	eXDelaySell = (1 << 1),      /**< 卖 */
	eXDelayBoth = (1 << 2),   /**< 买卖 */

} eXDelay;

/**
 * 禁止买卖标志
 */
typedef enum  {
	eXBlockNone = 0,     /**< 允许买卖 */
	eXBlockSell = 1,     /**< 禁止卖 */
	eXBlockBuy = 2,      /**< 禁止买 */
	eXBlockBoth = 3,     /**< 禁止买卖 */
} eXBlockType;

/**
 *	缓存数据类型
 */
typedef enum  {
	eDCust = 0,			 	/**< 客户数据 */
	eDStock = 1,			 	/**< 证券信息数据 */
	eDInvest = 2,		 		/**< 投资者数据 */
	eDCash = 3,			 	/**< 资金数据 */
	eDHold = 4,			 	/**< 持仓数据 */
	eDIssue = 5,				/**< 发行数据 */
	eDOrder = 7,				/**< 委托 */
	eDTrade = 8,				/**< 成交 */
	eDStrategy = 9,           /**< 策略 */

	eDOrderReq = 6,        	/**< 订单请求 */

	eMTickOrder = 10,		 	/**< 逐笔委托 */
	eMTickTrade = 11,		 	/**< 逐笔成交 */
	eMSnapshot = 12,		 	/**< 快照 */
	eMStatus = 13,		 		/**< 行情状态 */
	eMOrderBook = 14,       	/**< 订单薄 */
	eMTick = 15,				/**< 收到的逐笔数据 */
	eMBlock = 16,              /**< 板块数据 */
	eMBlockInfo = 17,          /**< 板块信息数据 */

	eSPlot = 20,				/**< 策略 */
	eSBlkSec = 21,    			/**< 黑名单证券 */
	eSSession = 22,			/**< 会话管理 */

	eTTdMon = 30,      		/**< 交易监控 */
	eTMdMon = 31,      		/**< 行情监控 */
	eTCounter = 32,			/**< 计数器 */
	eTPrsApp = 33,          /**< 进程计数 */
	
	eBHold = 40,            /**< 回测持仓计数 */

} eXDataType;

/**
 * 逐笔成交(深圳：撤单or成交)
 */
typedef enum  {
	eXL2ExecCancel = 4,     /**< 撤销 '4' */
	eXL2ExecTrade = 9       /**< 成交 'F' */
} eXL2ExecType;


typedef enum 
{
	eXL2DriveBid = 1,     /**< 主动买单 */
	eXL2DriveAsk = 2,     /**< 主动卖单 */
	eXL2DriveUnknown = 0  /**< 未知 */
}eXDriveType;

#define XSHMKEYCONECT(key)    XSHM_##key##_IDX

typedef enum 
{
	XSHMKEYCONECT(monitor)  = 0,
	XSHMKEYCONECT(app)  = 1,
	XSHMKEYCONECT(customer) = 2,
	XSHMKEYCONECT(customerHash) = 3,
	XSHMKEYCONECT(mktCache) = 4,
	XSHMKEYCONECT(mktSubscribe),
	XSHMKEYCONECT(mktSubList),
	XSHMKEYCONECT(snapshot),
	XSHMKEYCONECT(snapHash),
	XSHMKEYCONECT(stock),
	XSHMKEYCONECT(stockHash),
	XSHMKEYCONECT(issue) ,
	XSHMKEYCONECT(issueHash) ,
	XSHMKEYCONECT(orderBook) ,
	XSHMKEYCONECT(bookHash) ,
	XSHMKEYCONECT(priceSh) ,
	XSHMKEYCONECT(priceSz) ,
	XSHMKEYCONECT(pHash0) ,
	XSHMKEYCONECT(pHash1) ,
	XSHMKEYCONECT(pHash2),
	XSHMKEYCONECT(pHash3),
	XSHMKEYCONECT(CHNEL_DEFAULT),
	XSHMKEYCONECT(CHNNEL_2011) ,
	XSHMKEYCONECT(CHNNEL_2012) ,
	XSHMKEYCONECT(CHNNEL_2013) ,
	XSHMKEYCONECT(CHNNEL_2014) ,
	XSHMKEYCONECT(CHNNEL_2021) ,
	XSHMKEYCONECT(CHNNEL_2022) ,
	XSHMKEYCONECT(CHNNEL_2023)  ,
	XSHMKEYCONECT(CHNNEL_2024),
	XSHMKEYCONECT(CHNNEL_2031) ,
	XSHMKEYCONECT(CHNNEL_2032) ,
	XSHMKEYCONECT(CHNNEL_2033) ,
	XSHMKEYCONECT(CHNNEL_2034),
	XSHMKEYCONECT(CHNNEL_2061),
	XSHMKEYCONECT(CHNNEL_2071),
	XSHMKEYCONECT(CHNNEL_1),
	XSHMKEYCONECT(CHNNEL_2),
	XSHMKEYCONECT(CHNNEL_3),
	XSHMKEYCONECT(CHNNEL_4),
	XSHMKEYCONECT(CHNNEL_5),
	XSHMKEYCONECT(CHNNEL_6),
	XSHMKEYCONECT(CHNNEL_20),
	XSHMKEYCONECT(CHNNEL_801),
	XSHMKEYCONECT(resnapshot),
	XSHMKEYCONECT(reSnapCache),
	XSHMKEYCONECT(chnlHash) ,
	XSHMKEYCONECT(tradeCache),
	XSHMKEYCONECT(invest) ,
	XSHMKEYCONECT(investHash) ,
	XSHMKEYCONECT(investAcctHash) ,
	XSHMKEYCONECT(cash) ,
	XSHMKEYCONECT(cashHash) ,
	XSHMKEYCONECT(hold) ,
	XSHMKEYCONECT(holdHash),
	XSHMKEYCONECT(order) ,
	XSHMKEYCONECT(orderidHash) ,
	XSHMKEYCONECT(localidHash),
	XSHMKEYCONECT(trade) ,
	XSHMKEYCONECT(tradeHash) ,
	XSHMKEYCONECT(strategy) ,
	XSHMKEYCONECT(strategyHash),
	XSHMKEYCONECT(sessionHash),
	XSHMKEYCONECT(rtnCache) ,
	XSHMKEYCONECT(rspCache),
	XSHMKEYCONECT(strategyFrontHash),
	XSHMKEYCONECT(updateHash),
	XSHMKEYCONECT(klines),
	XSHMKEYCONECT(block),
	XSHMKEYCONECT(blockHash),
	XSHMKEYCONECT(blockinfo),
	XSHMKEYCONECT(blockInfoHash),
	
	XSHMKEYCONECT(backtestCash),
	XSHMKEYCONECT(backtestCashHash),
	XSHMKEYCONECT(backtestHold),
	XSHMKEYCONECT(backtestHoldHash),
	XSHMKEYCONECT(backtestOrder),
	XSHMKEYCONECT(backtestOrderHash),
	XSHM_KEY_MAX = 78
}eXSHMKEY;

#ifdef __cplusplus
}
#endif
#endif /* INCLUDE_XMAN_XDATATYPE_H_ */
