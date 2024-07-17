/**
 * @file XCommon.h
 * @version 0.10.0 2022/4/29
 * 			- 初始版本
 * @since 2022/4/29
 */

#ifndef INCLUDE_CORE_XCOM_H_
#define INCLUDE_CORE_XCOM_H_

#include "XDataStruct.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/


/**
 * @brief 内存初始化
 *
 * @return
 * @note 由框架处理,应用层不需要调用
 */
extern XVoid XManShmInit();

/**
 * @brief 加载内存
 * @return
 * @note 应用层需要通过此方法获取内存
 */
extern XVoid XManShmLoad();

/**
 * @brief 清楚内存
 * @return
 * @note 由框架处理,应用层不需要调用
 */
extern XVoid XManShmDelete();

/*****************************************************************************/
/**
 * @brief 获取监控数据
 * @return
 * @note 全局监控数据,保存所有委托数量、成交数量、快照数量等
 */
extern XMonitorT* XFndVMonitor();

extern XMonitorT* XFndVMonitorS(char* filename, int line);

extern XInt XPutVMonitor(XMonitorT* monitor);
/**
 * @brief 获取监控信息
 * @return
 */
extern XMonitorT* XGetVMonitor();
/**
 * @brief 根据类型获取对应的Id
 * @param type 数据类型  @see eXDataType
 * @return
 */
extern XIdx XGetIdByType(XChar type);

/**
 * @brief 根据策略类型获取策略编号
 * @param plotType 策略类型数据 @see eXPlotStatus
 * @return
 */
extern XPlotId XGenPlotId(XNum plotType, XIdx id);

/**
 * @brief 根据交易所查找行情监控
 * @param exchange 交易所
 * @return
 */
extern XMonitorMdT* XFndVMdMonitor(XNum exchange);

/**
 * @brief 根据客户号获取交易监控
 * @param customer   客户号
 * @return
 */
extern XMonitorTdT* XFndVTdMonitor(XChar* customer);

/**
 * @brief 存放交易监控
 * @param customer  客户号
 * @param envno 环境号
 * @return
 */
extern XInt XPutTdMonitor(XChar* customer, XNum envno);

/**
 * @brief 存放行情监控
 * @param customer  客户号
 * @return
 */
extern XInt XPutMdMonitor(XNum exchange);
/*****************************************************************************/

/**
 * @brief 根据客户号获取客户信息位置
 * @param customer  客户号
 * @return
 */
extern XIdx XFndCustomerByKey(XChar* customer);

/**
 * @brief 获取客户详情
 * @param customer  客户号
 * @return
 */
extern XCustT* XFndVCustomerByKey(XChar* customer);

/**
 * @brief 根据Id获取客户详情
 * @param idx  索引
 * @return
 */
extern XCustT* XFndVCustomerById(XIdx idx);

/**
 * @brief 存放客户信息
 * @param cust  客户信息
 * @return
 */
extern XInt XPutVCustomer(XCustT* cust);

/**
 * @brief 根据客户号删除客户信息
 * @param customer   客户号
 * @return
 */
extern XInt XRmvVCustomerByKey(XChar* customer);

/**
 * @brief 根据客户Id删除客户信息
 * @param idx  索引
 * @return
 */
extern XInt XRmvVCustomerById(XIdx idx);

/*****************************************************************************/

/**
 * @brief 根据市场、证券代码查找证券信息ID
 * @param market   市场
 * @param security   证券代码
 * @return
 */
extern XIdx XFndStockByKey(XInt market, XChar* security);

/**
 * @brief 根据市场、证券代码查找证券信息
 * @param market    市场
 * @param security   证券代码
 * @return
 */
extern XStockT* XFndVStockByKey(XInt market, XChar* security);

/**
 * @brief 根据ID查找证券信息
 * @param idx    索引
 * @return
 */
extern XStockT* XFndVStockById(XIdx idx);

/**
 * @brief 获取存储信息的地址
 * @param idx  索引
 * @return
 */
extern XStockT* XGetVStock(XIdx idx);

/**
 * @brief 存放证券信息
 * @param stock   证券信息
 * @return
 */
extern XInt XPutVStock(XStockT* stock);

/**
 * @brief 存放证券信息
 * @param stock   证券信息
 * @return
 */
