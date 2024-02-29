/***********************************************************************
 *	@company	上海全创信息科技有限公司
 *	@file		demo_lev2md.cpp
 *	@brief		奇点接口demo——level2 行情获取
 *	@history	2020-09-01
 *	@author		n-sight
 *
 *	Windows：	1.请确认包.h .cpp 以及 .lib 文件都在相同目录；或者VS项目配置属性中【附加包含目录】以及【附加库目录】和【附加项依赖】正确设置相关路径
 *				2.预处理器定义 _CRT_SECURE_NO_WARNINGS ;
 *				3.使用VS2013以上版本编译通过
 *
 *	Linux：		1.库文件和头文件在同一目录下时， g++ demo_lev2md -o demo -L. -llev2mdapi
 *				2.当库文件和源文件不在同一目录时，请注意相应命令的不同
 *				3.运行时若找不到动态库，可export $LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
 ***********************************************************************/

/*****注意：本demo只用于演示行情的处理，有关交易相关接口请参考交易相关demo*****/
#include "CtpMkt.h"

#include <stdio.h>
#include <string.h>
#include "TORATstpLev2MdApi.h"
#include    "XLog.h"
#include    "XCom.h"
#include "XTimes.h"

using namespace TORALEV2API;

#ifdef  WINDOWS
#pragma comment(lib,"lev2mdapi.lib")
#endif 

#define MKT_SECURITYID_LEN 6

class Lev2MdSpi: public CTORATstpLev2MdSpi {
public:
	Lev2MdSpi(CTORATstpLev2MdApi *api) {
		m_api = api;
	}

	~Lev2MdSpi(void) {
	}

public:
	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected() {
		slog_info(0, "OnFrontConnected!\n");
		CTORATstpReqUserLoginField acc;
		memset(&acc, 0, sizeof(acc));
		strcpy(acc.LogInAccount, "lev2tester");
		acc.LogInAccountType = TORA_TSTP_LACT_UserID;
		strcpy(acc.Password, "123456");

		m_api->ReqUserLogin(&acc, ++m_req_id);

	}
	;

	virtual void OnFrontDisconnected(int nReason) {
		slog_info(0, "OnFrontDisconnected! nReason[%d]\n", nReason);

	}
	;

	///错误应答
	virtual void OnRspError(CTORATstpRspInfoField *pRspInfo, int nRequestID,
			bool bIsLast) {
		printf("OnRspError!\n");
	}
	;

	///登录请求响应
	virtual void OnRspUserLogin(CTORATstpRspUserLoginField *pRspUserLogin,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		if (pRspInfo && pRspInfo->ErrorID == 0) {

			traday = atoi(pRspUserLogin->TradingDay);

			printf("OnRspUserLogin [%s] Success!\n", pRspUserLogin->TradingDay);
			/*
			 当sub_arr中只有一个"00000000"的合约且ExchangeID填TORA_TSTP_EXD_SSE或TORA_TSTP_EXD_SZSE时,订阅单市场所有合约行情
			 当sub_arr中只有一个"00000000"的合约且ExchangeID填TORA_TSTP_EXD_COMM时,订阅全市场所有合约行情
			 支持类似 600*** 通配符,不支持 6*****1 类型通配符。
			 其它情况,订阅sub_arr集合中的合约行情
			 */
			//跨市场订阅
			int eid = TORA_TSTP_EXD_SSE;
			//char* Securities[1];
			char *Securities[4];
			Securities[0] = (char*) "60****";
			Securities[1] = (char*) "68****";
			Securities[2] = (char*) "0*****";
			Securities[3] = (char*) "3*****";
			//Securities[0] = (char*) "00000000";//8个0查全部
			//Securities[0] = (char*) "399001";//订阅指数使用SubscribeIndex，全部指数用"00000000"	

			int ret_md = m_api->SubscribeMarketData(Securities,
					sizeof(Securities) / sizeof(char*), eid);
			if (ret_md == 0) {
				printf("SubscribeMarketData:::Success,ret=%d\n", ret_md);
			} else {
				printf("SubscribeMarketData:::Failed, ret=%d)\n", ret_md);
			}

			//逐笔成交订阅（深圳）
			int ret_t = m_api->SubscribeTransaction(Securities,
					sizeof(Securities) / sizeof(char*), eid);
			if (ret_t == 0) {
				printf("SubscribeTransaction:::Success,ret=%d\n", ret_t);
			} else {
				printf("SubscribeTransaction:::Failed,ret=%d)\n", ret_t);
			}

			//逐笔委托订阅（不含上海XTS新债,但包括深圳可转债）
			int ret_od = m_api->SubscribeOrderDetail(Securities,
					sizeof(Securities) / sizeof(char*), eid);
			if (ret_od == 0) {
				printf("SubscribeOrderDetail:::Success,ret=%d\n", ret_od);
			} else {
				printf("SubscribeOrderDetail:::Failed, ret=%d)\n", ret_od);
			}

			//逐笔成交订阅（上海不包含可转债）
			ret_t = m_api->SubscribeNGTSTick(Securities,
					sizeof(Securities) / sizeof(char*), eid);
			if (ret_t == 0) {
				printf("SubscribeTransaction:::Success,ret=%d\n", ret_t);
			} else {
				printf("SubscribeTransaction:::Failed,ret=%d)\n", ret_t);
			}

#if 0	//指数行情订阅
			int ret_i = m_api->SubscribeIndex(Securities, sizeof(Securities) / sizeof(char *), eid);
			if (ret_i == 0)
			{
				printf("SubscribeIndex:::Success,ret=%d", ret_i);
			}
			else
			{
				printf("SubscribeIndex:::Failed, ret=%d)", ret_i);
			}
#endif
			//上海XTS新债快照订阅
			int xtsmd_size = sizeof(Securities) / sizeof(char*);
			for (int i = 0; i < xtsmd_size; i++) {
				printf("SubscribeXTSMarketData::Securities[%d]::%s\n", i,
						Securities[i]);
			}
			int ret_xts_md = m_api->SubscribeXTSMarketData(Securities,
					sizeof(Securities) / sizeof(char*), eid);
			if (ret_xts_md == 0) {
				printf("SubscribeXTSMarketData:::Success,ret=%d\n", ret_xts_md);
			} else {
				printf("SubscribeIndex:::Failed, ret=%d\n)", ret_xts_md);
			}

			//上海XTS新债逐笔（逐笔委托+逐笔成交）
			int ret_xts_t = m_api->SubscribeXTSTick(Securities,
					sizeof(Securities) / sizeof(char*), eid);
			if (ret_xts_t == 0) {
				printf("SubscribeIndex:::Success,ret=%d\n", ret_xts_t);
			} else {
				printf("SubscribeIndex:::Failed, ret=%d\n)", ret_xts_t);
			}

		} else {
			printf("OnRspUserLogin fail!\n");

		}
	}
	;

