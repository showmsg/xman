/*
 * XSignals.h
 *
 *  Created on: 2024年6月27日
 *      Author: Administrator
 */

#ifndef INCLUDE_FRAME_XSIGNALS_H_
#define INCLUDE_FRAME_XSIGNALS_H_

#include "XPlot.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void ZsAmtAucBuySignal(XSnapshotT *pSnapshot, XStockT *pStock, XSessioManageT *pSessionMan);

extern void ZsAmtConBuySignal(XRSnapshotT *snapshot, XStockT *pStock, XSessioManageT *pSessionMan);


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_FRAME_XSIGNALS_H_ */