extern XInt XPutStockByKey(XStockT* stock);
/**
 * @brief 更新证券信息
 * @param stock  证券信息
 * @return
 */
extern XInt XPutOrUpdVStockByKey(XStockT* stock);

/**
 * 根据市场、证券代码删除证券信息
 * @param market    市场
 * @param security  证券代码
 * @return
 */
extern XInt XRmvStockByKey(XInt market, XChar* security);

/**
 * @brief 根据ID删除证券信息
 * @param idx   索引
 * @return
 */
extern XInt XRmvVStockById(XIdx idx);

/*****************************************************************************/

/**
 * @brief 根据市场、证券代码查找证券信息
 * @param market       市场
 * @param security     证券代码
 * @return
 */
extern XIdx XFndIssueByKey(XInt market, XChar* security);

/**
 * @brief 根据市场、证券代码查找证券信息
 * @param market      市场
 * @param security    证券代码
 * @return
 */
extern XIssueT* XFndVIssueByKey(XInt market, XChar* security);

/**
 * @brief 根据Id查找证券信息
 * @param idx  -- 索引位置
 * @return
 */
extern XIssueT* XFndVIssueById(XIdx idx);

/**
 * @brief 更新证券信息，如果不存在则插入
 * @param issue    发行数据
 * @return
 */
extern XInt XPutOrUpdVIssueByKey(XIssueT* issue);

/**
 * @brief 根据市场、证券代码删除证券信息
 * @param market           市场
 * @param security         证券代码
 * @return
 */
extern XInt XRmvIssueByKey(XInt market, XChar* security);

/*****************************************************************************/

extern XIdx XFndCashByKey(XChar *customer, XInt accttype);
/**
 * @brief 获取资金信息
 * @param customer
 * @param invest
 * @param market
 * @param securityid
 * @return >0成功，其它失败
 */
extern XCashT* XFndVCashByKey(XChar *customer, XInt accttype);

/**
 * @brief 查找资金信息
 * @param idx
 * @return
 */
extern XCashT* XFndVCashById(XIdx idx);
/**
 * @brief 如果不存在会添加
 * @param cash
 * @return
 */
extern XInt XPutOrUpdVCashByKey(XCashT* cash);

/**
 * @brief 删除资金信息
 * @param customer
 * @param accttype
 * @return
 */
extern XInt XRmvCashByKey(XChar *customer, XInt accttype);

/**
 * @brief 删除资金信息
 * @param idx
 * @return
 */
extern XInt XRmvCashById(XIdx idx);
/**
 * @brief 冻结资金
 * @param customer
 * @param accttype
 * @param money
 * @return
 */
extern XInt XFrozenMoney(XChar *customer, XInt accttype, XMoney money);

/**
 * @brief 检查资金
 * @param customer
 * @param accttype
 * @param money
 * @return
 */
extern XInt XCheckMoney(XChar *customer, XInt accttype, XMoney money);

/*****************************************************************************/

/**
 * @brief 获取投资者账户
 * @param customer
 * @param market
 * @param invest
 * @return 投资者信息
 */
extern XInvestT* XFndVInvestByKey(XChar *customer, XInt market, XChar* investId);

/**
 * @brief 获取投资者账户位置
 * @param customer
 * @param market
 * @param acctType
 * @return 投资者信息位置
 */
extern XIdx XFndInvestByAcctType(XChar* customer, XInt market, XInt acctType);

/**
 * @brief 获取投资者账户
 * @param customer
 * @param market
 * @param acctType
 * @return 投资者信息
 */
extern XInvestT* XFndVInvestByAcctType(XChar* customer,  XInt market, XInt acctType);

/**
 * @brief 获取投资者账户
 * @param idx
 * @return 投资者信息
 */
extern XInvestT* XFndVInvestById(XIdx idx);
/**
 * @brief 存放资金信息
 * @param customer
 * @param invest
 * @param market
 * @param securityid
 * @return
 */
extern XInt XPutVInvest(XInvestT* invest);

/**
 * @brief 如果不存在会添加
 * @param cash
 * @return
 */
extern XInt XPutOrUpdVInvestByKey(XInvestT* invest);

/**
 * @brief 删除资金信息
 * @param customer
 * @param market
 * @param investId
 * @return
 */
extern XInt XRmvInvestByKey(XChar *customer, XInt market, XChar* investId);

