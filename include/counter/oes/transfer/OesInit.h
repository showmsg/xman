
#ifndef INCLUDE_CORE_OESINIT_H_
#define INCLUDE_CORE_OESINIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Oes2X.h"
#include "XBase.h"

/**
 * @brief OES柜台初始化
 * @param customer
 * @return
 */
extern XInt XOesInit(char* customer);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_CORE_OESINIT_H_ */