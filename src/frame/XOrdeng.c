/*
 * @file Ordeng.c
 * @brief 订单处理
 * @version 2.0.0
 * @date 2022-12-16
 *
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */
#include "OesCom.h"
#include "XOrdeng.h"
#include "XLog.h"
#include "XTimes.h"
#include "XUtils.h"

/**
 * 解冻本地持仓
 */
static XVoid
_OnUpdHoldByOrderLoc (XOrderT *pFndOrder)
{
  XHoldT *pHold = NULL;

  // 更新持仓
  if (0 != pFndOrder->locFrzHold)
    {
      pHold = XFndVHoldByKey (
          pFndOrder->request.customerId, pFndOrder->request.investId,
          pFndOrder->request.market, pFndOrder->request.securityId);
      if (NULL != pHold)
        {
          pHold->locFrz -= pFndOrder->locFrzHold; // 解冻本地冻结
          pHold->sellAvlHld = pHold->countSellAvlHld - pHold->locFrz;
        }
      pFndOrder->locFrzHold = 0;
    }
}
/**
 * 解冻本地资金
 */
static XVoid
_OnUpdCashByOrderLoc (XOrderT *pFndOrder)
{
  XCashT *pFndCash = NULL;

  if (0 != pFndOrder->locFrzMoney)
    {
      pFndCash = XFndVCashByKey (pFndOrder->request.customerId,
                                 pFndOrder->request.acctType);
      if (NULL != pFndCash)
        {
          pFndCash->locFrz -= pFndOrder->locFrzMoney;

          pFndCash->curAvailable = pFndCash->countAvailable - pFndCash->locFrz;
        }
      pFndOrder->locFrzMoney = 0;
    }
}

/**
 * 更新策略委托情况
 */
static XVoid
_OnUpdStrategyByOrderLoc (XOrderT *pFndOrder)
{
  XStrategyT *pStrategy = NULL;
  // 找到对应的策略
  if (0 != pFndOrder->request.plotid && pFndOrder->_locFlag)
    {
      pStrategy = XFndVStrategyByKey (pFndOrder->request.customerId,
                                      pFndOrder->request.plotid);
      if (NULL != pStrategy
          && pStrategy->setting.market == pFndOrder->request.market
          && 0
                 == strcmp (pFndOrder->request.securityId,
                            pStrategy->setting.securityId))
        {
          if (pFndOrder->request.bsType == eXBuy)
            {
              switch (pFndOrder->ordStatus)
                {
                // 废单、部分成交、撤单
                case eXOrdStatusInvalid:
                case eXOrdStatusCanceled:
                case eXOrdStatusPCanceled:
                  pStrategy->_buyValid += pFndOrder->trdQty;
                  break;
                case eXOrdStatusDeclared:
                  // TODO 如果是确认订单没有成交,把订单编号更新在此处

                case eXOrdStatusPFilled:
                case eXOrdStatusFilled:
                  pStrategy->_buyValid += pFndOrder->request.ordQty;
                  break;
                default:
                  break;
                }

              pStrategy->_buyRtn += pFndOrder->request.ordQty;
            }
          else
            {
              switch (pFndOrder->ordStatus)
                {
                // 废单、部分成交、撤单
                case eXOrdStatusInvalid:
                case eXOrdStatusCanceled:
                case eXOrdStatusPCanceled:
                  pStrategy->_sellValid += pFndOrder->trdQty;

                  break;
                case eXOrdStatusDeclared:
                case eXOrdStatusPFilled:
                case eXOrdStatusFilled:
                  pStrategy->_sellValid += pFndOrder->request.ordQty;

                  break;
                default:
                  break;
                }
              pStrategy->_sellRtn += pFndOrder->request.ordQty;
            }
        }
    }
}

/**
 * 在收到订单响应后，后续撤单、废单更新策略状态
 */