/**
 * @brief 删除资金信息
 * @param customer
 * @param market
 * @param accType
 * @return
 */
extern XInt XRmvInvestOnlyHash(XChar* customer, XInt market, XInt accType);

/*****************************************************************************/

/**
 * @brief 根据客户号、投资者账户、市场、证券代码查找持仓
 * @param customer
 * @param invest
 * @param market
 * @param securityid
 * @return
 */
extern XIdx XFndHold(XChar *customer, XChar *invest, XInt market, XChar *securityid);

/**
 * @brief 根据客户号、投资者账户、市场、证券代码查找持仓
 * @param customer
 * @param invest
 * @param market
 * @param securityid
 * @return
 */
extern XHoldT* XFndVHoldByKey(XChar *customer, XChar *invest, XInt market,
		XChar *securityid);

/**
 * @brief 根据ID查找持仓
 * @param idx
 * @return
 */
extern XHoldT* XFndVHoldById(XIdx idx);

/**
 * @brief 存放持仓信息
 * @param hold
 * @return
 */
extern XInt XPutVHold(XHoldT* hold);

/**
 * @brief 插入或更新key:customer+invest+market+security
 * @param hold
 * @return
 */
extern XInt XPutOrUpdVHoldByKey(XHoldT* hold);

/**
 * @brief 根据客户号、投资者账户、市场、证券代码删除持仓
 * @param customer
 * @param invest
 * @param market
 * @param securityid
 * @return
 */
extern XInt XRmvVHoldByKey(XChar *customer, XChar *invest, XInt market,
		XChar *securityid);

/**
 * @brief 根据ID删除持仓
 * @param idx
 * @return
 */
extern XInt XRmvVHoldById(XIdx idx);

/**
 * @brief 冻结持仓
 * @param customer
 * @param invest
 * @param market
 * @param securityid
 * @param holding
 * @retval
 */
extern XInt XFrozenHold(XChar *customer, XChar *invest, XInt market, XChar *securityid,
		XSumQty holding);

/*****************************************************************************/

/**
 * @brief 根据本地订单编号找订单，请优先使用柜台编号
 * @param customer
 * @param invest
 * @param market
 * @param securityid
 * @param localid
 * @return >0成功，其它失败
 */
extern XIdx XFndOrderByLoc(XChar *customer, XInt envno, XLocalId localid);

/**
 * @brief 根据柜台编号找订单
 * @param customer
 * @param invest
 * @param market
 * @param securityid
 * @param orderid
 * @return >0成功，其它失败
 */
extern XIdx XFndOrderByCnt(XChar *customer, XInt market, XSystemId orderid);

/**
 * @brief 根据本地订单编号找订单
 * @param customer
 * @param invest
 * @param market
 * @param securityid
 * @param localid
 * @return
 */
extern XOrderT* XFndVOrderByLoc(XChar *customer, XInt envno, XLocalId localid);

/**
 * @brief 根据柜台订单编号找订单
 * @param customer
 * @param invest
 * @param market
 * @param securityid
 * @param orderid
 * @return
 */
extern XOrderT* XFndVOrderByCnt(XChar *customer, XInt market, XSystemId orderid);

/**
 * @brief 根据ID获取订单信息
 * @param idx
 * @return
 */
extern XOrderT* XFndVOrderById(XIdx idx);

/**
 * @brief 找到更新数据的位置
 * @param idx
 * @return
 */
extern XOrderT* XGetVOrder(XIdx idx);

/**
 * @brief 根据柜台编号插入订单,同时更新本地订单的hash
 * @param order
 * @return
 */
extern XInt XPutOrUpdVOrderByCnt(XOrderT* order);

/**
 * @brief 根据柜台编号插入Or更新订单
 * @param order
 * @return
 */
extern XInt XPutOrUpdOrderHashByCnt(XOrderT* order);
/**
 * @brief 根据本地订单编号插入hash
 * @param order
 * @return
 */
extern XInt XPutOrderHashByLoc(XOrderT *order);

/**
 * @brief 根据本地订单编号插入订单,并返回订单编号
 * @param order
 * @return -1 错误
 *         > 0 索引位置
 */
extern XIdx XPutVOrder(XOrderT* order);

/**
 * @brief 找到本地订单，然后按OrderId插入Hash
 * @param order
 * @return
 */
