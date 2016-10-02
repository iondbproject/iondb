#ifndef TEST_DICTIONARY_H_
#define TEST_DICTIONARY_H_

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "../planckunit/src/planck_unit.h"
#include "../../dictionary/dictionary_types.h"
#include "../../dictionary/dictionary.h"
#include "../../dictionary/ion_master_table.h"
#include "../../dictionary/flat_file/flat_file_dictionary_handler.h"

#ifdef  __cplusplus
extern "C" {
#endif
void
test_mock(
);

planck_unit_suite_t *
mock_getsuite(
);

void
runalltests_mock(
);

#ifdef  __cplusplus
}
#endif

#endif
