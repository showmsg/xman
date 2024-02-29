/*
 * @file SzReBuild.c
 * @brief 基于逐笔成交的订单流构建,采用被动成交技术即根据交易所逐笔成交来匹配成交，如果ask<bid则不输出盘口
 * @author kangyq
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * 2023.9.26
 * 上交所交易规则:股票收盘集合竞价，债券和基金无收盘集合竞价
 * 深交所交易规则:所有都有收盘集合竞价
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 * 
 */
#include "XSzRebuild.h"
#include "XTimes.h"
#include "XLog.h"
#include "XUtils.h"

//#define __REBUILD_TEST__

#define __REBUILD_TEST_SZCODE__ "300116"

static XNum g_PriceRoom[] = { 1, 100, 1000, 10000 };
static XMonitorMdT *l_pMonitor = NULL;

/**
 * @brief 找到价格档位插入位置
 */
static XIdx _GetStorePricePos(XIdx maxPriceIdx) {
	XIdx idx = -1;

	if (l_pMonitor->usedSzPos != l_pMonitor->delSzNums && l_pMonitor->delSzIdxs[l_pMonitor->usedSzPos] != 0) {
		idx = l_pMonitor->delSzIdxs[l_pMonitor->usedSzPos];
		l_pMonitor->delSzIdxs[l_pMonitor->usedSzPos] = 0;
		l_pMonitor->usedSzPos = (l_pMonitor->usedSzPos + 1) & (MAX_DEL_PRICELEVEL_CNT - 1);
		return (idx);
	} else {
		l_pMonitor->maxSzPriceIdx++;
		return (l_pMonitor->maxSzPriceIdx);
	}
}

/**
 * @brief 删除头结点的价格档位,价格档位下移，同时记录删除位置
 */
static void _DelPricePos(XIdx curIdx, XIdx nextIdx, XInt bs, XOrderBookT* pOrderBook)
{
	//如果删除的是头，则更新头结点位置
	if(bs == eXBuy && curIdx == pOrderBook->buyLevelIdx)
	{
		pOrderBook->buyLevelIdx = nextIdx;
#ifdef __DEBUG_INFO__
		slog_debug(0, "[%d-%s],头结点切换[%lld]", pOrderBook->snapshot.market, pOrderBook->snapshot.securityId, pOrderBook->buyLevelIdx);
#endif
		//更新删除位置
		l_pMonitor->delSzIdxs[l_pMonitor->delSzNums] = curIdx;
		l_pMonitor->delSzNums = (l_pMonitor->delSzNums + 1) & (MAX_DEL_PRICELEVEL_CNT - 1);
	}
	else if(bs == eXSell && curIdx == pOrderBook->sellLevelIdx)
	{
		pOrderBook->sellLevelIdx = nextIdx;
#ifdef __DEBUG_INFO__
		slog_debug(0, "[%d-%s],头结点切换[%lld]", pOrderBook->snapshot.market, pOrderBook->snapshot.securityId, pOrderBook->sellLevelIdx);
#endif
		//更新删除位置
		l_pMonitor->delSzIdxs[l_pMonitor->delSzNums] = curIdx;
		l_pMonitor->delSzNums = (l_pMonitor->delSzNums + 1) & (MAX_DEL_PRICELEVEL_CNT - 1);
	}

	
}

/**
 * @brief 找到卖的价格档位,返回的是前置节点的信息，下一层在此节点上再去寻找
 */
static XIdx _FndSellPos(XIdx headPos, XInt market, XChar* securityId, XInt bs, XPriceLevelT* pInsPrice, XInt level)
{
	XIdx idx = -1;

	XPriceLevelT* pRoom = NULL, *pPreRoom = NULL, *pPreHead = NULL, *pHead = NULL;

	
	pRoom = XFndVPriceLevel(market, securityId, bs, pInsPrice->level[level].roomPx, level);

	//确认头结点是否切换
	if(NULL != pRoom)
	{
		//变换头结点
		if(pInsPrice->entry.price < pRoom->entry.price)
		{
			pPreHead = XFndVPriceLevelById(pRoom->prev, market);
			if(NULL != pPreHead)
			{
				//前置节点连接
				pPreRoom = XFndVPriceLevel(market, securityId, bs, pPreHead->level[level].roomPx, level);
				if(NULL != pPreRoom)
				{
					pPreRoom->level[level].forward = pInsPrice->idx;
					if(level == 0)
					{
						pPreRoom->next = pInsPrice->idx;
						//pInsPrice->prev = pPreRoom->idx;
					}
				}
			}
			//当前节点连接
			pInsPrice->level[level].forward = pRoom->level[level].forward;
			pRoom->level[level].forward = 0;
			if(level == 0)
			{
				pInsPrice->prev = pRoom->prev;
				pInsPrice->next = pRoom->idx;
				pRoom->prev = pInsPrice->idx;

			}
			XPutOrUpdatePriceLevel(market, securityId, bs, pInsPrice->level[level].roomPx, level, pInsPrice);
		}

		
	}
	//不是头结点，确认level层的链接,从小到大排序
	else
	{
		//从头结点往后找,插入到对应的位置
		pHead = XFndVPriceLevelById(headPos, market);		
		while(NULL != pHead)
		{
			//找到插入位置,在头结点之后
			if(pInsPrice->entry.price < pHead->entry.price)
			{
				break;
			}
			idx = pHead->idx;
			pPreHead = pHead;
			pHead = XFndVPriceLevelById(pHead->level[level].forward, market);

			if(pHead != NULL && pPreHead->entry.price >= pHead->entry.price)
			{
#ifdef __DEBUG_INFO__
			slog_debug(0, "[%d-%s] [%d]>[%d]", market, securityId, pPreHead->entry.price, pHead->entry.price);
			exit(0);
#endif			
			}

		}

		if(NULL != pPreHead)
		{
			//当前节点的后一节点调换			
			pPreHead->level[level].forward = pInsPrice->idx;
			if(level == 0)
			{
				pPreHead->next = pInsPrice->idx;
				pInsPrice->prev = pPreHead->idx;
			}	
		}

		if(NULL != pHead)
		{
			pInsPrice->level[level].forward = pHead->idx;
			if(level == 0)
			{
				pInsPrice->next = pHead->idx;
				pHead->prev = pInsPrice->idx;
			}
		}
		
		
		//在数据发生变化的时候更新
		XPutOrUpdatePriceLevel(market, securityId, bs, pInsPrice->level[level].roomPx, level, pInsPrice);
	}
#ifdef __DEBUG_INFO__
	slog_debug(0, "[%d-%s],head[%lld],插入价格[%d],插入位置[%lld],prev[%lld],next[%lld],forward[%d-%lld]", 
	market, securityId, headPos, pInsPrice->entry.price, 
	    pInsPrice->idx, pInsPrice->prev, pInsPrice->next, level, pInsPrice->level[level].forward);
#endif
	return (idx);
}

