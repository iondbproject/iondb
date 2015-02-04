/*
 * ion_bench.c
 *
 *  Created on: Sep 21, 2014
 *      Author: Kris
 */

#include "dictionary/dictionary.h"
#include "benchmark/benchmark.h"

#include "dictionary/skiplist/slhandler.h"
//#include "dictionary/openaddresshash/oadictionaryhandler.h"
//#include "dictionary/openaddressfilehash/oafdictionaryhandler.h"
//#include "dictionary/flatfilestore/ffdictionaryhandler.h"

//#include <SD.h>
//#include "sd_if/SD_stdio_c_iface.h"

#include "lfsr/lfsr.h"

#define MAP_TESTS 1
#define CUR_TESTS 2

#define TEST_CASE 1

#define I1C 50
#define I2C 100
#define I3C 150
#define I4C 200

#define SECONDARY_INSERT_COUNT 200

#define G1C 50
#define G2C 100
#define G3C 150
#define G4C 200

#define D1C 50
#define D2C 100
#define D3C 150
#define D4C 200

#define CURSOR_INSERT_COUNT 150

#define E1C 5
#define E2C 10
#define E3C 15

#define FCN_NAME ((char*) __func__)
/**< MAKE_ION_KEY :: int -> ion_key_t (unsigned char*) */
//#define MAKE_ION_KEY(x) ({static int _x = x; (ion_key_t) &_x;})
#define MAKE_ION_KEY(x) (_keyswap = x, (ion_key_t) &_keyswap)
//#define MAKE_ION_KEY(x) (ion_key_t) &(int){x};
#define ION_KEY_TO_INT(key) *((int*) key)

static int _keyswap;

/** CONFIG PARAMS **/

/**< Handler of dict to test. */
err_t 				(*handler_fptr)(dictionary_handler_t*) 	= sldict_init;
//void 				(*handler_fptr)(dictionary_handler_t*) 	= oadict_init;
//void 				(*handler_fptr)(dictionary_handler_t*) 	= oafdict_init;
//void 				(*handler_fptr)(dictionary_handler_t*) 	= ffdict_init;

/**< Type of key to test. */
key_type_t 			key_type 								= key_type_numeric_signed;
/**< Size of key to test. (Default = 2) */
ion_key_size_t		key_size 								= 2;
/**< Size of value to test. (Default = 8) */
ion_value_size_t	value_size 								= 8;
/**< Size of dict to test. */
int					dict_size 								= 10;
/**< Value payload. */
ion_value_t		test_value 	= (ion_value_t) (char*){"IonDB Test String"};
/**< Number sequence */
lfsr_t keygen;
/**< LFSR Seed */
#define KEY_SEED 0xACE1u

char*
bench_empty_test(
)
{
	return FCN_NAME;
}

char*
bench_create_dict(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);

	int nowram = free_ram();
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	printf("Ram used: %d\n", nowram - free_ram());

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_insert_single(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
	int nowram = free_ram();
	err_t status = dictionary_insert(&dict, key, test_value);
	printf("Ram used: %d\n", nowram - free_ram());

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_insert_maximum(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	err_t status = err_ok;
	int count = 0;
	while(status != err_out_of_memory)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		status = dictionary_insert(&dict, key, test_value);
		if(status == err_max_capacity)
		{
			printf("Too full\n");
		}
		count++;
	}
	dictionary_delete_dictionary(&dict);
	printf("Got %d inserts max.\n", count);
	return FCN_NAME;
}

char*
bench_insert_1(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int i;
	for(i = 0; i < I1C; i++)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}