XInt XUpdVOrderByLoc(XOrderT* order);
/**
 * @brief 根据id更新订单
 * @param idx
 * @param order
 * @return
 */
extern XInt XUpdOrderByIdx(XOrderT* order);

/**
 * @brief 根据柜台编号删除订单
 * @param order
 * @return
 */
extern XInt XRmvVOrderByCnt(XOrderT* order);

/**
 * @brief 根据本地订单编号删除订单，（无orderid情况）
 * @param order
 * @return
 */
extern XInt XRmvVOrderByLoc(XOrderT* order);

/*****************************************************************************/
/**
 * @brief 获取成交更新的位置
 * @param idx
 * @return
 */
extern XTradeT* XGetVTrade(XIdx idx);

/**
 * @brief 获取对应成交
 * @param customer
 * @param market
 * @param tradeid
 * @return
 */
extern XIdx XFndTradeByKey(XChar* customer, XInt market, XSystemId tradeid);

/**
 * @brief 获取对应成交
 * @param customer
 * @param market
 * @param tradeid
 * @return
 */
extern XTradeT* XFndVTradeByKey(XChar* customer, XInt market, XSystemId tradeid);

/**
 * @brief 根据id找到对应成交信息
 * @param idx
 * @return
 */
extern XTradeT* XFndVTradeById(XIdx idx);

/**
 * @brief 存放持仓信息
 * @param hold
 * @return
 */
extern XInt XPutVTrade(XTradeT* trade);

/**
 * @brief 根据客户、市场、成交编号删除订单
 * @param customer
 * @param market
 * @param tradeid
 * @return
 */
extern XInt XRmvTradeByKey(XChar* customer, XInt market, XSystemId tradeid);

/**
 * @brief 根据ID删除订单
 * @param idx
 * @return
 */
extern XInt XRmvTradeById(XIdx idx);

/*****************************************************************************/

/**
 * @brief 根据市场、证券代码获取该证券最高价
 * @param market
 * @param securityid
 * @return
 */
extern XPrice XGetUpLimPx(int market, char *securityid);

/**
 * @brief 根据市场、证券代码获取产品类型
 * @param market
 * @param securityid
 * @return
 */
extern XInt XGetPrdType(XInt market, XChar* security);

/**
 * @brief 根据订单请求生成订单
 * @param orderequest
 * @return
 */
extern XIdx XGenOrder(XOrderReqT *orderequest);

/*****************************************************************************/


static inline XInt isMktOpenTime(XShortTime updatetime) {
	if (updatetime >= MARKET_CALL_BEFORE && updatetime <= MARKET_CLOSING) {
		return (1);
	}
	return (0);
}

static inline XInt isMktCallAucTime(XShortTime updatetime) {
	if (updatetime >= MARKET_CALL_BEFORE && updatetime <= MARKET_CALL_MID) {
		return (1);
	}
	return (0);
}

static inline XInt isMktConAucTime(XShortTime updatetime) {
	if ((updatetime >= MARKET_CALL_BEFORE && updatetime <= MARKET_MORN_END)
			|| (updatetime >= MARKET_AFTER_BEGIN && updatetime < MARKET_CLOSING)) {
		return (1);
	}
	return (0);
}
static inline XInt isMktConEndTime(XShortTime updatetime) {
	if (updatetime > MARKET_BEFORE_CLOSING && updatetime < MARKET_CLOSING) {
		return (1);
	}
	return (0);
}

static inline XInt isMktClosedTime(XShortTime updatetime) {
	if (updatetime >= MARKET_CLOSED) {
		return (1);
	}
	return (0);
}

static inline XInt isMktClosingTime(XShortTime updatetime)
{
	if (updatetime >= MARKET_CLOSING) {
		return (1);
	}
	return (0);
}

/*****************************************************************************/

/**
 * @brief 返回进程管理信息
 * @return
 */
extern XIdx XGetAppCnt();

/**
 * @brief 找到进程信息
 * @param idx
 * @return
 */
extern XProcInfoT* XFndVAppById(XIdx idx);

/**
 * @brief 放到进程管理列表
 * @param procs
 * @return
 */
extern XInt XPutVApp(XProcInfoT* procs);

/**
 * @brief 放到进程管理列表
 * @param procsName进程名
 * @return
 */
extern XInt XPubVAppByName(XCChar* procsName);
/**
 * @brief 启动进程
 * @param procs
 * @param num
 * @return
 */
