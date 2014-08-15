/******************************************************************************/
/**
@file		slhandler.c
@author		Kris Wallperington
@brief		Unit tests for skiplist handler interface.
*/
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./../../../CuTest.h"
#include "./../../../../dictionary/skiplist/sltypes.h"
#include "./../../../../dictionary/skiplist/slstore.h"
#include "./../../../../dictionary/skiplist/slhandler.h"
#include "./../../../../dictionary/dicttypes.h"
#include "./../../../../dictionary/dictionary.h"

CuSuite*
skiplist_handler_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	return suite;
}

void
runalltests_skiplist_handler()
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= skiplist_handler_getsuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("----\nSkiplist Handler Tests:\n%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);
}
