/*
 * @file Mds2X.c
 * @brief 柜台行情转换
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */
#include "Mds2X.h"
#include "XDataStruct.h"
#include "mds_api/mds_api.h"

inline int market_from_mds(int mds_mkt) {
	return ((mds_mkt == MDS_EXCH_SSE) ? eXMarketSha : eXMarketSza);
}

inline XChar mktstatus_from_mds(int xmkt, const char tradingphase[]) {
	XChar market_status = eXMktConAuct;
	if (eXMarketSha == xmkt) {
		if (tradingphase[0] == 'T') {
			market_status = eXMktConAuct;
		} else if (tradingphase[0] == 'S') {
			market_status = eXMktPreOpen;
		} else if (tradingphase[0] == 'C') {
			market_status = eXMktCallAuct;
		} else if (tradingphase[0] == 'B') {
			market_status = eXMktPause;
		} else if (tradingphase[0] == 'E') {
			market_status = eXMktClosing;
		}
	} else if (eXMarketSza == xmkt) {
		if (tradingphase[0] == 'T') {
			market_status = eXMktConAuct;
		} else if (tradingphase[0] == 'S') {
			market_status = eXMktPreOpen;
		} else if (tradingphase[0] == 'O' || tradingphase[0] == 'C') {
			market_status = eXMktCallAuct;
		} else if (tradingphase[0] == 'B') {
			market_status = eXMktPause;
		} else if (tradingphase[0] == 'E') {
			market_status = eXMktClosing;
		}
	}
	return (market_status);
}

inline XChar secstatus_from_mds(int xmkt, const char tradingphase[]) {
	XChar security_status = eXSecPause;
	if (eXMarketSha == xmkt) {
		if (tradingphase[1] == '1') {
			security_status = eXSecTrading;
		} else if (tradingphase[0] == 'P') {
			security_status = eXSecPause;
		} else if (tradingphase[1] == '0') {
			security_status = eXSecTmpPause;
		} else if (tradingphase[3] == '0') {
			security_status = eXSecTmpPause;
		}
	} else if (eXMarketSza == xmkt) {
		if (tradingphase[1] == 0) {
			security_status = eXSecTrading;
		} else if (tradingphase[0] == 'H') {
			security_status = eXSecTmpPause;
		} else if (tradingphase[1] == 1) {
			security_status = eXSecPause;
		}
	}
	return (security_status);
}

inline XChar bs_from_mds(char bsType) {
	XChar iret = eXBuy;

	switch (bsType) {
	case MDS_L2_ORDER_SIDE_BUY:
	case MDS_L2_ORDER_SIDE_BORROW:
		iret = eXBuy;
		break;
	case MDS_L2_ORDER_SIDE_SELL:
	case MDS_L2_ORDER_SIDE_LEND:
		iret = eXSell;
		break;
	default:
		break;
	}

	return (iret);
}
inline XChar ordtype_from_mds(char ordType) {
	XChar iret = eXOrdLimit;
	switch (ordType) {
	/** 限价 */
	case MDS_L2_ORDER_TYPE_LMT:
		iret = eXOrdLimit;
		break;
		/** 市价 */
	case MDS_L2_ORDER_TYPE_MKT:
		iret = eXOrdMkt;
		break;
		/** 本方最优 */
	case MDS_L2_ORDER_TYPE_SAMEPARTY_BEST:
		iret = eXOrdSelf;
		break;
		/** 上海新增订单 */
	case MDS_L2_SSE_ORDER_TYPE_ADD:
		iret = eXOrdAdd;
		break;
		/** 上海订单撤销 */
	case MDS_L2_SSE_ORDER_TYPE_DELETE:
		iret = eXOrdDel;
		break;
	default:
		break;
	}
	return (iret);
}

inline XChar exectype_from_mds(char execType) {
	XChar iret = eXL2ExecCancel;
	switch (execType) {
	case MDS_L2_TRADE_EXECTYPE_CANCELED:
		iret = eXL2ExecCancel;
		break;
	case MDS_L2_TRADE_EXECTYPE_TRADE:
		iret = eXL2ExecTrade;
		break;
	default:
		break;
	}

	return (iret);
}

inline XChar prodtype_from_mds(int producttype) {
	XChar iret = -1;
	switch (producttype) {
	case MDS_MD_PRODUCT_TYPE_STOCK:
		break;
	case MDS_MD_PRODUCT_TYPE_INDEX:
		break;
	case MDS_MD_PRODUCT_TYPE_OPTION:
		break;
	default:
		break;
	}
	return (iret);
}

inline XBool cancel_from_order(int market, char ordType) {
	if (market == MDS_EXCH_SSE && ordType == MDS_L2_SSE_ORDER_TYPE_DELETE) {
		return (true);
	}
	return (false);
}

inline XBool cancel_from_trade(int exeType) {
	if (exeType == MDS_L2_TRADE_EXECTYPE_CANCELED) {
		return (true);
	}
	return (false);
}