/**
 * @brief 插入当前价格档位到队列中
 * 
 * @param headpos 
 * @param market 
 * @param securityId 
 * @param bs 
 * @param pInsPrice 
 */
static XIdx _insertSell(XInt headpos, XInt market, XChar* securityId,  XInt bs, XPriceLevelT *pInsPrice)
{
	XPriceLevelT *pHead = NULL;
	XPrice roomPx;
	XInt i = 0;
	XIdx idx = pInsPrice->idx; //是不是头结点
	XIdx iFndPos = -1;

	for(i = 3; i >= 0; i--)
	{
		roomPx = pInsPrice->entry.price / g_PriceRoom[i];
		pInsPrice->level[i].roomPx = roomPx;
	}

	pHead = XFndVPriceLevelById(headpos, market);
	if(NULL != pHead)
	{	
		//不切换头结点
		if(pInsPrice->entry.price > pHead->entry.price)
		{
			idx = headpos;
		}

		for(i = 3; i >= 0; i--)
		{
			iFndPos = (iFndPos == -1) ? headpos : iFndPos;
			//返回的是前置结点
			iFndPos = _FndSellPos(iFndPos, market, securityId, bs, pInsPrice, i);
		}
	}
	else
	{
		//更新
		for(i = 3; i >= 0; i--)
		{
			XPutOrUpdatePriceLevel(market, securityId, bs, pInsPrice->level[i].roomPx, i, pInsPrice);
		}
	}
	
	return (idx);
}

/**
 * @brief 找到买的价格档位
 */
static XIdx _FndBuyPos(XIdx headPos, XInt market, XChar* securityId, XInt bs, XPriceLevelT* pInsPrice, XInt level)
{
	XIdx idx = -1;
	XPriceLevelT* pRoom = NULL, *pPreRoom = NULL, *pPreHead = NULL, *pHead = NULL;

	pRoom = XFndVPriceLevel(market, securityId, bs, pInsPrice->level[level].roomPx, level);
	//1.如果插入位置有头结点,插入数据和原结点比较是否需要切换
	if(NULL != pRoom)
	{
		//如果插入结点价格大于头结点,交换位置
		if(pInsPrice->entry.price > pRoom->entry.price)
		{
#ifdef __DEBUG_INFO__
			slog_debug(0, "[%d-%s] 插入价格[%d], 头结点[%d]", market, securityId, pInsPrice->entry.price, pRoom->entry.price);
#endif
			//找到前置节点，连接
			pPreHead = XFndVPriceLevelById(pRoom->prev, market);
			if(NULL != pPreHead)
			{
				pPreRoom = XFndVPriceLevel(market, securityId, bs, pPreHead->level[level].roomPx, level);
				if(NULL != pPreRoom)
				{
					pPreRoom->level[level].forward = pInsPrice->idx;
					if(level == 0)
					{
						pPreRoom->next = pInsPrice->idx;
						//pInsPrice->prev = pPreRoom->idx;
					}
				}
			}
			//如果交换的结点有前结点，前结点的forward要更新
			//把原结点的forward更新为当前
			//把原结点的forward置空
			pInsPrice->level[level].forward = pRoom->level[level].forward;
			pRoom->level[level].forward = 0;
			if(level == 0)
			{
				pInsPrice->prev = pRoom->prev;
				pInsPrice->next = pRoom->idx;
				pRoom->prev = pInsPrice->idx;
			}
#ifdef __DEBUG_INFO__
			slog_debug(0, "[%d-%s], 当前[%lld], prev[%lld], next[%lld], forward[%lld]", market, securityId, pInsPrice->idx,
			pInsPrice->prev, pInsPrice->next, pInsPrice->level[level].forward);
#endif
			//只有在节点发生变化的时候更新
			XPutOrUpdatePriceLevel(market, securityId, bs, pInsPrice->level[level].roomPx, level, pInsPrice);
		}
	}
	//2.如果没有头结点，从头的位置开始往下查找进行插入，如果插入的数据大于头结点，插在此位置之前
	else
	{
		//头的位置从上次找到的高结点的位置开始,如果高结点没有为头结点
		//遍历当前位置往后，如果插入数据大于当前结点终止，插入该数据
		//如果有上一结点，连接上下结点
		//如果插入的是头结点，直接连接
		pHead = XFndVPriceLevelById(headPos, market);
		while(NULL != pHead)
		{		
			//找到插入位置,在头结点之后
			if(pInsPrice->entry.price > pHead->entry.price)
			{
				break;
			}
			idx = pHead->idx; //返回插入点前的节点
			pPreHead = pHead;

			pHead = XFndVPriceLevelById(pHead->level[level].forward, market);

			//如果前一个值小于后一个退出
			if(pHead != NULL && pPreHead->entry.price <= pHead->entry.price)
			{
#ifdef __DEBUG_INFO__
			slog_debug(0, "[%d-%s] [%d]>[%d]", market, securityId, pPreHead->entry.price, pHead->entry.price);
			exit(0);
#endif
			}
		}

		if(NULL != pPreHead)
		{
			//当前节点的后一节点调换		
			pPreHead->level[level].forward = pInsPrice->idx;
			if(level == 0)
			{
				pPreHead->next = pInsPrice->idx;
				pInsPrice->prev = pPreHead->idx;
			}	
		}

		if(NULL != pHead)
		{
			pInsPrice->level[level].forward = pHead->idx;
			if(level == 0)
			{
				pInsPrice->next = pHead->idx;
				pHead->prev = pInsPrice->idx;
			}
		}
	
		//在数据发生变化的时候更新
		XPutOrUpdatePriceLevel(market, securityId, bs, pInsPrice->level[level].roomPx, level, pInsPrice);
	}
#ifdef __DEBUG_INFO__
	slog_debug(0, "[%d-%s] head[%lld],插入价格[%d],插入位置[%lld],prev[%lld],next[%lld],forward[%d-%lld]", 
	market, securityId, headPos, pInsPrice->entry.price, 
	    pInsPrice->idx, pInsPrice->prev, pInsPrice->next, level, pInsPrice->level[level].forward);
#endif
	return (idx);
}
/**
 * @brief 买价格按照由高到低排列,插入新的价格档位到买的队列中
 *  TODO 此处更新有问题
 * @param headpos 
 * @param market 
 * @param securityId 
 * @param bs 
 * @param pInsPrice 
 */
