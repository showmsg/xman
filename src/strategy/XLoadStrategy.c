/*
 * @file XLoadStrategy.c
 * @brief 策略加载
 * @author kangyq
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */

#include <getopt.h>
#include "XINI.h"
#include "XCom.h"
#include "XLog.h"
#include "XCSV.h"
#include "XUtils.h"

/** 最大篮子数量 */
#define MAX_BASKET_CNT (3000)

int read_basket(const char *trade_file, XSettingT trade_list[]) {
	const char *col = NULL;
	unsigned rowcount = 0;
	XCsvHandleT handle;
	XInt iret = -1;

	iret = XCsvOpen(&handle, trade_file);
	if (iret) {
		slog_error(0, "文件不存在[%s]", trade_file);
		return (0);
	}

	while ((!XCsvReadLine(&handle))) {
		if (rowcount >= MAX_BASKET_CNT) {
			slog_error(0, "篮子数量超过最大限制");
			break;
		}
		if (handle.colSize < 12) {
			slog_error(0, "列数错误");
			break;
		}
		col = handle.GetFieldByCol(&handle, 0);
		if (col) {
			trade_list[rowcount].market = atoi(col);
		}
		col = handle.GetFieldByCol(&handle, 1);
		if (col) {
			memcpy(trade_list[rowcount].securityId, col, strlen(col));
		}
		col = handle.GetFieldByCol(&handle, 2);
		if (col) {
			trade_list[rowcount].bsType = atoi(col);
		}

		col = handle.GetFieldByCol(&handle, 3);
		if (col) {
			trade_list[rowcount].conPx = atoi(col);
		}
		col = handle.GetFieldByCol(&handle, 4);
		if (col) {
			trade_list[rowcount].ordPx = atoi(col);
		}

		col = handle.GetFieldByCol(&handle, 5);
		if (col) {
			trade_list[rowcount].qtyType = atoi(col);
		}

		col = handle.GetFieldByCol(&handle, 6);
		if (col) {
			if (!trade_list[rowcount].qtyType) {
				trade_list[rowcount].ordQty = atoi(col);
			} else {
				trade_list[rowcount].money = atoi(col);
			}

		}

		col = handle.GetFieldByCol(&handle, 7);
		if (col) {
			trade_list[rowcount].cdl = atoi(col);
		}

		col = handle.GetFieldByCol(&handle, 8);
		if (col) {
			trade_list[rowcount].askQty = atol(col);
		}

		col = handle.GetFieldByCol(&handle, 9);
		if (col) {
			trade_list[rowcount].buyMoney = atoi(col) * 10000;
		}

		col = handle.GetFieldByCol(&handle, 10);
		if (col) {
			trade_list[rowcount].buyCtrlMoney = atoi(col) * 10000;
		}

		col = handle.GetFieldByCol(&handle, 11);
		if (col) {
			trade_list[rowcount].isnontrade = atoi(col);
		}

		col = handle.GetFieldByCol(&handle, 12);
		if (col) {
			trade_list[rowcount].sign = atoi(col);
		}

		/** 开盘涨幅 */
		col = handle.GetFieldByCol(&handle, 13);
		if (col) {
			trade_list[rowcount].kpzf = atoi(col);
		}

		/** 半路版，涨停卖出挂单倍数 */
		col = handle.GetFieldByCol(&handle, 14);
		if (col) {
			trade_list[rowcount].upperQtyMulty = atoi(col);
		}

		/** 半路版，涨停卖出挂单倍数,开盘15分钟-60 */
		col = handle.GetFieldByCol(&handle, 15);
		if (col) {
			trade_list[rowcount].upperQtyMultyMin = atoi(col);
		}

		/** 封涨后下一分钟成交金额 */
		col = handle.GetFieldByCol(&handle, 16);
		if (col) {
			trade_list[rowcount].nxtCtrlMoney = atoi(col);
		}

		/** 封涨后后续连续2分钟成交金额 */
		col = handle.GetFieldByCol(&handle, 17);
		if (col) {
			trade_list[rowcount].followCtrlMoney = atoi(col);
		}
		trade_list[rowcount].batchSellTimes = 8;

		col = handle.GetFieldByCol(&handle, 18);
		if (col) {
			trade_list[rowcount].highPx = atoi(col);
		}
		col = handle.GetFieldByCol(&handle, 19);
			if (col) {
					trade_list[rowcount].lowPx = atoi(col);
			}

		rowcount++;
	}
	XCsvClose(&handle);

	return (rowcount);
}

