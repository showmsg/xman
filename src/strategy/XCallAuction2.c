/*
 * @file XBondRob.c
 * @brief 可转债盘前或收盘抢板
 * @author kangyq
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */

#include "XCom.h"
#include "XTimes.h"
#include "XBus.h"
#include "XLog.h"

#define XAUCTION_ROB_BTIME              (91457000)

#define XAUCTION_ROB_ETIME              (91505000)
#define XAUCTION_ROB_ATIME              (91754000)
#define XAUCTION_CALC_GAP               (2765)

XNum XGetBuyPos(XStrategyT *pStrategy) {
	XOrderT *pOrder = NULL;
	XNum i = 0;
	XNum idx = -1;

	for (i = 0; i < MAX_GRID_LEVEL; i++) {
		if (0 == pStrategy->buyList[i]) {
			idx = i;
			break;
		} else {
			pOrder = XFndVOrderByLoc(pStrategy->plot.customerId,
					pStrategy->envno, pStrategy->buyList[i]);
			if (NULL != pOrder) {
				//全部成交，部分撤单，全部撤单，非法订单
				if (pOrder->ordStatus == eXOrdStatusInvalid) {
					pStrategy->buyList[i] = 0;
					idx = i;
					break;
				}
			}
		}
	}

	return (idx);
}

XNum XGetSellPos(XStrategyT *pStrategy) {
	XOrderT *pOrder = NULL;
	XNum i = 0;
	XNum idx = -1;

	for (i = 0; i < MAX_GRID_LEVEL; i++) {
		if (0 == pStrategy->sellList[i]) {
			idx = i;
			break;
		} else {
			pOrder = XFndVOrderByLoc(pStrategy->plot.customerId,
					pStrategy->envno, pStrategy->sellList[i]);
			if (NULL != pOrder) {
				//全部成交，部分撤单，全部撤单，非法订单
				if (pOrder->ordStatus == eXOrdStatusInvalid) {
					pStrategy->sellList[i] = 0;
					idx = i;
					break;
				}
			}
		}
	}

	return (idx);
}

static XBool BasketBuy(XStrategyT *pStrategy, XInt updateTime) {
	XNum idx = 0;
	XOrderReqT order = { 0 };
	XLocalId localid = 0;

	idx = XGetBuyPos(pStrategy);
	if (idx < 0) {
		return (false);
	}

	CPStrategy2Ord(pStrategy, &order);

	order.bsType = eXBuy;
	order.ordPrice = pStrategy->setting.ordPx;
	order.ordQty = pStrategy->setting.ordQty;
	order.ordType = eXOrdLimit;
	order._mktTime = updateTime;

	localid = XPutOrderReq(&order);
	pStrategy->buyList[idx] = localid;
	pStrategy->_buySends += order.ordQty;
	pStrategy->_lastBuyPx = order.ordPrice;
	pStrategy->_buyLocTime = XGetClockTime();
	pStrategy->_slipBuyTimes++;
	slog_info(0, "5.[%lld-%d-%s], 委托本地编号[%d], 委托时间[%d-%lld], idx[%lld]",
			pStrategy->plotid, pStrategy->setting.market,
			pStrategy->setting.securityId, localid, updateTime,
			XNsTime2I(pStrategy->_buyLocTime), idx);

	return (true);
}

static XBool BasketSell(XStrategyT *pStrategy, XInt updateTime) {
	XNum idx = 0;
	XOrderReqT order = { 0 };
	XLocalId localid = 0;

	idx = XGetSellPos(pStrategy);
	if (idx < 0) {
		return (false);
	}

	CPStrategy2Ord(pStrategy, &order);

	order.bsType = eXSell;
	order.ordPrice = pStrategy->setting.ordPx;
	order.ordQty = pStrategy->setting.ordQty;
	order.ordType = eXOrdLimit;
	order._mktTime = updateTime;

	localid = XPutOrderReq(&order);
	pStrategy->sellList[idx] = localid;
	pStrategy->_sellSends += order.ordQty;
	pStrategy->_lastSellPx = order.ordPrice;
	pStrategy->_sellLocTime = XGetClockTime();
	pStrategy->_slipSellTimes++;
	slog_info(0, "5.[%lld-%d-%s], 委托本地编号[%d], 委托时间[%d-%lld], idx[%lld]",
			pStrategy->plotid, pStrategy->setting.market,
			pStrategy->setting.securityId, localid, updateTime,
			XNsTime2I(pStrategy->_sellLocTime), idx);

	return (true);
}

