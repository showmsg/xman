/*
 * @file Oes2X.c
 * @brief 柜台交易转换
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */
#include "Oes2X.h"
#include "XCom.h"
#include "oes_api/oes_api.h"

inline int accttype_from_oes(int accttype) {
	int acct_type = eXInvSpot;
	switch (accttype) {
	case OES_CASH_TYPE_SPOT:
		acct_type = eXInvSpot;
		break;
	case OES_CASH_TYPE_CREDIT:
		acct_type = eXInvCrd;
		break;
	case OES_CASH_TYPE_OPTION:
		acct_type = eXInvOpt;
		break;
	default:
		break;
	}
	return (acct_type);
}

inline int market_from_oes(int market) {
	int xmkt = -1;

	switch(market)
	{
	case OES_MKT_SH_ASHARE:
		xmkt = eXMarketSha;
		break;
	case OES_MKT_SZ_ASHARE:
		xmkt = eXMarketSza;
		break;
	case OES_MKT_EXT_HK:
		xmkt = eXMarketHK;
		break;
	default:
		break;
	}
	return (xmkt);
}

inline int mktid_to_oes(int xman_mkt)
{

	return ((xman_mkt == eXMarketSha) ? OES_EXCH_SSE : OES_EXCH_SZSE);
}

/**
 * 通过Invest和市场，定位当前需要填充的市场
 */
inline int market_to_oes(int accttype, int market) {
	int imarket = -1;
	if (accttype == eXInvSpot && market == eXMarketSha) {
		imarket = OES_MKT_SH_ASHARE;
	} else if (accttype == eXInvSpot && market == eXMarketSza) {
		imarket = OES_MKT_SZ_ASHARE;
	} else if (accttype == eXInvOpt && market == eXMarketSha) {
		imarket = OES_MKT_SH_OPTION;
	}

	else if (accttype == eXInvOpt && market == eXMarketSza) {
		imarket = OES_MKT_SZ_OPTION;
	}

	return (imarket);
}
inline int sectype_from_oes(int sectype) {
	int security_type = eXStock;
	switch (sectype) {
	case OES_SECURITY_TYPE_STOCK:
		security_type = eXStock;
		break;
	case OES_SECURITY_TYPE_BOND:
		security_type = eXBond;
		break;
	case OES_SECURITY_TYPE_ETF:
		security_type = eXETF;
		break;
	case OES_SECURITY_TYPE_FUND:
		security_type = eXFund;
		break;
	case OES_SECURITY_TYPE_OPTION:
		security_type = eXOpt;
		break;
	default:
		break;
	}

	return (security_type);
}

