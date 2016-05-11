/******************************************************************************/
/**
@file		skip_list_tests.h
@author		Kris Wallperington
@brief		Contains test defines and constants unique to skip_list tests.
*/
/******************************************************************************/
#ifndef SKIP_LIST_TESTS_H_
#define SKIP_LIST_TESTS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./../../../CuTest.h"
#include "./../../../../dictionary/skip_list/skip_list_types.h"
#include "./../../../../dictionary/skip_list/skip_list.h"
#include "./../../../../dictionary/skip_list/skip_list_handler.h"
#include "../../../../dictionary/dictionary_types.h"
#include "./../../../../dictionary/dictionary.h"

#define PRINT_HEADER() io_printf("=== [%d:%s] ===\n", __LINE__, __func__);

#endif /* SKIP_LIST_TESTS_H_ */