extern XInt XProcStart(XProcInfoT procs[], XInt num);

/**
 * @brief 停止进程
 * @return
 */
extern XInt XProcStop();

/**
 * @brief 检查进程是否存在
 * @param pid
 * @return
 */
extern XInt XCheckPid(pid_t pid);

/*****************************************************************************/
/**
 * @brief 找到订单薄位置
 * @param market
 * @param securityid
 * @return
 */
extern XIdx XFndOrderBook(XInt market, XChar *securityid);

/**
 * @brief 存放订单薄索引
 * @param market
 * @param securityid
 * @return
 */
extern XIdx XPutOrderBookHash(XInt market, XChar *securityid);

/**
 * @brief 找到订单薄信息
 * @param market
 * @param securityid
 * @return
 */
extern XOrderBookT* XFndVOrderBookByKey(XInt market, XChar* securityid);

/**
 * @brief 找到订单薄信息
 * @param idx
 * @return
 */
extern XOrderBookT* XFndVOrderBookById(XIdx idx);

/**
 * @brief 存放订单薄信息
 * @param OrderBook
 * @return
 */
extern XInt XPutVOrderBook(XOrderBookT* OrderBook);

/**
 * @brief 存放订单薄信息
 * @param OrderBook
 * @return
 */
extern XInt XPutOrdUpdVOrderBookByKey(XOrderBookT* OrderBook);

/**
 * @brief 删除订单薄信息
 * @param market
 * @param securityid
 * @return
 */
extern XInt XRmvVOrderBookByKey(XInt market, XChar *securityid);

/**
 * @brief 删除订单薄信息
 * @param idx
 * @return
 */
extern XInt XRmvVOrderBookById(XIdx idx);

/**
 * @brief 根据Id找到重构后的行情
 * @param idx
 * @return XRSnapshotT 重构的快照行情
 */
extern XRSnapshotT* XFndVRSnapshotById(XIdx idx);

/**
 * @brief 根据市场和证券代码找到重构后的行情位置
 * @param market
 * @param securityid
 * @return
 */
extern XIdx XFndRSnapshot(XInt market, XChar *securityid);

/**
 * @brief 更新重构行情数据,如果不存在插入，存在则更新
 * @param snapshot
 * @return
 */
extern XInt XPutOrUpdVRSnapshot(XSnapshotT* snapshot);

/**
 * @brief 根据市场和证券代码找到重构后的行情
 * @param market
 * @param securityid
 * @return
 */
extern XRSnapshotT* XFndVRSnapshotByKey(XInt market, XChar* securityid);

/**
 * @brief 更新重构行情数据,如果不存在插入，存在则更新
 * @param snapshot
 * @return
 */
extern XInt XPutOrUpdVRSnapshot(XRSnapshotT* snapshot);

/**
 * @brief 找到逐笔委托数据
 * @param market
 * @param channel
 * @param seqno
 * @return
 */
extern XTickOrderT* XFndVTickOrder(XInt market, XChar* security, XInt channel, XSeqNum seqno);

/**
 * @brief 存放逐笔委托数据
 * @param priceOrder
 * @return
 */
extern XInt XPutVTickOrder(XTickOrderT* priceOrder);

/**
 * @brief 更新逐笔委托数据
 * @param priceOrder
 * @return
 */
extern XInt XUpdateVTickOrderByKey(XTickOrderT* priceOrder);

/**
 * @brief 删除逐笔委托数据
 * @param market
 * @param channel
 * @param seqno
 * @return
 */
extern XInt XRmvVTickOrderByKey(XInt market, XChar* security, XInt channel, XSeqNum seqno);

/**
 * @brief 获取重构行情的价格档位地址
 * @param market
 * @param security
 * @param bs
 * @param price
 * @return
 */
extern XIdx XFndPriceLevel(XInt market, XChar* security, XInt bs, XPrice price, XInt level);

/**
 * @brief 获取重构行情的价格档位信息
 * @param market
 * @param security
 * @param bs
 * @param price
 * @return
 */
extern XPriceLevelT* XFndVPriceLevel(XInt market, XChar* security, XInt bs, XPrice price, XInt level);

/**
 * @brief 找到价格档位
 * @param idx
 * @return
 */
extern XPriceLevelT* XFndVPriceLevelById(XIdx idx, XInt market);