	/***********************************响应应答函数***********************************/
	///登出请求响应
	virtual void OnRspUserLogout(CTORATstpUserLogoutField *pUserLogout,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("OnRspUserLogout!\n");

	}
	;

	///订阅快照行情应答
	virtual void OnRspSubMarketData(
			CTORATstpSpecificSecurityField *pSpecificSecurity,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		if (pRspInfo && pRspInfo->ErrorID == 0 && pSpecificSecurity) {
			printf(
					"OnRspSubMarketData SecurityID[%s] ExchangeID[%c] Success!\n",
					pSpecificSecurity->SecurityID,
					pSpecificSecurity->ExchangeID);

		}
	}
	;

	///取消订阅行情应答
	virtual void OnRspUnSubMarketData(
			CTORATstpSpecificSecurityField *pSpecificSecurity,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		printf("OnRspUnSubMarketData SecurityID[%s] ExchangeID[%c]!\n",
				pSpecificSecurity->SecurityID, pSpecificSecurity->ExchangeID);

	}
	;

	// 订阅指数行情应答
	virtual void OnRspSubIndex(
			TORALEV2API::CTORATstpSpecificSecurityField *pSpecificSecurity,
			TORALEV2API::CTORATstpRspInfoField *pRspInfo, int nRequestID,
			bool bIsLast) {
		if (pRspInfo && pRspInfo->ErrorID == 0 && pSpecificSecurity) {
			printf("OnRspSubIndex SecurityID[%s] ExchangeID[%c] Success!\n",
					pSpecificSecurity->SecurityID,
					pSpecificSecurity->ExchangeID);

		}
	}
	;

	// 订阅逐笔成交行情应答
	virtual void OnRspSubTransaction(
			CTORATstpSpecificSecurityField *pSpecificSecurity,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		if (pRspInfo && pRspInfo->ErrorID == 0 && pSpecificSecurity) {
			printf(
					"OnRspSubTransaction SecurityID[%s] ExchangeID[%c] Success!\n",
					pSpecificSecurity->SecurityID,
					pSpecificSecurity->ExchangeID);

		}
	}
	;

	///订阅逐笔委托行情应答
	virtual void OnRspSubOrderDetail(
			CTORATstpSpecificSecurityField *pSpecificSecurity,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
		if (pRspInfo && pRspInfo->ErrorID == 0 && pSpecificSecurity) {

			printf(
					"OnRspSubOrderDetail SecurityID[%s] ExchangeID[%c] Success!\n",
					pSpecificSecurity->SecurityID,
					pSpecificSecurity->ExchangeID);

		}
	}
	;

	//订阅新债逐笔行情应答
	virtual void OnRspSubXTSTick(
			CTORATstpSpecificSecurityField *pSpecificSecurity,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

	}
	;

	///订阅上海XTS债券行情应答
	virtual void OnRspSubXTSMarketData(
			CTORATstpSpecificSecurityField *pSpecificSecurity,
			CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

	}
	;

	/***********************************回报函数***********************************/

