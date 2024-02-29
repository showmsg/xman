/***********************************************************************
 *	@company	上海全创信息科技有限公司
 *	@file		demo_trade_stock.cpp
 *	@brief		traderapi demo
 *	@history	2022-8-26
 *	@author		n-sight
 *
 *	Windows：	1.请确认包.h .cpp 以及 .lib 文件都在相同目录；或者VS项目配置属性中【附加包含目录】以及【附加库目录】和【附加项依赖】正确设置相关路径
 *				2.预处理器定义 _CRT_SECURE_NO_WARNINGS ;
 *				3.使用VS2013以上版本编译通过
 *
 *	Linux：		1.库文件和头文件在同一目录下时， g++ demo_trade_stock.cpp -o demo -L. -lfasttraderapi
 *				2.当库文件和源文件不在同一目录时，请注意相应路径的不同
 3.运行时若找不到动态库，可export $LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
 ***********************************************************************/
#include "CtpInit.h"

#include "TORATstpTraderApi.h"
#include <stdio.h>
#include <string.h>
#include "XLog.h"
#include "XCom.h"

#ifdef WINDOWS
	#pragma comment(lib,"fasttraderapi.lib")
#endif

using namespace std;
using namespace TORASTOCKAPI;

////投资者账户
//const char *InvestorID = "00030557";
///*该默认账号为共用连通测试使用，自有测试账号请到n-sight.com.cn注册并从个人中心获取交易编码，不是网站登录密码，不是手机号
// 实盘交易时，取客户号，请注意不是资金账号
// 或咨询金科顾问*/
//
////操作员账户
//const char *UserID = "00030557";	   //同客户号保持一致即可
//
////资金账户
//const char *AccountID = "00030557";		//以Req(TradingAccount)查询的为准
//
////上海交易所股东账号
//const char *SH_ShareHolderID = "A00030557";	//以Req(ShareholderAccount)查询的为准
//
////登陆密码
//const char *Password = "17522830";		//N视界注册模拟账号的交易密码，不是登录密码
//
//const char *DepartmentID = "0003";		//默认客户号的前4位

class DemoTradeSpi: public CTORATstpTraderSpi {
public:
	DemoTradeSpi(CTORATstpTraderApi *api) {
		m_api = api;
		m_req_id = 1;
		bEnd = false;
	}

	~DemoTradeSpi() {

	}
	void SetLoginInfo(CTORATstpReqUserLoginField *login) {
		memset(&m_loginInfo, 0, sizeof(CTORATstpReqUserLoginField));
		memcpy(&m_loginInfo, login, sizeof(CTORATstpReqUserLoginField));
	}
private:
	virtual void OnFrontConnected() {
		slog_info(0, "TradeApi OnFrontConnected");

		// 获取终端信息
		int ret = m_api->ReqGetConnectionInfo(m_req_id++);
		if (ret != 0) {
			slog_error(0, "ReqGetConnectionInfo fail, ret[%d]", ret);
		}
	}

	virtual void OnFrontDisconnected(int nReason) {
		slog_error(3, "TradeApi OnFrontDisconnected: [%d]", nReason);
	}

