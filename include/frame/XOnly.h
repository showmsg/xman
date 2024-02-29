/*
 * @file XOnly.h
 *
 *  Created on: 2022年8月8日
 *      Author: DELL
 */

#ifndef INCLUDE_XMAN_OES_XONLY_H_
#define INCLUDE_XMAN_OES_XONLY_H_
#include "XTypes.h"

typedef enum md_type_t
{
	frn,
	lv1,
	vl2,
	eqt,
	x
}XMdType;
typedef struct _md_snaps_t
{
	long long idx;
	unsigned long long ns;
	char sym[16];
	double ntime;
	double etime;
	double last_px;
	double to;
	double oi;
	long long cumv;
	double bid_px[10];
	double ask_px[10];
	int bid_size[10];
	int ask_size[10];
	int depth;
	XMdType type;
}XMdSnapT;


#endif /* INCLUDE_XMAN_OES_XONLY_H_ */