	///快照行情通知
	virtual void OnRtnMarketData(CTORATstpLev2MarketDataField *pDepthMarketData,
			const int FirstLevelBuyNum, const int FirstLevelBuyOrderVolumes[],
			const int FirstLevelSellNum,
			const int FirstLevelSellOrderVolumes[]) {

		if (pDepthMarketData) {
			l2lData.head.type = eMSnapshot;
			l2lData.head.dataLen = XL2L_SIZE;

			memset(&snapshot, 0, XSNAPSHOT_BASE_SIZE);
			// 接收时间
#ifdef __LATENCY__
		  snapshot._recvTime = XGetClockTime ();
		#endif
			snapshot.market = market_from_ctp(pDepthMarketData->ExchangeID);
			snapshot.traday = traday;
			memcpy(snapshot.securityId, pDepthMarketData->SecurityID,
			MKT_SECURITYID_LEN);

			snapshot.updateTime = pDepthMarketData->DataTimeStamp * 1000;
			snapshot.openPx = pDepthMarketData->OpenPrice * 10000;
			snapshot.highPx = pDepthMarketData->HighestPrice * 10000;
			snapshot.lowPx = pDepthMarketData->LowestPrice * 10000;
			snapshot.preClosePx = pDepthMarketData->PreClosePrice * 10000;
			snapshot.upperPx = pDepthMarketData->UpperLimitPrice * 10000;
			snapshot.lowerPx = pDepthMarketData->LowerLimitPrice * 10000;

			snapshot.tradePx = pDepthMarketData->LastPrice * 10000;
			snapshot.volumeTrade = pDepthMarketData->TotalVolumeTrade;
			snapshot.amountTrade = pDepthMarketData->TotalValueTrade * 10000;
			snapshot.numTrades = pDepthMarketData->NumTrades;
			// 卖价

			snapshot.ask[0] = pDepthMarketData->AskPrice1 * 10000;
			snapshot.bid[0] = pDepthMarketData->BidPrice1 * 10000;
			snapshot.askqty[0] = pDepthMarketData->AskVolume1;
			snapshot.bidqty[0] = pDepthMarketData->BidVolume1;

			snapshot.ask[1] = pDepthMarketData->AskPrice2 * 10000;
			snapshot.bid[1] = pDepthMarketData->BidPrice2 * 10000;
			snapshot.askqty[1] = pDepthMarketData->AskVolume2;
			snapshot.bidqty[1] = pDepthMarketData->BidVolume2;

			snapshot.ask[2] = pDepthMarketData->AskPrice3 * 10000;
			snapshot.bid[2] = pDepthMarketData->BidPrice3 * 10000;
			snapshot.askqty[2] = pDepthMarketData->AskVolume3;
			snapshot.bidqty[2] = pDepthMarketData->BidVolume3;

			snapshot.ask[3] = pDepthMarketData->AskPrice4 * 10000;
			snapshot.bid[3] = pDepthMarketData->BidPrice4 * 10000;
			snapshot.askqty[3] = pDepthMarketData->AskVolume4;
			snapshot.bidqty[3] = pDepthMarketData->BidVolume4;

			snapshot.ask[4] = pDepthMarketData->AskPrice5 * 10000;
			snapshot.bid[4] = pDepthMarketData->BidPrice5 * 10000;
			snapshot.askqty[4] = pDepthMarketData->AskVolume5;
			snapshot.bidqty[4] = pDepthMarketData->BidVolume5;

			snapshot.ask[5] = pDepthMarketData->AskPrice6 * 10000;
			snapshot.bid[5] = pDepthMarketData->BidPrice6 * 10000;
			snapshot.askqty[5] = pDepthMarketData->AskVolume6;
			snapshot.bidqty[5] = pDepthMarketData->BidVolume6;

			snapshot.ask[6] = pDepthMarketData->AskPrice7 * 10000;
			snapshot.bid[6] = pDepthMarketData->BidPrice7 * 10000;
			snapshot.askqty[6] = pDepthMarketData->AskVolume7;
			snapshot.bidqty[6] = pDepthMarketData->BidVolume7;

			snapshot.ask[7] = pDepthMarketData->AskPrice8 * 10000;
			snapshot.bid[7] = pDepthMarketData->BidPrice8 * 10000;
			snapshot.askqty[7] = pDepthMarketData->AskVolume8;
			snapshot.bidqty[7] = pDepthMarketData->BidVolume8;

			snapshot.ask[8] = pDepthMarketData->AskPrice9 * 10000;
			snapshot.bid[8] = pDepthMarketData->BidPrice9 * 10000;
			snapshot.askqty[8] = pDepthMarketData->AskVolume9;
			snapshot.bidqty[8] = pDepthMarketData->BidVolume9;

			snapshot.ask[9] = pDepthMarketData->AskPrice10 * 10000;
			snapshot.bid[9] = pDepthMarketData->BidPrice10 * 10000;
			snapshot.askqty[9] = pDepthMarketData->AskVolume10;
			snapshot.bidqty[9] = pDepthMarketData->BidVolume10;

			l2lData.snapshot = snapshot;
			l2lData.head.market = snapshot.market;

			XPushCache(XSHMKEYCONECT(mktCache), &l2lData);
		}
	}
	;