static XIdx _insertBuy(XInt headpos, XInt market, XChar* securityId,  XInt bs, XPriceLevelT *pInsPrice)
{
	XPriceLevelT *pHead = NULL;
	XPrice roomPx;
	XInt i = 0;
	XIdx idx = pInsPrice->idx;
	XIdx iFndPos = -1;

	for(i = 3; i >= 0; i--)
	{
		roomPx = pInsPrice->entry.price / g_PriceRoom[i];
		pInsPrice->level[i].roomPx = roomPx;
	}
	pHead = XFndVPriceLevelById(headpos, market);
	if(NULL != pHead)
	{	
		//不切换头结点
		if(pInsPrice->entry.price < pHead->entry.price)
		{
			idx = headpos;
		}

		for(i = 3; i >= 0; i--)
		{
			iFndPos = (iFndPos == -1) ? headpos : iFndPos;
			iFndPos = _FndBuyPos(iFndPos, market, securityId, bs, pInsPrice, i);
			 
		}
	}
	else
	{
		//更新
		for(i = 3; i >= 0; i--)
		{
			XPutOrUpdatePriceLevel(market, securityId, bs, pInsPrice->level[i].roomPx, i, pInsPrice);
			/**
			slog_debug(0, "level[%d], price[%d], forward[%lld]", i, pInsPrice->level[i].roomPx, pInsPrice->level[i].forward);
			if(i == 0)
			{
				slog_debug(0, "cur[%lld], prev[%lld], next[%lld]", pInsPrice->idx, pInsPrice->prev, pInsPrice->next);
			}
			*/
		}
	}
	
	return (idx);
}

/**
 * @brief 删除买的价格档位
 */
static void _DeletePrice(XPriceLevelT* pPrice, XInt market, XChar* securityId, XInt bs) {

	XInt i = 0;
	XPriceLevelT* pPrev = NULL, *pNext = NULL,  *pRoom = NULL, *pPreRoom = NULL;

#ifdef __DEBUG_INFO__
	slog_debug(0, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> _DeletePrice位置[%lld],价格档位[%d], prev[%lld],next[%lld]", 
	pPrice->idx,  pPrice->entry.price, pPrice->prev, pPrice->next);
#endif

	pPrev = XFndVPriceLevelById(pPrice->prev, market);
	if(NULL != pPrev)
	{
		pPrev->next = pPrice->next;
		pPrev->level[0].forward = pPrice->level[0].forward;
	}
	
	pNext = XFndVPriceLevelById(pPrice->next, market);
	if(NULL != pNext)
	{
		pNext->prev = pPrice->prev;
	}

	for(i = 1; i <= 3; i++)
	{
		pPreRoom = NULL;
		//删除的是不是头结点，是，需要找到新的头结点，删除旧的头结点
		pRoom = XFndVPriceLevel(market, securityId, bs, pPrice->level[i].roomPx, i);
		if(pRoom == NULL)
		{
			continue;
		}
		//删除的是头结点
		if(NULL != pPrev)
		{
			pPreRoom = XFndVPriceLevel(market, securityId, bs, pPrev->level[i].roomPx, i);
		}
		
		//删除的不是头结点,直接过
		if(pRoom->idx != pPrice->idx)
		{
			continue;
		}

		//有后继的节点
		if(NULL != pNext && pNext->level[i].roomPx == pPrice->level[i].roomPx)
		{
			
			if(pPreRoom != NULL)
			{
				pPreRoom->level[i].forward = pNext->idx;
			}
			
			pNext->level[i].forward = pPrice->level[i].forward;
			XPutOrUpdatePriceLevel(market, securityId, bs, pNext->level[i].roomPx, i, pNext);
			
//			slog_debug(0, "删除不是头结点,有后继更新[%lld]", pNext->idx);
		}
		//没有后续节点
		else
		{
			if(pPreRoom != NULL)
			{
				pPreRoom->level[i].forward = pPrice->level[i].forward;
			}
//			slog_debug(0, "删除头结点[%d] [%lld]", i, pPrice->idx);
			XRmvPriceHashByKey(market, securityId, bs, pPrice->level[i].roomPx, i);				
		}
	}

	XRmvPriceHashByKey(market, securityId, bs, pPrice->level[0].roomPx, 0);
	XRmvPriceDataById(pPrice->idx, market);
//	slog_debug(0, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< _DeletePrice");
}