	virtual void OnRspGetConnectionInfo(
			CTORATstpConnectionInfoField *pConnectionInfoField,
			CTORATstpRspInfoField *pRspInfo, int nRequestID) {
		if (pRspInfo->ErrorID == 0) {
			slog_info(0,
					"inner_ip_address[%s] inner_port[%d] outer_ip_address[%s] outer_port[%d] mac_address[%s]",
					pConnectionInfoField->InnerIPAddress,
					pConnectionInfoField->InnerPort,
					pConnectionInfoField->OuterIPAddress,
					pConnectionInfoField->OuterPort,
					pConnectionInfoField->MacAddress);

			int ret = m_api->ReqUserLogin(&m_loginInfo, m_req_id++);
			if (ret != 0) {
				slog_error(0, "TradeApi ReqUserLogin fail, ret[%d]", ret);
			}
		} else {
			slog_error(3,
					"get connection info fail! error_id[%d] error_msg[%s]",
					pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		}
	}

	void OnRspUserLogin(CTORATstpRspUserLoginField *pRspUserLoginField,
			CTORATstpRspInfoField *pRspInfo, int nRequestID) {
		if (pRspInfo->ErrorID == 0) {
			slog_info(0, "TradeApi OnRspUserLogin: OK! [%d]", nRequestID);

			m_front_id = pRspUserLoginField->FrontID;
			m_session_id = pRspUserLoginField->SessionID;

			// 查询合约
			CTORATstpQrySecurityField field;
			memset(&field, 0, sizeof(field));

			// 以下字段不填表示不设过滤条件，即查询全部合约
//			field.ExchangeID = TORA_TSTP_EXD_SSE;
//			strcpy(field.SecurityID, "600000");

			int ret = m_api->ReqQrySecurity(&field, m_req_id++);
			if (ret != 0) {
				slog_error(0, "ReqQrySecurity fail, ret[%d]\n", ret);
			}
			return;

		}
	}

	virtual void OnRspUserPasswordUpdate(
			CTORATstpUserPasswordUpdateField *pUserPasswordUpdateField,
			CTORATstpRspInfoField *pRspInfo, int nRequestID) {
		if (pRspInfo->ErrorID == 0) {
			printf("OnRspUserPasswordUpdate: OK! [%d]\n", nRequestID);
		} else {
			printf("OnRspUserPasswordUpdate: Error! [%d] [%d] [%s]\n",
					nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		}
	}

	virtual void OnRspOrderInsert(CTORATstpInputOrderField *pInputOrderField,
			CTORATstpRspInfoField *pRspInfo, int nRequestID) {
		if (pRspInfo->ErrorID == 0) {
			printf("OnRspOrderInsert: OK! [%d] [%d] [%s]\n", nRequestID,
					pInputOrderField->OrderRef, pInputOrderField->OrderSysID);
		} else {
			printf("OnRspOrderInsert: Error! [%d] [%d] [%s]\n", nRequestID,
					pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		}
	}

	virtual void OnRspOrderAction(
			CTORATstpInputOrderActionField *pInputOrderActionField,
			CTORATstpRspInfoField *pRspInfo, int nRequestID) {
		if (pRspInfo->ErrorID == 0) {
			printf("OnRspOrderAction: OK! [%d] [%d] [%s] \n", nRequestID,
					pInputOrderActionField->OrderActionRef,
					pInputOrderActionField->CancelOrderSysID);
		} else {
			printf("OnRspOrderAction: Error! [%d] [%d] [%s]\n", nRequestID,
					pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		}
	}

	virtual void OnRspInquiryJZFund(
			CTORATstpRspInquiryJZFundField *pRspInquiryJZFundField,
			CTORATstpRspInfoField *pRspInfo, int nRequestID) {
		if (pRspInfo->ErrorID == 0) {
			printf("OnRspInquiryJZFund: OK! [%d] [%.2f] [%.2f]\n", nRequestID,
					pRspInquiryJZFundField->UsefulMoney,
					pRspInquiryJZFundField->FetchLimit);
		} else {
			printf("OnRspInquiryJZFund: Error! [%d] [%d] [%s]\n", nRequestID,
					pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		}
	}

	virtual void OnRspTransferFund(
			CTORATstpInputTransferFundField *pInputTransferFundField,
			CTORATstpRspInfoField *pRspInfo, int nRequestID) {
		if (pRspInfo->ErrorID == 0) {
			printf("OnRspTransferFund: OK! [%d] [%d]", nRequestID,
					pInputTransferFundField->ApplySerial);
		} else {
			printf("OnRspTransferFund: Error! [%d] [%d] [%s]\n", nRequestID,
					pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		}
	}

	virtual void OnRtnTransferFund(CTORATstpTransferFundField *pTransferFund) {

	}

	virtual void OnRtnOrder(CTORATstpOrderField *pOrder) {

	}

	virtual void OnRtnTrade(CTORATstpTradeField *pTrade) {

	}

	virtual void OnRtnMarketStatus(CTORATstpMarketStatusField *pMarketStatus) {

	}

	int market_from_ctp(TTORATstpMarketIDType mk) {
		int market = 0;
		switch (mk) {
		case TORA_TSTP_MKD_SHA:
			market = eXMarketSha;
			break;
		case TORA_TSTP_MKD_SZA:
			market = eXMarketSza;
			break;
		default:
			break;
		}
		return market;
	}

	int subsectype_from_ctp(TTORATstpSecurityTypeType typ) {
		int sectype = 0;

		switch (typ) {
		//上海A股
		case TORA_TSTP_STP_SHAShares:
			sectype = eXSubSecASH;
			break;
			//深圳主板
		case TORA_TSTP_STP_SZMainAShares:
			sectype = eXSubSecSME;
			break;
			///深圳创业板
		case TORA_TSTP_STP_SZGEM:
			break;
			///深圳中小企业板
		case TORA_TSTP_STP_SZSME:
			sectype = eXSubSecSME;
			break;
			////科创板产品（上市后前5个交易日）
		case TORA_TSTP_STP_SHKC:
			sectype = eXSubSecKSH;
			break;
			///上海科创板存托凭证
		case TORA_TSTP_STP_SHKCCDR:
			sectype = eXSubSecKCDR;
			break;
			///上海存托凭证

		case TORA_TSTP_STP_SHCDR:
			///深圳主板、中小板创新企业股票或存托凭证
		case TORA_TSTP_STP_SZCDR:
			///深圳创业板创新企业股票或存托凭证(注册制)
		case TORA_TSTP_STP_SZGEMCDR:
			sectype = eXSubSecCDR;
			break;

			///上海可转债
		case TORA_TSTP_STP_SHBondConversion:

			///深圳可转债
		case TORA_TSTP_STP_SZBondConversion:
			///上海科创板可转债
		case TORA_TSTP_STP_SHKCBondConversion:
			///深圳创业板可转债(注册制)
		case TORA_TSTP_STP_SZGEMBondConversionReg:
			sectype = eXSubSecCCF;
			break;
			///上海质押式回购
		case TORA_TSTP_STP_SHRepo:
			///深圳质押式回购
		case TORA_TSTP_STP_SZRepo:
			sectype = eXSubSecPRP;
			break;
		default:
			break;
		}

		return sectype;
	}

	int sectype_from_ctp(TTORATstpProductIDType typ) {
		int sectype = 0;
		switch (typ) {
		///上海股票
		case TORA_TSTP_PID_SHStock:
			///深圳股票
		case TORA_TSTP_PID_SZStock:
			///上海科创板
		case TORA_TSTP_PID_SHKC:
			sectype = eXStock;
			break;
			///上海基金
		case TORA_TSTP_PID_SHFund:
			///深圳基金
		case TORA_TSTP_PID_SZFund:
			sectype = eXFund;
			break;
			///上海债券
		case TORA_TSTP_PID_SHBond:
			///深圳债券
		case TORA_TSTP_PID_SZBond:
			///上海质押式回购
		case TORA_TSTP_PID_SHRepurchase:
			///深圳质押式回购
		case TORA_TSTP_PID_SZRepurchase:
			sectype = eXBond;
			break;
		default:
			break;
		}
		return sectype;
	}
	int prdtype_from_ctp(TTORATstpProductIDType typ) {
		int prdtype = 0;

		switch (typ) {
		///上海股票
		case TORA_TSTP_PID_SHStock:
			///深圳股票
		case TORA_TSTP_PID_SZStock:
			///上海科创板
		case TORA_TSTP_PID_SHKC:
			///上海债券
		case TORA_TSTP_PID_SHBond:
			///深圳债券
		case TORA_TSTP_PID_SZBond:
			///上海基金
		case TORA_TSTP_PID_SHFund:
			///深圳基金
		case TORA_TSTP_PID_SZFund:
			prdtype = eXEquity;
			break;

			///上海质押式回购
		case TORA_TSTP_PID_SHRepurchase:
			///深圳质押式回购
		case TORA_TSTP_PID_SZRepurchase:
			prdtype = eXBondPR;
			break;
		default:
			break;
		}
		return prdtype;
	}
	int secstatus_from_ctp(TTORATstpSecurityStatusType status) {
		int secstaus = eXSecTrading;
		switch (status) {
		///停牌
		case 0x1:
			///长期停牌
		case 0x40:
			secstaus = eXSecPause;
			break;
			///除权
		case 0x2:
			///除息
		case 0x4:
			secstaus = eXSecDiv;
			break;
			///St
		case 0x8:
			///*st
		case 0x10:
			secstaus = eXSecST;
			break;
			///上市首日
		case 0x20:
			///恢复上市首日
		case 0x20000:
			secstaus = eXSecFirstTrading;
			break;

		default:
			break;
		}
		return secstaus;
	}
	virtual void OnRspQrySecurity(CTORATstpSecurityField *pSecurity,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		if (pSecurity) {
			memset(&stock, 0, sizeof(XStockT));

			stock.market = market_from_ctp(pSecurity->MarketID);
			memcpy(stock.securityId, pSecurity->SecurityID, 6);

			pStock = XFndVStockByKey(stock.market, stock.securityId);
			if (NULL != pStock) {
				memcpy(&stock, pStock, sizeof(XStockT));
			}

			memcpy(stock.securityName, pSecurity->ShortSecurityName,
					strlen(pSecurity->ShortSecurityName));
			if (pSecurity->UnderlyingSecurityID) {
				memcpy(stock.baseSecurityId, pSecurity->UnderlyingSecurityID,
						strlen(pSecurity->UnderlyingSecurityID));
			}
			stock.secType = sectype_from_ctp(pSecurity->ProductID);
			stock.subSecType = subsectype_from_ctp(pSecurity->SecurityType);

			stock.prdType = prdtype_from_ctp(pSecurity->ProductID);

			stock.secStatus = secstatus_from_ctp(pSecurity->SecurityStatus);

			stock.preClose = pSecurity->PreClosePrice * 10000;
			stock.priceTick = pSecurity->PriceTick * 10000;

			stock.lmtBuyMinQty = pSecurity->MinFixPriceOrderBuyVolume;
			stock.lmtSellMinQty = pSecurity->MinLimitOrderSellVolume;
			stock.lmtBuyMaxQty = pSecurity->MaxFixPriceOrderBuyVolume;
			stock.lmtSellMaxQty = pSecurity->MaxFixPriceOrderSellVolume;
			stock.buyUnit = pSecurity->VolumeMultiple;  //TODO

			stock.upperPrice = pSecurity->UpperLimitPrice * 10000;
			stock.lowerPrice = pSecurity->LowerLimitPrice * 10000;

			stock.outstandingShare = pSecurity->TotalEquity;
			stock.publicfloatShare = pSecurity->CirculationEquity;

			XPutOrUpdVStockByKey(&stock);
			slog_debug(0, "[%d-%s],len=%d,[%s]", stock.market, stock.securityId, strlen(pSecurity->ShortSecurityName), pSecurity->ShortSecurityName);
		}
		if (bIsLast) {
			slog_info(0, "查询合约结束[%d] ErrorID[%d] ", nRequestID, pRspInfo->ErrorID);

			// 查询股东账户
			CTORATstpQryShareholderAccountField field_sa;
			memset(&field_sa, 0, sizeof(field_sa));

			// 以下字段不填表示不设过滤条件，即查询所有股东账号
			//field.ExchangeID = TORA_TSTP_EXD_SSE;

			int ret_sa = m_api->ReqQryShareholderAccount(&field_sa, m_req_id++);
			if (ret_sa != 0) {
				slog_error(0, "ReqQryShareholderAccount fail, ret[%d]", ret_sa);
			}

		}
	}

	virtual void OnRspQryInvestor(CTORATstpInvestorField *pInvestor,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		if (pInvestor) {
			printf(
					"OnRspQryInvestor[%d]: InvestorID[%s] InvestorName[%s] Operways[%s]\n",
					nRequestID, pInvestor->InvestorID, pInvestor->InvestorName,
					pInvestor->Operways);

		}

		if (bIsLast) {
			printf("查询投资者结束[%d] ErrorID[%d] ErrorMsg[%s]\n", nRequestID,
					pRspInfo->ErrorID, pRspInfo->ErrorMsg);
#if 1
			// 查询股东账户
			CTORATstpQryShareholderAccountField field_sa;
			memset(&field_sa, 0, sizeof(field_sa));

			// 以下字段不填表示不设过滤条件，即查询所有股东账号
			//field.ExchangeID = TORA_TSTP_EXD_SSE;

			int ret_sa = m_api->ReqQryShareholderAccount(&field_sa, m_req_id++);
			if (ret_sa != 0) {
				slog_error(0, "ReqQryShareholderAccount fail, ret[%d]", ret_sa);
			}

#endif

		}
	}
	int acctype_from_ctp(TTORATstpShareholderIDTypeType typ) {
		int acctype = 0;

		switch(typ)
		{
		case TORA_TSTP_SIDT_Normal:
			acctype = eXInvSpot;
			break;
		case TORA_TSTP_SIDT_Credit:
			acctype = eXInvCrd;
			break;
		case TORA_TSTP_SIDT_Derivatives:
			acctype = eXInvOpt;
			break;
		default:
			break;
		}

		return acctype;
	}

	virtual void OnRspQryShareholderAccount(
			CTORATstpShareholderAccountField *pShareholderAccount,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		if (pShareholderAccount) {

			memset(&invest, 0, sizeof(XInvestT));
			invest.acctType = eXInvSpot;
			invest.market = market_from_ctp(pShareholderAccount->MarketID);
			memcpy(invest.investId, pShareholderAccount->ShareholderID,
					strlen(pShareholderAccount->ShareholderID));
			memcpy(invest.customerId, pShareholderAccount->InvestorID,
					strlen(pShareholderAccount->InvestorID));
			pInvest = XFndVInvestByKey(invest.customerId, invest.market,
					invest.investId);
			if (NULL != pInvest) {
				memcpy(&invest, pInvest, sizeof(XInvestT));
			}

			invest.acctType = acctype_from_ctp(
					pShareholderAccount->ShareholderIDType);

			//市值未处理
			slog_debug(0, "%s [%d-%s]", invest.customerId, invest.market, invest.investId);
			XPutOrUpdVInvestByKey(&invest);
		}

		if (bIsLast) {
			slog_info(0, "查询股东账户结束[%d] ErrorID[%d] ", nRequestID,
					pRspInfo->ErrorID);

			// 查询资金账户
			CTORATstpQryTradingAccountField field_ta;
			memset(&field_ta, 0, sizeof(field_ta));

			//以下字段不填表示不设过滤条件，即查询所有资金账号
			//strcpy(field_ta.InvestorID, InvestorID);
			//strcpy(field_ta.DepartmentID, DepartmentID);
			//strcpy(field_ta.AccountID, AccountID);
			//CurrencyID
			//AccountType

			int ret_ta = m_api->ReqQryTradingAccount(&field_ta, m_req_id++);
			if (ret_ta != 0) {
				slog_error(0, "ReqQryTradingAccount fail, ret[%d]", ret_ta);
			}
		}
	}

	//ReqQryIPOInfo(CTORATstpQryIPOInfoField *pQryIPOInfoField, int nRequestID)
	//ReqQryRationalInfo(CTORATstpQryRationalInfoField *pQryRationalInfoField, int nRequestID)
	//ReqQryIPOQuota(CTORATstpQryIPOQuotaField *pQryIPOQuotaField, int nRequestID)
	virtual void OnRspQryIPOQuota(CTORATstpIPOQuotaField *pIPOQuotaField, CTORATstpRspInfoField *pRspInfoField, int nRequestID, bool bIsLast)
	{

	}
	virtual void OnRspQryTradingAccount(
			CTORATstpTradingAccountField *pTradingAccount,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		if (pTradingAccount) {

			memset(&cash, 0, sizeof(XCashT));
			cash.acctType = acctype_from_ctp(pTradingAccount->AccountType); //和股东账户类型可能不同 TODO

			memcpy(cash.customerId, pTradingAccount->InvestorID,
					strlen(pTradingAccount->InvestorID));

			memcpy(cash.accountId, pTradingAccount->AccountID,
					strlen(pTradingAccount->AccountID));
			pCash = XFndVCashByKey(cash.customerId, cash.acctType);
			if (NULL != pCash) {
				memcpy(&cash, pCash, sizeof(XCashT));
			}

			cash.beginBalance = pTradingAccount->PreDeposit * 10000;
			cash.beginAvailable = pTradingAccount->PreDeposit * 10000;
			cash.beginDrawable = pTradingAccount->PreDeposit * 10000;

			cash.curAvailable = pTradingAccount->UsefulMoney * 10000;
			cash.countAvailable = pTradingAccount->UsefulMoney * 10000;
			cash.frozenAmt = pTradingAccount->FrozenCash * 10000;
			cash.balance = pTradingAccount->UsefulMoney * 10000;

			slog_debug(0, "[%s-%s] %lld", cash.customerId, cash.accountId, cash.curAvailable);
			XPutOrUpdVCashByKey(&cash);
		}

		if (bIsLast) {
			slog_info(0, "查询资金账户结束[%d] ErrorID[%d]", nRequestID,
					pRspInfo->ErrorID);

			// 查询持仓
			CTORATstpQryPositionField field;
			memset(&field, 0, sizeof(CTORATstpQryPositionField));

			// 以下字段不填表示不设过滤条件，即查询所有持仓
			//strcpy(field.SecurityID, "600000");

			int ret = m_api->ReqQryPosition(&field, m_req_id++);
			if (ret != 0) {
				slog_error(0, "ReqQryPosition fail, ret[%d]", ret);
			}
		}
	}

	virtual void OnRspQryOrder(CTORATstpOrderField *pOrder,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		if (pOrder) {
			printf(
					" OnRspQryOrder[%d]:SecurityID[%s] OrderLocalID[%s] OrderRef[%d] OrderSysID[%s] VolumeTraded[%d] OrderStatus[%c] OrderSubmitStatus[%c] StatusMsg[%s]\n",
					nRequestID, pOrder->SecurityID, pOrder->OrderLocalID,
					pOrder->OrderRef, pOrder->OrderSysID, pOrder->VolumeTraded,
					pOrder->OrderStatus, pOrder->OrderSubmitStatus,
					pOrder->StatusMsg);
		}

		if (bIsLast) {
			printf("查询报单结束[%d] ErrorID[%d] ", nRequestID, pRspInfo->ErrorID);
		}
	}

	virtual void OnRspQryPosition(CTORATstpPositionField *pPosition,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		if (pPosition) {

			memset(&hold, 0, sizeof(XHoldT));

			hold.market = market_from_ctp(pPosition->MarketID);
			memcpy(hold.securityId, pPosition->SecurityID,
					strlen(pPosition->SecurityID));
			memcpy(hold.customerId, pPosition->InvestorID,
					strlen(pPosition->InvestorID));
			memcpy(hold.investId, pPosition->ShareholderID,
					strlen(pPosition->ShareholderID));
			pHold = XFndVHoldByKey(hold.customerId, hold.investId, hold.market,
					hold.securityId);
			if (NULL != pHold) {
				memcpy(&hold, pHold, sizeof(XHoldT));
			}
			hold.locFrz = 0;
			hold.countSellAvlHld = pPosition->AvailablePosition;
			hold.sellAvlHld = hold.countSellAvlHld + hold.locFrz;
			hold.orgHld = pPosition->HistoryPos;
			hold.orgAvlHld = pPosition->HistoryPos
					- pPosition->HistoryPosFrozen;
			hold.orgCostAmt = pPosition->TotalPosCost;
			hold.totalBuyHld = pPosition->TodayTotalOpenVolume;
			//hold.totalSellHld = pPosition->
			hold.totalBuyAmt = pPosition->TodayTotalBuyAmount * 10000;
			hold.totalSellAmt = pPosition->TodayTotalSellAmount * 10000;
			hold.sellFrz = pPosition->CurrentPosition
					- pPosition->AvailablePosition;
			hold.sumHld = pPosition->CurrentPosition;
			hold.costPrice = pPosition->OpenPosCost;

			slog_debug(0, "[%s-%d-%s] %lld", hold.customerId, hold.market, hold.securityId, hold.sumHld);
			XPutOrUpdVHoldByKey(&hold);
		}

		if (bIsLast) {
			slog_info(0, "查询持仓结束[%d] ErrorID[%d]", nRequestID, pRspInfo->ErrorID);

			bEnd = true;
		}
	}
public:
	bool GetEnd()
	{
		return bEnd;
	}
private:
	CTORATstpTraderApi *m_api;
	int m_req_id;
	int m_front_id;
	int m_session_id;
	XStockT stock, *pStock = NULL;
	XInvestT invest, *pInvest = NULL;
	XCashT cash, *pCash = NULL;
	XHoldT hold, *pHold = NULL;
	CTORATstpReqUserLoginField m_loginInfo;
public:
	bool bEnd;
};

int XCtpInit(char *customer) {

	CTORATstpReqUserLoginField field;
	XCustT *pCustomer = NULL;

	xslog_init(XSHM_SDB_FILE, (char*) "ctpinit");

	XManShmLoad();
	// 打印接口版本号
	slog_info(0, "TradeApiVersion:[%s]\n", CTORATstpTraderApi::GetApiVersion());

	/**
	pCustomer = XFndVCustomerByKey(customer);
	if (NULL == pCustomer) {
		slog_error(0, "请确认对应的用户[%s]与使用的柜台系统<>当前适配器[%d]是否匹配?", customer,
				eXCounterCtp);
		return (-1);
	}

	*/
	// 创建接口对象
	// pszFlowPath为私有流和公有流文件存储路径，若订阅私有流和公有流且创建多个接口实例，每个接口实例应配置不同的路径
	// bEncrypt为网络数据是否加密传输，考虑数据安全性，建议以互联网方式接入的终端设置为加密传输
	CTORATstpTraderApi *demo_trade_api =
			CTORATstpTraderApi::CreateTstpTraderApi("./flow", false);

	// 创建回调对象
	DemoTradeSpi trade_spi(demo_trade_api);

	//strcpy(field.LogInAccount, pCustomer->customerId);
	strcpy(field.LogInAccount, "00030557");
	field.LogInAccountType = TORA_TSTP_LACT_UserID;
	//strcpy(field.Password, pCustomer->password);
	strcpy(field.Password, "17522830");

	strcpy(field.UserProductInfo, "quantin");
	// 按照监管要求填写终端信息
	//strcpy(field.TerminalInfo, pCustomer->remark);
	strcpy(field.TerminalInfo, "PC;IIP=123.112.154.118;IPORT=50361;LIP=192.168.118.107;MAC=54EE750B1713FCF8AE5CBD58;HD=TF655AY91GHRVL;@quantin");
	trade_spi.SetLoginInfo(&field);

	// 注册回调接口
	demo_trade_api->RegisterSpi(&trade_spi);

	//模拟环境，TCP 直连Front方式
	// 注册单个交易前置服务地址
	//demo_trade_api->RegisterFront((char*) pCustomer->ip);
	demo_trade_api->RegisterFront((char*) "tcp://210.14.72.21:4400");
	//slog_info(0, "[%s] TD_TCP_FensAddress[sim or 24H]::%s, remark[%s]", pCustomer->customerId, pCustomer->ip, pCustomer->remark);

	// 订阅公有流和私有流
	demo_trade_api->SubscribePrivateTopic(TORA_TERT_RESUME);
	demo_trade_api->SubscribePublicTopic(TORA_TERT_RESUME);
	/*	************************************
	 *	TORA_TERT_RESTART，从日初开始
	 *	TORA_TERT_RESUME,  从断开时候开始
	 *	TORA_TERT_QUICK ，从最新时刻开始
	 ****************************************/

	// 启动
	demo_trade_api->Init();
	while (!trade_spi.GetEnd()) {

		sleep(1);
	}

	// 释放，注意不允许在回调函数内调用Release接口，否则会导致线程死锁
	demo_trade_api->Release();

	return 0;
}