inline int subsecttype_from_oes(int subsectype) {
	int subsec_type = eXSubDefault;

	switch (subsectype) {
	//可转债、可交换债
	case OES_SUB_SECURITY_TYPE_BOND_CCF:
		subsec_type = eXSubSecCCF;
		break;
	case OES_SUB_SECURITY_TYPE_BOND_GBF:
		subsec_type = eXSubSecGBF;
		break;
	case OES_SUB_SECURITY_TYPE_BOND_CBF:
		subsec_type = eXSubSecCBF;
		break;
	case OES_SUB_SECURITY_TYPE_BOND_EXG:
	case OES_SUB_SECURITY_TYPE_BOND_CPF:
		subsec_type = eXSubSecCPF;
		break;
	case OES_SUB_SECURITY_TYPE_BOND_FBF:
		subsec_type = eXSubSecFBF;
		break;
	case OES_SUB_SECURITY_TYPE_BOND_PRP:
		subsec_type = eXSubSecPRP;
		break;
		//股票
	case OES_SUB_SECURITY_TYPE_STOCK_ASH:
	case OES_SUB_SECURITY_TYPE_STOCK_CDR:
	case OES_SUB_SECURITY_TYPE_STOCK_HLTCDR:
		subsec_type = eXSubSecASH;
		break;
	case OES_SUB_SECURITY_TYPE_STOCK_SME:
		subsec_type = eXSubSecSME;
		break;
		//创业板
	case OES_SUB_SECURITY_TYPE_STOCK_GEM:
	case OES_SUB_SECURITY_TYPE_STOCK_GEMCDR:
		subsec_type = eXSubSecGEM;
		break;
		//科创板
	case OES_SUB_SECURITY_TYPE_STOCK_KSH:
	case OES_SUB_SECURITY_TYPE_STOCK_KCDR:
		subsec_type = eXSubSecKSH;
		break;
		// 单市场股票ETF
	case OES_SUB_SECURITY_TYPE_ETF_SINGLE_MKT:
		subsec_type = eXSubSecSNGLETF;
		break;
		//债券ETF
	case OES_SUB_SECURITY_TYPE_ETF_BOND:
		subsec_type = eXSubSecBNDETF;
		break;
		//跨市场
	case OES_SUB_SECURITY_TYPE_ETF_CROSS_MKT:
		subsec_type = eXSubSecCRSETF;
		break;
		//货币
	case OES_SUB_SECURITY_TYPE_ETF_CURRENCY:
		subsec_type = eXSubSecCRYETF;
		break;
		//黄金
	case OES_SUB_SECURITY_TYPE_ETF_GOLD:
		subsec_type = eXSubSecGLDETF;
		break;
		//跨境
	case OES_SUB_SECURITY_TYPE_ETF_CROSS_BORDER:
		subsec_type = eXSubSecCRFETF;
		break;
		//商品ETF
	case OES_SUB_SECURITY_TYPE_ETF_COMMODITY_FUTURES:
		subsec_type = eXSubSecPRDETF;
		break;
		//LOF
	case OES_SUB_SECURITY_TYPE_FUND_LOF:
	case OES_SUB_SECURITY_TYPE_FUND_GRADED:
		subsec_type = eXSubSecLOF;
		break;
		//开放式基金
	case OES_SUB_SECURITY_TYPE_FUND_OEF:
		subsec_type = eXSubSecOEF;
		break;
	case OES_SUB_SECURITY_TYPE_FUND_CEF:
		subsec_type = eXSubSecCEF;
		break;
	default:
		break;
	}

	return (subsec_type);
}

inline int issuetype_from_oes(int producttype, int issuetype) {
	int issue_type = -1;

	//producttype 为新股、新债(IPO、公开增发);配股配债

	switch (issuetype) {

	case 0:
		//配股配债
		if (producttype == OES_PRODUCT_TYPE_ALLOTMENT) {
			issue_type = eXIssAllot;
		}
		break;
		//新股额度申购
	case OES_ISSUE_TYPE_MKT_QUOTA:
		issue_type = eXIssQuota;
		break;
	case OES_ISSUE_TYPE_CASH:

		//公开增发
		if (producttype == OES_PRODUCT_TYPE_IPO) {
			issue_type = eXIssCash;
		}
		break;
		//新可转债申购
	case OES_ISSUE_TYPE_CREDIT:
		issue_type = eXIssCredit;
		break;
	default:
		break;
	}
	return (issue_type);
}

inline int ordtype_to_oes(int market, int ordtype) {
	int order_type = -1;

	if (market == eXMarketSha) {
		switch (ordtype) {
		case eXOrdLimit:
			order_type = OES_ORD_TYPE_SH_LMT;

			break;
		case eXOrdBest5:
			order_type = OES_ORD_TYPE_SH_MTL_BEST_5;

			break;
		case eXOrdBest:

			order_type = OES_ORD_TYPE_SH_MTL_BEST;

			break;
		case eXOrdBestParty:

			order_type = OES_ORD_TYPE_SH_MTL_SAMEPARTY_BEST;

			break;
		case eXOrdBest5FAK:

			order_type = OES_ORD_TYPE_SH_FAK_BEST_5;

			break;
		case eXOrdLmtFOK:
			order_type = OES_ORD_TYPE_SH_LMT_FOK;

			break;
		case eXOrdMktFOK:

			break;
		case eXOrdFAK:

			break;
		default:
			break;
		}
	} else {
		switch (ordtype) {
		case eXOrdLimit:

			order_type = OES_ORD_TYPE_SZ_LMT;

			break;
		case eXOrdBest5:

			break;
		case eXOrdBest:

			order_type = OES_ORD_TYPE_SZ_MTL_BEST;

			break;
		case eXOrdBestParty:

			order_type = OES_ORD_TYPE_SZ_MTL_SAMEPARTY_BEST;

			break;
		case eXOrdBest5FAK:

			order_type = OES_ORD_TYPE_SZ_FAK_BEST_5;

			break;
		case eXOrdLmtFOK:

			order_type = OES_ORD_TYPE_SZ_LMT_FOK;

			break;
		case eXOrdMktFOK:

			order_type = OES_ORD_TYPE_SZ_FOK;

			break;
		case eXOrdFAK:

			order_type = OES_ORD_TYPE_SZ_FAK;

			break;
		default:
			break;
		}
	}
	return (order_type);
}

