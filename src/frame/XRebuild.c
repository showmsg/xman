#include "XRebuild.h"
#include "XLog.h"
#include "XTimes.h"

/**
 * @brief 打印价格档位队列,即千档行情
 */
static inline XPrice _PrintLink(XInt market, XChar *security, XIdx idx, XInt bs,
		int line) {
	XInt j = 0;
	XPriceLevelT *pPriceLevel = NULL, *pFndPriceLevel = NULL;
	XPrice roomPx = -1;
	XPrice price = 0;

	if (idx <= 0) {
		return (price);
	}

	pPriceLevel = XFndVPriceLevelById(idx, market);
	slog_debug(0, "");
	slog_debug(0,
			"#####################[%d-%s],买卖[%d]head[%lld]############################",
			market, security, bs, idx);

	while (NULL != pPriceLevel) {

		slog_debug(0,
				"[%d-%s] [%.3f(%lld)], prev[%lld], next[%lld],forward[%lld]",
				market, security, pPriceLevel->entry.price * 0.0001,
				pPriceLevel->idx, pPriceLevel->prev, pPriceLevel->next,
				pPriceLevel->level[0].forward);

		roomPx = pPriceLevel->level[0].roomPx;
		pFndPriceLevel = XFndVPriceLevel(market, security, bs, roomPx, 0);
		if (pFndPriceLevel != NULL && pFndPriceLevel->entry.qty > 0) {
			j++;
			slog_debug(0,
					"0.[%d-%s]roomPx[%d], 当前节点[%8lld]-价格[%.3f]-委托总量[%lld]-更新时间[%d], 前一节点[%lld], 后一节点[%lld],forward[%lld]",
					market, security, roomPx, pPriceLevel->idx,
					pPriceLevel->entry.price * 0.0001, pPriceLevel->entry.qty,
					pPriceLevel->entry.updateTime, pPriceLevel->prev,
					pPriceLevel->next, pPriceLevel->level[0].forward);

			if (price == 0) {
				price = pPriceLevel->entry.price;
			}
		}

		roomPx = pPriceLevel->level[1].roomPx;
		pFndPriceLevel = XFndVPriceLevel(market, security, bs, roomPx, 1);
		if (pFndPriceLevel != NULL) {

			slog_debug(0,
					"1.[%d-%s]roomPx[%d], 当前节点[%8lld]-价格[%.3f]-委托总量[%lld]-更新时间[%d], 前一节点[%lld], 后一节点[%lld],forward[%lld]",
					market, security, roomPx, pPriceLevel->idx,
					pPriceLevel->entry.price * 0.0001, pPriceLevel->entry.qty,
					pPriceLevel->entry.updateTime, pPriceLevel->prev,
					pPriceLevel->next, pPriceLevel->level[1].forward);

		}

		roomPx = pPriceLevel->level[2].roomPx;
		pFndPriceLevel = XFndVPriceLevel(market, security, bs, roomPx, 2);
		if (pFndPriceLevel != NULL) {

			slog_debug(0,
					"2.[%d-%s]roomPx[%d], 当前节点[%8lld]-价格[%.3f]-委托总量[%lld]-更新时间[%d], 前一节点[%lld], 后一节点[%lld],forward[%lld]",
					market, security, roomPx, pPriceLevel->idx,
					pPriceLevel->entry.price * 0.0001, pPriceLevel->entry.qty,
					pPriceLevel->entry.updateTime, pPriceLevel->prev,
					pPriceLevel->next, pPriceLevel->level[2].forward);

		}

		roomPx = pPriceLevel->level[3].roomPx;
		pFndPriceLevel = XFndVPriceLevel(market, security, bs, roomPx, 3);
		if (pFndPriceLevel != NULL) {

			slog_debug(0,
					"3.[%d-%s]roomPx[%d], 当前节点[%8lld]-价格[%.3f]-委托总量[%lld]-更新时间[%d], 前一节点[%lld], 后一节点[%lld],forward[%lld]",
					market, security, roomPx, pPriceLevel->idx,
					pPriceLevel->entry.price * 0.0001, pPriceLevel->entry.qty,
					pPriceLevel->entry.updateTime, pPriceLevel->prev,
					pPriceLevel->next, pPriceLevel->level[3].forward);

		}

		//lastPx = pPriceLevel->price;

		if (j > 1000) {
			break;
		}

		pPriceLevel = XFndVPriceLevelById(pPriceLevel->next, market);

	}

	return (price);

}

