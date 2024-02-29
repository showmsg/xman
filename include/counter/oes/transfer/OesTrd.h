#ifndef INCLUDE_CORE_OESTRD_H_
#define INCLUDE_CORE_OESTRD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Oes2X.h"
#include "XBase.h"

/**
 * @brief OES柜台交易服务进程
 * @param customer
 * @return
 */
extern XVoid XOesTrd(XVoid *param);


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_CORE_OESTRD_H_ */