static XVoid
_OnUpdStrategyByOrderCnt (XOrderT *pFndOrder)
{
  XStrategyT *pStrategy = NULL;

  if (0 != pFndOrder->request.plotid && pFndOrder->_locFlag
      && pFndOrder->_nonDeal)
    {
      pStrategy = XFndVStrategyByKey (pFndOrder->request.customerId,
                                      pFndOrder->request.plotid);
      if (NULL != pStrategy
          && pStrategy->setting.market == pFndOrder->request.market
          && 0
                 == strcmp (pFndOrder->request.securityId,
                            pStrategy->setting.securityId))
        {
          if (pFndOrder->request.bsType == eXBuy)
            {
              //撤单后,减去有效委托数量
              if (eXOrdStatusInvalid == pFndOrder->ordStatus
                  || eXOrdStatusCanceled == pFndOrder->ordStatus
                  || eXOrdStatusPCanceled == pFndOrder->ordStatus)
                {
                  pStrategy->_buyValid
                      -= pFndOrder->request.ordQty - pFndOrder->trdQty;

                  pFndOrder->_nonDeal = false;
                }
            }
          else
            {
              //撤单后,减去有效委托数量
              if (eXOrdStatusInvalid == pFndOrder->ordStatus
                  || eXOrdStatusCanceled == pFndOrder->ordStatus
                  || eXOrdStatusPCanceled == pFndOrder->ordStatus)
                {
                  pStrategy->_sellValid
                      -= pFndOrder->request.ordQty - pFndOrder->trdQty;
                  pFndOrder->_nonDeal = false;
                }
            }
        }
    }
}

static XVoid
_UpdOrder (XOrderT *pOrder)
{
  XOrderT *pFndOrder = NULL;
#ifdef __HAS_FRONT__
  XStrategyT *pStrategy = NULL;
  XTradeCache webCache = { 0 };
#endif

  pFndOrder = XFndVOrderByCnt (pOrder->request.customerId,
                               pOrder->request.market, pOrder->ordid);

  if (NULL == pFndOrder)
    {
      pFndOrder = XFndVOrderByLoc (pOrder->request.customerId, pOrder->envno,
                                   pOrder->request.localId);
      if (NULL == pFndOrder)
        {
          pOrder->productType = XGetPrdType (pOrder->request.market,
                                             pOrder->request.securityId);

          //如果不是从策略系统发出,发送时间为确认时间
          pOrder->_sendLocTime = pOrder->_cnfLocTime;

          // 插入该笔数据，订单由外部申报
          XPutOrUpdVOrderByCnt (pOrder);
          /** 非从本地报出的委托，如果是卖单冻结持仓,本地不处理 */

          slog_info (
              3,
              "新订单响应[%d-%s], "
              "客户号[%s],股东帐户[%s],本地订单编号[%d-%d],柜台编号[%lld], "
              "委托数量[%d],委托价格[%d],买卖方向[%d-%d]",
              pOrder->request.market, pOrder->request.securityId,
              pOrder->request.customerId, pOrder->request.investId,
              pOrder->envno, pOrder->request.localId, pOrder->ordid,
              pOrder->request.ordQty, pOrder->request.ordPrice,
              pOrder->request.bsType, pOrder->request.isCancel);
        }

      //重启时,重复的订单走这个逻辑但是错误的,重复的订单不保存后续的 TODO
      else
        {
          // 首次获得回报，更新数据
          /** 本地发出去卖单收到回报，解冻本地冻结，增加冻结持仓 */
          pFndOrder->ordid = pOrder->ordid;
          pFndOrder->ordStatus = pOrder->ordStatus;
          pFndOrder->errorno = pOrder->errorno;

          if (!pFndOrder->_sendTime)
            {
              pFndOrder->_sendTime = pOrder->_sendTime;
            }
          if (!pFndOrder->_cnfTime)
            {
              pFndOrder->_cnfTime = pOrder->_cnfTime;
            }
          if (!pFndOrder->_cnfLocTime)
            {
              pFndOrder->_cnfLocTime = pOrder->_cnfLocTime;
            }
          if(!pFndOrder->_cnfExTime)
          {
            pFndOrder->_cnfExTime = pOrder->_cnfExTime;
          }
          if (pOrder->errmsg)
            {
              memcpy (pFndOrder->errmsg, pOrder->errmsg, ERRORMSG_LEN);
            }

          // 更新持仓
          _OnUpdHoldByOrderLoc (pFndOrder);

          _OnUpdCashByOrderLoc (pFndOrder);

          _OnUpdStrategyByOrderLoc (pFndOrder);

          // 更新以ordid存放
          XPutOrUpdOrderHashByCnt (pFndOrder);

        }
    }
  else
    {
      // 再次获得回报，更新数据
      pFndOrder->ordStatus = pOrder->ordStatus;
      pFndOrder->errorno = pOrder->errorno;

      if (!pFndOrder->_sendTime)
      {
        pFndOrder->_sendTime = pOrder->_sendTime;
      }
      if (!pFndOrder->_cnfTime)
      {
        pFndOrder->_cnfTime = pOrder->_cnfTime;
      }
      if (!pFndOrder->_cnfLocTime)
      {
        pFndOrder->_cnfLocTime = pOrder->_cnfLocTime;
      }  
      if (!pFndOrder->_cnfExTime)
      {
        pFndOrder->_cnfExTime = pOrder->_cnfExTime;
      }

      if (pOrder->errmsg)
        {
          memcpy (pFndOrder->errmsg, pOrder->errmsg, ERRORMSG_LEN);
        }

      // 更新持仓
      _OnUpdHoldByOrderLoc (pFndOrder);

      _OnUpdCashByOrderLoc (pFndOrder);

      _OnUpdStrategyByOrderCnt (pFndOrder);
    }

#ifdef __HAS_FRONT__
  if (NULL != pFndOrder)
    {
      webCache.head.type = eDOrder;
      webCache.head.dataLen = XORDER_SIZE;
      webCache.ordrsp = *pFndOrder;
      XPushCache (XSHMKEYCONECT (rtnCache), &webCache);
    }

  // 推送策略变动
  if (NULL != pStrategy)
    {
      webCache.head.type = eDStrategy;
      webCache.head.dataLen = XSTRATEGY_SIZE;
      webCache.strategy = *pStrategy;
      XPushCache (XSHMKEYCONECT (rtnCache), &webCache);
    }
#endif
}