XBool XGetExchValidBuyRsp(XStrategyT *pStrategy) {
	XOrderT *pOrder = NULL;
	XNum i = 0;
	XBool bvalid = false; //默认允许报单

	for (i = 0; i < MAX_GRID_LEVEL; i++) {
		//如果有挂单且挂单是废单时,允许报单，否则不允许报单
		if (0 != pStrategy->buyList[i]) {

			//没有得到响应,不继续报单;得到相应不是确认、成交、撤单、资金不足的继续报单
			pOrder = XFndVOrderByLoc(pStrategy->plot.customerId,
					pStrategy->envno, pStrategy->buyList[i]);

			if (NULL != pOrder) {
				if(pOrder->_cnfExTime)
				{
					pStrategy->cfmTime = pOrder->_cnfExTime;
				}
				switch (pOrder->ordStatus) {

				//已确认为交易所确认时才进行处理
				case eXOrdStatusDeclared:
					if (pStrategy->cfmTime) {
						bvalid = true;
					}
					break;
				case eXOrdStatusCanceled:
				case eXOrdStatusPCanceled:
				case eXOrdStatusFilled:
					bvalid = true;
					break;
				case eXOrdStatusInvalid:
					if (pOrder->errorno == XEMONEY_IS_NOT_ENOUGH) {
						bvalid = true;
					} else {
						pStrategy->buyList[i] = 0;
					}
					break;

				default:
					break;
				}
			}
		}
		if (bvalid) {
			break;
		}
	}

	return (bvalid);
}

XBool XGetExchValidBuyTime(XStrategyT *pStrategy, XLongTime curTime) {
	XOrderT *pOrder = NULL;
	XNum i = 0;
	XBool bvalid = false; //默认允许报单

	for (i = 0; i < MAX_GRID_LEVEL; i++) {
		//如果有挂单且挂单是废单时,允许报单，否则不允许报单
		if (0 != pStrategy->buyList[i]) {

			//没有得到响应,不继续报单;得到相应不是确认、成交、撤单、资金不足的继续报单
			pOrder = XFndVOrderByLoc(pStrategy->plot.customerId,
					pStrategy->envno, pStrategy->buyList[i]);

			//正常响应是在480us
			if (NULL != pOrder) {
				if (pOrder->_cnfExTime == 0
						&& curTime > pOrder->_sendLocTime + 1500000LL) {
					bvalid = true;
				}
			}
		}
		if (bvalid) {
			break;
		}
	}

	return (bvalid);
}

XBool XGetExchValidSellTime(XStrategyT *pStrategy, XLongTime curTime) {
	XOrderT *pOrder = NULL;
	XNum i = 0;
	XBool bvalid = false; //默认允许报单

	for (i = 0; i < MAX_GRID_LEVEL; i++) {
		//如果有挂单且挂单是废单时,允许报单，否则不允许报单
		if (0 != pStrategy->sellList[i]) {

			//没有得到响应,不继续报单;得到相应不是确认、成交、撤单、资金不足的继续报单
			pOrder = XFndVOrderByLoc(pStrategy->plot.customerId,
					pStrategy->envno, pStrategy->sellList[i]);

			//正常响应是在480us
			if (NULL != pOrder) {
				if (pOrder->_cnfExTime == 0
						&& curTime > pOrder->_sendLocTime + 1500000LL) {
					bvalid = true;
				}
			}
		}
		if (bvalid) {
			break;
		}
	}

	return (bvalid);
}

