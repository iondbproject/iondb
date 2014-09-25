/*
 * ion_bench.c
 *
 *  Created on: Sep 21, 2014
 *      Author: Kris
 */

#include <stdio.h>
#include <avr/io.h>
#include "serial.h"
#include "dictionary/dictionary.h"
#include "dictionary/skiplist/slhandler.h"
#include "benchmark/benchmark.h"

#define FCN_NAME ((char*) __func__)

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,_FDEV_SETUP_WRITE);

/** CONFIG PARAMS **/

/**< Handler of dict to test. */
err_t 				(*handler_fptr)(dictionary_handler_t*) 	= sldict_init;
/**< Type of key to test. */
key_type_t 			key_type 								= key_type_numeric_signed;
/**< Size of key to test. (Default = 2) */
ion_key_size_t		key_size 								= 2;
/**< Size of value to test. (Default = 8) */
ion_value_size_t	value_size 								= 8;
/**< Size of dict to test. */
int					dict_size 								= 7;
/**< Random seed to use. */
#define 			RANDOM_SEED 							1337
/**< Maximum key allowed */
#define 			MAX_INT_KEY 							9999

char*
bench_empty_test(
	void
)
{
	return FCN_NAME;
}

char*
bench_create_dict(
	void
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	return FCN_NAME;
}

char*
bench_insert_10(
	void
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);

	int i;
	for(i = 0; i < 10; i++)
	{
		ion_key_t 		key 	= (ion_key_t) &(int){random() % MAX_INT_KEY};
		ion_value_t		value 	= (ion_value_t) (char*){"IonDB Test String"};
		err_t status = dictionary_insert(&dict, key, value);
		printf("status: %d\n", status);
	}

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}


char*
bench_insert_15(
	void
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);

	int i;
	for(i = 0; i < 15; i++)
	{
		ion_key_t 		key 	= (ion_key_t) &(int){random() % MAX_INT_KEY};
		ion_value_t		value 	= (ion_value_t) (char*){"IonDB Test String"};
		err_t status = dictionary_insert(&dict, key, value);
		printf("status: %d\n", status);
	}

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_insert_20(
	void
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);

	int i;
	for(i = 0; i < 20; i++)
	{
		ion_key_t 		key 	= (ion_key_t) &(int){random() % MAX_INT_KEY};
		ion_value_t		value 	= (ion_value_t) (char*){"IonDB Test String"};
		err_t status = dictionary_insert(&dict, key, value);
		printf("status: %d\n", status);
	}

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_insert_35(
	void
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);

	int i;
	for(i = 0; i < 35; i++)
	{
		ion_key_t 		key 	= (ion_key_t) &(int){random() % MAX_INT_KEY};
		ion_value_t		value 	= (ion_value_t) (char*){"IonDB Test String"};
		err_t status = dictionary_insert(&dict, key, value);
		printf("status: %d\n", status);
	}

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}


void
ion_bench_make_suite(
	bmk_suite_t* suite
)
{
	suite->test_count = 0;
	bmk_register(suite, bench_empty_test);
	bmk_register(suite, bench_create_dict);
	bmk_register(suite, bench_insert_10);
	bmk_register(suite, bench_insert_15);
	bmk_register(suite, bench_insert_20);
	bmk_register(suite, bench_insert_35);
}

int
main(
	void
)
{
	uart_init();
	stdout = &mystdout;
	sei();
	srand(RANDOM_SEED);

	bmk_suite_t suite;
	ion_bench_make_suite(&suite);
	bmk_run_test_all(&suite);
}
