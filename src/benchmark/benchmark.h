/*
 * benchmark.h
 *
 *  Created on: Sep 20, 2014
 *      Author: Kris
 */

#ifndef BENCHMARK_H_
#define BENCHMARK_H_

#include "./../kv_system.h"
#include "./../kv_io.h"
#include "./../milli/millisec.h"
#include "./../ram/ramutil.h"

#define MAX_TESTS 50 /**< Maximum number of tests in a suite */

/* What a single benchmark test is defined as.*/
typedef char* (*bmk_test_ptr)(void);

typedef struct bmk_suite
{
	int 			test_count; /**< Number of tests in the suite */
	bmk_test_ptr 	tests[MAX_TESTS]; 	/**< Array of test fptrs to run */
} bmk_suite_t;

/* Given a suite and a test fptr, registers the test to the suite. */
void
bmk_register(
	bmk_suite_t*,
	bmk_test_ptr
);

/* Given a test fptr, run the test and output the results. */
void
bmk_run_test_single(
	bmk_test_ptr
);

/* Given a suite, run every single test in it using "bmk_run_test_single". */
void
bmk_run_test_all(
	bmk_suite_t*
);

#endif /* BENCHMARK_H_ */