/**
 * @brief 存入或更新重构行情的价格档位信息
 * @param market
 * @param security
 * @param bs
 * @param price
 * @param level 层次
 * @param priceLevel 价格档位
 * @return
 */
extern XInt XPutOrUpdatePriceData(XInt market, XChar *security, XInt bs, XPrice price,
		XInt level, XPriceLevelT *priceLevel);

/**
 * @brief 存入或更新重构行情的价格档位信息
 * @param market
 * @param security
 * @param bs
 * @param price
 * @param level 层次
 * @param priceLevel 价格档位
 * @return
 */
extern XInt XPutOrUpdatePriceLevel(XInt market, XChar *security, XInt bs, XPrice price,
		XInt level, XPriceLevelT *priceLevel);

/**
 * @brief 删除重构行情的价格档位信息
 * @param market
 * @param security
 * @param bs
 * @param price
 * @param level 层次
 * @param priceLevel 价格档位
 * @return
 */
extern XInt XRmvPriceHashByKey(XInt market, XChar *security, XInt bs, XPrice price,
		XInt level);

/**
 * @brief 删除重构行情的价格档位信息
 * @param idx
 * @return
 */
extern XInt XRmvPriceDataById(XIdx idx, XInt market);

/**
 * @brief 删除价格档位
 * @param market
 * @param security
 * @param bs
 * @param price
 * @return
 */
extern XInt XRmvPriceLevelByKey(XInt market, XChar* security, XInt bs, XPrice price, XInt level);

/*****************************************************************************/

/**
 * @brief 查找快照数据地址
 * @param market
 * @param security
 * @return
 */
extern XIdx XFndSnapshot(XInt market, XChar* security);

/**
 * @brief 查找快照数据
 * @param market
 * @param security
 * @return
 */
extern XSnapshotT* XFndVSnapshotByKey(XInt market, XChar* security);

/**
 * @brief 根据地址位置找到快照数据
 * @param idx
 * @return
 */
extern XSnapshotT* XFndVSnapshotById(XIdx idx);

/**
 * @brief 存入快照数据
 * @param snapshot
 * @return
 */
extern XInt XPutSnapshot(XSnapshotT* snapshot);

/**
 * @brief 更新快照数据，如果没有插入
 * @param snapshot
 * @return
 */
extern XInt XUpdateSnapshot(XSnapshotT* snapshot);

/**
 * @brief 删除快照数据
 * @param market
 * @param security
 * @return
 */
extern XInt XRmvSnapshotByKey(XInt market, XChar* security);

/**
 * @brief 更新逐笔数据存放位置
 * @param market
 * @param channel
 * @return
 */
extern XInt XUpdTickSeq(XInt market, XNum channel, XL2LT* l2data);

/*****************************************************************************/

extern XInt XIsNullCache(XInt type);

extern XInt XGetReadCache(XInt type);

extern XVoid* XPopCache(XInt type, XInt readId);

extern XVoid* XPopCacheByPos(XInt type, XInt readId, XLLong curPos);

extern XInt XPushCache(XInt type, XVoid* data);

extern XVoid XReleaseCache(XInt type, XInt readId);

/*****************************************************************************/

/**
 * @brief 查找客户会话信息
 * @param customer
 * @param sessionid 会话编号
 * @param frontid
 * @return
 */
extern XVoid* XFndVSessionByKey(XInt sessionid);

/**
 * @brief 存入客户会话信息
 * @param session  会话信息
 * @return
 */
extern XInt XPutOrUpdVSession(XInt sessionid, XVoid *session);

/*****************************************************************************/

/**
 * @brief 查找策略编号
 * @param customer  客户信息
 * @param plotid    策略编号
 * @return
 */
extern XIdx XFndStrategyByKey(XChar *customer, XPlotId plotid);

/**
 * @brief 查找策略编号
 * @param customer  客户信息
 * @param sessionid 会话编号
 * @param frontid   客户前端编号
 * @param security  证券代码
 * @return
 */
extern XIdx XFndStrategyByFrontId(XChar *customer, XNum sessionid, XNum frontid, XChar* security, XInt bs);

/**
 * @brief 查找策略编号
 * @param customer  客户信息
 * @param plotid    策略编号
 * @return
 */
extern XStrategyT* XFndVStrategyByKey(XChar *customer, XPlotId plotid);

