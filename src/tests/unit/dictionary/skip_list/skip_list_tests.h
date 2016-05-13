/******************************************************************************/
/**
@file		sltests.h
@author		Kris Wallperington
@brief		Contains test defines and constants unique to skip_list tests.
*/
/******************************************************************************/
#ifndef SLTESTS_H_
#define SLTESTS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./../../../CuTest.h"
#include "../../../planck_unit.h"
#include "../../../../dictionary/skiplist/skip_list_types.h"
#include "../../../../dictionary/skiplist/skip_list.h"
#include "../../../../dictionary/skiplist/skip_list_handler.h"
#include "../../../../dictionary/dictionary_types.h"
#include "./../../../../dictionary/dictionary.h"

#define PRINT_HEADER() io_printf("=== [%d:%s] ===\n", __LINE__, __func__);

#endif /* SLTESTS_H_ */
