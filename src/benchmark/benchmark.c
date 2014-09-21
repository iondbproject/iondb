/*
 * benchmark.c
 *
 *  Created on: Sep 20, 2014
 *      Author: Kris
 */

#include "benchmark.h"

void
bmk_register(
	bmk_suite_t* suite,
	bmk_test_ptr bmk_test
)
{
	suite->tests[suite->test_count] = bmk_test;
	suite->test_count++;

#if DEBUG > 0
	printf("Added new test. There are now %d tests.\n", suite->test_count);
#endif

}

void
bmk_run_test_single(
	bmk_test_ptr bmk_test
)
{
	int 	start_ram 	= free_ram();
	ms_start_timer();
	char* 	fcn_name 	= bmk_test();
	int 	elp_time 	= ms_stop_timer();
	int 	ram_used 	= start_ram - free_ram();

	printf("%s used %db RAM and finished in %dms.\n", fcn_name, ram_used, elp_time);
}

void
bmk_run_test_all(
	bmk_suite_t* suite
)
{
	ms_timer_init();
	int i;
	for(i = 0; i < suite->test_count; i++)
	{
		int ram_use = free_ram();
		bmk_run_test_single(suite->tests[i]);
		printf("Delta ram: %d\n", free_ram() - ram_use);
	}

	printf("Finished %d tests.\n", suite->test_count);
}