XBool XGetExchValidSellRsp(XStrategyT *pStrategy) {
	XOrderT *pOrder = NULL;
	XNum i = 0;
	XBool bvalid = false; //默认允许报单

	for (i = 0; i < MAX_GRID_LEVEL; i++) {
		//如果有挂单且挂单是废单时,允许报单，否则不允许报单
		if (0 != pStrategy->sellList[i]) {

			//没有得到响应,不继续报单;得到相应不是确认、成交、撤单、资金不足的继续报单
			pOrder = XFndVOrderByLoc(pStrategy->plot.customerId,
					pStrategy->envno, pStrategy->sellList[i]);

			if (NULL != pOrder) {
				if(pOrder->_cnfExTime)
				{
					pStrategy->cfmTime = pOrder->_cnfExTime;
				}
				switch (pOrder->ordStatus) {

				//已确认为交易所确认时才进行处理
				case eXOrdStatusDeclared:
					if (pStrategy->cfmTime) {
						bvalid = true;
					}
					break;
				case eXOrdStatusCanceled:
				case eXOrdStatusPCanceled:
				case eXOrdStatusFilled:
					bvalid = true;
					break;
				case eXOrdStatusInvalid:
					if (pOrder->errorno == XEMONEY_IS_NOT_ENOUGH) {
						bvalid = true;
					} else {
						pStrategy->sellList[i] = 0;
					}
					break;

				default:
					break;
				}
				/**
				 slog_debug(0, "[%lld-%d-%s], 状态[%d], 交易所确认时间[%lld], 有效[%d]",
				 pStrategy->plotid, pStrategy->setting.market,
				 pStrategy->setting.securityId, pOrder->ordStatus,
				 pOrder->_cnfExTime, bvalid);
				 */
			}
		}
		if (bvalid) {
			break;
		}
	}

	return (bvalid);
}

#define XTRY_SEND_TIMES        (5)
static XBool DealFirstBuyOrder(XStrategyT *pStrategy, XLongTime lCurTime,
		XInt updateTime) {
	XBool bSendOk = false;

	// == 5 没有状态或者订单未生成, false，不会再报单
	// == 5 有订单生成，但是状态为确认,当前订单不继续报
	// == 5 有订单生成，但是状态为废单,则状态为置报单为0，以便后续继续报单
	bSendOk = XGetExchValidBuyRsp(pStrategy);

	//临近91454000时，下单频率变高,否则500ms下单一次
	if (pStrategy->cfmTime < 91454000) {
		if (!bSendOk && 0 == pStrategy->buyList[0]) {
			if (lCurTime > pStrategy->_buyLocTime + 500000000LL) {
				BasketBuy(pStrategy, updateTime);
				slog_debug(0, "[%lld-%d-%s],确认时间[%d]", pStrategy->plotid, pStrategy->setting.market, pStrategy->setting.securityId, pStrategy->cfmTime);
			}
		}
	} else {
		if (!bSendOk && !pStrategy->buyList[XTRY_SEND_TIMES]) {

			//根据反馈时间决定延迟报单时间
			if (lCurTime > pStrategy->_buyLocTime + 2100000LL)
			{
				BasketBuy(pStrategy, updateTime);
				slog_debug(0, "[%lld-%d-%s],确认时间[%d]", pStrategy->plotid, pStrategy->setting.market, pStrategy->setting.securityId, pStrategy->cfmTime);
			}
		}

	}
	//如果超过对应的响应时间间隔,认为开盘
	if (!bSendOk) {
		bSendOk = XGetExchValidBuyTime(pStrategy, lCurTime);
		if(bSendOk)
		{
			slog_debug(0, "[%lld-%d-%s],超时", pStrategy->plotid, pStrategy->setting.market, pStrategy->setting.securityId);
		}
	}

	return (bSendOk);
}

static void CancelFirstBuyOrder(XStrategyT *pStrategy) {
	XInt i = 0;
	XOrderT *pOrder = NULL;
	XBool bFirst = true;

	if (NULL == pStrategy) {
		return;
	}

	for (i = 0; i < XTRY_SEND_TIMES; i++) {
		if (0 == pStrategy->buyList[i]) {
			continue;
		}

		pOrder = XFndVOrderByLoc(pStrategy->plot.customerId, pStrategy->envno,
				pStrategy->buyList[i]);

		//如果是第一个有效的确认,不撤单；后续的撤单
		if (NULL != pOrder && eXOrdStatusDeclared == pOrder->ordStatus) {
			//发送撤单
			if (!bFirst && !pOrder->_isSendCtrl) {
				XCancelByLocalId(pStrategy, pStrategy->buyList[i]);
			}
			bFirst = false;
		}
	}

}