inline int ordtype_from_oes(int ordtype) {
	int order_type = -1;

	switch (ordtype) {
	case OES_ORD_TYPE_LMT:
		order_type = eXOrdLimit;
		break;
	case OES_ORD_TYPE_SH_MTL_BEST_5:
		order_type = eXOrdBest5;
		break;
	case OES_ORD_TYPE_MTL_BEST:
		order_type = eXOrdBest;
		break;

	case OES_ORD_TYPE_MTL_SAMEPARTY_BEST:
		order_type = eXOrdBestParty;
		break;
	case OES_ORD_TYPE_FAK_BEST_5:
		order_type = eXOrdBest5FAK;
		break;
	case OES_ORD_TYPE_SHOPT_LMT_FOK:
		order_type = eXOrdLmtFOK;
		break;
	case OES_ORD_TYPE_SZ_FOK:
		order_type = eXOrdMktFOK;
		break;
	case OES_ORD_TYPE_SZ_FAK:
		order_type = eXOrdFAK;
		break;

	default:
		break;
	}

	return (order_type);
}

inline int bstype_to_oes(int accttype, int producttype, int bstype) {
	int bs_type = -1;

	/** 现货 & 股票 & 买卖 */
	if (accttype == eXInvSpot && producttype == eXEquity && bstype == eXBuy) {
		bs_type = OES_BS_TYPE_BUY;
	} else if (accttype == eXInvSpot && producttype == eXEquity
			&& bstype == eXSell) {
		bs_type = OES_BS_TYPE_SELL;
	}

	/** 现货 & IPO & 申购 */
	else if (accttype == eXInvSpot && producttype == eXIPO
			&& bstype == eXDeem) {
		bs_type = OES_BS_TYPE_SUBSCRIPTION;
	}
	/** 现货 & 公开增发 */
	else if (accttype == eXInvSpot && producttype == eXIPO && bstype == eXBuy) {
		bs_type = OES_BS_TYPE_SUBSCRIPTION;
	}
	/** 现货 & 配股 & 卖 */
	else if (accttype == eXInvSpot && producttype == eXAllot
			&& bstype == eXSell) {
		bs_type = OES_BS_TYPE_ALLOTMENT;
	}
	/** 现货 & 股票 & 信用卖  = 债券质押式回购-逆回购*/
	else if (accttype == eXInvSpot && producttype == eXBondPR
			&& bstype == eXCSell) {
		bs_type = OES_BS_TYPE_CREDIT_SELL;
	}
	/** 申购 */
	else if (bstype == eXDeem) {
		bs_type = OES_BS_TYPE_CREATION;
	}

	/** 赎回 */
	else if (bstype == eXRedeem) {
		bs_type = OES_BS_TYPE_REDEMPTION;
	}

	return (bs_type);
}
inline int iscancel_from_oes(int bstype) {
	if (OES_BS_TYPE_CANCEL == bstype) {
		return (1);
	}
	return (0);
}
inline int bstype_from_oes(int bstype) {
	int bs_type = -1;

	switch (bstype) {
	//买
	case OES_BS_TYPE_BUY:
		bs_type = eXBuy;
		break;
		//卖
	case OES_BS_TYPE_SELL:
		bs_type = eXSell;
		break;
		//新股、新债、申购
	case OES_BS_TYPE_SUBSCRIPTION:
	case OES_BS_TYPE_CREATION:
		bs_type = eXDeem;
		break;
		//赎回
	case OES_BS_TYPE_REDEMPTION:
		bs_type = eXRedeem;
		break;
		//配股、配债
	case OES_BS_TYPE_ALLOTMENT:
		bs_type = eXSell;
		break;
		//逆回购
	case OES_BS_TYPE_CREDIT_SELL:
		bs_type = eXCSell;
		break;
	default:
		break;
	}

	return (bs_type);
}

