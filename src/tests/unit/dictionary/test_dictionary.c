/*
 * dictionary.c
 *
 *  Created on: Jul 15, 2014
 *	  Author: workstation
*/

#include "test_dictionary.h"

void
test_dictionary_compare_numerics(
	planck_unit_test_t *tc
) {
	ion_key_t	key_one;
	ion_key_t	key_two;

	key_one = &(int) {
		1
	};
	key_two = &(int) {
		1
	};

	PLANCK_UNIT_ASSERT_TRUE(tc, ION_IS_EQUAL == dictionary_compare_signed_value(key_one, key_two, sizeof(int)));

	key_one = &(int) {
		1
	};
	key_two = &(int) {
		2
	};

	PLANCK_UNIT_ASSERT_TRUE(tc, ION_ZERO > dictionary_compare_signed_value(key_one, key_two, sizeof(int)));

	key_one = &(int) {
		2
	};
	key_two = &(int) {
		0
	};

	PLANCK_UNIT_ASSERT_TRUE(tc, ION_ZERO < dictionary_compare_signed_value(key_one, key_two, sizeof(int)));

	int i;

	for (i = 1; i < 10; i++) {
		PLANCK_UNIT_ASSERT_TRUE(tc, ION_ZERO < dictionary_compare_signed_value(&i, key_two, sizeof(int)));
	}

	/* case for unsigned signed char */
	{
		ion_byte_t	*key_one;
		ion_byte_t	*key_two;

		key_one = &(ion_byte_t) {
			UCHAR_MAX
		};
		key_two = &(ion_byte_t) {
			0
		};

		char result = dictionary_compare_unsigned_value(key_one, key_two, sizeof(ion_byte_t));

		PLANCK_UNIT_ASSERT_TRUE(tc, ION_ZERO < result);
	}

	{
		unsigned short	*key_one;
		unsigned short	*key_two;

		key_one = &(unsigned short) {
			USHRT_MAX
		};
		key_two = &(unsigned short) {
			0
		};

		PLANCK_UNIT_ASSERT_TRUE(tc, ION_ZERO < dictionary_compare_unsigned_value(key_one, key_two, sizeof(unsigned short)));
	}

	{
		unsigned int	*key_one;
		unsigned int	*key_two;

		key_one = &(unsigned int) {
			UINT_MAX
		};
		key_two = &(unsigned int) {
			0
		};

		PLANCK_UNIT_ASSERT_TRUE(tc, ION_ZERO < dictionary_compare_unsigned_value(key_one, key_two, sizeof(unsigned int)));
	}

	{
		unsigned long	*key_one;
		unsigned long	*key_two;

		key_one = &(unsigned long) {
			ULONG_MAX
		};
		key_two = &(unsigned long) {
			0
		};

		PLANCK_UNIT_ASSERT_TRUE(tc, ION_ZERO < dictionary_compare_unsigned_value(key_one, key_two, sizeof(unsigned long)));
	}

	{
		long	*key_one;
		long	*key_two;

		key_one = &(long) {
			LONG_MAX
		};
		key_two = &(long) {
			0
		};

		PLANCK_UNIT_ASSERT_TRUE(tc, ION_ZERO < dictionary_compare_signed_value(key_one, key_two, sizeof(long)));
	}
	{
		char	*key_one;
		char	*key_two;

		key_one = &(char) {
			CHAR_MAX
		};
		key_two = &(char) {
			0
		};

		PLANCK_UNIT_ASSERT_TRUE(tc, ION_ZERO < dictionary_compare_signed_value(key_one, key_two, sizeof(char)));
	}

	{
		char	*key_one;
		char	*key_two;

		key_one = &(char) {
			0
		};
		key_two = &(char) {
			-1
		};

		PLANCK_UNIT_ASSERT_TRUE(tc, ION_ZERO < dictionary_compare_signed_value(key_one, key_two, sizeof(char)));
	}
}