static XBool AuctionBuy(XStrategyT *pStrategy, XSnapshotT *snapshot,
		XMonitorMdT *pMonitorMd, XBool bFirst, XBool bSendOk) {
	XLongTime lCurTime = 0, calTime = 0;

	lCurTime = XGetClockTime();
	/** 在9:19:30看当前行情进行撤单 */
	if (snapshot->updateTime >= pStrategy->plot.endTime
			|| pMonitorMd->updateTime >= pStrategy->plot.endTime) {

		if (pStrategy->_buyTrades >= pStrategy->setting.ordQty) {
			slog_debug(0, "[%lld-%d-%s],已全部成交,暂停", pStrategy->plotid,
					pStrategy->setting.market, pStrategy->setting.securityId);
			pStrategy->status = eXPlotStop;

			return (bSendOk);
		}

		if (pStrategy->version != snapshot->version
				&& snapshot->updateTime < 93000000
				&& snapshot->updateTime > 0) {
			slog_info(0,
					"[%lld-%d-%s],时间[%d],买一价[%d(%d)],卖二量[%lld],买一量[%lld],买二量[%lld], 封单比例[%d]",
					pStrategy->plotid, pStrategy->setting.market,
					pStrategy->setting.securityId, snapshot->updateTime,
					snapshot->bid[0], pStrategy->setting.conPx,
					snapshot->askqty[1], snapshot->bidqty[0],
					snapshot->bidqty[1], pStrategy->setting.cdl);
		}
		//不能按最新行情来触发，在临界点后可能存在无行情,导致无法撤单
		//只要在9:19:57之后，卖一有数量就撤单
		if (snapshot->updateTime < MARKET_CALL_MID
				&& snapshot->updateTime > 0) {
			if ((snapshot->bid[0] != pStrategy->setting.conPx
					|| snapshot->askqty[1] != 0
					|| snapshot->bidqty[1]
							< pStrategy->setting.cdl * 0.0001
									* (snapshot->bidqty[0] + snapshot->bidqty[1])) /* 封单数量 */) {

				XCancelBuy(pStrategy, 1);

			}
		}
		//超过9：30只要不满足封涨停
		else if (snapshot->updateTime >= MARKET_MORN_BEGIN) {

			if (snapshot->version == pStrategy->version) {
				return (bSendOk);
			}

			if ((snapshot->askqty[0] != 0
					|| snapshot->bid[0] != pStrategy->setting.conPx
					|| (snapshot->bid[0] == pStrategy->setting.conPx
							&& snapshot->bidqty[0] - snapshot->askqty[0]
									< pStrategy->setting.cdl * 0.0001
											* pStrategy->setting.askQty))) {

				XCancelBuy(pStrategy, 1);

			}
		}
	}
	//只要时间小于91545秒，都可以报单，防止2个交易所时间不一致导致的问题
	else if (pMonitorMd->updateTime < XAUCTION_ROB_ATIME
			&& snapshot->updateTime < XAUCTION_ROB_ETIME) {

		//应该报出去的时间
		calTime =
				(XLongTime) (XMsC2S(XAUCTION_ROB_BTIME)
						- XMsC2S(pMonitorMd->_mktFirstTime)
						- pStrategy->plot.ordGapTime) * 1000000
						+ pMonitorMd->_locFirstTime;

		if (bFirst && lCurTime > pStrategy->_calTime + 50000000000LL) {
			/**
			 slog_debug(0,
			 "[%lld-%d-%s], 本地时间[%d],计算报单时间[%d],敲门时间[%d],第一笔行情时间[%d]",
			 pStrategy->plotid, pStrategy->setting.market,
			 pStrategy->setting.securityId, XNsTime2I(lCurTime),
			 XNsTime2I(calTime), XAUCTION_ROB_BTIME,
			 pMonitorMd->_mktFirstTime);
			 */
			pStrategy->_calTime = lCurTime;
		}

		//收到开盘后行情就不再继续报单
		if (lCurTime >= calTime) {

			if (bFirst) {
				bSendOk = DealFirstBuyOrder(pStrategy, lCurTime,
						snapshot->updateTime);
				if (!bSendOk) {
					return (bSendOk);
				} else {
					CancelFirstBuyOrder(pStrategy);
				}
			} else {
				XGetExchValidBuyRsp(pStrategy);
				if (bSendOk && 0 == pStrategy->buyList[0]) {
					BasketBuy(pStrategy, snapshot->updateTime);
				}
			}
		}
	}

	return (bSendOk);
}