XVoid UpdateSnapshot(XOrderBookT *pOrderBook, XInt isTrade) {
	//比较买一和卖一,如果买一 > 卖一,则等后续成交来更新
	//异常，如果高于卖价的成交，则跳过低的卖价档位；如果低于买价的成交，则跳过高于买价的档位
	XPriceLevelT *pBuyPrice = NULL, *pSellPrice = NULL;
	XInt iCount = 0;
	XIdx buyidx = -1, sellidx = -1;
	XRSnapshotT *snapshot = &(pOrderBook->snapshot);
#ifdef __DEBUG_INFO__	
	XPrice buyPx = -1, sellPx = -1;
#endif

	snapshot->idx = pOrderBook->idx;

#ifdef __LATENCY__
    snapshot->_genTime = XGetClockTime ();
#endif

	memset(&snapshot->ask, 0, sizeof(snapshot->ask));
	memset(&snapshot->bid, 0, sizeof(snapshot->bid));
	memset(&snapshot->askqty, 0, sizeof(snapshot->askqty));
	memset(&snapshot->bidqty, 0, sizeof(snapshot->bidqty));
#ifdef __DEBUG_INFO__

        slog_debug(0, "[%d-%s] 买位置[%lld],卖位置[%lld]", snapshot->market, snapshot->securityId, pOrderBook->buyLevelIdx, pOrderBook->sellLevelIdx);
//	sellPx = _PrintLink(pOrderBook->market, pOrderBook->securityId, pOrderBook->sellLevelIdx, eXSell, __LINE__);

//	buyPx = _PrintLink(pOrderBook->market, pOrderBook->securityId, pOrderBook->buyLevelIdx, eXBuy, __LINE__);
	
	if(snapshot->updateTime >= 93000000  && sellPx != 0 && sellPx < buyPx)
	{
		slog_warn(0, "[%d-%s], 最新价[%d],买一价[%d], 卖一价[%d], 时间[%d]", 
		snapshot->market, snapshot->securityId, snapshot->tradePx, buyPx, sellPx, snapshot->updateTime);
	}
		
#endif

	//计算盘口
	if (snapshot->updateTime >= 92500000) {
		pBuyPrice = XFndVPriceLevelById(pOrderBook->buyLevelIdx,
				snapshot->market);
		pSellPrice = XFndVPriceLevelById(pOrderBook->sellLevelIdx,
				snapshot->market);

		//如果是成交更新,档位价优于成交价的更新,BestPx
		//如果是委托,档位价优于对手价的更新,BestPx TODO,如果是买,小于卖单
		//如果是删除,删除的是盘口的更新,BestPx TODO,如果是买,小于卖单

		if (NULL != pBuyPrice && NULL != pSellPrice) {
			buyidx = pBuyPrice->idx;
			sellidx = pSellPrice->idx;

			if (!isTrade) {
				//输出价格档位
				while (buyidx > 0) {
					if (iCount >= 10) {
						break;
					}
					pBuyPrice = XFndVPriceLevelById(buyidx, snapshot->market);
					if (NULL == pBuyPrice || pBuyPrice->entry.qty <= 0) {
						break;
					}

					snapshot->bid[iCount] = pBuyPrice->entry.price;
					snapshot->bidqty[iCount] = pBuyPrice->entry.qty;
					snapshot->bidcqty[iCount] = pBuyPrice->entry.cqty;
					iCount++;

					buyidx = pBuyPrice->next;
				}

				iCount = 0;
				while (sellidx > 0) {
					if (iCount >= 10) {
						break;
					}
					pSellPrice = XFndVPriceLevelById(sellidx, snapshot->market);
					if (NULL == pSellPrice || pSellPrice->entry.qty <= 0) {
						break;
					}

					snapshot->ask[iCount] = pSellPrice->entry.price;
					snapshot->askqty[iCount] = pSellPrice->entry.qty;
					snapshot->askcqty[iCount] = pSellPrice->entry.cqty;
					iCount++;

					sellidx = pSellPrice->next;
				}

				if (snapshot->ask[0] > snapshot->bid[0]) {
					pOrderBook->bestBidPx = snapshot->bid[0];
					pOrderBook->bestAskPx = snapshot->ask[0];
				} else {
					if (snapshot->askqty[0] > snapshot->bidqty[0]) {
						pOrderBook->bestBidPx = snapshot->bid[1];
						pOrderBook->bestAskPx = snapshot->ask[0];
					} else {
						pOrderBook->bestBidPx = snapshot->bid[1];
						pOrderBook->bestAskPx = snapshot->ask[0];
					}
				}
			} else {
				//主动买
				if (isTrade == eXBuy) {
					//输出价格档位
					while (buyidx > 0) {
						if (iCount >= 10) {
							break;
						}
						pBuyPrice = XFndVPriceLevelById(buyidx,
								snapshot->market);
						if (NULL == pBuyPrice || pBuyPrice->entry.qty <= 0) {
							break;
						}
						if (pBuyPrice->entry.price <= snapshot->tradePx) {
							snapshot->bid[iCount] = pBuyPrice->entry.price;
							snapshot->bidqty[iCount] = pBuyPrice->entry.qty;
							snapshot->bidcqty[iCount] = pBuyPrice->entry.cqty;
							iCount++;

						}

						buyidx = pBuyPrice->next;
					}

					iCount = 0;
					while (sellidx > 0) {
						if (iCount >= 10) {
							break;
						}
						pSellPrice = XFndVPriceLevelById(sellidx,
								snapshot->market);
						if (NULL == pSellPrice || pSellPrice->entry.qty <= 0) {
							break;
						}
						if (pSellPrice->entry.price >= snapshot->tradePx) {
							snapshot->ask[iCount] = pSellPrice->entry.price;
							snapshot->askqty[iCount] = pSellPrice->entry.qty;
							snapshot->askcqty[iCount] = pSellPrice->entry.cqty;
							iCount++;
						}
						sellidx = pSellPrice->next;
					}

					if (snapshot->ask[0] > snapshot->bid[0]) {
						pOrderBook->bestBidPx = snapshot->bid[0];
						pOrderBook->bestAskPx = snapshot->ask[0];
					} else {
						if (snapshot->askqty[0] > snapshot->bidqty[0]) {
							pOrderBook->bestBidPx = snapshot->bid[1];
							pOrderBook->bestAskPx = snapshot->ask[0];
						} else {
							pOrderBook->bestBidPx = snapshot->bid[1];
							pOrderBook->bestAskPx = snapshot->ask[0];
						}
					}
				} else {
					//输出价格档位
					while (buyidx > 0) {
						if (iCount >= 10) {
							break;
						}
						pBuyPrice = XFndVPriceLevelById(buyidx,
								snapshot->market);
						if (NULL == pBuyPrice || pBuyPrice->entry.qty <= 0) {
							break;
						}
						if (pBuyPrice->entry.price <= snapshot->tradePx) {
							snapshot->bid[iCount] = pBuyPrice->entry.price;
							snapshot->bidqty[iCount] = pBuyPrice->entry.qty;
							snapshot->bidcqty[iCount] = pBuyPrice->entry.cqty;
							iCount++;

						}

						buyidx = pBuyPrice->next;
					}

					iCount = 0;
					while (sellidx > 0) {
						if (iCount >= 10) {
							break;
						}
						pSellPrice = XFndVPriceLevelById(sellidx,
								snapshot->market);
						if (NULL == pSellPrice || pSellPrice->entry.qty <= 0) {
							break;
						}
						if (pSellPrice->entry.price >= snapshot->driveBidPx) {
							snapshot->ask[iCount] = pSellPrice->entry.price;
							snapshot->askqty[iCount] = pSellPrice->entry.qty;
							snapshot->askcqty[iCount] = pSellPrice->entry.cqty;
							iCount++;
						}
						sellidx = pSellPrice->next;
					}

					if (snapshot->ask[0] > snapshot->bid[0]) {
						pOrderBook->bestBidPx = snapshot->bid[0];
						pOrderBook->bestAskPx = snapshot->ask[0];
					} else {
						if (snapshot->askqty[0] > snapshot->bidqty[0]) {
							pOrderBook->bestBidPx = snapshot->bid[1];
							pOrderBook->bestAskPx = snapshot->ask[0];
						} else {
							pOrderBook->bestBidPx = snapshot->bid[1];
							pOrderBook->bestAskPx = snapshot->ask[0];
						}
					}
				}
			}
			//生成行情,上海基金和债券没有收盘集合竞价
			if (snapshot->ask[0] > snapshot->bid[0]
					&& snapshot->updateTime > 92500000) {
				snapshot->version++;
				XPushCache(XSHMKEYCONECT(reSnapCache), snapshot);
			}
		} else if (NULL != pBuyPrice && NULL == pSellPrice) {
			pOrderBook->bestBidPx = pBuyPrice->entry.price;
			pOrderBook->bestAskPx = 0;

			buyidx = pBuyPrice->idx;

			//输出价格档位
			while (buyidx > 0) {
				if (iCount >= 10) {
					break;
				}
				pBuyPrice = XFndVPriceLevelById(buyidx, snapshot->market);
				if (NULL == pBuyPrice || pBuyPrice->entry.qty <= 0) {
					break;
				}
				snapshot->bid[iCount] = pBuyPrice->entry.price;
				snapshot->bidqty[iCount] = pBuyPrice->entry.qty;
				snapshot->bidcqty[iCount] = pBuyPrice->entry.cqty;
				iCount++;

				buyidx = pBuyPrice->next;
			}
			if (snapshot->bid[0] > 0 && snapshot->updateTime > 92500000) {
				snapshot->version++;
				XPushCache(XSHMKEYCONECT(reSnapCache), snapshot);
			}
		} else if (NULL == pBuyPrice && NULL != pSellPrice) {
			pOrderBook->bestBidPx = 0;
			pOrderBook->bestAskPx = pSellPrice->entry.price;

			sellidx = pSellPrice->idx;

			iCount = 0;
			while (sellidx > 0) {
				if (iCount >= 10) {
					break;
				}
				pSellPrice = XFndVPriceLevelById(sellidx, snapshot->market);
				if (NULL == pSellPrice || pSellPrice->entry.qty <= 0) {
					break;
				}

				snapshot->ask[iCount] = pSellPrice->entry.price;
				snapshot->askqty[iCount] = pSellPrice->entry.qty;
				snapshot->askcqty[iCount] = pSellPrice->entry.cqty;
				iCount++;

				sellidx = pSellPrice->next;
			}
			if (snapshot->ask[0] > 0 && snapshot->updateTime > 92500000) {
				snapshot->version++;
				XPushCache(XSHMKEYCONECT(reSnapCache), snapshot);
			}
		}
	}
#ifdef __DEBUG_INFO__
	slog_debug(0, "[%d-%s], 买一价[%d], 卖一价[%d], 时间[%d]", 
		snapshot->market, snapshot->securityId, pOrderBook->bestBidPx, pOrderBook->bestAskPx, snapshot->updateTime);
#endif
}