void
test_dictionary_master_table(
	planck_unit_test_t *tc
) {
	ion_err_t err;

	/* Cleanup, just in case */
	err = ion_close_master_table();
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	fremove(ION_MASTER_TABLE_FILENAME);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == ion_master_table_file);

	/* Test init */
	printf("expect hello\n");
	err = ion_init_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != ion_master_table_file);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, ion_master_table_next_id);

	/*************/

	/* Test create */
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dictionary;

	ffdict_init(&handler);
	err = ion_master_table_create_dictionary(&handler, &dictionary, key_type_numeric_signed, sizeof(int), 10, 20);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, ion_master_table_next_id);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != ion_master_table_file);
	/***************/

	/* Test close */
	err = ion_close_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == ion_master_table_file);
	/**************/

	/* Test re-open */
	err = ion_init_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != ion_master_table_file);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, ion_master_table_next_id);

	/****************/

	/* Test lookup 1st dictionary */
	ion_dictionary_config_info_t config;

	err = ion_lookup_in_master_table(1, &config);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, config.id);
	PLANCK_UNIT_ASSERT_TRUE(tc, key_type_numeric_signed == config.type);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(int), config.key_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 10, config.value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 20, config.dictionary_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_type_flat_file_t, config.dictionary_type);

	/******************************/

	/* Test create 2nd dictionary */
	ion_dictionary_handler_t	handler2;
	ion_dictionary_t			dictionary2;

	ffdict_init(&handler2);
	err = ion_master_table_create_dictionary(&handler2, &dictionary2, key_type_numeric_signed, sizeof(short), 7, 14);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 3, ion_master_table_next_id);
	/******************************/

	/* Test 2nd lookup */
	err = ion_lookup_in_master_table(2, &config);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, config.id);
	PLANCK_UNIT_ASSERT_TRUE(tc, key_type_numeric_signed == config.type);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(short), config.key_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 7, config.value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 14, config.dictionary_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_type_flat_file_t, config.dictionary_type);
	/*******************/

	/* Test delete */
	err = ion_delete_dictionary(&dictionary, dictionary.instance->id);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != ion_master_table_file);
	/***************/

	/* Test lookup on non-existent row */
	err = ion_lookup_in_master_table(1, &config);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_item_not_found, err);
	/***********************************/

	/* Test close dictionary */

	err = ion_close_dictionary(&dictionary2);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != ion_master_table_file);

	/* Test open dictionary */

	err = ion_open_dictionary(&handler2, &dictionary2, 2);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, dictionary2.instance->id);
	PLANCK_UNIT_ASSERT_TRUE(tc, key_type_numeric_signed == dictionary2.instance->key_type);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(short), dictionary2.instance->record.key_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 7, dictionary2.instance->record.value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_type_flat_file_t, dictionary2.instance->type);

	/* Test close dictionary */

	err = ion_close_dictionary(&dictionary2);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);

	/* Test delete closed dictionary */

	err = ion_delete_dictionary(&dictionary2, 2);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);

	/* Test close master table */

	err = ion_close_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == ion_master_table_file);

	/* Test delete master table */

	err = ion_delete_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == ion_master_table_file);
	/**************/
}

void
test_dictionary_master_table2(
	planck_unit_test_t *tc
) {
	ion_err_t err;

	/* Cleanup, just in case */
	err = ion_close_master_table();
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	fremove(ION_MASTER_TABLE_FILENAME);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == ion_master_table_file);

	/* Test init */
	printf("expect hello\n");
	err = ion_init_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != ion_master_table_file);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, ion_master_table_next_id);

	/*************/

	/* Test create */
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dictionary;

	ffdict_init(&handler);
	err = ion_master_table_create_dictionary(&handler, &dictionary, key_type_numeric_signed, sizeof(int), 10, 20);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, ion_master_table_next_id);
	/***************/

	/* Test close */
	err = ion_close_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == ion_master_table_file);
	/**************/

	/* Test re-open */
	err = ion_init_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != ion_master_table_file);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, ion_master_table_next_id);

	/****************/

	/* Test lookup 1st dictionary */
	ion_dictionary_config_info_t config;

	err = ion_lookup_in_master_table(1, &config);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, config.id);
	PLANCK_UNIT_ASSERT_TRUE(tc, key_type_numeric_signed == config.type);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(int), config.key_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 10, config.value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 20, config.dictionary_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_type_flat_file_t, config.dictionary_type);

	/******************************/

	/* Test create 2nd dictionary */
	ion_dictionary_handler_t	handler2;
	ion_dictionary_t			dictionary2;

	ffdict_init(&handler2);
	err = ion_master_table_create_dictionary(&handler2, &dictionary2, key_type_numeric_signed, sizeof(short), 7, 14);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 3, ion_master_table_next_id);
	/******************************/

	/* Test 2nd lookup */
	err = ion_lookup_in_master_table(2, &config);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, config.id);
	PLANCK_UNIT_ASSERT_TRUE(tc, key_type_numeric_signed == config.type);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(short), config.key_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 7, config.value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 14, config.dictionary_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_type_flat_file_t, config.dictionary_type);
	/*******************/

	/* Test delete */
	err = ion_delete_dictionary(&dictionary, dictionary.instance->id);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	/***************/

	/* Test lookup on non-existent row */
	err = ion_lookup_in_master_table(1, &config);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_item_not_found, err);
	/***********************************/

	/* Test close dictionary */

	err = ion_close_dictionary(&dictionary2);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);

	/* Test open dictionary */

	err = ion_open_dictionary(&handler2, &dictionary2, 2);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, dictionary2.instance->id);
	PLANCK_UNIT_ASSERT_TRUE(tc, key_type_numeric_signed == dictionary2.instance->key_type);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(short), dictionary2.instance->record.key_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 7, dictionary2.instance->record.value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_type_flat_file_t, dictionary2.instance->type);

	/* Test close dictionary */

	err = ion_close_dictionary(&dictionary2);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);

	/* Test delete closed dictionary */

	err = ion_delete_dictionary(&dictionary2, 2);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);

	/* Test close master table */

	err = ion_close_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == ion_master_table_file);

	/* Test delete master table */

	err = ion_delete_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	/**************/
}

planck_unit_suite_t *
dictionary_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_dictionary_compare_numerics);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_dictionary_master_table);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_dictionary_master_table2);

	return suite;
}

void
runalltests_dictionary(
) {
	planck_unit_suite_t *suite = dictionary_getsuite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