int read_plot(XIniT *strategyhndl, const char *section, XPlotT *request) {

	const char *pSectionConf = NULL;

	pSectionConf = XINIGet(strategyhndl, section, "plotType");

	if (NULL == pSectionConf) {
		return (-1);
	}
	if (!strcmp(pSectionConf, "ConRob")) {
		request->plotType = eXMidRobPlot;
		slog_info(3, "盘中抢板策略......");
	} else if (!strcmp(pSectionConf, "AucRob")) {
		request->plotType = eXAuctionRobPlot;
		slog_info(3, "集合竞价抢板策略......");
	} else if (!strcmp(pSectionConf, "HalfRob")) {
		request->plotType = eXHalfRobPlot;
		slog_info(3, "盘中半路抢板策略......");
	} else if (!strcmp(pSectionConf, "Sell")) {
		request->plotType = eXSellPlot;
		slog_info(3, "卖出策略......");
	} else if (!strcmp(pSectionConf, "Test")) {
		request->plotType = eXTest;
		slog_info(3, "测试策略......");
	}
	else if (!strcmp(pSectionConf, "ETF")) {
			request->plotType = eXEtfPlot;
			slog_info(3, "ETF策略......");
	}
	else if(!strcmp(pSectionConf, "T0"))
	{
		request->plotType = eXT0Plot;
		slog_info(3, "T0策略......");
	}
	else if(!strcmp(pSectionConf, "Grid"))
	{
		request->plotType = eXGridPlot;
		slog_info(3, "网格策略......");
	}

	pSectionConf = XINIGet(strategyhndl, section, "isForbidTrade");
	if (NULL != pSectionConf) {
		request->isForbid = atoi(pSectionConf);
	}

	slog_info(3, "[%s]:是否禁止交易{%d}", section, request->isForbid);

	pSectionConf = XINIGet(strategyhndl, section, "customer");
	if (NULL == pSectionConf || 0 == strlen(pSectionConf)) {
		slog_error(0, "[%s]:账户为空", section);
		return (-1);
	} else {
		memcpy(request->customerId, pSectionConf, strlen(pSectionConf));
	}
	slog_info(3, "[%s]:账户{%s}", section, request->customerId);

	pSectionConf = XINIGet(strategyhndl, section, "file");
	if (NULL == pSectionConf) {
		slog_error(0, "[%s]篮子信息为空", section);
		return (-1);
	} else {
		memcpy(request->baskfile, pSectionConf, strlen(pSectionConf));
	}
	slog_info(3, "[%s]:篮子信息[%s]", section, request->baskfile);

	pSectionConf = XINIGet(strategyhndl, section, "allowHoldQty");
	if (NULL != pSectionConf) {
		request->allowHoldQty = atol(pSectionConf);
		slog_info(3, "[%s]:允许持有数量[%d]", section, request->allowHoldQty);
	}

	pSectionConf = XINIGet(strategyhndl, section, "slipBuyTimes");
	if (NULL != pSectionConf) {
		request->slipBuyTimes = atoi(pSectionConf) ? atoi(pSectionConf) : 3;
		slog_info(3, "[%s]:买滑点次数[%d]", section, request->slipBuyTimes);
	} else {
		request->slipBuyTimes = 3;
	}
	pSectionConf = XINIGet(strategyhndl, section, "slipSellTimes");
	if (NULL != pSectionConf) {
		request->slipSellTimes = atoi(pSectionConf) ? atoi(pSectionConf) : 3;
		slog_info(3, "[%s]:卖滑点次数[%d]", section, request->slipSellTimes);
	} else {
		request->slipSellTimes = 3;
	}

	pSectionConf = XINIGet(strategyhndl, section, "ordGapTime");
	if (NULL == pSectionConf) {
		slog_warn(3, "[%s]:委托间隔，为0不撤单", section);
	} else {
		request->ordGapTime = atoi(pSectionConf);
	}
	slog_info(3, "[%s]:委托时间间隔[%d],为0不控制", section, request->ordGapTime);

	pSectionConf = XINIGet(strategyhndl, section, "ctrGapTime");
	if (NULL == pSectionConf) {
		slog_warn(3, "[%s]:撤单时间间隔，为0不撤单", section);
	} else {
		request->ctrGapTime = atoi(pSectionConf);
	}
	slog_info(3, "[%s]:撤单时间间隔[%d]，为0不撤单", section, request->ctrGapTime);

	pSectionConf = XINIGet(strategyhndl, section, "isCtrlStop");
	if (NULL != pSectionConf) {
		request->isCtrlStop = atoi(pSectionConf);
	} else {
		request->isCtrlStop = 0;
	}
	slog_info(3, "[%s]:撤单后是否停止报单[%d]，为0不停止", section, request->isCtrlStop);

	pSectionConf = XINIGet(strategyhndl, section, "isOpenStop");
	if (NULL != pSectionConf) {
		request->isOpenStop = atoi(pSectionConf);
	} else {
		request->isOpenStop = 1;
	}
	slog_info(3, "[%s]:开盘涨停后是否停止报单[%d]，为0不停止", section, request->isCtrlStop);

	pSectionConf = XINIGet(strategyhndl, section, "isUpperStop");
	if (NULL != pSectionConf) {
		request->isUpperStop = atoi(pSectionConf);
	} else {
		request->isUpperStop = 0;
	}
	slog_info(3, "[%s]:涨停打开后是否停止报单[%d]，为0不停止", section, request->isUpperStop);

	pSectionConf = XINIGet(strategyhndl, section, "beginTime");
	if (NULL != pSectionConf) {
		request->beginTime = atoi(pSectionConf);
	}
	slog_info(3, "[%s]:开始时间[%d]", section, request->beginTime);

	pSectionConf = XINIGet(strategyhndl, section, "endTime");
	if (NULL != pSectionConf) {
		request->endTime = atoi(pSectionConf);
	}
	slog_info(3, "[%s]:结束时间[%d]", section, request->endTime);

	pSectionConf = XINIGet(strategyhndl, section, "isAutoCtrl");
	if (NULL != pSectionConf) {
		request->isAutoCtrl = atoi(pSectionConf);
	} else {
		request->isAutoCtrl = 1;
	}
	slog_info(3, "[%s]:是否允许自动撤单[%d]", section, request->isAutoCtrl);

	pSectionConf = XINIGet(strategyhndl, section, "ctrlUpRatio");
	if (NULL != pSectionConf) {
		request->ctrlUpRatio = atoi(pSectionConf);
	} else {
		request->ctrlUpRatio = 1000;
	}
	slog_info(3, "[%s]:是否允许自动撤单[%d]", section, request->ctrlUpRatio);

	return (0);
}