/**
 * 更新撤单对应的响应
 */
static XVoid
_UpdCtrlSelf (XOrderT *pOrder)
{
  XOrderT *pFndOrder = NULL;
  // 更新原撤单委托信息
  pFndOrder = XFndVOrderByCnt (pOrder->request.customerId,
                               pOrder->request.market, pOrder->ordid);
  if (NULL == pFndOrder)
    {
      pFndOrder = XFndVOrderByLoc (pOrder->request.customerId, pOrder->envno,
                                   pOrder->request.localId);

      if (NULL != pFndOrder)
        {
          pFndOrder->request.orgOrdId = pOrder->request.orgOrdId;
          pFndOrder->ordid = pOrder->ordid;
          pFndOrder->ordStatus = pOrder->ordStatus;
          pFndOrder->errorno = pOrder->errorno;

          if(!pFndOrder->_cnfLocTime)
          {
            pFndOrder->_cnfLocTime = pOrder->_cnfLocTime;
          }
          if(!pFndOrder->_sendTime)
          {
            pFndOrder->_sendTime = pOrder->_sendTime;
          }
          if(!pFndOrder->_cnfTime)
          {
            pFndOrder->_cnfTime = pOrder->_cnfTime;
          }
          if(!pFndOrder->_cnfExTime)
          {
            pFndOrder->_cnfExTime = pOrder->_cnfExTime;
          }
          if (pOrder->errmsg)
            {
              memcpy (pFndOrder->errmsg, pOrder->errmsg, ERRORMSG_LEN);
            }
          XPutOrUpdOrderHashByCnt (pFndOrder);

        }
    }
  else
    {
      pFndOrder->ordStatus = pOrder->ordStatus;
      pFndOrder->errorno = pOrder->errorno;
      if(!pFndOrder->_cnfLocTime)
      {
        pFndOrder->_cnfLocTime = pOrder->_cnfLocTime;
      }
      if(!pFndOrder->_sendTime)
      {
        pFndOrder->_sendTime = pOrder->_sendTime;
      }
      if(!pFndOrder->_cnfTime)
      {
        pFndOrder->_cnfTime = pOrder->_cnfTime;
      }
      if(!pFndOrder->_cnfExTime)
      {
        pFndOrder->_cnfExTime = pOrder->_cnfExTime;
      }
      if (pOrder->errmsg)
        {
          memcpy (pFndOrder->errmsg, pOrder->errmsg, ERRORMSG_LEN);
        }
    }
}