static XVoid AddFixedBidOrder(XOrderBookT* pOrderBook, XTickOrderT *tickOrder)
{
    XIdx pos = -1;
    XPriceLevelT pInsPrice, *pFndPrice = NULL;

    
    //找到价格档位,不存在新建档位,存在更新
    //找到对应价位,存数据,档位存在，但是档位数量为累计委托数量为0
	pFndPrice = XFndVPriceLevel(tickOrder->market, tickOrder->securityId, tickOrder->bsType, tickOrder->ordPx, 0);

	if(NULL == pFndPrice)
	{
        memset(&pInsPrice, 0, XPRICELEVEL_SIZE);
		//slog_debug(0, "插入价格档位[%d]", bestPx);
		pos = _GetStorePricePos(l_pMonitor->maxSzPriceIdx);
		pInsPrice.idx = pos;
		pInsPrice.entry.price = tickOrder->ordPx;    //档位价格
		pInsPrice.entry.qty += tickOrder->ordQty;
		pInsPrice.entry.updateTime = tickOrder->updateTime;
		pInsPrice.entry.numOrders  = 1;
		tickOrder->_priceIdx = pInsPrice.idx;
        tickOrder->_leaveQty = tickOrder->ordQty;
#ifdef __DEBUG_INFO__
                slog_debug (0, "[%d-%s],买头结点[%lld], bestBidPx[%d]",
                            tickOrder->market, tickOrder->securityId,
                            pOrderBook->buyLevelIdx, tickOrder->ordPx);
#endif
        pOrderBook->buyLevelIdx = _insertBuy(pOrderBook->buyLevelIdx, tickOrder->market, tickOrder->securityId, tickOrder->bsType, &pInsPrice);

	}
	else
	{

		pFndPrice->entry.qty += tickOrder->ordQty;
		pFndPrice->entry.updateTime = tickOrder->updateTime;
		pFndPrice->entry.numOrders++;
		tickOrder->_priceIdx = pFndPrice->idx;
		tickOrder->_leaveQty = tickOrder->ordQty;
#ifdef __DEBUG_INFO__
		slog_debug(0, "[%d-%s-%d]价格档位[%lld][%d]-[%d] 委托笔数[%d]......", tickOrder->market, tickOrder->securityId,  tickOrder->bsType, pFndPrice->idx, pFndPrice->entry.price, pFndPrice->entry.qty, pFndPrice->entry.numOrders);
#endif	
	}
    XPutVTickOrder(tickOrder);

    GenOrdKLine(&(pOrderBook->snapshot), tickOrder);
}

static XVoid AddFixedOfferOrder(XOrderBookT* pOrderBook, XTickOrderT *tickOrder)
{
    XIdx pos = -1;
    XPriceLevelT pInsPrice, *pFndPrice = NULL;

    //找到价格档位,不存在新建档位,存在更新
    //找到对应价位,存数据,档位存在，但是档位数量为累计委托数量为0(TODO)
	pFndPrice = XFndVPriceLevel(tickOrder->market, tickOrder->securityId, tickOrder->bsType, tickOrder->ordPx, 0);
	if(NULL == pFndPrice)
	{
		memset(&pInsPrice, 0, XPRICELEVEL_SIZE);
		pos = _GetStorePricePos(l_pMonitor->maxSzPriceIdx);
		pInsPrice.idx = pos;
		pInsPrice.entry.price = tickOrder->ordPx;    //档位价格
		pInsPrice.entry.qty += tickOrder->ordQty;
		pInsPrice.entry.updateTime = tickOrder->updateTime;
		pInsPrice.entry.numOrders  = 1;
		tickOrder->_priceIdx = pInsPrice.idx;
		tickOrder->_leaveQty = tickOrder->ordQty;
#ifdef __DEBUG_INFO__			
			slog_debug(0, "[%d-%s],卖头结点[%lld], bestAskPx[%d]", tickOrder->market, tickOrder->securityId, pOrderBook->sellLevelIdx, tickOrder->ordPx);
#endif
        pOrderBook->sellLevelIdx = _insertSell(pOrderBook->sellLevelIdx, tickOrder->market, tickOrder->securityId, tickOrder->bsType,  &pInsPrice);

	}
	else
	{

		pFndPrice->entry.qty += tickOrder->ordQty;
		pFndPrice->entry.updateTime = tickOrder->updateTime;
		pFndPrice->entry.numOrders++;
		tickOrder->_priceIdx = pFndPrice->idx;
		tickOrder->_leaveQty = tickOrder->ordQty;
#ifdef __DEBUG_INFO__
		slog_debug(0, "[%d-%s-%d]价格档位[%lld][%d]-[%lld] 委托笔数[%d]......", tickOrder->market, tickOrder->securityId,  tickOrder->bsType, pFndPrice->idx, pFndPrice->entry.price, pFndPrice->entry.qty, pFndPrice->entry.numOrders);
#endif	
	}
    XPutVTickOrder(tickOrder);

    GenOrdKLine(&(pOrderBook->snapshot), tickOrder);
}

//添加本方最优买
static XVoid AddOwnBidOrder(XOrderBookT* pOrderBook, XTickOrderT *tickOrder)
{
	tickOrder->_leaveQty = tickOrder->ordQty;
    XPutVTickOrder(tickOrder);
}

//添加本方最优卖
static XVoid AddOwnOfferOrder(XOrderBookT* pOrderBook, XTickOrderT *tickOrder)
{
	tickOrder->_leaveQty = tickOrder->ordQty;
    XPutVTickOrder(tickOrder);
}

//添加市价买
static XVoid AddMktBidOrder(XOrderBookT* pOrderBook, XTickOrderT *tickOrder)
{
	tickOrder->_leaveQty = tickOrder->ordQty;
    XPutVTickOrder(tickOrder);
}

//添加市价卖
static XVoid AddMktOfferOrder(XOrderBookT* pOrderBook, XTickOrderT *tickOrder)
{
	tickOrder->_leaveQty = tickOrder->ordQty;
    XPutVTickOrder(tickOrder);
}

