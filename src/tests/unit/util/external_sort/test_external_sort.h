#if !defined(PLANCK_UNIT_TEST_FILE_SORT_H_)
#define PLANCK_UNIT_TEST_FILE_SORT_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../../../util/sort/external_sort/external_sort.h"
#include "../../../../util/sort/external_sort/flash_min_sort.h"
#include "../../../planckunit/src/planck_unit.h"

#if defined(__cplusplus)
extern "C" {
#endif

void
runalltests_file_sort(
);

#if defined(__cplusplus)
}
#endif

#endif /* PLANCK_UNIT_TEST_FILE_SORT_H_ */