// 更新撤单对应的原始订单
static XVoid
_UpdCtrlOrg (XOrderT *pOrder)
{
  XOrderT *pFndOrder = NULL;
#ifdef __HAS_FRONT__
  XTradeCache webCache = { 0 };
#endif

  // 更新原委托信息撤单信息
  pFndOrder
      = XFndVOrderByCnt (pOrder->request.customerId, pOrder->request.market,
                         pOrder->request.orgOrdId);
  if (NULL == pFndOrder)
    {
      pFndOrder = XFndVOrderByLoc (pOrder->request.customerId,
                                   pOrder->request.orgEnvno,
                                   pOrder->request.orgLocalId);
      if (NULL == pFndOrder)
        {
          slog_warn (3,
                     "[%d-%s],客户号[%s], 股东账户[%s], 本地编号[%d-%lld], "
                     "原始本地编号[%d], 原始柜台编号[%d-%lld]",
                     pOrder->request.market, pOrder->request.securityId,
                     pOrder->request.customerId, pOrder->request.investId,
                     pOrder->request.localId, pOrder->ordid,
                     pOrder->request.orgLocalId, pOrder->request.orgEnvno,
                     pOrder->request.orgOrdId);
        }
      else
        {
          pFndOrder->errorno = pOrder->errorno;
          pFndOrder->request.clocalId = pOrder->request.localId;
          pFndOrder->request.isCancel = pOrder->request.isCancel;
          pFndOrder->request.orgEnvno = pOrder->request.orgEnvno;
          pFndOrder->request.orgLocalId = pOrder->request.orgLocalId;
          pFndOrder->request.orgOrdId = pOrder->request.orgOrdId;
          pFndOrder->_isSendCtrl = false;
          if (pOrder->ordStatus == eXOrdStatusPCanceled
              || pOrder->ordStatus == eXOrdStatusCanceled)
            {
              pFndOrder->ordStatus = pOrder->ordStatus;
            }
          if (pOrder->errmsg)
            {
              memcpy (pFndOrder->errmsg, pOrder->errmsg, ERRORMSG_LEN);
            }

          // 更新以ordid存放
          XPutOrUpdOrderHashByCnt (pFndOrder);
        }
    }
  else
    {
      /** 如果是撤单成功，则解冻卖持仓 */
      pFndOrder->errorno = pOrder->errorno;
      pFndOrder->request.isCancel = pOrder->request.isCancel;
      pFndOrder->request.orgEnvno = pOrder->request.orgEnvno;
      pFndOrder->request.clocalId = pOrder->request.localId;
      pFndOrder->request.orgLocalId = pOrder->request.orgLocalId;
      pFndOrder->request.orgOrdId = pOrder->request.orgOrdId;
      pFndOrder->_isSendCtrl = false;
      if (pOrder->ordStatus == eXOrdStatusPCanceled
          || pOrder->ordStatus == eXOrdStatusCanceled)
        {
          pFndOrder->ordStatus = pOrder->ordStatus;
        }
      if (pOrder->errmsg)
        {
          memcpy (pFndOrder->errmsg, pOrder->errmsg, ERRORMSG_LEN);
        }
    }

#ifdef __HAS_FRONT__
  if (NULL != pFndOrder)
    {
      webCache.head.type = eDOrder;
      webCache.head.dataLen = XORDER_SIZE;
      webCache.ordrsp = *pFndOrder;
      XPushCache (XSHMKEYCONECT (rtnCache), &webCache);
    }
#endif
}

/**
 * 更新委托
 */
static XVoid
_OnRtnOrder (XOrderT *pInOrder)
{
  XOrderT *pOrder = NULL;

  pOrder = pInOrder;

#ifdef __BTEST__
  slog_debug (0,
              "[%d-%s]回报: "
              "本地单号[%d-%d],柜台编号[%lld],股东账户[%s],买卖方向[%d],"
              "是否撤单[%d],订单状态[%d],错误信息[%d],发送时间[%lld],确认时间[%lld]",
              pOrder->request.market, pOrder->request.securityId,
              pOrder->envno, pOrder->request.localId, pOrder->ordid,
              pOrder->request.investId, pOrder->request.bsType,
              pOrder->request.isCancel, pOrder->ordStatus, pOrder->errorno,
			  pOrder->_sendTime, pOrder->_cnfTime);
#endif

  if (pOrder->request.isCancel == false)
    {
      _UpdOrder (pOrder);
    }
  /** 撤单 */
  else
    {
      _UpdCtrlOrg (pOrder);
      _UpdCtrlSelf (pOrder);
    }
    //做风控 TODO
}

/**
 * 更新成交
 */