//买/卖撤单
static XBool RevokeOrder(XOrderBookT* pOrderBook, XSeqNum SeqNo, XTickTradeT* tickTrade)
{
    XTickOrderT *pFndOrgTickOrder = NULL;
    XPriceLevelT *pFndPrice = NULL;

    pFndOrgTickOrder = XFndVTickOrder(tickTrade->market, tickTrade->securityId, tickTrade->channel, SeqNo);
    if(NULL != pFndOrgTickOrder)
    {
		//大单处理
		if(pFndOrgTickOrder->ordQty >= BIGORDER_VOLUME || pFndOrgTickOrder->ordMoney >= BIGORDER_MONEY)
		{
			if(pFndOrgTickOrder->bsType == eXBuy)
			{
				pOrderBook->snapshot.bigBuyOrdAmt -= tickTrade->tradeMoney;				
				pOrderBook->snapshot.bigBuyOrdQty -= tickTrade->tradeQty;
				if(pFndOrgTickOrder->ordQty == tickTrade->tradeQty)
				{
					pOrderBook->snapshot.bigBuyOrdCnt--;
				}
			}
			else
			{
				pOrderBook->snapshot.bigSellOrdAmt -= tickTrade->tradeMoney;				
				pOrderBook->snapshot.bigSellOrdQty -= tickTrade->tradeQty;
				if(pFndOrgTickOrder->ordQty == tickTrade->tradeQty)
				{
					pOrderBook->snapshot.bigSellOrdCnt--;
				}
			}
			
		}

        //找到价格档位
        pFndPrice = XFndVPriceLevelById(pFndOrgTickOrder->_priceIdx, tickTrade->market);
        if(NULL != pFndPrice)
        {
            pFndPrice->entry.qty -= tickTrade->tradeQty;
            pFndPrice->entry.updateTime = tickTrade->updateTime;
			pFndPrice->entry.numOrders--;
			pFndPrice->entry.cqty += tickTrade->tradeQty;
#ifdef __DEBUG_INFO__
            slog_debug(0, "撤单的价格档位[%d],买卖方向[%d],委托数量[%lld], 撤单数量[%lld]", pFndPrice->entry.price, pFndOrgTickOrder->bsType, pFndPrice->entry.qty, tickTrade->tradeQty);
#endif
            //该档位已经没有委托数量,删除价格档位
            if(pFndPrice->entry.numOrders == 0)
            {
                _DelPricePos(pFndPrice->idx, pFndPrice->next, pFndOrgTickOrder->bsType, pOrderBook);
                _DeletePrice(pFndPrice, tickTrade->market, tickTrade->securityId, pFndOrgTickOrder->bsType);
				//如果该价格档位都没了,对应该价格档位的订单也没了
            }
        }
		
        //删除逐笔委托
        XRmvVTickOrderByKey(tickTrade->market, tickTrade->securityId, tickTrade->channel, SeqNo);
    }
	return (true);
}

//处理市价未完成订单
static XBool DealMktPrice(XOrderBookT* pOrderBook)
{
    //找到原始订单
    XTickOrderT* tickOrder = NULL;
    XIdx pos = -1;
    XPriceLevelT pInsPrice, *pFndPrice = NULL;
    XPrice bestPx = -1;

    tickOrder = XFndVTickOrder(pOrderBook->snapshot.market, pOrderBook->snapshot.securityId, pOrderBook->snapshot._channel, pOrderBook->_ordSeqno);
    if(NULL == tickOrder)
    {
        return false;
    }

    //如果是买单
    if(tickOrder->ordType == eXOrdMkt)
    {
        if(tickOrder->bsType == eXBuy)
        {
            bestPx = pOrderBook->bestBidPx;
        }
        else
        {
            bestPx = pOrderBook->bestAskPx;
        }
    }
    else if(tickOrder->ordType == eXOrdSelf)
    {
        if(tickOrder->bsType == eXBuy)
        {
            bestPx = pOrderBook->bestAskPx;
        }
        else
        {
            bestPx = pOrderBook->bestBidPx;
        }
    }
    //找到对应的档位
    //找到价格档位,不存在新建档位,存在更新
    //找到对应价位,存数据,档位存在，但是档位数量为累计委托数量为0(TODO)
	pFndPrice = XFndVPriceLevel(pOrderBook->snapshot.market, pOrderBook->snapshot.securityId, tickOrder->bsType, bestPx, 0);
	if(NULL == pFndPrice)
	{
		memset(&pInsPrice, 0, XPRICELEVEL_SIZE);
		pos = _GetStorePricePos(l_pMonitor->maxSzPriceIdx);
		pInsPrice.idx = pos;
		pInsPrice.entry.price = bestPx;    //档位价格
		pInsPrice.entry.cqty += tickOrder->_leaveQty;
		pInsPrice.entry.updateTime = tickOrder->updateTime;
		pInsPrice.entry.numOrders  = 1;
		tickOrder->_priceIdx = pInsPrice.idx;

        if(tickOrder->bsType == eXBuy)
        {
#ifdef __DEBUG_INFO__			
			slog_debug(0, "[%d-%s],买头结点[%lld], bestBidPx[%d], 订单类型[%d]", pOrderBook->snapshot.market, pOrderBook->snapshot.securityId, pOrderBook->buyLevelIdx, bestPx, tickOrder->ordType);
#endif
            pOrderBook->buyLevelIdx = _insertBuy(pOrderBook->buyLevelIdx, pOrderBook->snapshot.market, pOrderBook->snapshot.securityId, tickOrder->bsType, &pInsPrice);
        }
        else
        {
#ifdef __DEBUG_INFO__			
			slog_debug(0, "[%d-%s],卖头结点[%lld], bestBidPx[%d],订单类型[%d]", pOrderBook->snapshot.market, pOrderBook->snapshot.securityId, pOrderBook->buyLevelIdx, bestPx, tickOrder->ordType);
#endif
            pOrderBook->sellLevelIdx = _insertSell(pOrderBook->sellLevelIdx, pOrderBook->snapshot.market, pOrderBook->snapshot.securityId, tickOrder->bsType, &pInsPrice);
        }

	}
	else
	{

		pFndPrice->entry.qty += tickOrder->_leaveQty;
		pFndPrice->entry.updateTime = tickOrder->updateTime;
		pFndPrice->entry.numOrders++;
		tickOrder->_priceIdx = pFndPrice->idx;
#ifdef __DEBUG_INFO__
		slog_debug(0, "[%d-%s-%d]价格档位[%lld][%d]-[%d] 委托笔数[%d]......", pOrderBook->snapshot.market, pOrderBook->snapshot.securityId,  tickOrder->bsType, pFndPrice->idx, pFndPrice->entry.price, pFndPrice->entry.qty, pFndPrice->entry.numOrders);
#endif	
	}

    return (true);
}

