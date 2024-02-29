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
#include "CtpTrd.h"
#include "TORATstpTraderApi.h"
#include <stdio.h>
#include <string.h>
#include "XCom.h"
#include "XLog.h"
#include "XTimes.h"

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

class MyAPI {

public:

	MyAPI() {
		m_req_id = 0;
		TradeAPI = CTORATstpTraderApi::CreateTstpTraderApi("./flow", false);
	}

	~MyAPI() {

	}
	void Release() {
		TradeAPI->Release();
	}
	int market_to_ctp(int market) {
		int mk = 0;

		return mk;
	}

	int bs_to_ctp(int bstype) {
		int bs = 0;

		return bs;
	}
	int ordtype_to_ctp(int ordtype) {
		int od = 0;

		return od;
	}
	int ReqOrderInsert(XOrderT *pOrder) {
		memset(&m_inputOrder, 0, sizeof(CTORATstpInputOrderField));

		m_inputOrder.ExchangeID = market_to_ctp(pOrder->request.market);
		memcpy(m_inputOrder.ShareholderID, pOrder->request.investId,
				strlen(pOrder->request.investId));
		memcpy(m_inputOrder.SecurityID, pOrder->request.securityId,
				strlen(pOrder->request.securityId));
		m_inputOrder.Direction = bs_to_ctp(pOrder->request.bsType);
		m_inputOrder.VolumeTotalOriginal = pOrder->request.ordQty;
		m_inputOrder.LimitPrice = pOrder->request.ordPrice / 10000.0;
		m_inputOrder.OrderPriceType = ordtype_to_ctp(pOrder->request.ordType);
		m_inputOrder.TimeCondition = TORA_TSTP_TC_GFD;
		m_inputOrder.VolumeCondition = TORA_TSTP_VC_AV;
		m_inputOrder.OrderRef = pOrder->request.localId;

		TradeAPI->ReqOrderInsert(&m_inputOrder, m_req_id++);

		return 0;
	}

	int ReqOrderAction(XOrderT *pOrder) {
		memset(&m_inputAction, 0, sizeof(CTORATstpInputOrderActionField));

		m_inputAction.SessionID = pOrder->request.orgEnvno;
		m_inputAction.ExchangeID = market_to_ctp(pOrder->request.market);
		m_inputAction.ActionFlag = TORA_TSTP_AF_Delete;
		m_inputAction.OrderActionRef = pOrder->request.localId;
		m_inputAction.OrderRef = pOrder->request.orgLocalId;

		sprintf(m_inputAction.OrderSysID, "%lld", pOrder->request.orgOrdId);

		TradeAPI->ReqOrderAction(&m_inputAction, m_req_id++);
		return 0;
	}

private:
	CTORATstpInputOrderField m_inputOrder;
	CTORATstpInputOrderActionField m_inputAction;
	int m_req_id;
public:
	CTORATstpTraderApi *TradeAPI;
};

