/**
 * @file XBskBatch.c
 * @brief 篮子买卖
 * @author kangyq
 * @version 2.0.0
 * @date 2022-12-16
 * 
 * @copyright 上海量赢科技有限公司 Copyright (c) 2022
 */

#include "XCom.h"
#include "XCSV.h"
#include    <getopt.h>
#include "XLog.h"

typedef struct _xBondCnv
{
	XChar market;
	XSecurityId securityId;
	XInt bs;
	XInt zgjz;				/** 转股价值 */
	XInt zgjyl;				/** 转股溢价率 */
	XPrice price;			/** 转债价格 */
	XPrice zPrice;			/** 正股价格 */
	XPrice convPx;			/** 转股价 */
}XBondCnvT;


static XInt ReadBondCnv(XChar* convFile, XBondCnvT bondcnv[])
{
	XCsvHandleT handle;
	XChar* col = NULL;
	XInt iret = -1;
    XInt i = 0;

	iret = XCsvOpen(&handle, convFile);
	if (iret)
	{
		slog_error(0, "文件不存在[%s]", convFile);
		return (0);
	}
	slog_debug(0, "更新转股价");

	while ((!XCsvReadLine(&handle)))
	{
		if(handle.colSize < 8)
		{
			continue;
		}
		col = handle.GetFieldByCol(&handle, 0);
		if(col)
		{
			bondcnv[i].market = atoi(col);
		}
		col = handle.GetFieldByCol(&handle, 1);
		if(col)
		{
			memcpy(bondcnv[i].securityId, col, strlen(col));
		}

		col = handle.GetFieldByCol(&handle, 2);
		if(col)
		{
			bondcnv[i].bs = atoi(col) ;
		}

		col = handle.GetFieldByCol(&handle, 3);
		if(col)
		{
			bondcnv[i].zgjz = atoi(col);
		}

        col = handle.GetFieldByCol(&handle, 4);
		if(col)
		{
			bondcnv[i].zgjyl = atoi(col);
		}

        col = handle.GetFieldByCol(&handle, 5);
		if(col)
		{
			bondcnv[i].price = atoi(col);
		}

        col = handle.GetFieldByCol(&handle, 6);
		if(col)
		{
			bondcnv[i].zPrice = atoi(col);
		}

        col = handle.GetFieldByCol(&handle, 7);
		if(col)
		{
			bondcnv[i].convPx = atoi(col);
		}
        i++;
	}

	XCsvClose(&handle);

	return (i);
}
static void usage()
{
    printf("###############################################################\n");
	printf("#     篮子交易 1.0.0 \n");
	printf("# -c [客户号] \n");
	printf("# -q [数量] \n");
	printf("###############################################################\n");
}
int main(int argc, char *argv[]) {
	XInt iBuy = 0, i;
    XBondCnvT bondcnv[1000];
    XOrderReqT order = { 0 };
    XCustT *pCust = NULL;
    XChar* customer = NULL;
    XQty qty = 0;
    XSnapshotT *pSnapshot = NULL;
    int opt;
	int option_index = 0;
	const char *option = "hc:q:";
	const struct option long_options[] = { 
        { "help", 0, NULL, 'h' }, 
        { "customer", 1, NULL, 'c'}, 
        { "qty", 1, NULL, 'q' }, 
        { NULL, 0, NULL, 0 } };

	while ((opt = getopt_long(argc, argv, option, long_options, &option_index))
			!= -1) {
		switch (opt) {
		case 'h':
		case '?':
			usage();
			exit(0);
			break;
		case 'c':
			customer = optarg;
			break;
		case 'q':
			qty = atoi(optarg);
			break;
		default:
			break;
		}
	}

	if(NULL == customer || strlen(customer) == 0 || qty <= 0)
	{
		usage();
		exit(0);
	}

	XManShmLoad();
	xslog_init(XSHM_SDB_FILE, "zgjz");

	slog_info(0, "批量买入转债 启动......");
 
    memset(&bondcnv, 0, 1000 * sizeof(XBondCnvT));

	iBuy = ReadBondCnv("../data/outer/closebuy.csv", bondcnv);

    pCust = XFndVCustomerByKey(customer);
	if (NULL == pCust) {
		slog_error(0, "未找到客户号");
		return (-1);
	}

    for(i = 0; i < iBuy; i++)
    {
        pSnapshot = XFndVSnapshotByKey(bondcnv[i].market, bondcnv[i].securityId);
        if(NULL == pSnapshot)
        {
            continue;
        }
        memset(&order, 0, sizeof(XOrderReqT));
        order.market = bondcnv[i].market;
        memcpy(order.securityId, bondcnv[i].securityId, strlen(bondcnv[i].securityId));
        memcpy(order.customerId, pCust->customerId, strlen(pCust->customerId));
	    order.acctType = eXInvSpot;
        order.isCancel = 0;
        order.bsType = bondcnv[i].bs;
		if(bondcnv[i].price == 0)
		{
			order.ordPrice = pSnapshot->tradePx;
		}
		else
		{
			order.ordPrice = bondcnv[i].price;
		}
		
		
        order.ordType = eXOrdLimit;
        order.ordQty = qty;
        XPutOrderReq(&order);

        slog_info(0, ">>>>>> 发出订单[%s],证券代码[%d-%s],证券账户[%s],委托价格[%d],委托数量[%d],是否撤单[%d]",
				order.customerId, order.market, order.securityId, order.investId,
				order.ordPrice, order.ordQty, order.isCancel);
    }

	return (0);
}