XInt XLoadHis(XInt market, XChar *security, XPlotId plotid) {
	XIdx i;
	XOrderT *pFndOrder = NULL;
	XStrategyT *pStrategy = NULL;
	XMonitorT *pMonitor = NULL;

	pMonitor = XFndVMonitor();
	if (NULL == pMonitor) {
		return (-1);
	}

	for (i = 0; i < pMonitor->iTOrder; i++) {

		pFndOrder = XFndVOrderById(i + 1);
		if (NULL == pFndOrder) {
			continue;
		}

		// 找到对应的策略
		if (!pFndOrder->_locFlag && plotid == pFndOrder->request.plotid) {
			pStrategy = XFndVStrategyByKey(pFndOrder->request.customerId,
					pFndOrder->request.plotid);
			if (NULL != pStrategy
					&& pStrategy->setting.market == pFndOrder->request.market
					&& 0
							== strcmp(pFndOrder->request.securityId,
									pStrategy->setting.securityId)) {

				switch (pFndOrder->ordStatus) {
				case eXOrdStatusDeclared:
				case eXOrdStatusPFilled:
				case eXOrdStatusFilled:
					if (pFndOrder->request.bsType == eXBuy) {
						pStrategy->_buyValid += pFndOrder->request.ordQty;
						pStrategy->_buyTrades += pFndOrder->trdQty;
					} else {
						pStrategy->_sellValid += pFndOrder->request.ordQty;
						pStrategy->_sellTrades += pFndOrder->trdQty;
					}
					break;
				case eXOrdStatusPCanceled:
					if (pFndOrder->request.bsType == eXBuy) {
						pStrategy->_buyValid += pFndOrder->trdQty;
						pStrategy->_buyTrades += pFndOrder->trdQty;
					} else {
						pStrategy->_sellValid += pFndOrder->trdQty;
						pStrategy->_sellTrades += pFndOrder->trdQty;
					}
					break;
				default:
					break;
				}
			}
		}
	}
	return (0);
}

XInt PushStrategy(XStrategyT *pStrategy) {
	XInt iret = -1;
	XTradeCache webCache = { 0 };

	webCache.head.type = eDStrategy;
	webCache.head.dataLen = XSTRATEGY_SIZE;
	webCache.strategy = *pStrategy;

	XPushCache(XSHMKEYCONECT(rtnCache), &webCache);

	return (iret);
}