class DemoTradeSpi: public CTORATstpTraderSpi {
public:
	DemoTradeSpi(CTORATstpTraderApi *api) {
		m_api = api;
		m_req_id = 1;

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
			slog_warn(0, "ReqGetConnectionInfo fail, ret[%d]", ret);
		}
	}

	virtual void OnFrontDisconnected(int nReason) {
		printf("TradeApi OnFrontDisconnected: [%d]\n", nReason);
	}

	virtual void OnRspGetConnectionInfo(
			CTORATstpConnectionInfoField *pConnectionInfoField,
			CTORATstpRspInfoField *pRspInfo, int nRequestID) {
		if (pRspInfo->ErrorID == 0) {
			slog_info(0, "inner_ip_address[%s]"
					" inner_port[%d]"
					" outer_ip_address[%s]"
					" outer_port[%d]"
					" mac_address[%s]", pConnectionInfoField->InnerIPAddress,
					pConnectionInfoField->InnerPort,
					pConnectionInfoField->OuterIPAddress,
					pConnectionInfoField->OuterPort,
					pConnectionInfoField->MacAddress);

			// 以下内外网IP地址若不填则柜台系统自动采集，若填写则以终端填值为准报送
			//strcpy(field.MacAddress, pConnectionInfoField->MacAddress);
			//strcpy(field.InnerIPAddress, pConnectionInfoField->InnerIPAddress);
			//strcpy(field.OuterIPAddress, pConnectionInfoField->OuterIPAddress);

			int ret = m_api->ReqUserLogin(&m_loginInfo, m_req_id++);
			if (ret != 0) {
				slog_error(0, "TradeApi ReqUserLogin fail, ret[%d]", ret);
			}
		} else {
			slog_error(0,
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

		}
	}

	virtual void OnRspUserPasswordUpdate(
			CTORATstpUserPasswordUpdateField *pUserPasswordUpdateField,
			CTORATstpRspInfoField *pRspInfo, int nRequestID) {

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

	int ordtype_from_ctp(TTORATstpOrderPriceTypeType odPrTyp) {
		int ot = 0;
		switch (odPrTyp) {
		///限价
		case TORA_TSTP_OPT_LimitPrice:
			ot = eXOrdLimit;
			break;
			///最优价
		case TORA_TSTP_OPT_BestPrice:
			ot = eXOrdBest;
			break;
			///五档价
		case TORA_TSTP_OPT_FiveLevelPrice:
			ot = eXOrdBest5;
			break;
			///本方最有
		case TORA_TSTP_OPT_HomeBestPrice:
			ot = eXOrdBestParty;
			break;
		default:
			break;
		}
		return ot;
	}
	int bstype_from_ctp(TTORATstpDirectionType dirc) {
		int bs = 0;
		switch (dirc) {
		///买入
		case TORA_TSTP_D_Buy:
			bs = eXBuy;
			break;
			///卖出
		case TORA_TSTP_D_Sell:
			bs = eXSell;
			break;
			///新股申购
		case TORA_TSTP_D_IPO:
			bs = eXDeem;
			break;
			///逆回购
		case TORA_TSTP_D_ReverseRepur:
			bs = eXCSell;
			break;
			///正回购
		case TORA_TSTP_D_Repurchase:
			bs = eXCBuy;
			break;
		default:
			break;
		}
		return bs;
	}

	void del_char(char a[], char c) {
		int i, j;
		for (i = 0, j = 0; *(a + i) != '\0'; i++) {
			if (*(a + i) == c)
				continue;
			else {
				*(a + j) = *(a + i);
				j++;
			}
		}
		*(a + j) = '\0';
	}
	virtual void OnRspOrderInsert(CTORATstpInputOrderField *pInputOrderField,
			CTORATstpRspInfoField *pRspInfo, int nRequestID) {
		XOrderT order = { 0 };
		XTradeCache cache = { 0 };

		order.request.market = market_from_ctp(pInputOrderField->ExchangeID);
		memcpy(order.request.securityId, pInputOrderField->SecurityID,
				strlen(pInputOrderField->SecurityID));

		memcpy(order.request.investId, pInputOrderField->ShareholderID,
				strlen(pInputOrderField->ShareholderID));

		memcpy(order.request.customerId, pInputOrderField->InvestorID,
				strlen(pInputOrderField->InvestorID));

		order._cnfLocTime = XGetClockTime();
		order.envno = m_session_id;
		//order._sendTime = pInputOrderField->;
		//order._cnfTime = pOrder->ordCnfmTime;
		//order.ordStatus = ordstatus_from_ctp(pInputOrderField->);

		//memcpy(order.exchordId, pOrder->exchOrdId, EXCHORDID_LEN);
		//order.frzAmt = pOrder->frzAmt;
		//order.frzFee = pOrder->frzFee;

		//order.errorno = pOrder->ordRejReason;

		//memcpy(order.errmsg, pTmpStr, strlen(pTmpStr));

		order.request.ordType = ordtype_from_ctp(
				pInputOrderField->OrderPriceType);
		order.request.ordQty = pInputOrderField->VolumeTotalOriginal;
		order.request.bsType = bstype_from_ctp(pInputOrderField->Direction);
		//order.request.isCancel = iscancel_from_oes(pOrder->bsType);

		order.request.localId = pInputOrderField->OrderRef;
		order.ordid = atol(pInputOrderField->OrderSysID);
		order.counter = eXCounterCtp;
		order.request.ordPrice = pInputOrderField->LimitPrice;
		//order.envno = pOrder->clEnvId;
		order.request.plotid = pInputOrderField->IInfo;
		order.exeStatus = eXExec;

		if (pRspInfo->ErrorID != 0) {
			order.errorno = pRspInfo->ErrorID;
			order.ordStatus = eXOrdStatusInvalid;
		} else {
			order.ordStatus = eXOrdStatusDeclared;
		}

		cache.head.type = eDOrder;
		//	cache.head.num = ++l_Counter;
		cache.head.dataLen = XORDER_SIZE;
		cache.ordrsp = order;

		/**
		 slog_debug(0, "<<<<<<消息序号[%lld], 响应[%d-%s]订单号[%d]", cache.head.num,
		 order.request.market, order.request.securityId,
		 order.request.localId);
		 */
		XPushCache(XSHMKEYCONECT(tradeCache), &cache);
	}

	virtual void OnRspOrderAction(
			CTORATstpInputOrderActionField *pInputOrderActionField,
			CTORATstpRspInfoField *pRspInfo, int nRequestID) {
		XOrderT order = { 0 };
		XTradeCache cache = { 0 };

		order.request.market = market_from_ctp(
				pInputOrderActionField->ExchangeID);

		order._cnfLocTime = XGetClockTime();

		order.request.localId = pInputOrderActionField->OrderActionRef;
		order.ordid = atol(pInputOrderActionField->CancelOrderSysID);
		order.request.orgOrdId = atol(pInputOrderActionField->OrderSysID);
		order.request.orgLocalId = pInputOrderActionField->OrderRef;
		order.counter = eXCounterCtp;
		//order.request.ordPrice = pInputOrderField->LimitPrice;
		//order.envno = pOrder->clEnvId;
		order.request.plotid = pInputOrderActionField->IInfo;
		order.exeStatus = eXExec;
		order.request.isCancel = true;
		order.envno = m_session_id;

		if (pRspInfo->ErrorID != 0) {
			order.errorno = pRspInfo->ErrorID;
			order.ordStatus = eXOrdStatusInvalid;
		} else {
			order.ordStatus = eXOrdStatusDeclared;
		}
		cache.head.type = eDOrder;
		//	cache.head.num = ++l_Counter;
		cache.head.dataLen = XORDER_SIZE;
		cache.ordrsp = order;

		/**
		 slog_debug(0, "<<<<<<消息序号[%lld], 响应[%d-%s]订单号[%d]", cache.head.num,
		 order.request.market, order.request.securityId,
		 order.request.localId);
		 */
		XPushCache(XSHMKEYCONECT(tradeCache), &cache);
	}

	virtual void OnRspInquiryJZFund(
			CTORATstpRspInquiryJZFundField *pRspInquiryJZFundField,
			CTORATstpRspInfoField *pRspInfo, int nRequestID) {

	}

	virtual void OnRspTransferFund(
			CTORATstpInputTransferFundField *pInputTransferFundField,
			CTORATstpRspInfoField *pRspInfo, int nRequestID) {

	}

	virtual void OnRtnTransferFund(CTORATstpTransferFundField *pTransferFund) {

	}
	int ordstatus_from_ctp(TTORATstpOrderStatusType ordstatus) {
		int os = 0;

		switch (ordstatus) {
		///交易所已接收
		case TORA_TSTP_OST_Accepted:
			os = eXOrdStatusDeclared;
			break;
			///部分成交
		case TORA_TSTP_OST_PartTraded:
			os = eXOrdStatusPFilled;
			break;
			///全部成交
		case TORA_TSTP_OST_AllTraded:
			os = eXOrdStatusFilled;
			break;
			///部成部撤
		case TORA_TSTP_OST_PartTradeCanceled:
			os = eXOrdStatusPCanceled;
			break;
			///全部撤单
		case TORA_TSTP_OST_AllCanceled:
			os = eXOrdStatusCanceled;
			break;
			///交易所已拒绝
		case TORA_TSTP_OST_Rejected:
			os = eXOrdStatusInvalid;
			break;
		default:
			os = eXOrdStatusNew;
			break;
		}
		return os;
	}
	int iscancel_from_ctp(TTORATstpOrderStatusType od) {
		int iscl = 0;

		switch (od) {
		///部成部撤
		case TORA_TSTP_OST_PartTradeCanceled:
			iscl = 1;
			break;
			///全部撤单
		case TORA_TSTP_OST_AllCanceled:
			iscl = 1;
			break;

		default:
			break;
		}

		return iscl;

	}

	virtual void OnRtnOrder(CTORATstpOrderField *pOrder) {

		XOrderT order = { 0 };
		const char *pTmpStr = NULL;
		XTradeCache cache = { 0 };

		order.envno = pOrder->SessionID;
		order.request.market = market_from_ctp(pOrder->ExchangeID);

		memcpy(order.request.securityId, pOrder->SecurityID,
				strlen(pOrder->SecurityID));

		memcpy(order.request.investId, pOrder->ShareholderID,
				strlen(pOrder->ShareholderID));
		order.ordid = atol(pOrder->OrderSysID);
		order.ordStatus = ordstatus_from_ctp(pOrder->OrderStatus);

		memcpy(order.request.customerId, pOrder->InvestorID,
				strlen(pOrder->InvestorID));
		del_char(pOrder->InsertTime, ':');

		order._sendTime = atoi(pOrder->InsertTime); // HH:MM:SS
		del_char(pOrder->AcceptTime, ':');
		order._cnfTime = atoi(pOrder->AcceptTime);
		order._cnfExTime = XGetClockTime();

		//order.frzAmt = pOrder->frzAmt;
		//order.frzFee = pOrder->frzFee;
		order.counter = eXCounterCtp;
		//order.errorno = pOrder->ordRejReason;
		//pTmpStr = OesApi_GetErrorMsg(order.errorno);
		order.request.plotid = pOrder->IInfo;

		order.request.ordType = ordtype_from_ctp(pOrder->OrderPriceType);
		order.request.ordQty = pOrder->VolumeTotalOriginal;
		order.trdQty = pOrder->VolumeTraded;
		order.trdMoney = pOrder->Turnover * 10000;
		order.request.bsType = bstype_from_ctp(pOrder->Direction);

		//被撤订单的原始订单仍然为对应的买卖标志即不为撤单
		order.request.isCancel = iscancel_from_ctp(pOrder->OrderStatus);

		order.request.localId = pOrder->OrderRef;

		order.request.ordPrice = pOrder->LimitPrice * 10000;
		//order.request.orgOrdId = pOrder->origClOrdId;
		//order.request.orgLocalId = pOrder->origClSeqNo;
		//order.request.orgEnvno = pOrder->origClEnvId;
		//order.envno = pOrder->clEnvId;
		order.exeStatus = eXExec;

		cache.head.type = eDOrder;
		//	cache.head.num = ++l_Counter;
		cache.head.dataLen = XORDER_SIZE;
		cache.ordrsp = order;

		slog_debug(0,
				"OnRtnOrder:[%d-%s],customer[%s],invest[%s], localid[%d], ordid[%lld], "
						"plotid[%lld], ordstatus[%d], ordtype[%d],ordqty[%d],bstype[%d],iscancel[%d], ordprice[%d]"
						"sendtime[%lld],cnftime[%lld]", order.request.market,
				order.request.securityId, order.request.customerId,
				order.request.investId, order.request.localId, order.ordid,
				order.request.plotid, order.ordStatus, order.request.ordType,
				order.request.ordQty, order.request.bsType,
				order.request.isCancel, order.request.ordPrice, order._sendTime,
				order._cnfTime);
		/**
		 slog_debug(0, "<<<<<<%d 确认[%d-%s]订单号[%d]", cache.head.num,
		 order.request.market, order.request.securityId,
		 order.request.localId);
		 */

		XPushCache(XSHMKEYCONECT(tradeCache), &cache);
	}

	virtual void OnRtnTrade(CTORATstpTradeField *pTrade) {
		XTradeT trade = { 0 };
		XTradeCache cache = { 0 };

		trade.ordid = atol(pTrade->OrderSysID);
		trade.market = market_from_ctp(pTrade->ExchangeID);
		memcpy(trade.securityId, pTrade->SecurityID,
				strlen(pTrade->SecurityID));
		memcpy(trade.investId, pTrade->ShareholderID,
				strlen(pTrade->ShareholderID));
		trade.trdId = atol(pTrade->TradeID);

		memcpy(trade.customerId, pTrade->InvestorID,
				strlen(pTrade->InvestorID));

		trade.trdQty = pTrade->Volume;
		//trade.trdAmt = pTrade->trdAmt;
		trade.trdPrice = pTrade->Price * 10000;
		del_char(pTrade->TradeTime, ':');
		trade.trdTime = atoi(pTrade->TradeTime); // TradeTime为 HH:MM:SS

		//trade.cumQty = pTrade->cumQty;
		//trade.cumAmt = pTrade->cumAmt;

		trade.trdSide = bstype_from_ctp(pTrade->Direction);

		trade.counter = eXCounterCtp;

		cache.head.type = eDTrade;
		//	cache.head.num = ++l_Counter;
		cache.head.dataLen = XTRADE_SIZE;
		cache.trade = trade;

		slog_debug(0,
				"OnRtnTrade:[%d-%s],customer[%s],invest[%s], tradeid[%lld], trdqty[%d], trdprice[%d], trdtime[%d], trdside[%d]",
				trade.market, trade.securityId, trade.customerId,
				trade.investId, trade.trdId, trade.trdQty, trade.trdPrice,
				trade.trdTime, trade.trdSide);

		//TODO 买模拟增加可用资金，卖根据标的增加总持仓
		/**
		 slog_debug(0, "<<<<<<%lld 成交[%c-%s]订单号[%d]", cache.head.num, trade.market,
		 trade.securityId, trade.ordid);
		 */
		XPushCache(XSHMKEYCONECT(tradeCache), &cache);
	}

	virtual void OnRtnMarketStatus(CTORATstpMarketStatusField *pMarketStatus) {
	}

	virtual void OnRspQrySecurity(CTORATstpSecurityField *pSecurity,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

	}

	virtual void OnRspQryInvestor(CTORATstpInvestorField *pInvestor,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

	}

	virtual void OnRspQryShareholderAccount(
			CTORATstpShareholderAccountField *pShareholderAccount,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

	}

	virtual void OnRspQryTradingAccount(
			CTORATstpTradingAccountField *pTradingAccount,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

	}

	virtual void OnRspQryOrder(CTORATstpOrderField *pOrder,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

	}

	virtual void OnRspQryPosition(CTORATstpPositionField *pPosition,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

	}
private:
	CTORATstpTraderApi *m_api;
	int m_req_id;
	int m_front_id;
	int m_session_id;
	CTORATstpReqUserLoginField m_loginInfo;
};

XVoid XCtpTrd(XVoid *param) {
	XCustT *pCustomer = NULL;
	CTORATstpReqUserLoginField field;

	xslog_init(XSHM_SDB_FILE, (char*) "ctptrd");

	XManShmLoad();

	pCustomer = (XCustT*) param;

	if (NULL == pCustomer) {
		return;
	}

	// 创建接口对象
	// pszFlowPath为私有流和公有流文件存储路径，若订阅私有流和公有流且创建多个接口实例，每个接口实例应配置不同的路径
	// bEncrypt为网络数据是否加密传输，考虑数据安全性，建议以互联网方式接入的终端设置为加密传输
	MyAPI *demo_trade_api = new MyAPI();

	// 创建回调对象
	DemoTradeSpi trade_spi(demo_trade_api->TradeAPI);

	strcpy(field.LogInAccount, pCustomer->customerId);
	field.LogInAccountType = TORA_TSTP_LACT_UserID;
	strcpy(field.Password, pCustomer->password);

	strcpy(field.UserProductInfo, "quantin");

	// 按照监管要求填写终端信息
	strcpy(field.TerminalInfo, pCustomer->remark);

	trade_spi.SetLoginInfo(&field);

	// 注册回调接口
	demo_trade_api->TradeAPI->RegisterSpi(&trade_spi);

	// 注册单个交易前置服务地址
	demo_trade_api->TradeAPI->RegisterFront((char*) pCustomer->ip);
	slog_info(0, "[%s] TD_TCP_FensAddress[sim or 24H]::%s, remark[%s]",
			pCustomer->customerId, pCustomer->ip, pCustomer->remark);

	// 订阅公有流和私有流
	demo_trade_api->TradeAPI->SubscribePrivateTopic(TORA_TERT_RESUME);
	demo_trade_api->TradeAPI->SubscribePublicTopic(TORA_TERT_RESUME);
	/*	************************************
	 *	TORA_TERT_RESTART，从日初开始
	 *	TORA_TERT_RESUME,  从断开时候开始
	 *	TORA_TERT_QUICK ，从最新时刻开始
	 ****************************************/

	// 启动
	demo_trade_api->TradeAPI->Init();

	// 等待结束
	XOrderT *pCurOrder = NULL;
	XMonitorT *l_pMonitor = NULL;
	XMonitorTdT *l_pMonitorTd = NULL;
	XIdx lastIdx = -1;
	XInt iret = -1;

	l_pMonitor = XFndVMonitor();

	l_pMonitorTd = XFndVTdMonitor(field.LogInAccount);
	if (NULL == l_pMonitorTd) {
		slog_error(0, "未获取到行情信息[%s]", "");
//		return (-1);
	}

	for (;;) {

		//已经处理的订单
		if (l_pMonitorTd->iDealPos >= l_pMonitor->iTOrder) {
			continue;
		}

		lastIdx = l_pMonitorTd->iDealPos + 1;
		pCurOrder = XFndVOrderById(lastIdx);

		if (NULL == pCurOrder) {
			continue;
		}

		//不是本柜台或本账户数据，跳过
		if (pCurOrder->counter != eXCounterCtp
				|| strcmp(pCurOrder->request.customerId, field.LogInAccount)
						!= 0) {
			l_pMonitorTd->iDealPos = pCurOrder->idx;
			continue;
		}
		//未过风控或系统未启动处理的订单
		if (pCurOrder->exeStatus != eXExecRisk) {
			l_pMonitorTd->iDealPos = pCurOrder->idx;
			continue;
		}

		if (pCurOrder->request.isCancel == false) {
			//委托
			switch (pCurOrder->request.bsType) {
			case eXBuy:
			case eXSell:
			case eXDeem:
			case eXCSell:
				iret = demo_trade_api->ReqOrderInsert(pCurOrder);
				l_pMonitorTd->iDealPos = pCurOrder->idx;
				break;
			default:
				break;
			}
		} else {
			iret = demo_trade_api->ReqOrderAction(pCurOrder);
			l_pMonitorTd->iDealPos = pCurOrder->idx;
		}

	}
	//循环处理发单服务

	// 释放，注意不允许在回调函数内调用Release接口，否则会导致线程死锁
	demo_trade_api->Release();

}
