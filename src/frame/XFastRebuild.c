#include "XBus.h"
#include "XLog.h"
#include "XTimes.h"
#include "XUtils.h"
#include "XFastRebuild.h"

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
		//买涨停委托
		if (tickOrder->ordPx == pSnapshot->upperPx
				|| (tickOrder->ordType != eXOrdLimit
						&& pSnapshot->tradePx == pSnapshot->upperPx)) {
			k1[kcursor1].upperBuyOrdQty += tickOrder->ordQty;
			k5[kcursor5].upperBuyOrdQty += tickOrder->ordQty;
		}
		if (pSnapshot->tradePx
				&& tickOrder->ordPx > pSnapshot->tradePx * (1 + 0.01)) {
			k1[kcursor1].scanBidOrdQty += tickOrder->ordQty;
			k5[kcursor5].scanBidOrdQty += tickOrder->ordQty;
		}
	} else {
		//大的买单
		if (bBigOrder) {
			k1[kcursor1].bigSellOrdQty += tickOrder->ordQty;
			k5[kcursor5].bigSellOrdQty += tickOrder->ordQty;
		}
		//买涨停委托
		if (tickOrder->ordPx == pSnapshot->upperPx
				|| (tickOrder->ordType != eXOrdLimit
						&& pSnapshot->tradePx == pSnapshot->upperPx)) {
			k1[kcursor1].upperSellOrdQty += tickOrder->ordQty;
			k5[kcursor5].upperSellOrdQty += tickOrder->ordQty;
		}
		if (pSnapshot->tradePx
				&& tickOrder->ordPx > pSnapshot->tradePx * (1 + 0.01)) {
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

XVoid OnReSnapshot(XRSnapshotT *snapshot) {
	//如果是回测,每笔重构都触发行情计算
	XPutOrUpdVRSnapshot(snapshot);

#ifdef __BTEST__
	XSnapshotNotifyT notify;

	memset(&notify, 0, XSNAPSHOT_NOTIFY_SIZE);
	notify.idx = snapshot->idx;
	notify.market = snapshot->market;
	memcpy(notify.securityId, snapshot->securityId, SECURITYID_LEN);
	XPushCache(XSHMKEYCONECT(reSnapCache), &notify);
#else
	XPushCache(XSHMKEYCONECT(reSnapCache), snapshot);
#endif

}
