#ifndef TEST_DICTIONARY_H_
#define TEST_DICTIONARY_H_

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "../../planck-unit/src/planck_unit.h"
#include "../../../dictionary/dictionary_types.h"
#include "./../../../dictionary/dictionary.h"
#include "./../../../dictionary/ion_master_table.h"
#include "../../../dictionary/flat_file/flat_file_dictionary_handler.h"

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
