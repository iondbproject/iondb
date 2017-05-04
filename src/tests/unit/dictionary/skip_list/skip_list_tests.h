/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Contains test defines and constants unique to skiplist tests.
*/
/******************************************************************************/
#if !defined(SLTESTS_H_)
#define SLTESTS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../planck-unit/src/planck_unit.h"
#include "../../../../dictionary/skip_list/skip_list_types.h"
#include "../../../../dictionary/skip_list/skip_list.h"
#include "../../../../dictionary/skip_list/skip_list_handler.h"
#include "../../../../dictionary/dictionary_types.h"
#include "./../../../../dictionary/dictionary.h"

#if DEBUG
#define PRINT_HEADER() printf("=== [%d:%s] ===\n", __LINE__, __func__);
#else
#define PRINT_HEADER()
#endif

#endif /* SLTESTS_H_ */