	// 指数快照行情通知
	virtual void OnRtnIndex(CTORATstpLev2IndexField *pIndex) {

	}
	;

	int market_from_ctp(TTORATstpExchangeIDType exchid) {
		int market = 0;

		switch (exchid) {
		case TORA_TSTP_EXD_SSE:
			market = eXMarketSha;
			break;
		case TORA_TSTP_EXD_SZSE:
			market = eXMarketSza;
			break;
		default:
			break;
		}
		return market;
	}

	int bsType_from_ctp(TTORATstpLSideType bs) {
		int bstype = 0;
		switch (bs) {
		case TORA_TSTP_LSD_Buy:
			bstype = eXBuy;
			break;
		case TORA_TSTP_LSD_Sell:
			bstype = eXSell;
			break;
		default:
			break;
		}
		return bstype;
	}

	int trdType_from_ctp(TTORATstpTradeBSFlagType bs) {
		int bstype = 0;
		switch (bs) {
		case TORA_TSTP_TBSF_Buy:
			bstype = eXBuy;
			break;
		case TORA_TSTP_TBSF_Sell:
			bstype = eXSell;
			break;
		default:
			break;
		}
		return bstype;
	}
	int ordtype_from_ctp(TTORATstpLOrderTypeType type) {
		int ordtype = 0;
		switch (type) {
		case TORA_TSTP_LOT_Market:
			ordtype = eXOrdMkt;
			break;
		case TORA_TSTP_LOT_HomeBest:
			ordtype = eXOrdSelf;
			break;
		case TORA_TSTP_LOT_Limit:
			ordtype = eXOrdLimit;
			break;
		default:
			break;
		}
		return ordtype;
	}

	int ordstatus_from_ctp(TTORATstpLOrderStatusType type) {
		int ordtype = 0;
		switch (type) {
		case TORA_TSTP_LOS_Add:
			ordtype = eXOrdAdd;
			break;
		case TORA_TSTP_LOS_Delete:
			ordtype = eXOrdDel;
			break;
		default:
			break;
		}
		return ordtype;
	}
	///逐笔成交通知
	virtual void OnRtnTransaction(CTORATstpLev2TransactionField *pTransaction) {

		memset(&trade, 0, sizeof(XTickTradeT));

		l2lData.head.type = eMTickTrade;
		l2lData.head.dataLen = XL2L_SIZE;

		memcpy(&trade.securityId, pTransaction->SecurityID,
				strlen(pTransaction->SecurityID));
		trade.market = market_from_ctp(pTransaction->ExchangeID);
		//深圳逐笔成交，TradeTime的格式为【时分秒毫秒】例如例如100221530，表示10:02:21.530;
		//上海逐笔成交，TradeTime的格式为【时分秒百分之秒】例如10022153，表示10:02:21.53;
		if (TORA_TSTP_EXD_SSE == pTransaction->ExchangeID) {
			trade.updateTime = pTransaction->TradeTime * 10;
		} else {
			trade.updateTime = pTransaction->TradeTime;
		}
		trade.traday = traday;
		trade.tradePx = pTransaction->TradePrice * 10000;
		trade.tradeQty = pTransaction->TradeVolume;
		trade.isCancel = pTransaction->ExecType == TORA_TSTP_ECT_Fill ? 0 : 1;
		trade.askSeq = pTransaction->SellNo;
		trade.bidSeq = pTransaction->BuyNo;
		trade.channel = pTransaction->MainSeq;
		trade.bizIndex = pTransaction->SubSeq;
		trade.trdType = trdType_from_ctp(pTransaction->TradeBSFlag);
		/**
		slog_debug(0,
				"[%d-%s], updatetime[%d],tradeprice[%d], tradeqty[%d], iscancel[%d], buyno[%lld],sellno[%lld],bizindex[%lld],channel[%d],trdType[%d]",
				trade.market, trade.securityId, trade.updateTime, trade.tradePx,
				trade.tradeQty, trade.isCancel, trade.bidSeq, trade.askSeq,
				trade.bizIndex, trade.channel, trade.trdType);
		*/
		l2lData.trade = trade;
		XPushCache(XSHMKEYCONECT(mktCache), &l2lData);
	}
	;