inline int producttype_from_oes(int producttype, int subsectype) {
	int product_type = eXEquity;

	if (subsectype == OES_SUB_SECURITY_TYPE_BOND_PRP) {
		product_type = eXBondPR;
	} else {
		switch (producttype) {
		case OES_PRODUCT_TYPE_EQUITY:
			product_type = eXEquity;
			break;
		case OES_PRODUCT_TYPE_IPO:
			product_type = eXIPO;
			break;
		case OES_PRODUCT_TYPE_ALLOTMENT:
			product_type = eXAllot;
			break;
		case OES_PRODUCT_TYPE_OPTION:
			product_type = eXOption;
			break;
		default:
			break;
		}
	}
	return (product_type);
}
inline int ordstatus_from_oes(int ordstatus) {
	int order_status = eXOrdStatusNew;
	switch (ordstatus) {
	case OES_ORD_STATUS_NEW:
		order_status = eXOrdStatusDeclared;
		break;
	case OES_ORD_STATUS_DECLARED:
	case OES_ORD_STATUS_CANCEL_DONE:
		order_status = eXOrdStatusDeclared;
		break;
	case OES_ORD_STATUS_PARTIALLY_FILLED:
		order_status = eXOrdStatusPFilled;
		break;
	case OES_ORD_STATUS_PARTIALLY_CANCELED:
		order_status = eXOrdStatusPCanceled;
		break;

	case OES_ORD_STATUS_CANCELED:
		order_status = eXOrdStatusCanceled;
		break;
	case OES_ORD_STATUS_FILLED:
		order_status = eXOrdStatusFilled;
		break;
	case OES_ORD_STATUS_INVALID_OES:
	case OES_ORD_STATUS_INVALID_EXCHANGE:
	case OES_ORD_STATUS_INVALID_TGW_REJECT:
	case OES_ORD_STATUS_INVALID_TGW_COMM:
	case OES_ORD_STATUS_INVALID_TGW_TRY_AGAIN:
		order_status = eXOrdStatusInvalid;
		break;
	default:
		order_status = eXOrdStatusNew;
		break;
	}

	return (order_status);
}

inline int secstatus_from_oes(int securityStatus, int suspFlag, int temporarySuspFlag) {
	int secstatus = eXSecTrading;

	//停牌
	if (suspFlag != 0 || temporarySuspFlag != 0) {
		secstatus = eXSecPause;
	} else {

		if (OES_SECURITY_STATUS_NONE
				== (securityStatus & OES_SECURITY_STATUS_NONE)) {
			secstatus = eXSecTrading;
		} else if ((securityStatus & OES_SECURITY_STATUS_FIRST_LISTING)
				== OES_SECURITY_STATUS_FIRST_LISTING
				|| (securityStatus & OES_SECURITY_STATUS_RESUME_FIRST_LISTING)
						== OES_SECURITY_STATUS_RESUME_FIRST_LISTING) {

			secstatus = eXSecFirstTrading;
		} else if ((securityStatus & OES_SECURITY_STATUS_NEW_LISTING)
				== OES_SECURITY_STATUS_NEW_LISTING) {
			secstatus = eXSecNewList;
		} else if ((securityStatus & OES_SECURITY_STATUS_EXCLUDE_RIGHT)
				== OES_SECURITY_STATUS_EXCLUDE_RIGHT
				|| (securityStatus & OES_SECURITY_STATUS_EXCLUDE_DIVIDEN)
						== OES_SECURITY_STATUS_EXCLUDE_DIVIDEN) {
			secstatus = eXSecDiv;
		} else if ((securityStatus & OES_SECURITY_STATUS_SUSPEND)
				== OES_SECURITY_STATUS_SUSPEND) {
			secstatus = eXSecPause;
		} else if ((securityStatus & OES_SECURITY_STATUS_SPECIAL_TREATMENT)
				== OES_SECURITY_STATUS_SPECIAL_TREATMENT
				|| (securityStatus & OES_SECURITY_STATUS_X_SPECIAL_TREATMENT)
						== OES_SECURITY_STATUS_X_SPECIAL_TREATMENT) {
			secstatus = eXSecST;
		} else if ((securityStatus & OES_SECURITY_STATUS_DELIST_PERIOD)
				== OES_SECURITY_STATUS_DELIST_PERIOD
				|| (securityStatus & OES_SECURITY_STATUS_DELIST_TRANSFER)
						== OES_SECURITY_STATUS_DELIST_TRANSFER) {
			secstatus = eXSecDList;
		} else {
			secstatus = eXSecTrading;
		}
	}

	return (secstatus);
}