static XBool OnBidOfferTrade(XOrderBookT* pOrderBook, XSeqNum SeqNo, XTickTradeT* tickTrade)
{
    XTickOrderT *pFndOrgTickOrder = NULL;
    XPriceLevelT *pFndPrice = NULL;

    pFndOrgTickOrder = XFndVTickOrder(tickTrade->market, tickTrade->securityId, tickTrade->channel, SeqNo);
    if(NULL != pFndOrgTickOrder)
    {
#ifdef __DEBUG_INFO__
      slog_debug (0, "[%d-%s]找到原始订单, 委托数量[%d],成交数量[%d],档位位置[%lld]",
                  pFndOrgTickOrder->market, pFndOrgTickOrder->securityId,
                  pFndOrgTickOrder->ordQty, tickTrade->tradeQty, pFndOrgTickOrder->_priceIdx);
#endif
      // 委托数量减去成交数量
      pFndOrgTickOrder->_leaveQty -= tickTrade->tradeQty;
      // 找到价格档位
      pFndPrice = XFndVPriceLevelById (pFndOrgTickOrder->_priceIdx,
                                       tickTrade->market);
      if (NULL != pFndPrice)
        {
  
            pFndPrice->entry.qty -= tickTrade->tradeQty;
            pFndPrice->entry.updateTime = tickTrade->updateTime;
			if(pFndOrgTickOrder->_leaveQty == 0)
			{
				pFndPrice->entry.numOrders--;
			}

#ifdef __DEBUG_INFO__
                        slog_debug (0, "[%d-%s],剩余数量[%lld],剩余笔数[%d],该笔剩余[%d]",
                                  pFndOrgTickOrder->market, pFndOrgTickOrder->securityId,  pFndPrice->entry.qty
								  ,pFndPrice->entry.numOrders, pFndOrgTickOrder->_leaveQty
								  );
#endif
                        // 该档位已经没有委托数量,删除价格档位
                        // 如果订单剩余数量不为0，但是档位数量为0，丢单了
                        if (pFndPrice->entry.numOrders == 0)
                          {
#ifdef __DEBUG_INFO__
                slog_debug(0, "[%d-%s]删除买价格档位[%d], bs[%d], buyid[%lld],id[%lld]", 
                tickTrade->market, tickTrade->securityId, pFndPrice->entry.price, pFndOrgTickOrder->bsType, pOrderBook->buyLevelIdx,
                pFndPrice->idx);
#endif
                _DelPricePos(pFndPrice->idx, pFndPrice->next, pFndOrgTickOrder->bsType, pOrderBook);
                _DeletePrice(pFndPrice, tickTrade->market, tickTrade->securityId, pFndOrgTickOrder->bsType);

            }
        }
        
        if(pFndOrgTickOrder->_leaveQty == 0)
        {
            //删除逐笔委托,如果删除的逐笔委托是头结点，则删除priceLevel对应的数据
            XRmvVTickOrderByKey(tickTrade->market, tickTrade->securityId, tickTrade->channel, SeqNo);
        }
    
    }

	return (true);
}

