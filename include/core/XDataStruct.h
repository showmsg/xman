/**
 * @file XDataStruct.h
 * @version 0.10.0 2022/3/15
 * 			- 初始版本
 * @since 2022/3/15
 */

#ifndef INCLUDE_XMAN_XDATASTRUCT_H_
#define INCLUDE_XMAN_XDATASTRUCT_H_
#include "XDataType.h"
#include "XStrategy.h"

#ifdef __cplusplus
extern "C"
{
#endif
#pragma pack(push,1)


#define XSTRUCT_IDX_SIZE                               (sizeof(XIdx))

/**
 * @brief 授权信息
 */ 
typedef struct {
	XAccountId accountId;	/**< 资金账户 */
	XInt traday;			/**< 授权截止日期 */
}XLicenseT;

/**
 * @brief 证券黑名单信息数据
 */
typedef struct {
	XNum type;                       	/**<  策略类型 */
	XChar market;						/**<  市场  @see eXMarket */
	XChar _field[3];
	XSecurityId securityId;				/**<  证券代码 */
	XLong _field2[5];
} XMarketSecurityT, *pXMarketSecurityT;


#define XMARKETSEC_SIZE                       (sizeof(XMarketSecurityT))

/**
 * @brief 数据传输消息头
 */ 
typedef struct  {
	XNum type;							/**< 数据传递的类型 */
	XNum dataLen;						/**< 数据传递的长度 */
	XChar market;						/**< 市场 */
	XChar _field[7];
	XLong _field2[6];					/**< 填充64位对齐 */
} XTransHeadT, *pXTransHeadT;

/**********************************************************************************/

#define MAX_DEL_PRICELEVEL_CNT                   (65536)

/**
 * @brief 行情监控信息
 */
typedef struct  {
	XIdx idx;                                           	/**<  序号 */

	XTradeDay traday;                                  		/**<  交易日 */
	XShortTime updateTime;                      			/**<  当前交易所时间 */

	XIdx iMktSubCnt;                                        /**<  行情订阅数量,暂未使用 */
	XIdx iSnapshot;                                      	/**<  实际快照数（在静态数据中的数量） */
	XIdx iTick;												/**<  重建逐笔的数据  */

	XChar exchid;                                    		/**<  交易所  @see eXExchange */
	XChar status;                                  			/**<  市场状态 @see eXMktStatus */
	XChar isRunning;                                    	/**<  是否正常运行  */
	XChar _field;

	XShortTime _mktFirstTime;								/**< 第一笔行情收到的本地时间 */
	
	XLongTime _locFirstTime;								/**< 第一笔行情的本地时间 */
	
	XShortTime gaptime;								 		/**<  交易所与本地时间差 */
	/** 重构逐笔数据使用 */
	XNum usedSzPos;											/**<  最先用的删除位置  */
	XIdx delSzIdxs[MAX_DEL_PRICELEVEL_CNT];					/**< 保留价格档位的位置缓存 */
	XIdx delSzNums;											/**< 删除次数++ */
	XIdx maxSzPriceIdx;										/**<  最大的买位置  */

	/** 计算逐笔的数量 */
	XIdx totalSzOrders;										/**< 处理的逐笔委托条数 */
	XIdx totalSzTrades;										/**< 处理的逐笔成交条数 */

	XNum usedShPos;											/**<  最先用的删除位置  */
	XChar _field2[4];
	XIdx delShIdxs[MAX_DEL_PRICELEVEL_CNT];					/**< 保留价格档位的位置缓存 */
	XIdx delShNums;											/**< 删除次数++ */
	XIdx maxShPriceIdx;										/**<  最大的买位置  */
	
	/** 计算逐笔的数量 */
	XIdx totalShOrders;										/**< 处理的逐笔委托条数 */
	XIdx totalShTrades;										/**< 处理的逐笔成交条数 */
	XSeqNum shBiz;											/**< 上海biz */
	XSeqNum szBiz;											/**< 深圳biz */
	XNum shChannel;                                         /**< 上海频道 */
	XNum szChannel;									        /**< 深圳频道 */
	XNum uppCnt;                                            /**< 涨的家数 */
	XNum lowCnt;											/**< 跌的家数 */
	XNum eqlCnt;                                            /**< 平的家数 */
	XChar _field3[4];
	XLong _field4[2];                                       /**< 填充64位对齐 */
} XMonitorMdT, *pXMonitorMdT;

#define XMONITORMD_SIZE                                     (sizeof(XMonitorMdT))

/**
 * @brief 交易监控信息
*/
typedef struct  {
	XIdx idx;                                           	/**<  序号 */
	XIdx iHolding;            								/**<  持仓条数  */
	XIdx iOrder;              								/**<  系统中订单的数量 */
	XIdx iDealPos;          								/**<  已处理委托数量 */
	XIdx iInvest;             								/**<  投资者账户数量 */
	XIdx iCashAsset;          								/**<  资金账户数量 */
	XIdx iStockInfo;                        				/**<  静态数据数量,多账户情况如果已经存在不再查询  */
	XIdx iIssue;                                 			/**<  发行业务证券数量 */
	XIdx iTrade;              								/**<  成交的条数 */
	XTradeDay traday;                                  		/**<  交易日 */
	XChar status;                                         	/**<  市场状态 (目前只取闭市状态) */
	XChar isInitOK;   							    		/**<  是否初始化OK */
	XChar isAcceptOK;                                 		/**<  是否可以接收委托 */
	XChar isPurchased;       					    		/**<  是否已经做了申购 */
	XLocalId iMaxLocalId;                        			/**<  最大本地编号,如果本地编号比服务器存储大，以本地为主  */
	XNum envno;												/**< 环境号 */
	XLong _field[5];                                        /**< 填充64位对齐 */
} XMonitorTdT, *pXMonitorTdT;

#define XMONITORTD_SIZE                                    (sizeof(XMonitorTdT))

#define MAX_CUSTOMER_CNT                20
#define MAX_EXCHANGE_CNT                10

/**
 * @brief 交易+行情监控
 */
typedef struct  {
	XIdx iTOrder;                                    /**<  委托总数  */
	XIdx iTTrade;                                    /**<  成交总数  */
	XIdx iTCust;                                     /**<  客户数  */
	XIdx iTInvest;                                   /**<  股东客户数  */
	XIdx iTCash;                                	 /**<  资金账户数  */
	XIdx iTHold;                                     /**<  持仓记录数  */
	XIdx iTStock;                                    /**<  证券信息  */
	XIdx iTIssue;                                    /**<  发行数据 */
	XIdx iTPlot;                                     /**<  策略数量  */

	XIdx iTSnapshot;                                 /**<  快照总数量 */
	XIdx iTickCounter;								 /**<  逐笔数据计数器 TODO delete */
	XIdx iTOrderBook;                                /**< 重构后快照数量 */
	XIdx iTMdUser;                                   /**<  行情用户数 */
	XIdx iTTdUser;                                   /**<  交易用户数 */
	XIdx iTBlk;                                      /**<  策略股票黑白名单 TODO 回测时成交编号 */
	XIdx iCounter;										 /**< 计数器 TODO */
	XIdx iTPrsApp;										/**< 进程数 */
	XIdx iTSession;										/**< 会话总数 TODO */
	XIdx iTBlock;										/**< 板块数据 */
	XIdx iTBlockInfo;									/**< 板块信息数据 */
	XMonitorTdT monitorTd[MAX_CUSTOMER_CNT];        	 /**<  交易监控 */
	XMonitorMdT monitorMd[MAX_EXCHANGE_CNT];        	 /**<  行情监控 */

    XIdx iBHold;								      	/**< 回测持仓计数 */
    XLong _field[3];                                  /**< 填充64位对齐 */
} XMonitorT, *pXMonitorT;
/***************************************************************************************/

#define XMONITOR_SIZE                                     (sizeof(XMonitorT))

#define MAX_STOCK_CNT    30000

/**
 * @brief 静态数据信息
*/
typedef struct  {
	XIdx idx;                                            	/**<  ID */
	XChar market;                                      		/**<  市场 @see eXMarket */
	XChar isPriceLimit;                               		/**<  是否有涨跌幅限制  */
	XChar isDayTrading;                               		/**<  是否回转交易  */
	XChar secStatus;                  						/**<  是否可以交易 @see eXSecStatus */
	XChar hasExBond;										/**<  是否有对应可转债 @deprecated */
	XChar prdType;											/**<  产品类型 @see eXPrdType */
	XChar secType;                                      	/**<  证券类别 @see eXSecType  */
	XChar subSecType;                                 		/**<  证券子类别  @see eXSubSecType */

	XSecurityId securityId;                               	/**<  证券代码 */
	XSecurityName securityName;                           	/**<  证券名称 */
	XSecurityId baseSecurityId;                           	 /**<  基础证券代码 */

	XUnit buyUnit;                                        	 /**<  买基本单元 */
	XUnit sellUnit;                                       	 /**<  卖基本单元 */
	XQty mktBuyMaxQty;                                    	 /**<  市价最大买数量 */
	XQty mktBuyMinQty;										 /**<  市价最小买数量 */
	XQty mktSellMaxQty;										 /**<  市价最大卖数量 */
	XQty mktSellMinQty;										 /**<  市价最小卖数量 */
	XQty lmtBuyMaxQty;									     /**<  现价最大买数量 */
	XQty lmtBuyMinQty;										 /**<  现价最小买数量 */
	XQty lmtSellMaxQty;									     /**<  现价最大卖数量 */
	XQty lmtSellMinQty;										 /**<  现价最小卖数量 */
	XPrice preClose;										 /**<  前收盘价 */
	XUnit priceTick;                                  		/**<  价格tick  */
	XPrice upperPrice;          							/**<  连续竞价涨停价  */
	XPrice lowerPrice;           							/**<  连续竞价跌停价 */
	XSumQty outstandingShare;        						/**<  总股本 */
	XSumQty publicfloatShare;        						/**<  流通股本 */
	XPrice openUpperPrice;                              	/**<  集合竞价涨停价  */
	XPrice openLowerPrice;                               	/**<  集合竞价跌停价 */
	XTradeDay maturityDate;                 				/**<  可转债到期日  */
	XPrice convPx;											/**< 转股价值 */
	XSumQty ysVolumeTrade;									/**< 前一天成交量 */
	XBool ysMultiple;										/**< 昨天是否放量 */
	XChar _field[3];
	XInt upperTimes;										/**< 涨停天数 */

	XSumQty upperBidOrdQty;								/**< 累计涨停价买的数量，撤单扣减 */
	XNum upperBidOrdCnt;								/**< 累计涨停价买的次数，撤单扣减 */
	XSumQty lowerOfferOrdQty;							/**< 跌停价卖的数量,撤单扣减 */
	XNum lowerOfferOrdCnt;								/**< 跌停价卖的次数，撤单扣减 */
	XSumQty upperOfferOrdQty;							/**< 累计涨停价卖委托数量，撤单扣减 */
	XNum upperOfferOrdCnt;								/**< 累计涨停价卖委托次数，撤单扣减*/
	XSumQty lowerBidOrdQty;								/**< 累计跌停价买委托数量,撤单扣减*/
	XNum lowerBidOrdCnt;								/**< 累计跌停价买委托次数，撤单扣减*/
	XLong _field1[2];											/**< 64位对齐 */
} XStockT, *pXStockT;

#define XSTOCK_SIZE                                       (sizeof(XStockT))
/**
 *  @brief 发行
*/
typedef struct  {
	XIdx idx;                                      		/**<  ID */
	XChar market;             							/**<  市场   @see eXMarket */
	XChar prdType;         								/**<  产品类型 @see eXPrdType */
	XChar issueType;           							/**<  issueType  @see eXIssType */
	XChar secType;        								/**<  证券类型 @see eXSecType */
	XChar subSecType;             						/**<  子证券类型 @see eXSubSecType  */	
	XChar isCancel;            							/**<  是否允许撤单 */
	XChar isReapply;           							/**<  是否允许重复申购  */
	XChar _field1;

	XSecurityId securityId;     		           		/**<  发行代码 */
	XSecurityName securityName;     	           		/**<  证券名称 */

	XPrice issuePrice;          						/**<  发行价格 */
	XUnit qtyUnit;             							/**<  委托数量单位 4 */

	XQty minQty;              							/**<  最小申购数量 */
	XQty maxQty;              							/**<  最大申购数量 */
	XLong _field2[2];											/**< 64位对齐 */
} XIssueT, *pXIssueT;

#define XISSUE_SIZE                                         (sizeof(XIssueT))

/**
 * @brief 频道数据,用于判断频道是否丢单
 * 
 */
typedef struct 
{
	XNum channel;
	XChar _field[4];
	XSeqNum bizIndex;
	XSeqNum ordSeq;
	XSeqNum trdSeq;	
	XLong _field2[4];
}XChannelT, *pXChannelT;

#define SNAPSHOT_LEVEL                  10

/**
 * 价格档位
 */
typedef struct 
{
  XPrice price;                        /**< 价格 */
  XChar _field[4];
  XNum numOrders;                      /**< 委托笔数 */
  XShortTime updateTime;               /**< */
  XSumQty qty;                         /**< 委托数量 */
  XSumQty cqty;                        /**< 撤单的笔数 */
} XPriceEntryT;

/**
 * @brief 交易所快照基础数据
 * 
 */
typedef struct 
{
	XIdx idx;                                              		/**<  ID */

	XTradeDay traday;											/**<  交易日 */
	XChar market;                                          		/**<  市场  @see eXMarket */
	XBool isNoCloseAuction;										/**< 是否有收盘集合竞价 */
	XChar mrkStatus;                          	    			/**<  市场状态  @see eXMktStatus */
	XChar secStatus;           									/**<  证券状态 @see eXSecStatus */

	XSecurityId securityId;                          	    	/**<  证券代码  */

	XNum _channel;												/**< 频道号 */
	XPrice preClosePx;                                     		/**<  前收盘价 */

	XPrice upperPx;												/**< 涨停价 */
	XPrice lowerPx;												/**< 跌停价 */

	/** 直接从ticksnap拷贝 */
	XPrice openPx;                                         		/**<  开盘价 */
	XPrice highPx;                                         		/**<  最高价 */

	XPrice lowPx;                                          		/**<  最低价 */
	XPrice tradePx;                                        		/**<  最新价 */

	XNum numTrades;        										/**<  成交笔数 */
	XShortTime updateTime;                                     	/**<  更新时间 */

	XLongTime _recvTime;										/**<  落地行情时时间 */

	XSumQty volumeTrade;     									/**<  成交总量 */
	XMoney amountTrade;      									/**<  成交金额 */
	XSeqNum _bizIndex;											/**< 委托和成交连续编号,用于下单时跟踪 */

	XPrice driveBidPx;											/**< 主动买价(外盘) TODO delete */
	XPrice driveAskPx;											/**< 主动卖价(内盘) TODO delete */

	XPrice ask[SNAPSHOT_LEVEL];                 				/**<  卖价 */
	XSumQty askqty[SNAPSHOT_LEVEL];								/**<  卖量 */
	XPrice bid[SNAPSHOT_LEVEL];                  				/**<  买价 */
	XSumQty bidqty[SNAPSHOT_LEVEL];								/**<  买量 */
	XChar side;													/**< 外盘:B,内盘:S */
	XChar _field[7];
	XLong _field1[3];											/**< 64位对齐 */
}XSnapshotBaseT, *pXSnapshotBaseT;

#define XSNAPSHOT_BASE_SIZE                                   (sizeof(XSnapshotBaseT))

/**
 * @brief K线数据
 * 
 */
typedef struct 
{
	XPrice open;												/**< 开盘价 */
	XPrice high;												/**< 最高价 */
	XPrice low;													/**< 最低价 */
	XPrice close;												/**< 收盘价 */
	XShortTime updateTime;										/**< 最新行情 */
	XInt traday;
	XSumQty qty;												/**< 交易量 */
	XMoney  amt;												/**< 总成交额 */
	XSumQty    bigBuyOrdQty;                                    /**< 大单买数量 */
	XSumQty    bigSellOrdQty;									/**< 大单卖数量 */
	
	XSumQty    upperBuyOrdQty;									/**< 涨停价买入委托数量 */
	XSumQty    upperSellOrdQty;									/**< 涨停价卖出委托数量 */

	XNum numTrades;                                            /**< K线内成交笔数 */
	XBool bSave;											   /**< 保存后更改为true */
	XChar _field1[3];
	XSumQty scanBidOrdQty;										/**< 买扫单 */
	XSumQty scanOfferOrdQty;									/**< 卖扫单 */

	XLong _field[4];                                            /**< 填充64位对齐 */
}XKLineT, *pXKLineT;

typedef struct _XCalcData
{
	XMoney driverBuyTrdAmt;										/**< 主动买单金额  */
	XSumQty drverBuyTrdQty;										/**< 主动买单数量  */
	XMoney driverSellTrdAmt;									/**< 主动卖单金额 */
	XSumQty drverSellTrdQty;									/**< 主动卖单数量  */
	
	XNum bigBuyOrdCnt;											/**< 大单买入:数量10万股或200万元以上 */
	XNum bigSellOrdCnt;											/**< 大单卖出数量 */
	XMoney bigBuyOrdAmt;										/**< 大单买入金额*/
	XMoney bigSellOrdAmt;										/**< 大单卖出金额 */

	XMoney bigBuyTrdAmt;										/**< 大单买入成交金额 */
	XMoney bigSellTrdAmt;                                       /**< 大单卖出成交金额 */
	
	XNum totalBuyOrdCnt;										/**< 累计买次数 */
	XSumQty totalBuyOrdQty;										/**< 累计所有买单(去除撤单数量) */
	XMoney  totalBuyOrdAmt;										/**< 累计买单挂单金额 */
	XNum totalSellOrdCnt;										/**< 累计买次数 */				
	XSumQty totalSellOrdQty;									/**< 累计所有卖单(去除撤单数量) */
	XMoney totalSellOrdAmt;										/**< 累计所有卖单挂单金额 */
}XCalcDataT;


#define SUPERORDER_VOLUME                                   500000 //(5000手)

#define BIGORDER_MONEY                                      30000000000  //(300万)
#define BIGORDER_VOLUME                                     250000   //(2500手)

#define MIDORDER_VOLUME                                     100000  //(1000手)

#define SNAPSHOT_K1_CNT                                     (64)    /** 1分钟K线存储数量 */
#define SNAPSHOT_K5_CNT                                     (64)   /** 5分钟K线存储数量 */

/**
 * @brief 快照更新通知,使用通知机制异步触发策略执行
 * 
 */
typedef struct 
{
	XIdx idx;
	XChar market;
	XChar _field[7];
	XSecurityId securityId;
	XLong _field2[4];                                /**< 填充64位对齐 */
}XSnapshotNotifyT;

#define XSNAPSHOT_NOTIFY_SIZE               (sizeof(XSnapshotNotifyT))

/**
 * @brief 快照计算数据
 * 
 */
typedef struct  {
	XIdx idx;                                              		/**<  ID */

	XTradeDay traday;											/**<  交易日 */
	XChar market;                                          		/**<  市场  @see eXMarket */
	XBool isNoCloseAuction;										/**< 是否有收盘集合竞价 */
	XChar mrkStatus;                          	    			/**<  市场状态  @see eXMktStatus */
	XChar secStatus;           									/**<  证券状态 @see eXSecStatus */

	XSecurityId securityId;                          	    	/**<  证券代码  */

	XNum _channel;												/**< 频道号 */	
	XPrice preClosePx;                                     		/**<  前收盘价 */

	XPrice upperPx;												/**< 涨停价 */
	XPrice lowerPx;												/**< 跌停价 */

	/** 直接从ticksnap拷贝 */
	XPrice openPx;                                         		/**<  开盘价 */
	XPrice highPx;                                         		/**<  最高价 */

	XPrice lowPx;                                          		/**<  最低价 */
	XPrice tradePx;                                        		/**<  最新价 */

	XNum numTrades;        										/**<  成交笔数 */
	XShortTime updateTime;                                     	/**<  更新时间 */

	XLongTime _recvTime;										/**<  落地行情时时间 */

	XSumQty volumeTrade;     									/**<  成交总量 */
	XMoney amountTrade;      									/**<  成交金额 */
	XSeqNum _bizIndex;											/**< 深圳委托和成交连续编号,用于下单时跟踪 */

	XPrice driveBidPx;											/**< 主动买价(外盘) */
	XPrice driveAskPx;											/**< 主动卖价(内盘) */


	XPrice ask[SNAPSHOT_LEVEL];                 				/**<  卖价 */
	XSumQty askqty[SNAPSHOT_LEVEL];								/**<  卖量 */
	XPrice bid[SNAPSHOT_LEVEL];                  				/**<  买价 */
	XSumQty bidqty[SNAPSHOT_LEVEL];								/**<  买量 */
	
	XChar side;													/**< 外盘:B,内盘:S */
	XChar _field[7];
	XLong _field1[3];											/**< 64位对齐 */

	XSeqNum version;                                  			/**<  行情每更新一次记录 */
	
	/** 以下为计算字段 */
	XLongTime _genTime;											/**< 生成快照时间 */

	XSumQty bidcqty[SNAPSHOT_LEVEL];							/**< 撤单量 */
	XSumQty askcqty[SNAPSHOT_LEVEL];							/**< 撤单量 */

	XPrice vTradPx;												/**< 集合竞价虚拟成交价格 */
	XNum _storeCursor;											/**< 存储1分钟K线数据的位置 TODO DELETE */

	XNum kcursor1;												/**< 1分钟K线数据位置 */
	XNum kcursor5;												/**< 5分钟K线数据位置 */						

	XNum _upperTimes;												/**< 触发涨停的次数 */
	XNum _sealCursor;

	XShortTime _sealTime;											/**< 封板时间 */
	XShortTime _lastUpperTime;										/**< 最近一次触发涨停的时间 */

//	XSumQty _upBidQty;											/**< 以涨停价买的累计委托数量 */
//	XSumQty _upBidCQty;											/**< 以涨停价委托撤单的累计委托数量 */
	XSumQty _catchUpBidQty;										/**< 达到涨停后的累计委托量,打开涨停记0 */
	XSumQty _catchUpBidCQty;									/**< 达到涨停后的累计委托撤单量,打开涨停记0 */
	XSumQty _catchUpTrdQty;										/**< 触发涨停后成交量 */
	XInt _catchUpBidBuyCnt;										/**< 达到涨停大单买单次数 */
	XInt _catchUpBidCBuyCnt;									/**< 达到涨停大单买单撤单次数 */

	/**
	 * 内盘又称之为主动性卖出盘，表示投资者已买入价格成交累计的数量，用S表示；外盘就是主动性买入盘，表示的是以投资者卖出的价格成交累计的数量，用B表示
	 */

	XMoney outsideTrdAmt;										/**< 主动买单数量  */
	XMoney insideTrdAmt;									    /**< 主动卖单数量 */

	XSumQty auctionQty;											/**< 集合竞价成交量 */

	XNum bigBuyOrdCnt;											/**< 大单买入:数量10万股或200万元以上 */
	XNum bigSellOrdCnt;											/**< 大单卖出数量 */
	XMoney bigBuyOrdAmt;											/**< 大单买入金额*/
	XMoney bigSellOrdAmt;											/**< 大单卖出金额 */
    XSumQty bigBuyOrdQty;										/**< 大单买成交数量 */
    XSumQty bigSellOrdQty;										/**< 大单卖成交数量 */

    XMoney bigBuyTrdAmt;  										/**< 大单买入成交金额 */
    XMoney bigSellTrdAmt; 										/**< 大单卖出成交金额 */

    XNum totalBuyOrdCnt;     									/**< 累计买次数 */
    XSumQty totalBuyOrdQty;  									/**< 累计所有买单(去除撤单数量) */
    XMoney totalBuyOrdAmt;   									/**< 累计买单挂单金额 */
    XNum totalSellOrdCnt;    									/**< 累计买次数 */
    XSumQty totalSellOrdQty; 									/**< 累计所有卖单(去除撤单数量) */
    XMoney totalSellOrdAmt;  									/**< 累计所有卖单挂单金额 */

    XSumQty upperBidOrdQty; 									/**< 累计涨停价买的数量，撤单扣减 */
    XNum upperBidOrdCnt;    									/**< 累计涨停价买的次数，撤单扣减 */
    XSumQty lowerOfferOrdQty; 									/**< 跌停价卖的数量,撤单扣减 */
    XNum lowerOfferOrdCnt;    									/**< 跌停价卖的次数，撤单扣减 */
    XSumQty upperOfferOrdQty; 									/**< 累计涨停价卖委托数量，撤单扣减 */
    XNum upperOfferOrdCnt; 										/**< 累计涨停价卖委托次数，撤单扣减*/
    XSumQty lowerBidOrdQty; 									/**< 累计跌停价买委托数量,撤单扣减*/
    XNum lowerBidOrdCnt; 										/**< 累计跌停价买委托次数，撤单扣减*/

    XSumQty scanBidOrdQty;										 /**< 偏离主动买数量 */
    XSumQty scanOfferOrdQty;								     /**< 偏离主动卖次数 */

    XNum secUpBigBuyCnt;											/**< 在1秒内,最新价不是涨停价以涨停价超大单买的次数 */
    XNum secUpBuyTimes;                                            /**< 在1秒内，最新价不是涨停价以中单买的次数 */
    XNum secBuyTimes;											   /**< 在1秒内买的频次 */
    XNum lsecBuyTimes;                                             /**< 上一秒买的次数 */

    XLong _field3;                           					 /**< 填充64位对齐 */
} XSnapshotT, *pXSnapshotT;

/**
 * @brief 记录上次快照的更新时间
 * 
 */
typedef struct 
{
	XSeqNum version;
	XShortTime updateTime;
	XChar _field[4];
	XLong _field2[6];
}XUpdVerT,*pXUpdVerT;

#define XSNAPSHOT_SIZE                                        (sizeof(XSnapshotT))

typedef XSnapshotT XRSnapshotT;									/**< 重构后的快照，前期先保持与交易所快照相同 */

#define XRSNAPSHOT_SIZE                                       (sizeof(XRSnapshotT))

/**
 * @brief 重构的订单薄的价格档位
 */
typedef struct  {
	XIdx idx;					/**<  索引位置   */
	XIdx prev;					/***< 上一价格位置 */
	XIdx next;					/**< 下一价格位置 */
	struct level
	{
		XIdx forward;
		XPrice roomPx;
		XChar _field[4];
	}level[5];

	XPriceEntryT entry;            

	XLong _field2[7];              /**< 填充位 */
} XPriceLevelT, *pXPriceLevelT;

#define XPRICELEVEL_SIZE                                 (sizeof(XPriceLevelT))


/**
 * @brief 逐笔委托数据
 */
typedef struct  {

	/** ticksnap创建时首日时直接拷贝 */
	XTradeDay traday;											/**< 交易日YYYYMMDD */
	XChar market;                                          		/**<  市场  @see eXMarket */
	XChar _field[3];

	XSecurityId securityId;                          	    	/**<  证券代码 */

	XNum channel;												/**<  频道号 */
	XShortTime updateTime;										/**<  更新时间 HHMMSSsss*/

	XSeqNum seqno;												/**<  委托号 */
	XSeqNum bizIndex;											/**<  委托和成交连续编号 */


	XSeqNum ordSeq;												/**<  委托单独编号，只有上海除债券外行情 */

	/**
	 * 买卖方向
	 * - 深交所: '1'=买, '2'=卖, 'G'=借入, 'F'=出借
	 * - 上交所: '1'=买, '2'=卖
	 * @see eXBsType
	 */
	XChar bsType;

	XBool isCancel;									/**<  撤单由该标志标识 1为撤单 */
	/**
	 * 订单类型
	 * - 深交所: '1'=市价, '2'=限价, 'U'=本方最优
	 * - 上交所: 'A'=委托订单-增加(新订单), 'D'=委托订单-删除(撤单)
	 * @see eXOrdType
	 */
	XChar ordType;
	XBool _isCatchUpPx;									/**<  已促发涨停价  */
	XChar _field4[4];

	XPrice ordPx;										/**<  成交价格 */
	XQty ordQty;										/**<  订单数量 */

	/** 重构订单的数据处理 */
	XPrice _price;								/**<  如果为市价获取实际的挂档价格，否则同ordPx */
	XQty _leaveQty;								/**<  剩余数量  */

	XIdx _priceIdx;								/**<  价格链的位置,方便查找  */

	/** ticksnap后续更新时直接拷贝 */

	XLongTime _recvTime;								/**<  落地行情时时间 */
	XIdx idx;
	XMoney ordMoney;									/**< 委托资金,自己计算 */
    XLong _field5[2];                                  /**< 64位对齐 */

} XTickOrderT, *pXTickOrderT;

#define XTICKORDER_SIZE                                  (sizeof(XTickOrderT))

/**
 * @brief 逐笔成交
 */
typedef struct  {
	/** ticksnap创建时首日时直接拷贝 */
	XTradeDay traday;											/**< 交易日YYYYMMDD */
	XChar market;                                        	/**<  市场  @see eXMarket  */
	XBool isCancel;											/**<  撤单由该标志标识 1为撤单 */
	XChar trdType;											/**< 买 B，卖 S，其它N */
	XChar _field;

	XSecurityId securityId;                          	    /**<  证券代码  */

	XNum channel;											/**<  频道号 */
	XQty tradeQty;

	/** ticksnap更新时直接拷贝 */
	XPrice tradePx;											/**<  成交价格 */
	XShortTime updateTime;									/**<  更新时间 */

	XSeqNum bizIndex;										/**<  委托和成交连续编号 */
	XLongTime _recvTime;									/**<  落地行情时时间 */

	XSeqNum tradeSeq;										/**<  成交单独编号,只有上海除债券外行情 */
	XSeqNum bidSeq;										    /**<  买委托号 */
	XSeqNum askSeq;											/**<  卖委托号 */

	XMoney tradeMoney;										/**< 成交金额 */
	XLong _field2[5];										/**< 填充64位对齐 */
} XTickTradeT, *pXTickTradeT;

#define XTICKTRADE_SIZE                                    (sizeof(XTickTradeT))
/**
 * @brief 包含逐笔的缓存传输数据
 */
typedef struct  {
	XTransHeadT head;								/**<  通过消息头类型使用数据 */
//    XLong _field;
    union
        {
          XSnapshotBaseT snapshot; /**<  快照 */
          XTickOrderT order;       /**<  逐笔委托 */
          XTickTradeT trade;       /**<  逐笔成交 */
        };
} XL2LT, *pXL2LT;

#define XL2L_SIZE                                        (sizeof(XL2LT))

/**
 * @brief 重构订单薄
 */
typedef struct  {
	XIdx idx;                                          	/**<  ID */

	/** 以下为内部使用字段 */
	XIdx buyLevelIdx;									/**< 买价格档位，从大到小 */
	XIdx sellLevelIdx;									/**< 卖价格档位, 从小到大 */
	XSeqNum buySeqno;									/**< 最近的买成交序列号,深圳市场限价单处理使用 */
	XSeqNum sellSeqno;									/**< 最近的买成交序列号,深圳市场限价单处理使用 */
	XPrice bestBidPx;									/**< 最优买价 */
	XPrice bestAskPx;									/**< 最优卖价 */

	XBool bMktOrd;										/**< 是否市价委托 */
	XChar _field[7];
	XSeqNum _ordSeqno;
	XSnapshotT snapshot;
} XOrderBookT, *pXOrderBookT;

#define XORDERBOOK_SIZE                                  (sizeof(XOrderBookT))

/******************************************************************************************/
/**
 * @brief 账户数据单独管理
*/
typedef struct  {
	XIdx idx;                                   /**<  编号 */
	XCustomer customerId;                       /**<  客户号+柜台唯一 */

	XChar counter;                          	/**<  柜台 */
	XChar type;                        			/**<  交易:0x01(1),行情:0x10(2),交易和行情:0x11(3) */
	XChar exchid;                          		/**<  交易所  @see eXExchange */
	XChar _field;
	XBroker broker;                           	/**<  券商 */

	XPassword password;                        /**<  密码 */
	XHardware hd;                             	/**<  硬盘序列号 */
	XRemark remark;                          	/**<  备注 */
	XNum envno;									/**< 环境号 */
	XNum cpuid;									/**< 多账户时可以指定不同的核 */
	XChar ip[56];								/**< 交易地址 */
} XCustT, *pXCustT;

#define XCUST_SIZE                                (sizeof(XCustT))
/**
 * @brief 子账户具有独立的资金账户及持仓信息
 */
typedef struct  {
	XIdx idx;								/**<  子账户编号 */
	XIdx mainIdx;							/**<  主账户位置 */
	XPassword password;						/**<  子账户密码 */
} XSubCustT, *pXSubCustT;

/**
 * @brief 股东账户信息
*/
typedef struct  {
	XIdx idx;                                   /**<  编号 */
	XCustomer customerId;      					/**<  客户号 */
	XSeqNum seqno;                              /**<  客户号下面投资者账户编号,客户号+seqno唯一 */

	XChar market;               				/**<  市场  @see eXMarket */
	XChar acctType;             				/**<  1:现货;2:两融;3:期权,4:期货,5:黄金 */
	XChar isMain;				            	/**<  主辅账户，同一市场+账户类型，主账户唯一 */
	XChar _field[5];

	XInvestId investId;        					/**<  股东账户，同一客户投资者不能重复 */

	XQty mainQuota;            					/**<  新股权益 */
	XQty kcQuota;              					/**<  科创板权益 */
} XInvestT, *pXInvestT;

#define XINVEST_SIZE                                (sizeof(XInvestT))

/**
 * @brief 资金账户信息
*/
typedef struct  {
	XIdx idx;                                    	/**<  编号 */
	XCustomer customerId;    						/**<  客户号 */
	XSeqNum seqno;                             		/**<  资金账户编号，客户号+seqno唯一 */

	XChar acctType;           						/**<  0:现货;1:两融;2期权 */
	XChar isMain;                               	/**<  主辅账户，同一市场+账户类型，主账户唯一，辅账户可以有多个 */
	XChar _field[6];

	XAccountId accountId;     		           		/**<  资金账户 */
	XMoney beginBalance;      						/**<  日初资金 */
	XMoney beginAvailable;    						/**<  日初可用 */
	XMoney beginDrawable;     						/**<  日初可取 */
	XMoney balance;									/**<  资金余额 */
	XMoney frozenAmt;         						/**<  买冻结未成交资金 */
	XMoney curAvailable;         			    	/**<  当前可用 */
	XMoney totalBuy;                             	/**<  累计买费用 */
	XMoney totalSell;                            	/**<  累计卖费用 */
	XMoney totalFee;                             	/**<  累计交易费用 */
	XMoney locFrz;									/**< 本地资金冻结，委托返回后解冻 */
	XMoney countAvailable;							/**< 柜台可用 */

	XLong _field2[6];
} XCashT, *pXCashT;

#define XCASH_SIZE                                   (sizeof(XCashT))
/**
 * @brief 持仓账户信息
*/
typedef struct  {
	XIdx idx;                              			/**<  编号 */
	XCustomer customerId;    						/**<  客户号 */
	XInvestId investId;     						/**<  股东帐户 */

	XChar market;            						/**<  市场 @see eXMarket */
	XChar _field[7];

	XSecurityId securityId;    						/**<  产品代码 */
	XSumQty orgHld;      							/**<  日初持仓 */
	XSumQty orgAvlHld;   							/**<  日初可用 */
	XMoney orgCostAmt; 								/**<  日初持仓成本 */
	XSumQty totalBuyHld;       						/**<  累计买 */
	XSumQty totalSellHld;      						/**<  累计卖 */
	XMoney totalBuyAmt;                      		/**<  累计买金额 */
	XMoney totalSellAmt;                     		/**<  累计卖金额 */
	XSumQty sellFrz;        						/**<  卖冻结 */
	XSumQty locFrz;									/**<  记录本地冻结持仓，系统重启时为0;发单时在Order记录冻结持仓数量,收到确认和拒绝时解冻 */
	XSumQty sumHld;                           		/**<  总持仓 */
	XSumQty sellAvlHld;     						/**<  可卖持仓 */
	XSumQty countSellAvlHld;						/**<  柜台可用持仓 */
	XSumQty etfAvlHld;      						/**<  可用于etf申购的持仓 */

	XPrice costPrice;      							/**<  持仓成本价 */
	XChar _field2[4];

	XLong _field3[2];                              /**< 填充64位对齐 */
} XHoldT, *pXHoldT;

#define XHOLD_SIZE                                   (sizeof(XHoldT))

/**
 * @brief 订单请求
 */
typedef struct  {
	XIdx reqId;										/**< 请求编号，订单与请求对应 */

	XNum sessionId;									/**< 会话ID */
	XLocalId frontId;								/**< 前端编号，同一个会话唯一*/
	XCustomer customerId;     						/**<  客户号 16 */
	XInvestId investId;        						/**<  股东账户如果未指定就自动获取第一个 10 */
	XPlotId plotid;                               	/**<  策略编号,前端生成 */

	XNum plotType;									/**<  策略类型 */
	XChar acctType;                          		/**<  账户类型:现货,两融,期权 @see eXInvType */
	XChar market;            						/**<  市场 @see eXMarket */
	XChar _field[2];

	XSecurityId securityId;      					/**<  证券代码  */

	XLocalId localId;         						/**<  客户委托流水号  */
	XLocalId clocalId;         						/**<  客户撤单委托流水号  */

	XChar bsType;               					/**<  买卖类型, 1:买;2:卖出  @see eXBsType */
	XBool isCancel;									/**<  撤单由该标志标识 1为撤单 */
	XChar ordType;              					/**<  订单类型,市价or限价  @see eXOrdType */
	XBool nonGenLocId;								/**< 是否生成locId,针对抢单用户 */
	XChar _field2[4];

	XQty ordQty;               						/**<  委托数量  */
	XPrice ordPrice;             					/**<  委托价格  */

	XNum orgEnvno;            						/**<  环境号  */
	XLocalId orgLocalId;     						/**<  撤单时原始订单编号  */

	XSystemId orgOrdId;     						/**<  撤单时原始订单编号  */

	XPrice _lastPx;									/**<  下单时带入行情价格，后续进行比对 */
	XShortTime _lastTime;							/**<  下单时带入行情最新时间，后续进行比较 */
	XSeqNum _bizIndex;								/**< 下单时带入逐笔中委托和成交编号,用于跟踪 */
	XLongTime _mktTime;								/**< 该笔行情本地时间  */

	XLong _field3[6];                              /**< 填充64位对齐 */
} XOrderReqT, *pXOrderReqT;

#define XORDERREQ_SIZE                               (sizeof(XOrderReqT))
/**
 * @brief 委托信息
*/
typedef struct  {
	XIdx idx;                                  		/**<   Id */
	XSystemId ordid;                				/**<  柜台订单编号 */
	XExchId exchordId;       						/**<  交易所订单编号  */
	XMoney frzAmt;               					/**<  冻结金额  */
	XMoney frzFee;               					/**<  冻结费用 */

	XQty locFrzHold;								/**<  冻结持仓，发单时记录，收到确认和拒绝时解冻，同时根据此数量更新持仓中的locFrz 1 */
	XQty trdQty;               						/**<  成交数量  */

	XMoney locFrzMoney;								/**< 冻结资金，发单时记录，收到确认和拒绝时解冻，同时根据此更新资金中locFrz 1  */
	XMoney trdMoney;								/**<  成交金额 */

	XChar productType;              				/**<  产品类型 @see eXPrdType 1 */
	XChar ordStatus;            					/**<  订单状态 @see eXOrdStatus */
	XChar exeStatus;                         		/**<  撤单的时候，如果因某系统未交易失败需要继续撤单 @see eXExecStatus 1 */
	XChar counter;                           		/**<  对应使用哪家柜台;在适配器里面写死  0 */
	XBool _locFlag;									/**< 标识是本地发送的  0 */
	XBool _nonDeal;									/**< 未处理的标志 0 */
	XBool _isSendCtrl;								/**< 是否发送撤单 */
	XChar _field1;

	XLongTime _riskLocTime;							/**< 本地风控校验完毕时间 */
	XLongTime _sendLocTime;                          /**<  本地发送时间(本地) 0 */
	XLongTime _sendTime;            				/**<  委托时间(柜台)  */
	XLongTime _cnfLocTime;          				/**<  柜台确认时间(本地) 0 */
	XLongTime _cnfTime;           					/**<  确认时间(柜台)  */
	XLongTime _cnfExTime;							/**< 首次收到柜台确认时间 0 */

	XNum envno;                						/**<  客户环境号  */
	XNum errorno;              						/**<  错误原因   */

	XErrMsg errmsg;        				     		/**<  错误原因    */
	
	XOrderReqT request;                          	/**<  委托请求数据 */

	XLong _field2[7];                              /**< 填充64位对齐 */
} XOrderT, *pXOrderT;

#define XORDER_SIZE                                      (sizeof(XOrderT))
/**
 * @brief 成交信息
*/
typedef struct  {
	XIdx idx;                                           /**<  Id */
	XSystemId trdId;             					    /**<  成交编号 */
	XCustomer customerId; 								/**<  客户号 */

	XChar market;                                    	/**<  市场 @see eXMarket  */
	XChar trdSide;                                   	/**<  买卖方向 */
	XChar counter;                           			/**<  柜台 @see eXCounter */
	XChar _field;
	XShortTime trdTime;             					/**<  成交时间 HHMMSSSsss */

	XInvestId investId;       							/**<  股东帐户  */
	XSecurityId securityId;     						/**<  证券代码 */

	XQty trdQty;              							/**<  成交数量 */
	XPrice trdPrice;            						/**<  成交金额 */

	XMoney trdAmt;              						/**<  成交金额 */

	XQty cumQty;										/**< 累计成交数量 */
	XChar _field2[4];

	XMoney cumAmt;										/**< 累计成交金额 */
	XSystemId ordid;               						/**<  柜台订单编号 */
	XLong _field3[2];
} XTradeT, *pXTradeT;

#define XTRADE_SIZE                                     (sizeof(XTradeT))
/**
 * @brief 交易数据缓存
 */
typedef struct  {
	XTransHeadT head;								/**<  通过消息头类型使用数据 */
	union {
		XOrderReqT ordreq;							/**< 订单请求 */
		XOrderT ordrsp;								/**<  委托 */
		XTradeT trade;								/**<  成交 */
		XHoldT hold;								/**<  持仓 */
		XCashT cash;								/**<  资金 */
		XStrategyT strategy;						/**< 策略 */
//		XSnapshotBaseT snapshot;					/**< 快照 */
	};
} XTradeCache;

#define XTRADECACHE_SIZE                               (sizeof(XTradeCache))

/**
 * @brief 会话信息
 * 
 */
typedef struct 
{
	XCustomer customerId; 								/**<  客户号 */
	XNum sessionId;										/**< 会话ID */
	XChar isReady;                                      /**< 刚发起确认 */
	XChar _field[3];
	XNum readid;										/**< 环形队列的位置*/
	XNum sockfd;										/**< 会话fd */
	XNum maxFrontId;									/**< 最大前端编号 */
	XChar _field2[4];
	XLong _field3[3];
}XSessionT;


typedef struct 
{
	XNum msgType; 
	XNum socketfd;
	XNum sessionid;
	XChar _field[4];
	XChar msg[2096];
}XRspDataT;

#define XSESSION_SIZE                                  (sizeof(XSessionT))

/**
 * @brief 进程参数，涉及进程绑核及入参
 * 
 */
typedef struct 
{
	XNum cpuid;						/**< CPU 编号 */
	XInt market;				    /**< @see eXMarket */
	XBool resub;					/**< 是否支持盘中实时订阅，实时订阅时默认只订阅000001和600000 */
    XBool level;					/**< level1 or level2 */		
    XChar secType[7];               /**< @see eXSecType */
    XChar subFile[96];				/**< 行情订阅文件 */
	XCustomer customerId;			/**< 用户 */
}XBindParamT;

/**
 * @brief 进程管理数据,外部无需使用
 * 
 */
typedef struct 
{
	pid_t pid;							/**< 进程号 */
	XChar _field[4];
	void (*callBack)(XVoid* params); 	/**< 回调程序 */
	XVoid* params;				/**< 回调参数 */
	XChar processName[24];      		/**< 进程名 */
	XChar _field2[16];
}XProcInfoT;

/**
 * @brief 板块信息
 * 
 */
typedef struct 
{
	XIdx idx;						/**< 编号 */
	XChar blockNo[8];				/**< 板块编号 */
	XChar blockName[24];			/**< 板块名称 */
	XRatio zdf;						/**< 涨跌幅 */
	XNum count;						/**< 板块数量 */
    XIdx secIdx;					/**< 领涨股票 */
    XIdx beginIdx; 					/**< 开始索引 */
}XBlockT;

/**
 * @brief 板块信息
 * 
 */
typedef struct 
{
	XIdx idx;
	XChar blockNo[8];				/**< 板块编号 */						
	XChar market;					/**< 市场 */
	XChar _field[3];
	XRatio zdf;                     /**< 个股涨跌幅 */
	XSecurityId securityId;			/**< 证券代码 */
	XLong _field2[3];
}XBlockInfoT;

#pragma pack(pop)

#ifdef __cplusplus

}
#endif

#endif /* INCLUDE_XMAN_XDATASTRUCT_H_ */
