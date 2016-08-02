#ifndef TEST_DICTIONARY_H_
#define TEST_DICTIONARY_H_

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "../../planckunit/src/planck_unit.h"
#include "../../../dictionary/dictionary_types.h"
#include "./../../../dictionary/dictionary.h"
#include "./../../../dictionary/ion_master_table.h"
#include "../../../dictionary/bpp_tree/bpp_tree_handler.h"

#ifdef  __cplusplus
extern "C" {
#endif

void
runalltests_dictionary(
);

#ifdef  __cplusplus
}
#endif

#endif