static XBool AuctionSell(XStrategyT *pStrategy, XSnapshotT *snapshot,
		XMonitorMdT *pMonitorMd, XBool bFirst, XBool bSendOk) {

	XLongTime lCurTime = 0, calTime = 0;

	lCurTime = XGetClockTime();

	if (pMonitorMd->updateTime < XAUCTION_ROB_ATIME
			&& snapshot->updateTime < XAUCTION_ROB_ETIME) {

		//应该报出去的时间
		calTime =
				(XLongTime) (XMsC2S(XAUCTION_ROB_BTIME)
						- XMsC2S(pMonitorMd->_mktFirstTime)
						- pStrategy->plot.ordGapTime) * 1000000
						+ pMonitorMd->_locFirstTime;

		if (lCurTime > pStrategy->_calTime + 50000000000LL) {
			slog_debug(0,
					"[%lld-%d-%s], 本地时间[%d],计算报单时间[%d],敲门时间[%d],第一笔行情时间[%d]",
					pStrategy->plotid, pStrategy->setting.market,
					pStrategy->setting.securityId, XNsTime2I(lCurTime),
					XNsTime2I(calTime), XAUCTION_ROB_BTIME,
					pMonitorMd->_mktFirstTime);
			pStrategy->_calTime = lCurTime;
		}

		//市场第一笔报成功后,报后续订单,后续订单不等收到订单状态
		//收到开盘后行情就不再继续报单
		if (lCurTime >= calTime) {
			if (bFirst) {
				bSendOk = XGetExchValidSellRsp(pStrategy);
				if (!bSendOk) {
					bSendOk = XGetExchValidSellTime(pStrategy, lCurTime);
				}
				if (0 == pStrategy->sellList[0]) {
					BasketSell(pStrategy, snapshot->updateTime);
				}
			} else {
				XGetExchValidSellRsp(pStrategy);
				if (bSendOk && 0 == pStrategy->sellList[0]) {
					BasketSell(pStrategy, snapshot->updateTime);
				}
			}
		}
	}

	return (bSendOk);
}

static void TradeByMarket(XIdx iTPlot, XMonitorMdT *pMonitorMd, XInt market) {
	XIdx i;
	XSnapshotT *pExSnapshot = NULL, exsnap;
	XStrategyT *pStrategy = NULL;
	XStockT *pStock = NULL;
	XBool bSendOk = false;
	XLongTime lCurTime = 0, calTime = 0;
	XBool bFirst = true;

	lCurTime = XGetClockTime();

	//只看市场的第一笔报单,如果成功,后续就不看状态,直接结束不再报单，看撤单;如果第一笔报单未发送成功,则重复报只到成功
	for (i = 0; i < iTPlot; i++) {
		pStrategy = XFndVStrategyById(i + 1);
		if (NULL == pStrategy || pStrategy->status != eXPlotNormal
				|| pStrategy->plot.plotType != eXTest) {
			continue;
		}
		if (market != pStrategy->setting.market) {
			continue;
		}

		pStock = XFndVStockByKey(pStrategy->setting.market,
				pStrategy->setting.securityId);
		if (NULL == pStock) {
			continue;
		}

		memset(&exsnap, 0, XSNAPSHOT_SIZE);

		// 找到快照位置
		pExSnapshot = XFndVSnapshotByKey(pStrategy->setting.market,
				pStrategy->setting.securityId);
		if (NULL != pExSnapshot) {
			memcpy(&exsnap, pExSnapshot, XRSNAPSHOT_SIZE);
		}

		if (eXBuy == pStrategy->setting.bsType) {
			bSendOk = AuctionBuy(pStrategy, &exsnap, pMonitorMd, bFirst,
					bSendOk);
		} else {
			bSendOk = AuctionSell(pStrategy, &exsnap, pMonitorMd, bFirst,
					bSendOk);
		}
		if (exsnap.version > 0) {
			pStrategy->version = exsnap.version;
		}
		bFirst = false;

	}

}

int main(int argc, char *argv[]) {
	XMonitorT *pMonitor = NULL;
	XMonitorMdT *pMonitorMd = NULL;

	xslog_init(XSHM_SDB_FILE, "xauction");
	XManShmLoad();

	slog_debug(0, "xauction版本[%s]", __XMAN_VERSION__);

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return (-1);
	}
	pMonitorMd = XFndVMdMonitor(eXExchSec);

	for (;;) {
		if (isMktClosedTime(pMonitorMd->updateTime)) {
			break;
		}
		if (!pMonitorMd->_mktFirstTime) {
			continue;
		}

		TradeByMarket(pMonitor->iTPlot, pMonitorMd, eXMarketSha);
		TradeByMarket(pMonitor->iTPlot, pMonitorMd, eXMarketSza);
	}

	slog_info(3, "xauction 关闭");

	return (0);
}
