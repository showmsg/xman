/*
 * Mds2X.h
 *
 *  Created on: 2022年4月1日
 *      Author: kyle
 */

#ifndef INCLUDE_CORE_MDS2X_H_
#define INCLUDE_CORE_MDS2X_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "XTypes.h"

#define XMDS_CONFIG_FILE   "../conf/mds_client.conf"

/**
 * @brief 把mds的市场转为XMan对应市场
 * @param mds_mkt
 * @return
 */
extern int market_from_mds(int mds_mkt);

/**
 * @brief 把mds的市场状态转为XMan的市场状态
 * @param xmkt
 * @param tradingphase
 * @return
 */
extern XChar mktstatus_from_mds(int xmkt, const char tradingphase[]);

/**
 * @brief 把mds的证券状态转为XMan的证券状态
 * @param xmkt
 * @param tradingphase
 * @return
 */
extern XChar secstatus_from_mds(int xmkt, const char tradingphase[]);

/**
 * @brief 把mds的买卖类型转为XMan的买卖类型
 * @param bsType
 * @return
 */
extern XChar bs_from_mds(char bsType);

/**
 * @brief 把mds的订单类型转为XMan的订单类型
 * @param ordType
 * @return
 */
extern XChar ordtype_from_mds(char ordType);

/**
 * @brief mds执行状态转换
 * @param execType
 * @return
 */
extern XChar exectype_from_mds(char execType);

/**
 * @brief mds产品类型转换
 * @param producttype
 * @return
 */
extern XChar prodtype_from_mds(int producttype);

/**
 * @brief 上海通过委托转换是否为撤单
 * @param market
 * @param ordType
 * @return
 */
XBool cancel_from_order(int market, char ordType);

/**
 * @brief 深圳通过成交转换是否为撤单
 * @param exeType
 * @return
 */
XBool cancel_from_trade(int exeType);

#ifdef __cplusplus
}
#endif
#endif /* INCLUDE_CORE_MDS2X_H_ */