int init_strategy(XPlotT *request) {
	XInt iret = -1;
	XNum iBuyholdCnt = 0, i;
	XSettingT basketList[MAX_BASKET_CNT];
	XStockT *pStock = NULL;
	XStrategyT *pStrategy = NULL, strategy;
	XInvestT *pInvest = NULL;
	XMarketSecurityT subsec = { 0 };
	XIdx idx = -1;
	XHoldT *pHold = NULL;

	memset(basketList, 0, MAX_BASKET_CNT * sizeof(XSettingT));
	iBuyholdCnt = read_basket(request->baskfile, basketList);
	slog_info(0, "读取篮子文件[%s] (%d)条", request->baskfile, iBuyholdCnt);

	for (i = 0; i < iBuyholdCnt; i++) {
		pStock = XFndVStockByKey(basketList[i].market,
				basketList[i].securityId);
		if (NULL == pStock) {
			slog_error(0, "未找到证券信息[%d-%s]", basketList[i].market,
					basketList[i].securityId);
			continue;
		}

		idx = XFndCustomerByKey(request->customerId);
		if (idx < 1) {
			slog_error(0, "客户号未找到[%s]", request->customerId);
			continue;
		}

		pStrategy = XFndVStrategyByFrontId(request->customerId, 0,
				request->frontId, basketList[i].securityId,
				basketList[i].bsType);
		if (NULL == pStrategy) {
			// 添加策略
			memset(&strategy, 0, XSTRATEGY_SIZE);
			strategy.idx = XGetIdByType(eSPlot);

			strategy.acctType = eXInvSpot;

			/************************************* 从plot.conf *******************************************************/

			pInvest = XFndVInvestByAcctType(request->customerId,
					basketList[i].market, eXInvSpot);
			if (NULL != pInvest) {
				memcpy(strategy.investId, pInvest->investId, INVESTID_LEN);
			}

			strategy.plotid = XGenPlotId(request->plotType, strategy.idx);

			memcpy(&(strategy.plot), request, sizeof(XPlotT));

			/************************************************* 从篮子中读取 ******************************************/
			memcpy(&(strategy.setting), &basketList[i], sizeof(XSettingT));

			strategy.setting.plotid = strategy.plotid;
			if (eXBuy == basketList[i].bsType && basketList[i].conPx == -1) {
				strategy.setting.conPx = pStock->upperPrice;
			}
			else if (eXSell == basketList[i].bsType && basketList[i].conPx == -1) {
				strategy.setting.conPx = pStock->lowerPrice;
			} else {
				strategy.setting.conPx = basketList[i].conPx;
			}

			// 如果未设置报单价格，以涨停价买入
			if (eXBuy == basketList[i].bsType && basketList[i].ordPx == -1) {
				strategy.setting.ordPx = pStock->upperPrice;
			}
			//涨停信号时以跌停价下单
			else if(eXBuy == basketList[i].bsType && basketList[i].ordPx == 0)
			{
				strategy.setting.ordPx = pStock->lowerPrice;
			}
			else if (eXSell == basketList[i].bsType && basketList[i].ordPx == -1) {
				strategy.setting.ordPx = pStock->lowerPrice;
			} else {
				strategy.setting.ordPx = basketList[i].ordPx;
			}

			if (basketList[i].qtyType) {
				if (pStock->upperPrice && pStock->lmtBuyMinQty) {
					strategy.setting.ordQty = basketList[i].money * 10000
							/ (pStock->upperPrice * pStock->lmtBuyMinQty)
							* pStock->lmtBuyMinQty;

					//默认交易200股
					if (!strategy.setting.ordQty) {
						strategy.setting.ordQty = 200;
					}
				} else {
					slog_warn(0, "[%d-%s]无涨停价", basketList[i].market,
							basketList[i].securityId);
				}
			} else {
				strategy.setting.ordQty = basketList[i].ordQty;
			}

			//如果是卖,检查持仓
			if (strategy.setting.bsType == eXSell) {
				pHold = XFndVHoldByKey(strategy.plot.customerId,
						strategy.investId, strategy.setting.market,
						strategy.setting.securityId);

				if (strategy.setting.ordQty) {
					if (NULL != pHold
							&& strategy.setting.ordQty != pHold->sellAvlHld) {
						slog_warn(0,
								"[%s-%lld] 证券代码[%d-%s],卖出时委托数量[%d]不等于可卖数量[%lld]",
								strategy.plot.customerId, strategy.plotid,
								strategy.setting.market,
								strategy.setting.securityId,
								strategy.setting.ordQty, pHold->sellAvlHld);
					}
				} else {
					if (NULL != pHold) {
						strategy.setting.ordQty = pHold->sellAvlHld;
						slog_warn(0,
								"[%s-%lld] 证券代码[%d-%s],未设置卖出数量,以持仓可卖数量进行卖出[%lld]",
								strategy.plot.customerId, strategy.plotid,
								strategy.setting.market,
								strategy.setting.securityId, pHold->sellAvlHld);
					}
				}

			}

			if (basketList[i].isnontrade || request->isForbid
					|| strategy.setting.ordQty <= 0) {
				strategy.status = eXPlotStop;
			} else {
				strategy.status = eXPlotNormal;
			}
			/********************************************************************************************************/

			if (strategy.plot.allowHoldQty == 0) {
				strategy.plot.allowHoldQty = strategy.setting.ordQty;
			}

			XPutVStrategy(&strategy);

			subsec.type = exAppend;
			subsec.market = strategy.setting.market;
			memcpy(subsec.securityId, strategy.setting.securityId,
					strlen(strategy.setting.securityId));
			XPushCache(XSHMKEYCONECT(mktSubscribe), &subsec);

			XLoadHis(strategy.setting.market, strategy.setting.securityId,
					strategy.plotid);

			PushStrategy(&strategy);
			slog_info(3,
					"添加策略[%s-%lld],证券代码[%d-%s],信号价格[%d],买卖价格[%d],委托数量[%d],买一撤单率[%d],卖一量[%lld],买一委托金额[%lld], 买入当天最高价[%d],买入当天最低价[%d],状态[%d],方案[%d]",
					strategy.plot.customerId, strategy.plotid,
					strategy.setting.market, strategy.setting.securityId,
					strategy.setting.conPx, strategy.setting.ordPx,
					strategy.setting.ordQty, strategy.setting.cdl,
					strategy.setting.askQty, strategy.setting.buyMoney,
					strategy.setting.highPx, strategy.setting.lowPx,
					strategy.status, strategy.setting.sign);
		} else {
			/******************************** 从plot.conf更新 *************************************************/
			pStrategy->plot.ordGapTime = request->ordGapTime;
			pStrategy->plot.ctrGapTime = request->ctrGapTime;
			pStrategy->plot.beginTime = request->beginTime;
			pStrategy->plot.endTime = request->endTime;
			pStrategy->plot.isCtrlStop = request->isCtrlStop;
			pStrategy->plot.slipBuyTimes = request->slipBuyTimes;
			pStrategy->plot.slipSellTimes = request->slipSellTimes;
			pStrategy->plot.allowHoldQty = request->allowHoldQty;
			pStrategy->plot.isUpperStop = request->isUpperStop;
			pStrategy->plot.isAutoCtrl = request->isAutoCtrl;
			pStrategy->plot.ctrlUpRatio = request->ctrlUpRatio;
			/******************************* 从篮子中更新 *****************************************************/
			if (eXBuy == basketList[i].bsType && basketList[i].conPx == -1) {
				pStrategy->setting.conPx = pStock->upperPrice;
			} else if (eXSell == basketList[i].bsType && basketList[i].conPx == -1) {
				pStrategy->setting.conPx = pStock->lowerPrice;
			} else {
				pStrategy->setting.conPx = basketList[i].conPx;
			}

			if (eXBuy == basketList[i].bsType && basketList[i].ordPx == -1) {
				pStrategy->setting.ordPx = pStock->upperPrice;
			}
			//涨停信号时以跌停价下单
			else if(eXBuy == basketList[i].bsType && basketList[i].ordPx == 0)
			{
				pStrategy->setting.ordPx = pStock->lowerPrice;
			}
			else if (eXSell == basketList[i].bsType && basketList[i].ordPx == -1) {
				pStrategy->setting.ordPx = pStock->lowerPrice;
			} else {
				pStrategy->setting.ordPx = basketList[i].ordPx;
			}

			if (basketList[i].qtyType) {
				if (pStock->upperPrice && pStock->lmtBuyMinQty) {
					pStrategy->setting.ordQty = basketList[i].money * 10000
							/ (pStock->upperPrice * pStock->lmtBuyMinQty)
							* pStock->lmtBuyMinQty;

					//如果资金不够，默认交易200
					if (!pStrategy->setting.ordQty) {
						pStrategy->setting.ordQty = 200;
					}
				} else {
					slog_warn(0, "[%d-%s]无涨停价", basketList[i].market,
							basketList[i].securityId);
				}
			} else {
				pStrategy->setting.ordQty = basketList[i].ordQty;
			}

			//如果是卖,检查持仓
			if (pStrategy->setting.bsType == eXSell) {
				pHold = XFndVHoldByKey(pStrategy->plot.customerId,
						pStrategy->investId, pStrategy->setting.market,
						pStrategy->setting.securityId);

				if (pStrategy->setting.ordQty) {
					if (NULL != pHold
							&& pStrategy->setting.ordQty != pHold->sellAvlHld) {
						slog_warn(0,
								"[%s-%lld] 证券代码[%d-%s],卖出时委托数量[%d]不等于可卖数量[%lld]",
								pStrategy->plot.customerId, pStrategy->plotid,
								pStrategy->setting.market,
								pStrategy->setting.securityId,
								pStrategy->setting.ordQty, pHold->sellAvlHld);
					}
				} else {
					if (NULL != pHold) {
						pStrategy->setting.ordQty = pHold->sellAvlHld;
						slog_warn(0,
								"[%s-%lld] 证券代码[%d-%s],未设置卖出数量,以可卖数量进行卖出[%lld]",
								pStrategy->plot.customerId, pStrategy->plotid,
								pStrategy->setting.market,
								pStrategy->setting.securityId,
								pHold->sellAvlHld);
					}
				}

			}

			pStrategy->setting.cdl = basketList[i].cdl;
			pStrategy->setting.askQty = basketList[i].askQty;
			pStrategy->setting.buyMoney = basketList[i].buyMoney;
			pStrategy->setting.buyCtrlMoney = basketList[i].buyCtrlMoney;
			pStrategy->setting.batchSellTimes = basketList[i].batchSellTimes;
			pStrategy->setting.lowPx = basketList[i].lowPx;
			pStrategy->setting.highPx = basketList[i].highPx;
			pStrategy->setting.sign = basketList[i].sign;
			pStrategy->setting.upperQtyMulty = basketList[i].upperQtyMulty;
			pStrategy->setting.upperQtyMultyMin = basketList[i].upperQtyMultyMin;

			pStrategy->setting.followCtrlMoney = basketList[i].followCtrlMoney;
			pStrategy->setting.nxtCtrlMoney = basketList[i].nxtCtrlMoney;
			if (basketList[i].isnontrade || request->isForbid
					|| pStrategy->setting.ordQty <= 0) {
				pStrategy->status = eXPlotStop;
			} else {
				pStrategy->status = eXPlotNormal;
			}

			/*****************************************************************************************************************/

			if (pStrategy->plot.allowHoldQty == 0) {
				pStrategy->plot.allowHoldQty = pStrategy->setting.ordQty;
			}

			XUpdVStrategyByKey(pStrategy);

			PushStrategy(pStrategy);
			slog_info(3,
					"修改策略[%s-%lld],证券代码[%d-%s],信号价格[%d],买卖价格[%d],委托数量[%d],买一撤单率[%d],卖一量[%lld],买一委托金额[%lld], 买入当天最高价[%d],买入当天最低价[%d],状态[%d]，方案[%d]",
					pStrategy->plot.customerId, pStrategy->plotid,
					pStrategy->setting.market, pStrategy->setting.securityId,
					pStrategy->setting.conPx, pStrategy->setting.ordPx,
					pStrategy->setting.ordQty, pStrategy->setting.cdl,
					pStrategy->setting.askQty, pStrategy->setting.buyMoney,
					pStrategy->setting.highPx, pStrategy->setting.highPx,
					pStrategy->status, pStrategy->setting.sign);
		}
	}
	return (iret);
}