//处理逐笔委托
static XVoid OnOrder(XTickOrderT *tickOrder)
{
    XOrderBookT* pOrderBook = NULL, orderBook;
    XStockT* pStock;
    XBool bUpdate = false;
	XRSnapshotT *snapshot = NULL;
	XMoney ordMoney;

#ifdef __DEBUG_INFO__
	slog_debug(0, "============================== OnSzseOrder ================================");
	slog_debug(0, "[%d-%s],买卖[%d],ordPx[%d],ordQty[%d],ordType[%d],委托时间[%d],序号[%lld]", tickOrder->market, tickOrder->securityId, 
	tickOrder->bsType, tickOrder->ordPx, tickOrder->ordQty, tickOrder->ordType, tickOrder->updateTime, tickOrder->bizIndex);
#endif

    /** 初始化该证券的订单薄 */
	pOrderBook = XFndVOrderBook(tickOrder->market, tickOrder->securityId);
	
    if(NULL == pOrderBook)
    {
		memset(&orderBook, 0, XORDERBOOK_SIZE);
		orderBook.idx = 0;
		orderBook.snapshot.traday = tickOrder->traday;
		orderBook.snapshot.market = tickOrder->market;
		orderBook.snapshot._channel = tickOrder->channel;
		memcpy(orderBook.snapshot.securityId, tickOrder->securityId, SECURITYID_LEN);
		pStock = XFndVStockByKey(tickOrder->market, tickOrder->securityId);
		if(NULL != pStock)
		{
			orderBook.snapshot.preClosePx = pStock->preClose;
			orderBook.snapshot.upperPx = pStock->upperPrice;
			orderBook.snapshot.lowerPx = pStock->lowerPrice;
		}
    }
	else
	{
		memcpy(&orderBook, pOrderBook, XORDERBOOK_SIZE);
	}

	snapshot = &(orderBook.snapshot);
    //如果前面是市价委托,现在是限价了,找到前面的委托序号,更新到档位
    if(orderBook.bMktOrd)
    {
        
        bUpdate = DealMktPrice(&orderBook);
    }

    //******************************************************* 赋值 
    snapshot->_bizIndex = tickOrder->bizIndex;

    orderBook._ordSeqno = tickOrder->ordSeq;
    snapshot->updateTime = tickOrder->updateTime;
	snapshot->_recvTime = tickOrder->_recvTime;
    //*********************************************************
    switch(tickOrder->ordType)
    {
        /** 限价*/
		case eXOrdLimit:
        bUpdate = true;
        orderBook.bMktOrd = false;

        ordMoney = (XMoney)tickOrder->ordPx * tickOrder->ordQty;
		//计算委托金额,必须在保存tick之前存放
		tickOrder->ordMoney = ordMoney;
        if (tickOrder->bsType == eXBuy)
          {
            AddFixedBidOrder(&orderBook, tickOrder);
			snapshot->totalBuyOrdQty += tickOrder->ordQty;
			snapshot->totalBuyOrdCnt++;
			snapshot->totalBuyOrdAmt += ordMoney;

			if(tickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY)
			{
				snapshot->bigBuyOrdCnt++;
				snapshot->bigBuyOrdAmt += ordMoney;
				snapshot->bigBuyOrdQty += tickOrder->ordQty;
			}	
        }
        else
        {
            AddFixedOfferOrder(&orderBook, tickOrder);
			snapshot->totalSellOrdQty += tickOrder->ordQty;
			snapshot->totalSellOrdCnt++;
			snapshot->totalSellOrdAmt += ordMoney;
			//计算大单委托
			if(tickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY)
			{
				snapshot->bigSellOrdCnt++;
				snapshot->bigSellOrdAmt += ordMoney;
				snapshot->bigSellOrdQty += tickOrder->ordQty;
			}
        }
        break;
        /** 对手方 */
        case eXOrdMkt:
        orderBook.bMktOrd = true;
		

        if(tickOrder->bsType == eXBuy)
        {
			ordMoney = (XMoney)orderBook.bestAskPx * tickOrder->ordQty;
			//计算委托金额,必须在保存tick之前存放
			tickOrder->ordMoney = ordMoney;
            AddMktBidOrder(&orderBook, tickOrder);
			snapshot->totalBuyOrdQty += tickOrder->ordQty;
			snapshot->totalBuyOrdCnt++;
			snapshot->totalBuyOrdAmt += ordMoney;

			if(tickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY)
			{
				snapshot->bigBuyOrdCnt++;
				snapshot->bigBuyOrdAmt += ordMoney;
				snapshot->bigBuyOrdQty += tickOrder->ordQty;
			}
        }
        else
        {
			ordMoney = (XMoney)orderBook.bestBidPx * tickOrder->ordQty;
			//计算委托金额,必须在保存tick之前存放
			tickOrder->ordMoney = ordMoney;
            AddMktOfferOrder(&orderBook, tickOrder);
			snapshot->totalSellOrdQty += tickOrder->ordQty;
			snapshot->totalSellOrdCnt++;
			snapshot->totalSellOrdAmt += ordMoney;

			//计算大单委托
			if(tickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY)
			{
				snapshot->bigSellOrdCnt++;
				snapshot->bigSellOrdAmt += ordMoney;
				snapshot->bigSellOrdQty += tickOrder->ordQty;
			}
        }
        break;
        /** 本方 */
        case eXOrdSelf:
        orderBook.bMktOrd = true;
		
        if(tickOrder->bsType == eXBuy)
        {
			ordMoney = (XMoney)orderBook.bestBidPx * tickOrder->ordQty;
			//计算委托金额,必须在保存tick之前存放
			tickOrder->ordMoney = ordMoney;
            AddOwnBidOrder(&orderBook, tickOrder);
			snapshot->totalBuyOrdQty += tickOrder->ordQty;
			snapshot->totalBuyOrdCnt++;
			snapshot->totalBuyOrdAmt += ordMoney;

			if(tickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY)
			{
				snapshot->bigBuyOrdCnt++;
				snapshot->bigBuyOrdAmt += ordMoney;
				snapshot->bigBuyOrdQty += tickOrder->ordQty;
			}
        }
        else
        {
			ordMoney = (XMoney)orderBook.bestAskPx * tickOrder->ordQty;
			//计算委托金额,必须在保存tick之前存放
			tickOrder->ordMoney = ordMoney;
            AddOwnOfferOrder(&orderBook, tickOrder);
			snapshot->totalSellOrdQty += tickOrder->ordQty;
			snapshot->totalSellOrdCnt++;
			snapshot->totalSellOrdAmt += ordMoney;
			//计算大单委托
			if(tickOrder->ordQty >= BIGORDER_VOLUME || ordMoney >= BIGORDER_MONEY)
			{
				snapshot->bigSellOrdCnt++;
				snapshot->bigSellOrdAmt += ordMoney;
				snapshot->bigSellOrdQty += tickOrder->ordQty;
			}
        }
        break;
        default:
        break;
    }

    //更新价格档位,(9:25之前集合竞价,14:57-15:00集合竞价)
    if(bUpdate)
    {
        UpdateSnapshot(&orderBook, tickOrder->bsType);
    }

	XPutOrdUpdVOrderBookByKey(&orderBook);
}