/**
 * @brief 查找策略编号
 * @param customer  客户信息
 * @param sessionid 会话编号
 * @param frontid   客户前端编号
 * @param security  证券代码
 * @return
 */
extern XStrategyT* XFndVStrategyByFrontId(XChar *customer, XNum sessionid, XNum frontid, XChar* security, XInt bs);

/**
 * @brief 查找策略编号
 * @param idx
 * @return
 */
extern XStrategyT* XFndVStrategyById(XIdx idx);

/**
 * @brief 存入策略信息
 * @param strategy
 * @return
 */
extern XInt XPutVStrategy(XStrategyT *strategy);

/**
 * @brief 更新策略信息
 * @param strategy
 * @return
 */
extern XInt XUpdVStrategyByKey(XStrategyT *strategy);

/**
 * @brief 更新策略信息
 * @param strategy
 * @return
 */
extern XInt XUpdVStrategyByFrontId(XStrategyT *strategy);

/**
 * @brief 删除策略信息
 * @param customer
 * @param sessionid
 * @param frontid
 * @param security
 * @return
 */
extern XInt XRmvStrategyByFrontId(XChar *customer, XNum sessionid, XNum frontid, XChar* security, XInt bs);

/**
 * @brief 删除策略编号
 * @param customer  客户信息
 * @param plotid    策略编号
 * @return
 */
extern XInt XRmvStrategyByKey(XChar *customer, XPlotId plotid);

/**
 * @brief 删除策略编号
 * @param idx
 * @return
 */
extern XInt XRmvVStrategyById(XIdx idx);

/*****************************************************************************/
/**
 * @brief 生成订单并放入缓存中等待柜台接口处理
 * @param idx
 * @return
 */
extern XLocalId XPutOrderReq(XOrderReqT *pOrder);

/*****************************************************************************/

/*****************************************************************************/

/**
 * @brief 守护进程启动
 * 
 * @param procs 进程数据
 * @param num   进程数
 * @return XInt 
 */
extern XInt XAppStart(XProcInfoT procs[], XInt num);

/**
 * @brief 守护进程停止
 * 
 * @return XInt 
 */
extern XInt XAppStop();

/**
 * @brief 会话数据管理
 * 
 * @param market 
 * @param securityid 
 * @return XVoid* 
 */
extern XVoid* XFndVSessionManByKey(XInt sesionId, XInt market, XChar *securityid);

/**
 * @brief 会话数据管理
 * 
 * @param market 
 * @param securityid 
 * @param version 
 * @return XInt 
 */
extern XInt XPutOrUpdVSessionMan(XInt sessionId, XInt market, XChar* securityid, XVoid* version);

/**
 * @brief 会话数据管理
 *
 * @param market
 * @param securityid
 * @param version
 * @return XInt
 */
extern XInt XRmvSessionManByKey(XInt sessionId, XInt market, XChar *security);
/**
 * @brief 更新频道数据
 * 
 * @param channel 
 * @return XInt 
 */
extern XInt XPutOrUpdVChannelByKey(XChannelT* channel);

/**
 * @brief 查找频道数据
 * 
 * @param channel 频道号
 * @return XVoid* 
 */
extern XVoid* XFndVChannelByKey(XNum channel);

/**
 * @brief 判断深圳频道丢单,请确保订阅了频道所有数据
 * 
 * @param channelno 
 * @param bizIndex 
 * @return XInt 
 */
extern XInt IsSZSEChnlLoss(XNum channelno, XSeqNum bizIndex);

/**
 * @brief 判断上海频道委托丢单，请确保订阅了该频道所有数据
 * 
 * @param channelno 
 * @param ordSeq 
 * @return XInt 
 */
extern XInt IsSSEOrdChnlLoss(XNum channelno, XSeqNum ordSeq);

/**
 * @brief 判断上海频道委托成交，请确保订阅了该频道所有数据
 * 
 * @param channelno 
 * @param trdSeq 
 * @return XInt 
 */
extern XInt IsSSETrdChnlLoss(XNum channelno, XSeqNum trdSeq);

/**
 * @brief 获取K先数据
 * 
 * @param idx             股票对应的索引位置
 * @param kx              1分钟为0,5分钟为1
 * @param pos             存储的位置 
 * @return XKLineT 
 */
