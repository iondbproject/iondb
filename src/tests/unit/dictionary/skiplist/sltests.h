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
#include "./../../../planckunit.h"
#include "./../../../../dictionary/skiplist/sltypes.h"
#include "./../../../../dictionary/skiplist/slstore.h"
#include "./../../../../dictionary/skiplist/slhandler.h"
#include "./../../../../dictionary/dicttypes.h"
#include "./../../../../dictionary/dictionary.h"

#define PRINT_HEADER() io_printf("=== [%d:%s] ===\n", __LINE__, __func__);

#endif /* SLTESTS_H_ */