static XVoid
_OnRtnTrade (XTradeT *pInTrade)
{
  XTradeT *pTrade = NULL, *pFndTrade = NULL;
  XOrderT *pFndOrder = NULL;
  XStrategyT *pStrategy = NULL;
#ifdef __HAS_FRONT__
  XTradeCache webCache = { 0 };
#endif

  pTrade = pInTrade;

  /** 如果是卖单，则解冻卖持仓;如果买单则增加持仓 */
  pFndTrade
      = XFndVTradeByKey (pTrade->customerId, pTrade->market, pTrade->trdId);

  if (NULL != pFndTrade)
    {
      return;
    }

  XPutVTrade (pTrade);

  /** 更新委托 */
  pFndOrder
      = XFndVOrderByCnt (pTrade->customerId, pTrade->market, pTrade->ordid);

  if (NULL == pFndOrder)
    {
      // 未找到原始订单
      return;
    }

  // 找到对应的策略
  if (0 != pFndOrder->request.plotid)
    {
      pStrategy
          = XFndVStrategyByKey (pTrade->customerId, pFndOrder->request.plotid);
      if (NULL != pStrategy
          && pStrategy->setting.market == pFndOrder->request.market
          && 0
                 == strcmp (pFndOrder->request.securityId,
                            pStrategy->setting.securityId))
        {
          if (pTrade->trdSide == eXBuy)
            {
              pStrategy->_buyTrades += pTrade->trdQty;
            }
          else
            {
              pStrategy->_sellTrades += pTrade->trdQty;
            }
        }
    }

  //委托中的成交由成交推送计算
  pFndOrder->trdQty = pTrade->cumQty;
  pFndOrder->trdMoney = pTrade->cumAmt;

  /** 更新订单状态  */
  if (pFndOrder->request.ordQty - pFndOrder->trdQty <= 0)
    {
      pFndOrder->ordStatus = eXOrdStatusFilled;
    }
  else
    {
      pFndOrder->ordStatus = eXOrdStatusPFilled;
    }

#ifdef __HAS_FRONT__
  // 委托
  if (NULL != pFndOrder)
    {
      webCache.head.type = eDOrder;
      webCache.head.dataLen = XORDER_SIZE;
      memcpy (&(webCache.ordrsp), pFndOrder, XORDER_SIZE);
      XPushCache (XSHMKEYCONECT (rtnCache), &webCache);
    }

  // slog_debug(0, "###  推送成交 ########");
  // 成交
  webCache.head.type = eDTrade;
  webCache.head.dataLen = XTRADE_SIZE;
  memcpy (&(webCache.trade), pTrade, XTRADE_SIZE);
  XPushCache (XSHMKEYCONECT (rtnCache), &webCache);

  // 推送策略变动
  if (NULL != pStrategy)
    {
      webCache.head.type = eDStrategy;
      webCache.head.dataLen = XSTRATEGY_SIZE;
      webCache.strategy = *pStrategy;
      XPushCache (XSHMKEYCONECT (rtnCache), &webCache);
    }
#endif
}

/**
 * 更新持仓
 */
static XVoid
_OnRtnHold (XHoldT *pInHold)
{
  XHoldT *pHold = NULL, *pFndHold = NULL;

#ifdef __HAS_FRONT__
  XTradeCache webCache = { 0 };
#endif

  pHold = pInHold;

  if (NULL == pHold)
    {
      return;
    }
  pFndHold = XFndVHoldByKey (pHold->customerId, pHold->investId, pHold->market,
                             pHold->securityId);
  if (NULL != pFndHold)
    {
      pFndHold->countSellAvlHld = pHold->countSellAvlHld;
      pFndHold->sellFrz = pHold->sellFrz;
      pFndHold->sellAvlHld = pFndHold->countSellAvlHld - pFndHold->locFrz;
      pFndHold->sumHld = pHold->sumHld;
      pFndHold->totalBuyAmt = pHold->totalBuyAmt;
      pFndHold->totalBuyHld = pHold->totalBuyHld;
      pFndHold->totalSellAmt = pHold->totalSellAmt;
      pFndHold->totalSellHld = pHold->totalSellHld;
      pFndHold->etfAvlHld = pHold->etfAvlHld;
      pFndHold->costPrice = pHold->costPrice;

#ifdef __BTEST__
      slog_debug (0,
                  "[%s-%d-%s],总持仓[%lld],可用持仓[%lld],冻结持仓[%lld], "
                  "本地冻结[%lld]",
                  pFndHold->customerId, pFndHold->market, pFndHold->securityId,
                  pFndHold->sumHld, pFndHold->sellAvlHld, pFndHold->sellFrz,
                  pFndHold->locFrz);
#endif

#ifdef __HAS_FRONT__
      webCache.head.type = eDHold;
      webCache.head.dataLen = XHOLD_SIZE;
      webCache.hold = *pFndHold;
      XPushCache (XSHMKEYCONECT (rtnCache), &webCache);
#endif
    }
  else
    {
#ifdef __BTEST__
      slog_debug (0,
                  "[%s-%d-%s],总持仓[%lld],可用持仓[%lld],冻结持仓[%lld], "
                  "本地冻结[%lld]",
                  pHold->customerId, pHold->market, pHold->securityId,
                  pHold->sumHld, pHold->sellAvlHld, pHold->sellFrz,
                  pHold->locFrz);
#endif
      XPutOrUpdVHoldByKey (pHold);

#ifdef __HAS_FRONT__
      webCache.head.type = eDHold;
      webCache.head.dataLen = XHOLD_SIZE;
      webCache.hold = *pHold;
      XPushCache (XSHMKEYCONECT (rtnCache), &webCache);
#endif
    }
}
/**
 * 更新资金
 */