	///逐笔委托通知
	virtual void OnRtnOrderDetail(CTORATstpLev2OrderDetailField *pOrderDetail) {

		memset(&order, 0, sizeof(XTickOrderT));

		l2lData.head.type = eMTickOrder;
		l2lData.head.dataLen = XL2L_SIZE;

		order.traday = traday;
		order.market = market_from_ctp(pOrderDetail->ExchangeID);
		memcpy(order.securityId, pOrderDetail->SecurityID,
				strlen(pOrderDetail->SecurityID));
		order.updateTime = pOrderDetail->OrderTime;
		order.ordPx = pOrderDetail->Price * 10000;
		order.ordQty = pOrderDetail->Volume;
		order.ordType = ordtype_from_ctp(pOrderDetail->OrderType);
		order.channel = pOrderDetail->MainSeq;
		order.bizIndex = pOrderDetail->SubSeq;
		order.seqno = pOrderDetail->OrderNO;
		order.bsType = bsType_from_ctp(pOrderDetail->Side);
		if (TORA_TSTP_EXD_SSE == pOrderDetail->ExchangeID) {
			order.ordType = ordstatus_from_ctp(pOrderDetail->OrderStatus);
		}
		/**
		slog_debug(0,
				"[%d-%s], updatetime[%d],ordprice[%d], ordqty[%d], iscancel[%d], ordtype[%d],channel[%d], bizindex[%lld], seqno[%lld], bytype[%d]",
				order.market, order.securityId, order.updateTime, order.ordPx,
				order.ordQty, order.isCancel, order.ordType, order.channel,
				order.bizIndex, order.seqno, order.bsType);
		*/
		l2lData.order = order;
		XPushCache(XSHMKEYCONECT(mktCache), &l2lData);

	}
	;

	///上海XTS债券快照行情通知
	virtual void OnRtnXTSMarketData(
			CTORATstpLev2XTSMarketDataField *pMarketData,
			const int FirstLevelBuyNum, const int FirstLevelBuyOrderVolumes[],
			const int FirstLevelSellNum,
			const int FirstLevelSellOrderVolumes[]) {
		//if(!strcmp(pMarketData->SecurityID,"204001")||!strcmp(pMarketData->SecurityID,""))
		if (pMarketData) {

			l2lData.head.type = eMSnapshot;
			l2lData.head.dataLen = XL2L_SIZE;

			// 接收时间
#ifdef __LATENCY__
					  snapshot._recvTime = XGetClockTime ();
					#endif
			snapshot.market = market_from_ctp(pMarketData->ExchangeID);

			snapshot.traday = traday;
			memcpy(snapshot.securityId, pMarketData->SecurityID,
			MKT_SECURITYID_LEN);

			snapshot.updateTime = pMarketData->DataTimeStamp * 1000;
			//snapshot.traday = pDepthMarketData->;
			snapshot.openPx = pMarketData->OpenPrice * 10000;
			snapshot.highPx = pMarketData->HighestPrice * 10000;
			snapshot.lowPx = pMarketData->LowestPrice * 10000;
			snapshot.preClosePx = pMarketData->PreClosePrice * 10000;

			snapshot.tradePx = pMarketData->LastPrice * 10000;
			snapshot.volumeTrade = pMarketData->TotalVolumeTrade;
			snapshot.amountTrade = pMarketData->TotalValueTrade * 10000;
			snapshot.numTrades = pMarketData->NumTrades;
			// 卖价

			snapshot.ask[0] = pMarketData->AskPrice1 * 10000;
			snapshot.bid[0] = pMarketData->BidPrice1 * 10000;
			snapshot.askqty[0] = pMarketData->AskVolume1;
			snapshot.bidqty[0] = pMarketData->BidVolume1;

			snapshot.ask[1] = pMarketData->AskPrice2 * 10000;
			snapshot.bid[1] = pMarketData->BidPrice2 * 10000;
			snapshot.askqty[1] = pMarketData->AskVolume2;
			snapshot.bidqty[1] = pMarketData->BidVolume2;

			snapshot.ask[2] = pMarketData->AskPrice3 * 10000;
			snapshot.bid[2] = pMarketData->BidPrice3 * 10000;
			snapshot.askqty[2] = pMarketData->AskVolume3;
			snapshot.bidqty[2] = pMarketData->BidVolume3;

			snapshot.ask[3] = pMarketData->AskPrice4 * 10000;
			snapshot.bid[3] = pMarketData->BidPrice4 * 10000;
			snapshot.askqty[3] = pMarketData->AskVolume4;
			snapshot.bidqty[3] = pMarketData->BidVolume4;

			snapshot.ask[4] = pMarketData->AskPrice5 * 10000;
			snapshot.bid[4] = pMarketData->BidPrice5 * 10000;
			snapshot.askqty[4] = pMarketData->AskVolume5;
			snapshot.bidqty[4] = pMarketData->BidVolume5;

			snapshot.ask[5] = pMarketData->AskPrice6 * 10000;
			snapshot.bid[5] = pMarketData->BidPrice6 * 10000;
			snapshot.askqty[5] = pMarketData->AskVolume6;
			snapshot.bidqty[5] = pMarketData->BidVolume6;

			snapshot.ask[6] = pMarketData->AskPrice7 * 10000;
			snapshot.bid[6] = pMarketData->BidPrice7 * 10000;
			snapshot.askqty[6] = pMarketData->AskVolume7;
			snapshot.bidqty[6] = pMarketData->BidVolume7;

			snapshot.ask[7] = pMarketData->AskPrice8 * 10000;
			snapshot.bid[7] = pMarketData->BidPrice8 * 10000;
			snapshot.askqty[7] = pMarketData->AskVolume8;
			snapshot.bidqty[7] = pMarketData->BidVolume8;

			snapshot.ask[8] = pMarketData->AskPrice9 * 10000;
			snapshot.bid[8] = pMarketData->BidPrice9 * 10000;
			snapshot.askqty[8] = pMarketData->AskVolume9;
			snapshot.bidqty[8] = pMarketData->BidVolume9;

			snapshot.ask[9] = pMarketData->AskPrice10 * 10000;
			snapshot.bid[9] = pMarketData->BidPrice10 * 10000;
			snapshot.askqty[9] = pMarketData->AskVolume10;
			snapshot.bidqty[9] = pMarketData->BidVolume10;
			//snapshot.secStatus = secstatus_from_mds(snapshot.market, pSnapshotBody->TradingPhaseCode);
			//snapshot.mrkStatus = mktstatus_from_mds(snapshot.market, pSnapshotBody->TradingPhaseCode);

			l2lData.snapshot = snapshot;
			l2lData.head.market = snapshot.market;

			XPushCache(XSHMKEYCONECT(mktCache), &l2lData);
		}

	}
	;