extern XKLineT* GetKlinesByBlock(XIdx idx, XInt kx);



/*****************************************************************************/

/**
 * @brief 查找板块数据地址
 * @param market
 * @param security
 * @return
 */
extern XIdx XFndBlock(XChar* blockNo);

/**
 * @brief 查找板块数据
 * @param market
 * @param security
 * @return
 */
extern XBlockT* XFndVBlockByKey(XChar* blockNo);

/**
 * @brief 根据地址位置找到板块数据
 * @param idx
 * @return
 */
extern XBlockT* XFndVBlockById(XIdx idx);

/**
 * @brief 存入板块数据
 * @param block
 * @return
 */
extern XInt XPutBlock(XBlockT* block);

/**
 * @brief 更新板块数据，如果没有插入
 * @param block
 * @return
 */
extern XInt XPutOrUpdBlock(XBlockT* block);

/**
 * @brief 删除板块数据
 * @param block
 * @return
 */
extern XInt XRmvBlockByKey(XChar* blockNo);

/***********************************************************************/

/**
 * @brief 查找板块信息数据
 * @param blockNo    板块
 * @param securityId 证券代码
 * @param market     市场
 * @return 板块索引
 */
extern XIdx XFndBlockInfo(XChar *blockNo, XChar *securityId, XChar market);

/**
 * @brief 查找板块信息数据
 * @param blockNo    板块
 * @param securityId 证券代码
 * @param market     市场
 * @return 板块信息
 */
extern XBlockInfoT* XFndVBlockInfoByKey(XChar *blockNo, XChar *securityId, XChar market);

/**
 * @brief 查找板块信息数据
 * @param idx   索引
 * @return
 */
extern XBlockInfoT* XFndVBlockInfoById(XIdx idx);

/**
 * @brief 更新板块信息数据
 * @param blockinfo   板块信息数据
 * @return
 */
extern XInt XPutBlockInfo(XBlockInfoT *blockinfo);

/**
 * @brief 更新板块信息数据
 * @param blockinfo   板块信息数据
 * @return
 */
extern XInt XPutOrUpdBlockInfo(XBlockInfoT *blockinfo);

/**
 * @brief 删除板块信息数据
 * @param blockNo
 * @param securityId
 * @param market
 * @return
 */
extern XInt XRmvBlockInfoByKey(XChar *blockNo, XChar* securityId, XChar market);

/**
 * @brief 授权验证
 * @param 客户号
 * @param 交易日
 */ 
extern XBool CheckAuth(XChar* customerId, XInt traday);

//##########################################################################################

extern XInt XPutOrUpdBOrder(XOrderT* order);

extern XOrderT* XFndVBOrderById(XIdx idx);

extern XOrderT* XFndVBOrderByOrdid(XChar* customer, XInt market, XSystemId orderid);

extern XInt XPutOrUpdBHold(XHoldT* hold);

extern XHoldT* XFndVBHoldById(XIdx idx);

extern XIdx XFndBHoldByKey(XChar* customer, XChar* invest, XInt market, XChar* securityId);

extern XHoldT* XFndVBHoldByKey(XChar* customer, XChar* invest, XInt market, XChar* securityId);

extern XIdx XFndBCashByKey(XChar *customer, XInt accttype);

extern XCashT* XFndVBCashByKey(XChar* customer, XInt acctType);

extern XInt XPutOrUpdVBCashByKey(XCashT *cash);



typedef void (*XPcb)(XSignalT* notify);

#define XTHRD_NAME_LEN         (24)
typedef struct 
{
	XChar thrdName[XTHRD_NAME_LEN];
    XPcb callback;
}XParamT;

extern void SetCallback(XChar *thrdName, XPcb callback);

extern XVoid* XGetTradeCache();

extern XEtfT* XFndVEtfById(XIdx idx);

extern XIdx XFndEtfByKey(XInt market, XChar* securityId);

extern XEtfT* XFndVEtfByKey(XInt market, XChar* securityId);

extern XInt XPutVEtf(XEtfT* pEtf);

extern XInt XUpdVEtf(XEtfT* pEtf);

extern XEtfCompT* XFndVEtfCompById(XIdx idx);

extern XInt XPutVEtfComp(XEtfCompT* pEtf);

extern XInt XUpdVEtfComp(XEtfCompT* pEtf);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_CORE_XCOM_H_ */