static XVoid
_OnRtnCash (XCashT *pCash)
{
  XCashT *pFndCash = NULL;
#ifdef __HAS_FRONT__
  XTradeCache webCache = { 0 };
#endif

  pFndCash = XFndVCashByKey (pCash->customerId, pCash->acctType);
  if (NULL != pFndCash)
    {
      pFndCash->totalBuy = pCash->totalBuy;
      pFndCash->totalSell = pCash->totalSell;
      pFndCash->totalFee = pCash->totalFee;
      pFndCash->countAvailable = pCash->countAvailable;
      pFndCash->curAvailable = pFndCash->countAvailable - pFndCash->locFrz;

#ifdef __HAS_FRONT__
      webCache.head.type = eDCash;
      webCache.head.dataLen = XCASH_SIZE;
      webCache.cash = *pFndCash;
      XPushCache (XSHMKEYCONECT (rtnCache), &webCache);
#endif
    }
}

XVoid
XOrdeng (XVoid *params)
{
  XMonitorMdT *pMonitorMd = NULL;
  XInt readId = -1;
  XBool bClosed = false;
  XLongTime beginTime = 0;
  XBindParamT *pBind = NULL;
  FILE *fp = NULL;
  XInt iret = 0;

  slog_info (0, "XOrdeng启动......");

  pBind = (XBindParamT *)params;
  if (NULL != pBind)
    {
      iret = XBindCPU (pBind->cpuid);
      if (iret)
        {
          slog_warn (0, "绑核失败[%d]", pBind->cpuid);
        }
    }

  fp = fopen (XMAN_DATA_TRADEBIN, "wb");
  if (NULL == fp)
    {
      slog_error (0, "创建交易日志失败[%s]", XMAN_DATA_TRADEBIN);
      return;
    }

  pMonitorMd = XFndVMdMonitor (eXExchSec);

  if (NULL == pMonitorMd)
    {
      slog_error (0, "获取行情信息失败");
      return;
    }
  if (XIsNullCache (XSHMKEYCONECT (tradeCache)))
    {
      slog_error (0, "交易缓存不存在");
      return;
    }
  readId = XGetReadCache (XSHMKEYCONECT (tradeCache));

  for (;;)
    {
      // 收市后延迟5分钟关闭
      bClosed = isMktClosedTime (pMonitorMd->updateTime);
      if (bClosed)
        {
          if (!beginTime)
            {
              beginTime = XGetClockTime ();
            }
          if (XGetClockTime () - beginTime > 5 * 60 * XTIMS_S4NS)
            {
              break;
            }
        }

      XTradeCache *data
          = (XTradeCache *)XPopCache (XSHMKEYCONECT (tradeCache), readId);
      if (NULL == data)
        {
          continue;
        }

      fwrite (data, XTRADECACHE_SIZE, 1, fp);
      fflush (fp);
      switch (data->head.type)
        {
        /** 处理委托订单，买冻结资金或卖冻结持仓，检查通过后加入委托数据中等待处理
         */
        case eDOrderReq:

          //冻结资金或持仓
          XGenOrder (&(data->ordreq));
          break;

          /** 处理委托变化,收到响应，解冻卖冻结的持仓 */
        case eDOrder:
          //解冻资金或持仓
          _OnRtnOrder (&(data->ordrsp));
          break;

          /** 处理订单成交变化 */
        case eDTrade:
          _OnRtnTrade (&(data->trade));
          break;
          /** 处理资金变化 */
        case eDCash:
          _OnRtnCash (&(data->cash));
          break;

          /** 处理持仓变化 */
        case eDHold:
          _OnRtnHold (&(data->hold));
          break;
        default:
          break;
        }
    }

  fclose (fp);
  slog_info (3, ">>>>>> Ordeng正常退出");
  XReleaseCache (XSHMKEYCONECT (tradeCache), readId);
  exit (0);
}
