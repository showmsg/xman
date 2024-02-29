/**
 * @file test.c
 *
 *  Created on: 2022年6月23日
 *      Author: DELL
 */

#include "ezxml.h"

int main()
{
	ezxml_t root = ezxml_parse_file("securities_20220610.xml"), securities;

	for(securities = ezxml_child(root, "Security"); securities; securities = securities->next)
	{
		printf("%s %s\n", ezxml_child(securities, "SecurityID")->txt, ezxml_child(securities, "PrevClosePx")->txt);
	}

	ezxml_free(root);

	return 0;
}