//处理逐笔成交
static XVoid OnTrade(XTickTradeT* tickTrade)
{
    XOrderBookT* pOrderBook = NULL, orderBook;
    XStockT* pStock;
	XRSnapshotT *snapshot = NULL;

#ifdef __DEBUG_INFO__
	slog_debug(0, "================================  OnSzseTrade  =======================");
	slog_debug(0, "[%d-%s],成交价格[%d],成交数量[%d],是否撤单[%d],更新时间[%d],序号[%lld],买序号[%lld],卖序号[%lld]", 
	tickTrade->market, tickTrade->securityId, tickTrade->tradePx, tickTrade->tradeQty,
	tickTrade->isCancel, tickTrade->updateTime, tickTrade->bizIndex, tickTrade->bidSeq, tickTrade->askSeq);
#endif

    /** 初始化该证券的订单薄 */
	pOrderBook = XFndVOrderBook(tickTrade->market, tickTrade->securityId);
	
    if(NULL == pOrderBook)
    {
		memset(&orderBook, 0, XORDERBOOK_SIZE);
		orderBook.idx = 0;
		orderBook.snapshot.traday = tickTrade->traday;
		orderBook.snapshot.market = tickTrade->market;
		orderBook.snapshot._channel = tickTrade->channel;
		memcpy(orderBook.snapshot.securityId, tickTrade->securityId, SECURITYID_LEN);
		pStock = XFndVStockByKey(tickTrade->market, tickTrade->securityId);
		if(NULL != pStock)
		{
			orderBook.snapshot.preClosePx = pStock->preClose;
			orderBook.snapshot.upperPx = pStock->upperPrice;
			orderBook.snapshot.lowerPx = pStock->lowerPrice;
		}
    }
	else
	{
		memcpy(&orderBook, pOrderBook, XORDERBOOK_SIZE);
	}

	snapshot = &(orderBook.snapshot);

    //************************************************* 赋值
    snapshot->_bizIndex = tickTrade->bizIndex;
    snapshot->updateTime = tickTrade->updateTime;
	snapshot->_recvTime = tickTrade->_recvTime;
    //*************************************************

    if(tickTrade->isCancel)
    {
        if(tickTrade->bidSeq)
        {
            RevokeOrder(&orderBook, tickTrade->bidSeq, tickTrade);
            UpdateSnapshot(&orderBook, eXBuy);
        }
        else
        {
            RevokeOrder(&orderBook, tickTrade->askSeq, tickTrade);
            UpdateSnapshot(&orderBook, eXSell);
        }
    }
    else
    {
        //**********************************************************************
        snapshot->tradePx = tickTrade->tradePx;

        if(tickTrade->updateTime >= 93000000)
        {
			//主动单
			if(tickTrade->bidSeq < tickTrade->askSeq)
			{
				snapshot->driveAskPx = tickTrade->tradePx;
				snapshot->insideTrdAmt += tickTrade->tradeMoney;
				snapshot->side = eXSell;
			}
			else
			{
				snapshot->driveBidPx = tickTrade->tradePx;
				snapshot->outsideTrdAmt += tickTrade->tradeMoney;
				snapshot->side = eXBuy;
			}
        }
        //成交量
        snapshot->numTrades++;
        snapshot->volumeTrade += tickTrade->tradeQty;
        snapshot->amountTrade += tickTrade->tradeMoney;

        //开盘价
        if(0 == snapshot->openPx)
        {
            snapshot->openPx = tickTrade->tradePx;
        }

        //最高价
        if(tickTrade->tradePx > snapshot->highPx)
        {
            snapshot->highPx = tickTrade->tradePx;
        }

        //最低价
        if(0 == snapshot->lowPx)
        {
            snapshot->lowPx = tickTrade->tradePx;
        }
        else if(tickTrade->tradePx < snapshot->lowPx)
        {
            snapshot->lowPx = tickTrade->tradePx;
        }

		//如果时间小于9:30:00为集合竞价成交量
		if(tickTrade->updateTime < 93000000)
		{
			snapshot->auctionQty += tickTrade->tradeQty;
		}
        //**********************************************************************
        OnBidOfferTrade(&orderBook, tickTrade->bidSeq, tickTrade);
        OnBidOfferTrade(&orderBook, tickTrade->askSeq, tickTrade);

        GenTrdKLine(snapshot, tickTrade->tradePx, tickTrade->tradeQty, tickTrade->tradeMoney, tickTrade->updateTime);

        UpdateSnapshot(&orderBook, 0);
    }

	XPutOrdUpdVOrderBookByKey(&orderBook);
}

//交易
XVoid XSzRebuild(XVoid *params)
{
    XL2LT *l2l = NULL;
    XInt readId = -1;
    XTickTradeT trade;
	XTickOrderT order;
	XBindParamT *pBind = NULL;
    XInt iret = 0;
	XSnapshotBaseT snapshot;

	pBind = (XBindParamT *)params;
	if (NULL != pBind)
	{
		iret = XBindCPU(pBind->cpuid);
		if(iret)
		{
			slog_warn(0, "绑核失败[%d]", pBind->cpuid);
		}
	}

	l_pMonitor = XFndVMdMonitor(eXExchSec);

	if (NULL == l_pMonitor) {
		slog_error(0, "获取行情信息错误");
		return;
	}

    readId = XGetReadCache(XSHMKEYCONECT(mktCache));

    for (;;)
    {
        l2l = (XL2LT *)XPopCache(XSHMKEYCONECT(mktCache), readId);
		if(NULL == l2l || l2l->head.market != eXMarketSza)
		{
			continue;
		}
        switch (l2l->head.type)
        {
            case eMTickOrder:
                order = l2l->order;	
#ifdef __REBUILD_TEST__
			if(strncmp(order.securityId, __REBUILD_TEST_SZCODE__, strlen(__REBUILD_TEST_SZCODE__)) != 0)
			{
				break;
			}
#endif	
				l_pMonitor->szChannel = order.channel;			
				l_pMonitor->szBiz = order.bizIndex;
                OnOrder(&order);
                
                l_pMonitor->totalSzOrders++;
#ifdef __BTEST__

                if(l_pMonitor->traday == 0)
				{
					l_pMonitor->traday = order.traday;
				}
				__atomic_store_n(&l_pMonitor->updateTime, order.updateTime, __ATOMIC_RELAXED);
#endif
                break;
            case eMTickTrade:
                trade = l2l->trade;

#ifdef __REBUILD_TEST__
				if(strncmp(trade.securityId, __REBUILD_TEST_SZCODE__, strlen(__REBUILD_TEST_SZCODE__)) != 0)
				{
					break;
				}
#endif
				l_pMonitor->szChannel = trade.channel;
				l_pMonitor->szBiz = trade.bizIndex;
                OnTrade(&trade);
				l_pMonitor->totalSzTrades++;
#ifdef __BTEST__

				if(l_pMonitor->traday == 0)
				{
					l_pMonitor->traday = trade.traday;
				}
				__atomic_store_n(&l_pMonitor->updateTime, trade.updateTime, __ATOMIC_RELAXED);

#endif
            break;
			case eMSnapshot:
			snapshot = l2l->snapshot;

			l_pMonitor->updateTime =  snapshot.updateTime;
			
			if(l_pMonitor->traday == 0)
			{
				l_pMonitor->traday = snapshot.traday;
			}
			
			
			if(__atomic_load_n(&l_pMonitor->_locFirstTime, __ATOMIC_RELAXED) == 0)
			{
				__atomic_store_n(&l_pMonitor->_locFirstTime, XGetClockTime(), __ATOMIC_RELAXED);
				__atomic_store_n(&l_pMonitor->_mktFirstTime, snapshot.updateTime, __ATOMIC_RELAXED);
			}

			OnOrgSnapshot(&snapshot);
			break;
            default:
            break;
        }
    }
	XReleaseCache(XSHMKEYCONECT(mktCache), readId);
}
