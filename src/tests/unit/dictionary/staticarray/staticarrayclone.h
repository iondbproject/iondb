/******************************************************************************/
/**
@file	 staticarrayclone.h
@author	 Raffi Kudlac
@brief	 ###BRIEF HERE###
*/
/******************************************************************************/
#ifndef STATICARRAYCLONE_H_
#define STATICARRAYCLONE_H_

#include "./../../../../dictionary/staticarray/sahandler.h"
#include "./../../../CuTest.h"
#include "./../../../../dictionary/staticarray/staticarray.h"

#include "./../../../../dictionary/dicttypes.h"
#include "./../../../../dictionary/dictionary.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "./../../../../io.h"

	void create_tests(CuSuite *suite);
	void test_create(CuTest *tc);
	void test_create_invalid_key_size(CuTest *tc);
	void test_create_max_size(CuTest *tc);
	void test_create_invalid_value(CuTest *tc);


	void insert_tests(CuSuite *suite);
	void test_insert_valid_params(CuTest *tc);
	void test_insert_invalid_index(CuTest *tc);
	void test_insert_collision(CuTest *tc);

	void delete_tests(CuSuite *suite);
	void test_delete_valid_case(CuTest *tc);
	void test_delete_empty_slot(CuTest *tc);
	void test_delete_invalid_keysize(CuTest *tc);

	void get_tests(CuSuite *suite);
	void test_get_valid_case(CuTest *tc);
	void test_get_invalid_key(CuTest *tc);
	void test_get_from_empty_location(CuTest *tc);

	void update_tests(CuSuite *suite);
	void test_update_valid_case(CuTest *tc);
	void test_update_invalid_key(CuTest *tc);
	void test_update_empty_location(CuTest *tc);

	void destroy_tests(CuSuite *suite);
	void test_destroy(CuTest *tc);

	void test_key_to_index(CuTest *tc);
	void show_for_raymon(CuTest *tc);

	void runalltests_open_address_hash_handler();
	void initalize_hash_handler_tests(CuTest *tc);
	void create_hash_handler_tests(CuTest *tc);
	void destroy_hash_handler_tests(CuTest *tc);

	void test_valid_find(CuTest *tc);
	void test_blank_find(CuTest *tc);
	void test_invalid_key_equal_find(CuTest *tc);

	void test_valid_range_find(CuTest *tc);
	void test_one_range_find(CuTest *tc);
	void test_full_valid_range_find(CuTest *tc);
	void test_blank_valid_range_find(CuTest *tc);

	void test_invalid_lower_bound_find(CuTest *tc);
	void test_invalid_upper_bound_find(CuTest *tc);
	void test_invalid_bounds_range_find(CuTest *tc);

#endif /* STATICARRAYCLONE_H_ */
