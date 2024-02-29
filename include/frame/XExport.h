/**
 * @file XExport.h
 *
 *  Created on: 2022年6月9日
 *      Author: DELL
 */
#ifndef INCLUDE_CORE_XEXPORT_H_
#define INCLUDE_CORE_XEXPORT_H_

#include "XCom.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 打印内存基础信息
 * @return
 */
extern XVoid XExpPrint();

extern XVoid XMonitorPrint();

/**
 * @brief 导出委托订单
 * @param 文件名
 * @return
 */
extern XVoid XExpOrder(XChar* expFile);

/**
 * @brief 导出成交订单
 * @param 文件名
 * @return
 */
extern XVoid XExpTrade(XChar* expFile);

/**
 * @brief 导出持仓
 * @param 文件名
 * @return
 */
extern XVoid XExpHold(XChar* expFile);

/**
 * @brief 导出股东账户
 * @param 文件名
 * @return
 */
extern XVoid XExpInvest(XChar *expFile);

/**
 * @brief 导出证券信息
 * @param 文件名
 * @return
 */
extern XVoid XExpStock(XChar* expFile);

/**
 * @brief 导出资金账户
 * @param 文件名
 * @return
 */
extern XVoid XExpCash(XChar* expFile);

/**
 * @brief 导出K1
 * @param 文件名
 * @return
 */
extern XVoid XExpK1(XChar* expFile);

/**
 * @brief 导出K2
 * @param 文件名
 * @return
 */
extern XVoid XExpK5(XChar* expFile);

/**
 * @brief 导出交易所原始快照行情
 * @param 文件名
 * @return
 */
extern XVoid XExpSnapshot(XChar* expFile);

/**
 * @brief 导出重构和原始快照行情比较
 * @param 文件名
 * @return
 */
extern XVoid XExpDifSnapshot(XChar* expFile);

/**
 * @brief 导出策略委托信息
 * @param 文件名
 * @return
 */
extern XVoid XExpStrategy(XChar* expFile);

/**
 * @brief 导出价格档位
 * @param 文件名
 * @return
 */
extern XVoid XExpSnapLevel(XChar *expFile);

/**
 * @brief 导出收盘数据
 * @param 文件名
 * @return
 */
extern XVoid XExpKSnap(XChar *expFile);

/**
 * @brief 导出收盘涨跌统计数据
 * @param 文件名
 * @return
 */
extern XVoid XExpHisSnapshot(XChar *expFile);

/**
 * @brief 导出重构快照行情
 * @param 文件名
 * @return
 */
extern XVoid XExpRSnapshot(XChar *expFile);

/**
 * @brief 导出板块数据
 * @param 文件名
 * @return
 */
extern XVoid XExpBlock(XChar *expFile);

/**
 * @brief 导出板块信息数据
 * @param 文件名
 * @return
 */
extern XVoid XExpBlockInfo(XChar *expFile);

extern XVoid XExpPriceLevel(XChar market, XChar* securityId, XInt speed);

extern XVoid XPLevelPrint(XChar market, XChar* securityId, XInt speed);

extern XVoid XExpSellHold(XChar *expFile);

extern XVoid XPRLevelPrint(XChar market, XChar* securityId, XInt speed);

extern XVoid XRSnapPrint (XChar market, XChar *securityId);
/**
 * @brief 加载K线数据
 *
 * @param trade_file
 * @param knum
 * @return int
 */
extern int read_kline(const char *trade_file, XInt knum);

/**
 * @brief 加载历史成交量数据
 * 
 * @param trade_file 
 * @return int 
 */
extern int read_hissnapshot(const char *trade_file);

/**
 * @brief 读取板块分类数据
 * 
 * @param trade_file     板块文件
 * @return XInt        文件读取是否正常
 */
extern int read_block(const char *trade_file);

/**
 * @brief 读取板块信息数据
 * 
 * @param trade_file     板块文件
 * @return XInt        文件读取是否正常
 */
extern int read_blockinfo(const char *trade_file);

/**
 * @brief 读取可转债转股价格
 * 
 * @param convFile     转股价格文件
 * @return XInt        文件读取是否正常
 */
extern XInt read_stock_convpx(XChar* convFile);



extern XInt XGetUser(const char *userConf, XCustT pCust[]);

extern XVoid XSnapPrint(XInt market, XChar* securityId);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_CORE_XEXPORT_H_ */
