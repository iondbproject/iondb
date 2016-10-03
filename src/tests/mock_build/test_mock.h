#ifndef TEST_DICTIONARY_H_
#define TEST_DICTIONARY_H_

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "../planckunit/src/planck_unit.h"

#ifdef  __cplusplus
extern "C" {
#endif

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
