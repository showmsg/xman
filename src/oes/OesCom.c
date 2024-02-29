/*
 * @file Ordeng.c
 * @brief 订单处理
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */

#include "OesCom.h"
#include "XDataType.h"

XInt is_trade_period(XShortTime time, XInt period) {
	XInt iret = -1;

	switch (period) {
	case eXMktPreOpen:
		if (time < XTRADE_PREOPEN_TIME) {
			iret = 0;
		}
		break;
	case eXMktCallAuct:
		if (time >= XTRADE_PREOPEN_TIME && time <= XTRADE_CALLAUCT_TIME) {
			iret = 0;
		}
		break;
	case eXMktPause:
		if ((time > XTRADE_CALLAUCT_TIME && time < XTRADE_CONAUCT_TIME)
				|| (time > XTRADE_NOONBRK_TIME && time < XTRADE_AFTERAUCT_TIME)) {
			iret = 0;
		}
		break;

	case eXMktConAuct:
		if ((time >= XTRADE_CONAUCT_TIME && time <= XTRADE_NOONBRK_TIME)
				|| (time >= XTRADE_AFTERAUCT_TIME && time < XTRADE_CLOSING_TIME)) {
			iret = 0;
		}
		break;
	case eXMktClosing:
		if (time >= XTRADE_CLOSING_TIME && time < XTRADE_CLOSED_TIME) {
			iret = 0;
		}
		break;
	case eXMktClosed:
		if (time >= XTRADE_CLOSED_TIME) {
			iret = 0;
		}
		break;
	default:
		break;
	}

	return (iret);
}