	///上海XTS债券逐笔数据通知
	virtual void OnRtnXTSTick(CTORATstpLev2XTSTickField *pTick) {

		//逐笔委托
		if (TORA_TSTP_LTT_Add == pTick->TickType
				|| TORA_TSTP_LTT_Delete == pTick->TickType) {

			memset(&order, 0, sizeof(XTickOrderT));

			l2lData.head.type = eMTickOrder;
			l2lData.head.dataLen = XL2L_SIZE;

			order.traday = traday;
			order.market = market_from_ctp(pTick->ExchangeID);
			memcpy(order.securityId, pTick->SecurityID,
					strlen(pTick->SecurityID));
			order.updateTime = pTick->TickTime * 10;
			order.ordPx = pTick->Price * 10000;
			order.ordQty = pTick->Volume;
//			order.ordType = ; //债券只有限价
			order.channel = pTick->MainSeq;
			order.bizIndex = pTick->SubSeq;
			if (TORA_TSTP_LSD_Buy == pTick->Side) {
				order.seqno = pTick->BuyNo;
			} else {
				order.seqno = pTick->SellNo;
			}
			order.bsType = bsType_from_ctp(pTick->Side);

			/**
			slog_debug(0,
					"[%d-%s], updatetime[%d],ordprice[%d], ordqty[%d], iscancel[%d], ordtype[%d],channel[%d], bizindex[%lld], seqno[%lld], bytype[%d]",
					order.market, order.securityId, order.updateTime,
					order.ordPx, order.ordQty, order.isCancel, order.ordType,
					order.channel, order.bizIndex, order.seqno, order.bsType);
			*/
			l2lData.order = order;
			XPushCache(XSHMKEYCONECT(mktCache), &l2lData);
		} else if (TORA_TSTP_LTT_Trade == pTick->TickType) {
			memset(&trade, 0, sizeof(XTickTradeT));

			l2lData.head.type = eMTickTrade;
			l2lData.head.dataLen = XL2L_SIZE;

			trade.traday = traday;
			memcpy(&trade.securityId, pTick->SecurityID,
					strlen(pTick->SecurityID));
			trade.market = market_from_ctp(pTick->ExchangeID);
			//深圳逐笔成交，TradeTime的格式为【时分秒毫秒】例如例如100221530，表示10:02:21.530;
			//上海逐笔成交，TradeTime的格式为【时分秒百分之秒】例如10022153，表示10:02:21.53;
			trade.updateTime = pTick->TickTime * 10;
			trade.tradePx = pTick->Price * 10000;
			trade.tradeQty = pTick->Volume;
			trade.askSeq = pTick->SellNo;
			trade.bidSeq = pTick->BuyNo;
			trade.channel = pTick->MainSeq;
			trade.bizIndex = pTick->SubSeq;
			trade.trdType = trdType_from_ctp(pTick->TradeBSFlag);

			/**
			slog_debug(0,
					"[%d-%s], updatetime[%d],tradeprice[%d], tradeqty[%d], iscancel[%d], buyno[%lld],sellno[%lld],bizindex[%lld],channel[%d],bstype[%d]",
					trade.market, trade.securityId, trade.updateTime,
					trade.tradePx, trade.tradeQty, trade.isCancel, trade.bidSeq,
					trade.askSeq, trade.bizIndex, trade.channel, trade.trdType);
			*/
			l2lData.trade = trade;
			XPushCache(XSHMKEYCONECT(mktCache), &l2lData);
		}
	}
	;