XVoid OnOrgSnapshot(XSnapshotBaseT *snapbase) {
	XSnapshotT *pSnapshot = NULL, snapshot = { 0 };
	XStockT *pStock = NULL;
	XInt m;

	pSnapshot = XFndVSnapshotByKey(snapbase->market, snapbase->securityId);

	if (NULL == pSnapshot) {
		memcpy((char*) &snapshot, snapbase, XSNAPSHOT_BASE_SIZE);
		snapshot.version++;

		pStock = XFndVStockByKey(snapbase->market, snapbase->securityId);
		if (NULL != pStock) {
			snapshot.secStatus = pStock->secStatus;
			snapshot.upperPx = pStock->upperPrice;
			snapshot.lowerPx = pStock->lowerPrice;
		}
	} else {
		memcpy(&snapshot, pSnapshot, XSNAPSHOT_SIZE);
		snapshot.openPx = snapbase->openPx;
		snapshot.highPx = snapbase->highPx;
		snapshot.tradePx = snapbase->tradePx;
		snapshot.lowPx = snapbase->lowPx;
		snapshot.numTrades = snapbase->numTrades;
		snapshot.updateTime = snapbase->updateTime;
		snapshot._recvTime = snapbase->_recvTime;
		snapshot.volumeTrade = snapbase->volumeTrade;
		snapshot.amountTrade = snapbase->amountTrade;
		snapshot._bizIndex = snapbase->_bizIndex;
		snapshot.driveAskPx = snapbase->driveAskPx;
		snapshot.driveBidPx = snapbase->driveBidPx;

		for (m = SNAPSHOT_LEVEL - 1; m >= 0; m--) {
			snapshot.ask[m] = snapbase->ask[m];
			snapshot.bid[m] = snapbase->bid[m];
			snapshot.askqty[m] = snapbase->askqty[m];
			snapshot.bidqty[m] = snapbase->bidqty[m];
		}

		snapshot.version++;

	}

#ifdef __LATENCY__
	snapshot._genTime = XGetClockTime();
#endif

	XUpdateSnapshot(&snapshot);
}

