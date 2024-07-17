/**
 * @file XBase.h
 * @since 2022/1/26
 */

#ifndef INCLUDE_XMAN_XBASE_H_
#define INCLUDE_XMAN_XBASE_H_
#include "XError.h"
#include "XTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * 2022.10.26          篮子中添加买一撤单率和卖一撤单量参数,删除在plot.conf中的买一撤单率
 */


//协议变动版本号+1,版本更新日期刷为最新
/**
 * 2023.12.11 修改64位对齐
 */
#define __XMAN_VERSION__        "3.1.001"

/**< 无业务含义的函数返回值使用 */
typedef int XReturn;

/**< 索引位置 */
typedef long long XIdx;

/**< 行情里面流水号 */
typedef long long XSeqNum;

/**< 价格 */
typedef int XPrice;

/**< 单纯数量 */
typedef int XNum;

/**< 比例 */
typedef int XRatio;

/**< 报单数量 */
typedef int XQty;

/**< 统计数量 */
typedef long long XSumQty;

/**< 资金 */
typedef long long XMoney;

/**< 单位 */
typedef unsigned int XUnit;

/**< 费用 */
typedef int XFee;

/**< 交易日 */
typedef int XTradeDay;

/**< 短时间 */
typedef int XShortTime;

/**< 长时间 */
typedef long long XLongTime;

/**< 交易所报单编号 */
typedef long long XSystemId;

/**< 本地报单编号 */
typedef int XLocalId;

/**< 策略编号 */
typedef long long XPlotId;

/**< 最大订阅行情数量 */
#define MAX_MKTSUB_CNT       20000

/**< 证券代码长度 */
#define SECURITYID_LEN        16

/**< 投资者账户/股东账户 */
#define INVESTID_LEN          16

/**< 客户号长度 */
#define CUSTOMERID_LEN        16

/**< 证券名称长度 */
#define SECURITYNAME_LEN      40

/**< 资金账户长度 */
#define ACCOUNTID_LEN         16

/**< 交易所订单编号长度 */
#define EXCHORDID_LEN         16

/**< 错误信息长度 */
#define ERRORMSG_LEN          64

/**< 频道号长度 */
#define CHANNEL_NO_LEN        3

/**< ns到s的倍数 */
#define XTIMS_S4NS          (1000000000LL)

/**< ns到ms的倍数 */
#define XTIMS_MS4NS         (1000000LL)

/**< 实际的价格要除以10000 */
#define XPRICE_DIV     	     (0.0001)

/**< 实际的价格 * 10000 */
#define XPRICE_MULT          (10000)

/**< 客户号 */
typedef char XCustomer[CUSTOMERID_LEN];

/**< 股东帐户 */
typedef char XInvestId[INVESTID_LEN];

/**< 资金账号 */
typedef char XAccountId[ACCOUNTID_LEN];

/**< 证券代码 */
typedef char XSecurityId[SECURITYID_LEN];

/**< 基金代码 */
typedef char XFundId[SECURITYID_LEN];

/**< 证券名称 */
typedef char XSecurityName[SECURITYNAME_LEN];

/**< 行情流编号 */
typedef char XStreamId[CHANNEL_NO_LEN];

#define XPASSWORD_LEN          (128)
/**< 密码 */
typedef char XPassword[XPASSWORD_LEN];

/**< 会员代码 */
typedef char XBroker[12];

/**< 硬盘序列号 */
typedef char XHardware[24];

/**< 备注 */
typedef char XRemark[128];

/**< 交易所订单编号 */
typedef char XExchId[EXCHORDID_LEN];

/**< 错误信息 */
typedef char XErrMsg[ERRORMSG_LEN];

/**< 日志信息 */
typedef char XLogMsg[512];

/**< 内存配置文件目录 */
#define XSHM_SDB_FILE                   "../conf/sdb.conf"

#define XUSER_FILE                      "../conf/user.conf"

#define XSYSTEM_FILE                    "../conf/system.conf"

#define MARKET_CALL_BEFORE      (91500000)
#define MARKET_CALL_MID			(92000000)
#define MARKET_CALL_END         (92500000)
#define MARKET_MORN_BEGIN       (93000000)
#define MARKET_MORN_END         (113000000)
#define MARKET_AFTER_BEGIN      (130000000)
#define MARKET_BEFORE_CLOSING   (145600000)
#define MARKET_CLOSING          (150000000)
#define MARKET_CLOSED           (153000000)

/**< 交易数据导出 */
#define XMAN_DATA_SNAPSHOT       "../data/export/snapshot.csv"
#define XMAN_IMP_KSNAPSHOT_1    "../data/kline/k1.csv"
#define XMAN_IMP_KSNAPSHOT_5    "../data/kline/k5.csv"
#define XMAN_IMP_TICKS         "../data/kline/ticks.csv"
#define XMAN_IMP_HSNAPSHOT      "../data/kline/hsnapshot.csv"
#define XMAN_IMP_BLOCK          "../data/kline/block.csv"
#define XMAN_IMP_BLOCKINFO      "../data/kline/blockinfo.csv"
#define XMAN_IMP_CONVPX         "../data/kline/convbond.csv"

#define XMAN_DATA_STOCK          "../data/export/stock.csv"
#define XMAN_DATA_DIFFSNAP       "../data/export/difsnapshot.csv"
#define XMAN_DATA_ORDER          "../data/export/order.csv"
#define XMAN_DATA_TRADE          "../data/export/trade.csv"
#define XMAN_DATA_INVEST         "../data/export/invest.csv"
#define XMAN_DATA_STRATEGY       "../data/export/strategy.csv"
#define XMAN_DATA_HOLD           "../data/export/hold.csv"
#define XMAN_DATA_SNAPLEVEL      "../data/export/snaplevel.csv"
#define XMAN_DATA_CASH           "../data/export/cash.csv"
#define XMAN_DATA_RSNAP          "../data/export/rsnapshot.csv"
#define XMAN_DATA_BLOCK          "../data/export/block.csv"
#define XMAN_DATA_BLOCKINFO      "../data/export/blockinfo.csv"
#define XMAN_DATA_SELL          "../data/export/sell.csv"

/**< 静态数据 */
#define XMAN_DATA_STATIC         "../data/store/static.bin"

/**< 交易所逐笔数据 */
#define XMAN_DATA_MKTSTOREBIN        "../data/store/mktstore.bin"
#define XMAN_DATA_SHMKTBIN           "../data/store/shmkt.bin"
#define XMAN_DATA_SZMKTBIN           "../data/store/szmkt.bin"

#define XMAN_DATA_MKTSTORECSV        "../data/store/mktstore.csv"

/**< 重构好的快照行情 */
#define XMAN_DATA_TSNAPCSV          "../data/store/resnap.csv"

/**< 重构好的快照行情二进制 */
#define XMAN_DATA_TSNAPBIN          "../data/store/resnap.bin"

/**< 交易日志 */
#define XMAN_DATA_TRADEBIN          "../data/store/trade.bin"

#define XMAN_DATA_TRADECSV          "../data/store/trade.csv"

#define GET_BIT(value,bit) ((value)&(1<<(bit)))    /**< 读取指定位 */
#define CPL_BIT(value,bit) ((value)^=(1<<(bit)))   /**< 取反指定位 */

#define SET0_BIT(value,bit) ((value)&=~(1<<(bit))) /**< 把某个位置0 */
#define SET1_BIT(value,bit) ((value)|= (1<<(bit))) /**< 把某个位置1 */

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_XMAN_XBASE_H_ */
