/*
 * @file XPurchase.c
 * @brief 新股新债申购
 * @author kangyq
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */
#include "XCom.h"
#include "XTimes.h"
#include "XLog.h"

XVoid
Purchase (XCustT *pCust, XMonitorT *pMonitor)
{
  XOrderReqT order =
    { 0 };
  XInvestT *pInvest = NULL;
  XIssueT *pIssue = NULL;
  XHoldT *pHold = NULL;
  XIdx i = 0;
  XQty quota = 0;

  for (i = 0; i < pMonitor->iTIssue; i++)
    {
      pIssue = XFndVIssueById (i + 1);
      if (NULL == pIssue)
	{
	  continue;
	}
      pInvest = XFndVInvestByAcctType (pCust->customerId, pIssue->market, eXInvSpot);
      if (NULL == pInvest)
	{
	  continue;
	}
      switch (pIssue->issueType)
	{
	/** 新股 */
	case eXIssQuota:

	  /** 取额度 */
	  if (eXSubSecKSH == pIssue->subSecType || eXSubSecKCDR == pIssue->subSecType)
	    {
	      quota = pInvest->kcQuota;
	    }
	  else
	    {
	      quota = pInvest->mainQuota;
	    }
	  if (quota < pIssue->minQty)
	    {
	      break;
	    }
	  if (quota > pIssue->maxQty)
	    {
	      quota = pIssue->maxQty;
	    }

	  //下单
	  memcpy (order.customerId, pCust->customerId, sizeof(XCustomer));
	  order.market = pIssue->market;
	  memcpy (order.securityId, pIssue->securityId, sizeof(XSecurityId));
	  order.bsType = eXDeem;
	  order.ordPrice = pIssue->issuePrice;
	  order.ordQty = quota;
	  order.ordType = eXOrdLimit;
	  order.acctType = eXInvSpot;

//			memcpy(order.investId, pInvest->investId, sizeof(XInvestId));
	  XPutOrderReq (&order);

	  break;
	  /** 公开增发 */
	case eXIssCash:
	  break;
	  /** 信用申购 */
	case eXIssCredit:

	  //信用申购
	  memcpy (order.customerId, pCust->customerId, sizeof(XCustomer));
	  order.market = pIssue->market;
	  memcpy (order.securityId, pIssue->securityId, sizeof(XSecurityId));
	  order.bsType = eXDeem;
	  order.ordPrice = pIssue->issuePrice;
	  order.ordQty = pIssue->maxQty;
	  order.ordType = eXOrdLimit;
	  order.acctType = eXInvSpot;

	  XPutOrderReq (&order);
	  break;
	  /** 配股配债 */
	case eXIssAllot:

	  pHold = XFndVHoldByKey (pCust->customerId, pInvest->investId, pIssue->market, pIssue->securityId);
	  if (NULL == pHold)
	    {
	      break;
	    }

	  memcpy (order.customerId, pCust->customerId, sizeof(XCustomer));
	  order.market = pIssue->market;
	  memcpy (order.securityId, pIssue->securityId, sizeof(XSecurityId));
	  order.bsType = eXSell;
	  order.ordPrice = pIssue->issuePrice;
	  order.ordQty = pIssue->maxQty;
	  order.ordType = eXOrdLimit;
	  order.acctType = eXInvSpot;

	  XPutOrderReq (&order);

	  break;
	default:
	  break;
	}

    }

}

XVoid
Trade ()
{
  XInt i = 0;
  XMonitorT *pMonitor = NULL;
  XCustT *pCust = NULL;

  pMonitor = XFndVMonitor ();
  if (NULL == pMonitor)
    {
      return;
    }

  for (i = 0; i < pMonitor->iTCust; i++)
    {
      pCust = XFndVCustomerById (i + 1);
      if (NULL == pCust || pCust->type == eXUserMarket || pCust->counter != eXCounterOes)
	{
	  continue;
	}
      Purchase (pCust, pMonitor);
    }

}

int
main (int argc, char *argv[])
{
  XMonitorT *pMonitor = NULL;

  XManShmLoad ();
  xslog_init (XSHM_SDB_FILE, "xpurchase");
  slog_info(3, "xpurchase 启动......");

  pMonitor = XFndVMonitor ();
  if (NULL == pMonitor)
    {
      return (-1);
    }

  Trade ();

  slog_info(3, "xpurchase 关闭");

  return (0);
}