	virtual void OnRtnNGTSTick(CTORATstpLev2NGTSTickField *pTick) {

		//逐笔委托
		if (TORA_TSTP_LTT_Add == pTick->TickType
				|| TORA_TSTP_LTT_Delete == pTick->TickType) {

			memset(&order, 0, sizeof(XTickOrderT));

			l2lData.head.type = eMTickOrder;
			l2lData.head.dataLen = XL2L_SIZE;

			order.traday = traday;
			order.market = market_from_ctp(pTick->ExchangeID);
			memcpy(order.securityId, pTick->SecurityID,
					strlen(pTick->SecurityID));
			order.updateTime = pTick->TickTime * 10;
			order.ordPx = pTick->Price * 10000;
			order.ordQty = pTick->Volume;
			//			order.ordType = ; //债券只有限价
			order.channel = pTick->MainSeq;
			order.bizIndex = pTick->SubSeq;
			if (TORA_TSTP_LSD_Buy == pTick->Side) {
				order.seqno = pTick->BuyNo;
			} else {
				order.seqno = pTick->SellNo;
			}
			order.bsType = bsType_from_ctp(pTick->Side);

			/**
			slog_debug(0,
					"[%d-%s], updatetime[%d],ordprice[%d], ordqty[%d], iscancel[%d], ordtype[%d],channel[%d], bizindex[%lld], seqno[%lld], bytype[%d]",
					order.market, order.securityId, order.updateTime,
					order.ordPx, order.ordQty, order.isCancel, order.ordType,
					order.channel, order.bizIndex, order.seqno, order.bsType);
			*/
			l2lData.order = order;
			XPushCache(XSHMKEYCONECT(mktCache), &l2lData);
		} else if (TORA_TSTP_LTT_Trade == pTick->TickType) {
			memset(&trade, 0, sizeof(XTickTradeT));

			l2lData.head.type = eMTickTrade;
			l2lData.head.dataLen = XL2L_SIZE;

			trade.traday = traday;
			memcpy(&trade.securityId, pTick->SecurityID,
					strlen(pTick->SecurityID));
			trade.market = market_from_ctp(pTick->ExchangeID);
			//深圳逐笔成交，TradeTime的格式为【时分秒毫秒】例如例如100221530，表示10:02:21.530;
			//上海逐笔成交，TradeTime的格式为【时分秒百分之秒】例如10022153，表示10:02:21.53;
			trade.updateTime = pTick->TickTime * 10;
			trade.tradePx = pTick->Price * 10000;
			trade.tradeQty = pTick->Volume;
			trade.askSeq = pTick->SellNo;
			trade.bidSeq = pTick->BuyNo;
			trade.channel = pTick->MainSeq;
			trade.bizIndex = pTick->SubSeq;
			trade.trdType = trdType_from_ctp(pTick->TradeBSFlag);

			/**
			slog_debug(0,
					"[%d-%s], updatetime[%d],tradeprice[%d], tradeqty[%d], iscancel[%d], buyno[%lld],sellno[%lld],bizindex[%lld],channel[%d],bstype[%d]",
					trade.market, trade.securityId, trade.updateTime,
					trade.tradePx, trade.tradeQty, trade.isCancel, trade.bidSeq,
					trade.askSeq, trade.bizIndex, trade.channel, trade.trdType);
			*/
			l2lData.trade = trade;
			XPushCache(XSHMKEYCONECT(mktCache), &l2lData);
		}

	}
	;

private:
	CTORATstpLev2MdApi *m_api = NULL;
	int m_req_id;
	XL2LT l2lData;
	XTickOrderT order;
	XTickTradeT trade;
	XSnapshotBaseT snapshot;
	int traday;
};