int read_strategy(const char *strategyfile, int iSectionIdx) {
	XIniT *strategyhndl = NULL;
	const char *pSectionConf = NULL;
	char sTempSection[20];
	XPlotT request = { 0 };
	XInt iret = -1;
	XChar plotfile[256];

	memset(plotfile, 0, sizeof(plotfile));
	memset(sTempSection, 0, sizeof(sTempSection));
	sprintf(sTempSection, "%d", iSectionIdx);

	strategyhndl = XINILoad(strategyfile);
	if (!strategyhndl) {
		slog_error(0, "读取[%s]失败", strategyfile);
		return (-1);
	}

	pSectionConf = XINIGet(strategyhndl, sTempSection, "plottype");
	if (NULL == pSectionConf) {
		return (-1);
	}
	request.frontId = iSectionIdx;
	iret = read_plot(strategyhndl, sTempSection, &request);

	if (iret) {
		return (-1);
	}

	init_strategy(&request);

	return (0);
}

static void usage() {
	printf("###############################################################\n");
	printf("#    XMan策略交易平台(%s) \n", __XMAN_VERSION__);
	printf("# -h [help] 帮助\n");
	printf("# 默认不带参数,加载data\\plot.conf策略\n");
	printf(
			"# -t [type]操作类型,-1:默认,0:帮助,1:按证券代码+本地单号修改测量,2:按策略编号修改,3:按证券代码修改所有策略状态,4:按本地单号修改策略状态\n");
	printf("# -c [customer] 客户号\n");
	printf("# -m [market] 市场,1:上海,2:深圳\n");
	printf("# -s [security] 证券代码\n");
	printf("# -b [bstype] 买卖方向,1:买入,2:卖出\n");
	printf("# -l [localid] 策略请求编号,data\\plot.conf中的块内容\n");
	printf("# -r [run] 修改的策略状态,1:启动,其它停止\n");
	printf("# -p [plotid] 策略编号\n\n");
	printf("# -u [localids]篮子标号,data\\plot.conf块内容\n");
	printf("# -d [cdl]撤单率，万分比\n");
	printf("# 修改策略【按证券代码(需要输入-c -m -s -b -l -r)|按策略编号(-c -p -r)】\n");
	printf("# 如：\n");
	printf("# 1. 按证券代码启动策略\n");
	printf("# ./xload -t1 -ccustomer -m1 -s600837 -b1 -l3 -r1\n");
	printf("# 2. 按策略编号停止策略\n");
	printf("# ./xload -t2 -ccustomer -p30000487 -r0\n");
	printf("# 3. 按证券代码启动策略\n");
	printf("# ./xload -t3 -m1 -s600837 -r1\n");
	printf("# 4. 按本地单号停止策略状态\n");
	printf("# ./xload -t4 -l3 -r0\n");
	printf("# 5. 按篮子修改正确代码撤单率\n");
	printf("# ./xload -t5 -ccustomer -m1 -s600837 -b1 -u1,2 -d12\n");
	printf("###############################################################\n");
}
int main(int argc, char *argv[]) {

	XMonitorT *pMonitor = NULL;
	XStrategyT *pStrategy = NULL;
	XInt i, iret = -1;
	int opt;
	int option_index = 0;
	XInt itype = -1;
	XInt market = -1;
	XChar *security = NULL;
	XInt bstype = -1;
	XPlotId plotid = -1;
	XChar *customer = NULL;
	XLocalId localid = -1;
	XInt run = -1;
	XChar *pBaskets = NULL;
	XInt cdl = -1;
	XChar *temp[100];
	XInt icnt = 0, j;

	const char *option = "ht:m:s:b:p:c:l:r:d:u:";
	const struct option long_options[] = { { "help", no_argument, NULL, 'h' }, {
			"type", required_argument, NULL, 't' }, { "market",
			required_argument, NULL, 'm' }, { "security", required_argument,
			NULL, 's' }, { "bstype", required_argument, NULL, 'b' }, { "plotid",
			required_argument, NULL, 'p' }, { "customer", required_argument,
			NULL, 'c' }, { "localid", required_argument, NULL, 'l' }, { "run",
			required_argument, NULL, 'r' }, { "localids", required_argument,
			NULL, 'u' }, { "cdl", required_argument, NULL, 'd' }, { NULL, 0,
			NULL, 0 } };

	while ((opt = getopt_long(argc, argv, option, long_options, &option_index))
			!= -1) {
		switch (opt) {
		case 'h':
		case '?':
			itype = 0;
			usage();
			break;
			/** 操作类型 */
		case 't':
			itype = atoi(optarg);
			break;
			/** 客户号 */
		case 'c':
			customer = optarg;
			break;
		case 'l':
			localid = atoi(optarg);
			break;
			/** 市场 */
		case 'm':
			market = atoi(optarg);
			break;
			/** 证券代码 */
		case 's':
			security = optarg;
			break;
			/** 买卖方向 */
		case 'b':
			bstype = atoi(optarg);
			break;
			/** 策略Id */
		case 'p':
			plotid = atoll(optarg);
			break;
		case 'r':
			run = atoi(optarg);
			break;
		case 'u':
			pBaskets = optarg;
			break;
		case 'd':
			cdl = atoi(optarg);
			break;
		default:
			break;
		}
	}

	xslog_init(XSHM_SDB_FILE, "xload");
	slog_debug(0, "xload版本[%s]", __XMAN_VERSION__);

	XManShmLoad();

	switch (itype) {
	case 0:
		break;

		/** 根据证券代码+本地编号 修改策略状态 */
	case 1:
		pStrategy = XFndVStrategyByFrontId(customer, 0, localid, security,
				bstype);
		if (NULL != pStrategy) {

			/** 启动 */
			if (1 == run) {
				pStrategy->status = eXPlotNormal;
				slog_warn(0, ">>>>>> [%s-%s],策略编号[%lld],策略请求号[%d],启动", customer,
						security, pStrategy->plotid, localid);
			}
			/** 停止 */
			else {
				pStrategy->status = eXPlotStop;
				slog_warn(0, "<<<<<< [%s-%s],策略编号[%lld],策略请求号[%d],停止", customer,
						security, pStrategy->plotid, localid);
			}
		} else {
			slog_error(0, "未找到对应策略,账户[%s],本地请求编号[%d],市场[%d],证券代码[%s],买卖方向[%d]",
					customer, localid, market, security, bstype);
		}
		break;

		/** 根据策略编号修改策略状态 */
	case 2:
		pStrategy = XFndVStrategyByKey(customer, plotid);
		if (NULL != pStrategy) {
			/** 启动 */
			if (1 == run) {
				pStrategy->status = eXPlotNormal;
				slog_warn(0, ">>>>>> [%s-%s],策略编号[%lld],启动", customer,
						pStrategy->setting.securityId, pStrategy->plotid);
			}
			/** 停止 */
			else {
				pStrategy->status = eXPlotStop;
				slog_warn(0, "<<<<<< [%s-%s],策略编号[%lld],停止", customer,
						pStrategy->setting.securityId, pStrategy->plotid);
			}
		} else {
			slog_error(0, "未找到对应策略,账户[%s],策略编号[%lld]", customer, plotid);
		}

		break;
		/** 根据证券代码批量修改策略状态 */
	case 3:
		if (NULL == security || strlen(security) < 6) {
			slog_error(0, "证券代码不正确");
			break;
		}

		pMonitor = XFndVMonitor();
		if (NULL == pMonitor) {
			break;
		}
		for (i = 0; i < pMonitor->iTPlot; i++) {
			pStrategy = XFndVStrategyById(i + 1);
			if (NULL == pStrategy
					|| 0
							!= strncmp(pStrategy->setting.securityId, security,
									6)) {
				continue;
			}
			/** 启动 */
			if (1 == run) {
				pStrategy->status = eXPlotNormal;
				slog_warn(0, ">>>>>> [%s-%s],策略编号[%lld],启动",
						pStrategy->plot.customerId,
						pStrategy->setting.securityId, pStrategy->plotid);
			}
			/** 停止 */
			else {
				pStrategy->status = eXPlotStop;
				slog_warn(0, "<<<<<< [%s-%s],策略编号[%lld],停止",
						pStrategy->plot.customerId,
						pStrategy->setting.securityId, pStrategy->plotid);
			}
		}
		break;

		/** 根据本地单号修改策略状态 */
	case 4:

		if (localid < 1) {
			slog_error(0, "本地单号不正确");
			break;
		}
		pMonitor = XFndVMonitor();
		if (NULL == pMonitor) {
			break;
		}
		for (i = 0; i < pMonitor->iTPlot; i++) {
			pStrategy = XFndVStrategyById(i + 1);
			if (NULL == pStrategy || pStrategy->plot.frontId != localid) {
				continue;
			}
			/** 启动 */
			if (1 == run) {
				pStrategy->status = eXPlotNormal;
				slog_warn(0, ">>>>>> [%s-%s],策略编号[%lld],启动",
						pStrategy->plot.customerId,
						pStrategy->setting.securityId, pStrategy->plotid);
			}
			/** 停止 */
			else {
				pStrategy->status = eXPlotStop;
				slog_warn(0, "<<<<<< [%s-%s],策略编号[%lld],停止",
						pStrategy->plot.customerId,
						pStrategy->setting.securityId, pStrategy->plotid);
			}
		}
		break;
	case 5:
		if (NULL == security || strlen(security) < 6) {
			slog_error(0, "证券代码不正确");
			break;
		}

		if (NULL == customer || strlen(customer) < 1) {
			slog_error(0, "客户号为空");
			break;
		}

		if (NULL == pBaskets || strlen(pBaskets) < 1) {
			slog_error(0, "篮子编号为空");
			break;
		}

		XSplit(pBaskets, ",", strlen(pBaskets), temp, &icnt);

		pMonitor = XFndVMonitor();
		if (NULL == pMonitor) {
			break;
		}
		for (j = 0; j < icnt; j++) {
			slog_debug(0, "j:%d", temp[j]);
			for (i = 0; i < pMonitor->iTPlot; i++) {
				pStrategy = XFndVStrategyById(i + 1);

				if (NULL == pStrategy
						|| pStrategy->plot.plotType != eXMidRobPlot
						|| 0
								!= strncmp(pStrategy->setting.securityId,
										security, 6)
						|| 0
								!= strncmp(pStrategy->plot.customerId, customer,
										strlen(customer))
						|| pStrategy->plot.frontId != atoi(temp[j])) {
					continue;
				}

				pStrategy->setting.cdl = cdl;

				slog_info(0, "[%lld-%d-%s]localid:%d, cdl:%d",
						pStrategy->plotid, pStrategy->setting.market,
						pStrategy->setting.securityId, pStrategy->plot.frontId,
						pStrategy->setting.cdl);
				break;
			}
		}
		break;
	default:

		for (i = 0;; i++) {
			slog_info(0, ">>>>>> 策略集[%d].....", i + 1);
			iret = read_strategy("../data/plot.conf", i + 1);
			if (iret) {
				break;
			}
		}

		break;
	}
	return (0);
}
