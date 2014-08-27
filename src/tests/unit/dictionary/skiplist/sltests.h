/******************************************************************************/
/**
@file		sltests.h
@author		Kris Wallperington
@brief		Contains test defines and constants unique to skiplist tests.
*/
/******************************************************************************/
#ifndef SLTESTS_H_
#define SLTESTS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./../../../CuTest.h"
#include "./../../../../dictionary/skiplist/sltypes.h"
#include "./../../../../dictionary/skiplist/slstore.h"
#include "./../../../../dictionary/skiplist/slhandler.h"
#include "./../../../../dictionary/dicttypes.h"
#include "./../../../../dictionary/dictionary.h"

#define PRINT_HEADER() io_printf("=== [%d:%s] ===\n", __LINE__, __func__);

/** [[[ Skiplist test template ]]]
@brief 		Insert brief here

@param 		tc
				CuTest dependency

void
test_empty_skiplist_template(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_empty_skiplist_template");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);


	sl_destroy(&skiplist);
} */

/**
@brief 		Insert brief here

@param 		tc
				CuTest dependency
void
test_empty_handler_template(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_empty_handler_template");
} */

#endif /* SLTESTS_H_ */