//参数未做
XVoid XCtpMkt(XVoid *param) {

	xslog_init(XSHM_SDB_FILE, (char*) "ctpmkt");

	XManShmLoad();
	// 打印接口版本号
	slog_info(0, "Level2MdApiVersion::[%s]\n",
			CTORATstpLev2MdApi::GetApiVersion());

	CTORATstpLev2MdApi *lev2md_api;
	char LEV2MD_TCP_FrontAddress[64];
	char LEV2MD_MCAST_FrontAddress[64];
	char LEV2MD_MCAST_InterfaceIP[64];

//	if (argc == 1) //默认TCP连接测试环境
//			{
	strcpy(LEV2MD_TCP_FrontAddress, "tcp://210.14.72.17:16900"); //上海
	//strcpy(LEV2MD_TCP_FrontAddress, "tcp://210.14.72.17:6900");		//深圳
	/*******************************互联网Level2测试桩说明*****************************
	 * 7*24小时测试桩只做技术调试用途，无法验证业务完整性，历史某日行情轮播
	 * Level2测试桩，上海和深圳分为两个独立环境，且仅支持TCP方式在互联网上测试
	 * 需使用两个不同的实例进行处理或分别测试上海和深圳
	 * *************************************************************************/
//	} else if (argc == 3 && !strcmp(argv[1], "tcp")) //普通TCP方式
//			{
//		strcpy(LEV2MD_TCP_FrontAddress, argv[2]);
//	} else if (argc == 4 && !strcmp(argv[1], "udp")) //组播方式
//			{
//		strcpy(LEV2MD_MCAST_FrontAddress, argv[2]);
//		strcpy(LEV2MD_MCAST_InterfaceIP, argv[3]);
//	} else {
//		printf(
//				"/*********************************************demo运行说明************************************\n");
//		printf("* argv[1]: tcp udp \t\t\t\t=[%s]\n", argv[1]);
//		printf("* argv[2]: tcp::FrontIP upd::MCAST_IP\t\t=[%s]\n", argv[2]);
//		printf("* argv[3]: udp::InterfaceIP\t\t\t=[%s]\n", argv[3]);
//		printf("* Usage:\n");
//		printf("* 默认连模拟:		./demo\n");
//		printf("* 指定TCP地址:		./demo tcp tcp://210.14.72.17:6900\n");
//		printf("* 指定组播地址:		./demo udp udp://224.224.224.15:7889 x.x.x.x\n");
//		printf("* 上述x.x.x.x使用托管服务器中接收Level2行情的网口IP地址\n");
//		printf(
//				"* ******************************************************************************************/\n");
//
//		return -1;
//	}
	/*************************创建实例 注册服务*****************/
//	if (argc == 1 || !strcmp(argv[1], "tcp")) //默认或TCP方式
//			{
	printf("************* LEVEL2 MD TCP *************\n");
	//TCP订阅lv2行情，前置Front和FENS方式都用默认构造
	lev2md_api = CTORATstpLev2MdApi::CreateTstpLev2MdApi();
	/*
	 * 非缓存模式: CreateTstpLev2MdApi(TORA_TSTP_MST_TCP,false)
	 * 缓存模式：  CreateTstpLev2MdApi(TORA_TSTP_MST_TCP,true)
	 * 非缓存模式相比缓存模式处理效率更高,但回调处理逻辑耗时长可能导致连接中断时,建议使用缓存模式
	 */
	lev2md_api->RegisterFront(LEV2MD_TCP_FrontAddress);
	printf("Level2MD_TCP_FrontAddress::%s\n", LEV2MD_TCP_FrontAddress);
//	} else if (!strcmp(argv[1], "udp")) //组播普通行情
//			{
//		printf("************* LEVEL2 MD UDP *************\n");
//		//组播订阅lv2行情,实例需有参构造
//		lev2md_api = CTORATstpLev2MdApi::CreateTstpLev2MdApi(
//				TORA_TSTP_MST_MCAST);
//		//组播+缓存模式
//		// lev2md_api = CTORATstpLev2MdApi::CreateTstpLev2MdApi(TORA_TSTP_MST_MCAST, true);
//		/*
//		 * 非缓存模式: CreateTstpLev2MdApi(TORA_TSTP_MST_MCAST,false)
//		 * 缓存模式：  CreateTstpLev2MdApi(TORA_TSTP_MST_MCAST,true)
//		 * 非缓存模式相比缓存模式处理效率更高,但回调处理逻辑耗时长可能导致不能完整及时处理数据时,建议使用缓存模式
//		 */
//		lev2md_api->RegisterMulticast(LEV2MD_MCAST_FrontAddress,
//				LEV2MD_MCAST_InterfaceIP, NULL);
//		printf("LEV2MD_MCAST_FrontAddress::%s\n", LEV2MD_MCAST_FrontAddress);
//		printf("LEV2MD_MCAST_InterfaceIP::%s\n", LEV2MD_MCAST_InterfaceIP);
//		/**4.0.2以上版本支持不同组播行情源在同一个实例中注册多次，分别以不同组播地址、接收地址注册**/
//	} else {
//		printf(
//				"/*********************************************demo运行说明************************************\n");
//		printf("* argv[1]: tcp udp \t\t\t\t=[%s]\n", argv[1]);
//		printf("* argv[2]: tcp::FrontIP upd::MCAST_IP\t\t=[%s]\n", argv[2]);
//		printf("* argv[3]: udp::InterfaceIP\t\t\t=[%s]\n", argv[3]);
//		printf("* Usage:\n");
//		printf("* 默认连模拟:		./demo\n");
//		printf("* 指定TCP地址:		./demo tcp tcp://210.14.72.17:6900\n");
//		printf("* 指定组播地址:		./demo udp udp://224.224.224.15:7889 x.x.x.x\n");
//		printf("* 上述x.x.x.x使用托管服务器中接收Level2行情的网口IP地址\n");
//		printf(
//				"* ******************************************************************************************/\n");
//
//		return -1;
//	}

	// 创建回调对象
	Lev2MdSpi demo_spi(lev2md_api);

	// 注册回调接口
	lev2md_api->RegisterSpi(&demo_spi);

	// 启动
	lev2md_api->Init();
	///Init(cpuCores) 绑核参数说明
	///@param cpuCores：API内部线程绑核参数，默认不绑核运行
	//                  "0"表示API内部线程绑定到第0核上运行
	//					"0,5,18"表示API内部线程绑定到第0,第5，第18号核上运行                 
	///@remark 初始化运行环境,只有调用后,接口才开始工作
	// TCP模式收取行情时,非缓存模式1个线程,缓存模式2个线程,相应绑几个核心即可
	// 组播模式收取行情时,需要传递的核的数目= 注册的组播地址数目+[缓存模式? 1: 0]
	// 平台服务器PROXY模式时,线程数2,绑定不低于2个核心

	// 等待结束
	while (true) {

	}
	//lev2md_api->join();

	// 释放，注意不允许在回调函数内调用Release接口，否则会导致线程死锁
	lev2md_api->Release();

}
