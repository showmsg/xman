/*
 * convfromoes.h
 *
 *  Created on: 2020年5月7日
 *      Author: kangyq
 */

#ifndef SRC_CONVFROMOES_H_
#define SRC_CONVFROMOES_H_

#ifdef __cplusplus
extern "C" {
#endif

#define XOES_CONFIG_FILE   "../conf/oes_client.conf"

/**
 * @brief oes账户类型转为XMan
 * @param accttype
 * @return
 */
extern int accttype_from_oes(int accttype);

/**
 * @brief oes市场转为XMan
 * @param market
 * @return
 */
extern int market_from_oes(int market);

/**
 *
 */
extern int mktid_to_oes(int xman_mkt);
/**
 * @brief XMan市场转为oes
 * @param accttype
 * @param market
 * @return
 */
extern int market_to_oes(int accttype, int market);

/**
 * @brief oes证券类型转为xman
 * @param sectype
 * @return
 */
extern int sectype_from_oes(int sectype);

/**
 * @brief oes证券子类型转为xman
 * @param subsectype
 * @return
 */
extern int subsecttype_from_oes(int subsectype);

/**
 * @brief oes发行类型转为xman
 * @param producttype
 * @param issuetype
 * @return
 */
extern int issuetype_from_oes(int producttype, int issuetype);

/**
 * @brief xman订单类型转为oes
 * @param market
 * @param ordtype
 * @return
 */
extern int ordtype_to_oes(int market, int ordtype);

/**
 * @brief oes订单类型转为xman
 * @param ordtype
 * @return
 */
extern int ordtype_from_oes(int ordtype);

/**
 * @brief oes的产品类型转为xman
 * @param producttype
 * @param subsectype
 * @return
 */
extern int producttype_from_oes(int producttype, int subsectype);

/**
 * @brief xman买卖转为oes
 * @param accttype
 * @param producttype
 * @param bstype
 * @return
 */
extern int bstype_to_oes(int accttype, int producttype, int bstype);

/**
 * @brief xman是否撤单
 * @param bstype
 * @return
 */
extern int iscancel_from_oes(int bstype);

/**
 * @brief oes的买卖类型转为xman
 * @param bstype
 * @return
 */
extern int bstype_from_oes(int bstype);

/**
 * @brief oes订单状态转为xman
 * @param ordstatus
 * @return
 */
extern int ordstatus_from_oes(int ordstatus);

/**
 * @brief oes证券状态转为xman
 * @param securityStatus
 * @param suspFlag
 * @param temporarySuspFlag
 * @return
 */
extern int secstatus_from_oes(int securityStatus, int suspFlag, int temporarySuspFlag);

#ifdef __cplusplus
}
#endif

#endif /* SRC_CONVFROMOES_H_ */