char*
bench_insert_2(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int i;
	for(i = 0; i < I2C; i++)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_insert_3(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int i;
	for(i = 0; i < I3C; i++)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_insert_4(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int i;
	for(i = 0; i < I4C; i++)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_insert(&dict, key, test_value);
		/*if(status == err_out_of_memory)
		{
			printf("BONK\n");
		} */
	}

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_get_1(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int i;
	for(i = 0; i < SECONDARY_INSERT_COUNT; i++)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	lfsr_reset(&keygen);
	for(i = 0; i < G1C; i++)
	{
		ion_key_t 	key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		char 		value[value_size];
		dictionary_get(&dict, key, (ion_value_t) value);
	}

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_get_2(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int i;
	for(i = 0; i < SECONDARY_INSERT_COUNT; i++)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	lfsr_reset(&keygen);
	for(i = 0; i < G2C; i++)
	{
		ion_key_t 	key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		char 		value[value_size];
		dictionary_get(&dict, key, (ion_value_t) value);
	}

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_get_3(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int i;
	for(i = 0; i < SECONDARY_INSERT_COUNT; i++)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	lfsr_reset(&keygen);
	for(i = 0; i < G3C; i++)
	{
		ion_key_t 	key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		char 		value[value_size];
		dictionary_get(&dict, key, (ion_value_t) value);
	}

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_get_4(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int i;
	for(i = 0; i < SECONDARY_INSERT_COUNT; i++)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	lfsr_reset(&keygen);
	for(i = 0; i < G4C; i++)
	{
		ion_key_t 	key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		char 		value[value_size];
		dictionary_get(&dict, key, (ion_value_t) value);
	}

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_delete_1(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int i;
	for(i = 0; i < SECONDARY_INSERT_COUNT; i++)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	lfsr_reset(&keygen);
	int tstart = millis();
	for(i = 0; i < D1C; i++)
	{
		ion_key_t 	key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_delete(&dict, key);
	}

	printf("del: %d\n", millis() - tstart);
	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_delete_2(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int i;
	for(i = 0; i < SECONDARY_INSERT_COUNT; i++)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	lfsr_reset(&keygen);
	int tstart = millis();
	for(i = 0; i < D2C; i++)
	{
		ion_key_t 	key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		dictionary_delete(&dict, key);
	}
	
	printf("del: %d\n", millis() - tstart);
	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_delete_3(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int i;
	for(i = 0; i < SECONDARY_INSERT_COUNT; i++)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	lfsr_reset(&keygen);
	int tstart = millis();
	for(i = 0; i < D3C; i++)
	{
		ion_key_t 	key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		dictionary_delete(&dict, key);
	}
	
	printf("del: %d\n", millis() - tstart);
	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_delete_4(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int i;
	for(i = 0; i < SECONDARY_INSERT_COUNT; i++)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	lfsr_reset(&keygen);
	int tstart = millis();
	for(i = 0; i < D4C; i++)
	{
		ion_key_t 	key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		dictionary_delete(&dict, key);
	}

	printf("del: %d\n", millis() - tstart);
	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_equality_1(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int i;
	for(i = 0; i < CURSOR_INSERT_COUNT; i++)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	lfsr_reset(&keygen);
	for(i = 0; i < E1C; i++)
	{
		ion_key_t 	key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));

		dict_cursor_t *cursor = NULL;
		predicate_t predicate;
		predicate.type = predicate_equality;
		predicate.statement.equality.equality_value = key;
		err_t status = dict.handler->find(&dict, &predicate, &cursor);
		ion_record_t record;
		record.key 		= (ion_key_t) malloc(dict.instance->record.key_size);
		record.value 	= (ion_value_t) malloc(dict.instance->record.value_size);
		while(cursor->next(cursor, &record) != cs_end_of_results);
		cursor->destroy(&cursor);
		free(record.key);
		free(record.value);
	}

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_equality_2(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int i;
	for(i = 0; i < CURSOR_INSERT_COUNT; i++)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	lfsr_reset(&keygen);
	for(i = 0; i < E2C; i++)
	{
		ion_key_t 	key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		dict_cursor_t *cursor = NULL;
		predicate_t predicate;
		predicate.type = predicate_equality;
		predicate.statement.equality.equality_value = key;
		err_t status = dict.handler->find(&dict, &predicate, &cursor);
		ion_record_t record;
		record.key 		= (ion_key_t) malloc(dict.instance->record.key_size);
		record.value 	= (ion_value_t) malloc(dict.instance->record.value_size);
		while(cursor->next(cursor, &record) != cs_end_of_results);
		cursor->destroy(&cursor);
		free(record.key);
		free(record.value);
	}

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_equality_3(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int i;
	for(i = 0; i < CURSOR_INSERT_COUNT; i++)
	{
		ion_key_t 		key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	lfsr_reset(&keygen);
	for(i = 0; i < E3C; i++)
	{
		ion_key_t 	key 	= MAKE_ION_KEY(lfsr_get_next(&keygen));
		dict_cursor_t *cursor = NULL;
		predicate_t predicate;
		predicate.type = predicate_equality;
		predicate.statement.equality.equality_value = key;
		err_t status = dict.handler->find(&dict, &predicate, &cursor);
		ion_record_t record;
		record.key 		= (ion_key_t) malloc(dict.instance->record.key_size);
		record.value 	= (ion_value_t) malloc(dict.instance->record.value_size);
		while(cursor->next(cursor, &record) != cs_end_of_results);
		cursor->destroy(&cursor);
		free(record.key);
		free(record.value);
	}

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_range_1(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int min,max;
	int i;
	for(i = 0; i < CURSOR_INSERT_COUNT; i++)
	{
		ion_key_t key = MAKE_ION_KEY(lfsr_get_next(&keygen));
		if(i == 0)
		{
			min = ION_KEY_TO_INT(key);
			max = ION_KEY_TO_INT(key);
		}
		else
		{
			int curkey = ION_KEY_TO_INT(key);
			if(curkey > max)
			{
				max = curkey;
			}
			else if(curkey < min)
			{
				min = curkey;
			}
		}
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	int int_leq = min;
	int int_geq = min + (max - min) / 2;
	ion_key_t 	leq 	= (ion_key_t) &int_leq;
	ion_key_t 	geq 	= (ion_key_t) &int_geq;
	dict_cursor_t *cursor = NULL;
	predicate_t predicate;
	predicate.type = predicate_range;
	predicate.statement.range.leq_value = leq;
	predicate.statement.range.geq_value = geq;
	err_t status = dict.handler->find(&dict, &predicate, &cursor);
	ion_record_t record;
	record.key 		= (ion_key_t) malloc(dict.instance->record.key_size);
	record.value 	= (ion_value_t) malloc(dict.instance->record.value_size);
	while(cursor->next(cursor, &record) != cs_end_of_results);
	cursor->destroy(&cursor);
	free(record.key);
	free(record.value);

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

char*
bench_range_2(
)
{
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);

	int min,max;
	int i;
	for(i = 0; i < CURSOR_INSERT_COUNT; i++)
	{
		ion_key_t key = MAKE_ION_KEY(lfsr_get_next(&keygen));

		if(i == 0)
		{
			min = ION_KEY_TO_INT(key);
			max = ION_KEY_TO_INT(key);
		}
		else
		{
			int curkey = ION_KEY_TO_INT(key);
			if(curkey > max)
			{
				max = curkey;
			}
			else if(curkey < min)
			{
				min = curkey;
			}
		}
		err_t status = dictionary_insert(&dict, key, test_value);
	}

	int int_leq = min + (max - min) / 2;
	int int_geq = max;
	ion_key_t 	leq 	= (ion_key_t) &int_leq;
	ion_key_t 	geq 	= (ion_key_t) &int_geq;
	dict_cursor_t *cursor = NULL;
	predicate_t predicate;
	predicate.type = predicate_range;
	predicate.statement.range.leq_value = leq;
	predicate.statement.range.geq_value = geq;
	err_t status = dict.handler->find(&dict, &predicate, &cursor);
	ion_record_t record;
	record.key 		= (ion_key_t) malloc(dict.instance->record.key_size);
	record.value 	= (ion_value_t) malloc(dict.instance->record.value_size);
	while(cursor->next(cursor, &record) != cs_end_of_results);
	cursor->destroy(&cursor);
	free(record.key);
	free(record.value);

	dictionary_delete_dictionary(&dict);
	return FCN_NAME;
}

void
ion_bench_make_suite(
	bmk_suite_t* suite
)
{
	suite->test_count = 0;
#if TEST_CASE == 1
	//bmk_register(suite, &bench_insert_maximum);
	bmk_register(suite, &bench_empty_test);
	bmk_register(suite, &bench_create_dict);
	bmk_register(suite, &bench_insert_single);
	
	bmk_register(suite, &bench_insert_1);
	bmk_register(suite, &bench_insert_2);
	bmk_register(suite, &bench_insert_3);
	bmk_register(suite, &bench_insert_4); 

	bmk_register(suite, &bench_get_1);
	bmk_register(suite, &bench_get_2);
	bmk_register(suite, &bench_get_3);
	bmk_register(suite, &bench_get_4);

	bmk_register(suite, &bench_delete_1);
	bmk_register(suite, &bench_delete_2);
	bmk_register(suite, &bench_delete_3);
	bmk_register(suite, &bench_delete_4); 
#endif

#if TEST_CASE == 2
	bmk_register(suite, &bench_equality_1);
	bmk_register(suite, &bench_equality_2);
	bmk_register(suite, &bench_equality_3);

	bmk_register(suite, &bench_range_1);
	bmk_register(suite, &bench_range_2);
#endif
}

	
void setup()
{
	/* Arduino stuff */
	Serial.begin(9600);
	Serial.println("ready!");

	/* File stuff 
	//pinMode(10, OUTPUT);
	pinMode(53, OUTPUT);
	if (!SD.begin(4))
	{
		Serial.println("sd init failed!");
		Serial.flush();
		return;
	}
	FILE * file;
	//remove the file before starting
	fremove("FILE.BIN"); 

	/* LFSR stuff */
	lfsr_init_start_state(KEY_SEED, &keygen);

	/* Benchmark stuff */
	bmk_suite_t suite;
	ion_bench_make_suite(&suite);
	int i;
	bmk_run_test_all(&suite); 
}

void loop()
{
}