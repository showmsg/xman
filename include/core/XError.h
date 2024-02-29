/**
 * @file XError.h
 * @version 0.10.0 2022/3/15
 * 			- 初始版本
 * @since 2022/3/15
 */

#ifndef INCLUDE_XMAN_XERROR_H_
#define INCLUDE_XMAN_XERROR_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**< 未找到客户号 */
#define XCUST_NOT_FOUND        5000

/**< 未找到股东账户 */
#define XINVEST_NOT_FOUND      5001

/**< 委托数量不正确 */
#define XORDQTY_IS_INVALID     5002

/**< 委托价格不正确 */
#define XORDPRICE_IS_INVALID   5003

/**< 冻结资金错误 */
#define XFROZEN_MONEY_ERROR    5004

/**< 冻结持仓错误 */
#define XFROZEN_HOLD_ERROR     5005

/**< 资金不够 */
#define XEMONEY_IS_NOT_ENOUGH 1216

/**< 持仓不够 */
#define XHOLD_IS_NOT_ENOUGH   1217

#define XORDER_IS_INVALID     1225

#define XORDER_IS_REJECT     1024

/**< */
#define XORDER_CTRL_DUPLICATE 1227
#ifdef __cplusplus
}
#endif

#endif