/**
 * 分钟线内不做撤单统计,会出现撤单和委托不再一个区间情况
 */
XVoid GenOrdKLine(XRSnapshotT *pSnapshot, XTickOrderT *tickOrder) {
	XKLineT *k1 = NULL;
	XKLineT *k5 = NULL;
	XNum kcursor1 = pSnapshot->kcursor1 > 0 ? pSnapshot->kcursor1 : 1;
	XNum kcursor5 = pSnapshot->kcursor5 > 0 ? pSnapshot->kcursor5 : 1;

	XMoney ordMoney = 0;
	XBool bBigOrder = false;

	kcursor1 = (kcursor1 - 1) & (SNAPSHOT_K1_CNT - 1);
	kcursor5 = (kcursor5 - 1) & (SNAPSHOT_K5_CNT - 1);

	k1 = GetKlinesByBlock(pSnapshot->idx, 0);
	k5 = GetKlinesByBlock(pSnapshot->idx, 1);

	//大单
	ordMoney = (XMoney) tickOrder->ordPx * tickOrder->ordQty;
	if (tickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY) {
		bBigOrder = true;
	}
	if (tickOrder->bsType == eXBuy) {
		//大的买单
		if (bBigOrder) {
			k1[kcursor1].bigBuyOrdQty += tickOrder->ordQty;
			k5[kcursor5].bigBuyOrdQty += tickOrder->ordQty;
		}
		if (pSnapshot->ask[0]
				&& tickOrder->ordPx > pSnapshot->ask[0] * (1 + 0.01)) {
			k1[kcursor1].scanBidOrdQty += tickOrder->ordQty;
			k5[kcursor5].scanBidOrdQty += tickOrder->ordQty;
		}
	} else {
		//大的买单
		if (bBigOrder) {
			k1[kcursor1].bigSellOrdQty += tickOrder->ordQty;
			k5[kcursor5].bigSellOrdQty += tickOrder->ordQty;
		}
		if (pSnapshot->bid[0]
				&& tickOrder->ordPx > pSnapshot->bid[0] * (1 + 0.01)) {
			k1[kcursor1].scanOfferOrdQty += tickOrder->ordQty;
			k5[kcursor5].scanOfferOrdQty += tickOrder->ordQty;
		}
	}
}
XVoid GenTrdKLine(XRSnapshotT *pSnapshot, XPrice tradePx, XQty tradeQty,
		XMoney tradeMoney, XShortTime tradeTime) {
	XKLineT *k1 = NULL;
	XKLineT *k5 = NULL;
	XNum kcursor1 = pSnapshot->kcursor1 > 0 ? pSnapshot->kcursor1 : 1;
	XNum kcursor5 = pSnapshot->kcursor5 > 0 ? pSnapshot->kcursor5 : 1;

	kcursor1 = (kcursor1 - 1) & (SNAPSHOT_K1_CNT - 1);
	kcursor5 = (kcursor5 - 1) & (SNAPSHOT_K5_CNT - 1);

	k1 = GetKlinesByBlock(pSnapshot->idx, 0);
	k5 = GetKlinesByBlock(pSnapshot->idx, 1);
	/** 同一时间,更新 */
	/** 同一时间,更新 */
	if (tradeTime / 100000 == k1[kcursor1].updateTime / 100000) {
		k1[kcursor1].close = tradePx;
		k1[kcursor1].updateTime = tradeTime;
		k1[kcursor1].low =
				k1[kcursor1].low > tradePx ? tradePx : k1[kcursor1].low;
		k1[kcursor1].high =
				k1[kcursor1].high < tradePx ? tradePx : k1[kcursor1].high;
		k1[kcursor1].qty += tradeQty;
		k1[kcursor1].amt += tradeMoney;
		k1[kcursor1].numTrades++;
	} else {
		if (pSnapshot->updateTime < 93000000) {
			return;
		}

		pSnapshot->kcursor1++;
		kcursor1 = (pSnapshot->kcursor1 - 1) & (SNAPSHOT_K1_CNT - 1);
		memset(&k1[kcursor1], 0, sizeof(XKLineT));
		k1[kcursor1].open = tradePx;
		k1[kcursor1].close = tradePx;
		k1[kcursor1].updateTime = tradeTime;
		k1[kcursor1].low = tradePx;
		k1[kcursor1].high = tradePx;
		k1[kcursor1].qty = tradeQty;
		k1[kcursor1].amt = tradeMoney;
		k1[kcursor1].traday = pSnapshot->traday;
		k1[kcursor1].numTrades = 1;
	}

	/** 同一时间,更新 */
	if (tradeTime / 500000 == k5[kcursor5].updateTime / 500000) {
		k5[kcursor5].close = tradePx;
		k5[kcursor5].updateTime = tradeTime;
		k5[kcursor5].low =
				k5[kcursor5].low > tradePx ? tradePx : k5[kcursor5].low;
		k5[kcursor5].high =
				k5[kcursor5].high < tradePx ? tradePx : k5[kcursor5].high;
		k5[kcursor5].qty += tradeQty;
		k5[kcursor5].amt += tradeMoney;
		k5[kcursor5].numTrades++;
	} else {
		if (pSnapshot->updateTime < 93000000) {
			return;
		}

		pSnapshot->kcursor5++;
		kcursor5 = (pSnapshot->kcursor5 - 1) & (SNAPSHOT_K5_CNT - 1);
		memset(&k5[kcursor5], 0, sizeof(XKLineT));
		k5[kcursor5].open = tradePx;
		k5[kcursor5].close = tradePx;
		k5[kcursor5].updateTime = tradeTime;
		k5[kcursor5].low = tradePx;
		k5[kcursor5].high = tradePx;
		k5[kcursor5].qty = tradeQty;
		k5[kcursor5].amt = tradeMoney;
		k5[kcursor5].traday = pSnapshot->traday;
		k5[kcursor5].numTrades = 1;
	}

}
