#ifndef PLANCK_UNIT_LINEAR_HASH_NEW_BENCHMARKS_H
#define PLANCK_UNIT_LINEAR_HASH_NEW_BENCHMARKS_H

#include "../../dictionary/dictionary.h"
#include "../../dictionary/linear_hash_new/linear_hash.h"
#include "../../dictionary/linear_hash_new/linear_hash_handler.h"
#include "../../util/lfsr/lfsr.h"
#include "./../../tests/planck-unit/src/ion_time/ion_time.h"
#include <stdio.h>
#include "../../file/kv_stdio_intercept.h"

#if defined(__cplusplus)
extern "C" {
#endif


void
run_benchmarks(
);

#if defined(__cplusplus)
}
#endif

#endif /* PLANCK_UNIT_LINEAR_HASH_NEW_BENCHMARKS_H */
