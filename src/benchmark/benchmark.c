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
}

void
bmk_run_test_single(
	bmk_test_ptr bmk_test
)
{
	int start_ram = 0;
	int thetime = 0;
	int ram_used = 0;
	int elp_time = 0;
	char* fcn_name;

	/* Run test 3 times and average */
	int i;
	for(i = 0; i < 3; i++)
	{
		start_ram 	= free_ram();
		thetime 	= millis();
	 	fcn_name 	= bmk_test(); /* This also runs the test */
		elp_time 	+= millis() - thetime; 
		ram_used 	+= start_ram - free_ram();
	}

	elp_time = elp_time / 3;
	ram_used = ram_used / 3;

	printf("%s used %db RAM and finished in %dms.\n", fcn_name, ram_used, elp_time);
}

void
bmk_run_test_all(
	bmk_suite_t* suite
)
{
	int i;
	for(i = 0; i < suite->test_count; i++)
	{
		bmk_run_test_single(suite->tests[i]);
	}
	
	printf("Finished all %d tests.\n", suite->test_count);
}

