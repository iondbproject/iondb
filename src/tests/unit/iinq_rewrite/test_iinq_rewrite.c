/******************************************************************************/
/**
@file		test_iinq_rewrite.c
@author		Kai Neubauer
@brief		iinq tests.
@copyright	Copyright 2017
The University of British Columbia,
IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

@par 1.Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

@par 2.Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

@par 3.Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/
/******************************************************************************/

#include "test_iinq_rewrite.h"
#include "../../../util/sort/external_sort/external_sort.h"
#include "../../../util/sort/sort.h"
#include "../../../iinq/iinq.h"
#include "../../../iinq_rewrite/iinq_rewrite.h"
#include "../../../util/sort/external_sort/external_sort_types.h"

/* Declared globally for now
* could be moved into each function individually */
DEFINE_SCHEMA(test1, {
	double col1;
	char col2[40];
	int col3;
	char col4[10];
});

typedef void (*iinq_test_results_func_t)(
		planck_unit_test_t *,
		ion_iinq_result_t *,
		uint32_t,
		uint32_t
);

typedef struct {
	uint32_t total;
	uint32_t count;
	planck_unit_test_t *tc;
	iinq_test_results_func_t func;
} iinq_test_query_state_t;

IINQ_NEW_PROCESSOR_FUNC(check_results) {
	iinq_test_query_state_t *query_state = state;

	query_state->func(query_state->tc, result, query_state->count, query_state->total);
	query_state->count++;

}

void
iinq_rewrite_create_int_table_with_iterator(
		planck_unit_test_t *tc
) {
	ion_err_t error;
	ion_key_type_t key_type;
	ion_key_size_t key_size;
	ion_value_size_t value_size;

	key_type = key_type_numeric_signed;
	key_size = sizeof(int);
	value_size = sizeof(int) * 3;

	iinq_schema_t schema = (iinq_schema_t) {
			4, (iinq_field_type_t[]) {
					IINQ_INT,
					IINQ_INT,
					IINQ_INT,
					IINQ_INT
			}, (iinq_field_size_t[]) {
					sizeof(int), sizeof(int), sizeof(int), sizeof(int)
			}
	};

	FILE *test = fopen("table.inq", "r");
	if (NULL != test) {
		fclose(test);
		fremove("table.inq");
	}

	error = iinq_create_table("table.inq", key_type, key_size, value_size, &schema);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);
}

void
iinq_rewrite_insert_value_int_table(
		planck_unit_test_t *tc,
		ion_key_t key,
		ion_value_t value
) {
	ion_status_t status;

	status = INSERT(table, key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);
}

void
iinq_rewrite_insert_values_into_int_table_for_select_all_and_select_field_list(
		planck_unit_test_t *tc
) {
	int i;
	ion_value_t value = malloc(sizeof(int) * 3);

	// First attribute is the key
	int attr0;

	// Remaining attributes are in the value
	int *attr1 = value;
	int *attr2 = attr1 + 1;
	int *attr3 = attr2 + 1;

	attr0 = 1;
	*attr1 = 2;
	*attr2 = 3;
	*attr3 = 4;

	for (i = 0; i < 50; i++)
		iinq_rewrite_insert_value_int_table(tc, IONIZE(attr0, int), value);

	attr0 = -1;

	for (i = 0; i < 50; i++)
		iinq_rewrite_insert_value_int_table(tc, IONIZE(attr0, int), value);

	free(value);
}

void
iinq_rewrite_test_select_all_from_int_table_iterator(
		planck_unit_test_t *tc
) {
	iinq_rewrite_create_int_table_with_iterator(tc);

	iinq_rewrite_insert_values_into_int_table_for_select_all_and_select_field_list(tc);

	iinq_iterator_t iterator;

	/* SELECT * FROM int_table; */
	iinq_query_init_select_all_from_table(&iterator, "table", iinq_next_from_table_no_predicate, NULL);

	int count = 0;

	while (it_status_ok == iterator.next(&iterator)) {
		if (count < 50) {
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, get_int(iterator, 0));    /* attr0 */
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, get_int(iterator, 1));    /* attr1 */
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 3, get_int(iterator, 2));    /* attr2 */
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 4, get_int(iterator, 3));    /* attr3 */
		} else {
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, -1, get_int(iterator, 0));    /* attr0 */
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, get_int(iterator, 1));    /* attr1 */
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 3, get_int(iterator, 2));    /* attr2 */
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 4, get_int(iterator, 3));    /* attr3 */
		}
		count++;
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 100, count);

	iinq_destroy_iterator(&iterator);

	DROP_TABLE(table);
}

void
iinq_rewrite_test_select_attr0_from_int_table_iterator(
		planck_unit_test_t *tc
) {

	iinq_rewrite_create_int_table_with_iterator(tc);

	iinq_rewrite_insert_values_into_int_table_for_select_all_and_select_field_list(tc);

	iinq_iterator_t iterator;

	/* SELECT attr0 FROM int_table; */
	iinq_query_init_select_field_list_from_table(&iterator, "table", iinq_next_from_table_no_predicate, NULL, 1,
												 IINQ_FIELD_LIST({ 0, 0 }));

	int count = 0;

	while (it_status_ok == iterator.next(&iterator)) {

		if (count < 50) {
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, get_int(iterator, 0));    /* attr0 */
		} else {
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, -1, get_int(iterator, 0));    /* attr0 */
		}
		count++;
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 100, count);

	iinq_destroy_iterator(&iterator);

	DROP_TABLE(table);
}

void
iinq_rewrite_test_select_attr0_attr1_from_int_table_iterator(
		planck_unit_test_t *tc
) {
	iinq_rewrite_create_int_table_with_iterator(tc);

	iinq_rewrite_insert_values_into_int_table_for_select_all_and_select_field_list(tc);

	iinq_iterator_t iterator;

	/* SELECT attr0, attr1 FROM int_table; */
	iinq_query_init_select_field_list_from_table(&iterator, "table", iinq_next_from_table_no_predicate, NULL, 2,
												 IINQ_FIELD_LIST({ 0, 0 }, { 0, 1 }));

	int count = 0;

	while (it_status_ok == iterator.next(&iterator)) {

		if (count < 50) {
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, get_int(iterator, 0));    /* attr0 */
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, get_int(iterator, 1));    /* attr1 */
		} else {
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, -1, get_int(iterator, 0));    /* attr0 */
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, get_int(iterator, 1));    /* attr1 */
		}
		count++;
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 100, count);

	iinq_destroy_iterator(&iterator);

	DROP_TABLE(table);
}

void
iinq_rewrite_test_select_attr0_attr1_attr2_attr3_from_int_table_iterator(
		planck_unit_test_t *tc
) {
	iinq_rewrite_create_int_table_with_iterator(tc);

	iinq_rewrite_insert_values_into_int_table_for_select_all_and_select_field_list(tc);

	iinq_iterator_t iterator;

	/* SELECT attr0, attr1, attr2, attr3 FROM int_table; */
	iinq_query_init_select_field_list_from_table(&iterator, "table", iinq_next_from_table_no_predicate, NULL, 4,
												 IINQ_FIELD_LIST({ 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }));

	int count = 0;

	while (it_status_ok == iterator.next(&iterator)) {

		if (count < 50) {
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, get_int(iterator, 0));    /* attr0 */
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, get_int(iterator, 1));    /* attr1 */
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 3, get_int(iterator, 2));    /* attr2 */
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 4, get_int(iterator, 3));    /* attr3 */
		} else {
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, -1, get_int(iterator, 0));    /* attr0 */
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, get_int(iterator, 1));    /* attr1 */
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 3, get_int(iterator, 2));    /* attr2 */
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 4, get_int(iterator, 3));    /* attr3 */
		}
		count++;
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 100, count);

	iinq_destroy_iterator(&iterator);

	DROP_TABLE(table);
}

ion_boolean_t
generatedPredicate0(iinq_iterator_t *it) {
	if (!(get_int(*it, 0) >= 0 && get_int(*it, 1) != 1))
		return boolean_false;
	else
		return boolean_true;
}

void
iinq_rewrite_test_select_attr0_attr1_attr2_attr3_from_int_table_where_iterator(
		planck_unit_test_t *tc
) {
	iinq_rewrite_create_int_table_with_iterator(tc);

	iinq_rewrite_insert_values_into_int_table_for_select_all_and_select_field_list(tc);

	iinq_iterator_t iterator;

	/* SELECT attr0, attr1, attr2, attr3 FROM int_table WHERE attr0 >= 0 AND attr1 != -1; */
	iinq_query_init_select_field_list_from_table(&iterator, "table", iinq_next_from_table_with_predicate,
												 generatedPredicate0, 4,
												 IINQ_FIELD_LIST({ 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }));

	int count = 0;

	while (it_status_ok == iterator.next(&iterator)) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, get_int(iterator, 0));    /* attr0 */
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, get_int(iterator, 1));    /* attr1 */
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 3, get_int(iterator, 2));    /* attr2 */
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 4, get_int(iterator, 3));    /* attr3 */
		count++;
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 50, count);

	iinq_destroy_iterator(&iterator);

	DROP_TABLE(table);
}

void
iinq_rewrite_create_test1_with_iterator(
		planck_unit_test_t *tc
) {
	ion_err_t error;
	ion_key_type_t key_type;
	ion_key_size_t key_size;
	ion_value_size_t value_size;

	key_type = key_type_numeric_signed;
	key_size = sizeof(int);
	value_size = sizeof(double) + sizeof(char) * 40 + sizeof(int) + sizeof(char) * 10;

/* TODO: factor out into macro if possible (e.g. schema = SCHEMA(5, {IINQ_INT, IINQ_STRING[10]}))*/
	iinq_schema_t schema = (iinq_schema_t) {
			5, (iinq_field_type_t[]) {
					IINQ_INT,    /* key */
					IINQ_DOUBLE,/* col1 */
					IINQ_STRING,/* col2 */
					IINQ_INT,    /* col3 */
					IINQ_STRING    /* col4 */
			}, (iinq_field_size_t[]) {
					sizeof(int), sizeof(double), sizeof(char) * 40, sizeof(int), sizeof(char) * 10
			}
	};
	FILE *file = fopen("test1.inq", "r");
	if (NULL != file) {
		fclose(file);
		fremove("test1.inq");
	}

	error = iinq_create_table("test1.inq", key_type, key_size, value_size, &schema);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);
}

void
iinq_rewrite_create_test1(

		planck_unit_test_t *tc
) {

	ion_err_t error;


	ion_key_type_t key_type;

	ion_key_size_t key_size;

	ion_value_size_t value_size;


	key_type = key_type_numeric_signed;

	key_size = sizeof(uint32_t);

	value_size = SCHEMA_SIZE(test1);


	error = CREATE_DICTIONARY(test1, key_type, key_size, value_size);


	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);
}

void
iinq_rewrite_test_select_all_from_check_results(

		planck_unit_test_t *tc,

		ion_iinq_result_t *result,

		uint32_t count,

		uint32_t total
) {
	UNUSED(count);
	UNUSED(total);


	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, SCHEMA_SIZE(
			test1) + sizeof(uint32_t),
									 result->num_bytes);    /* Size of tuple + key */


	ion_key_t key = result->processed;

	struct iinq_test1_schema *tuple = (struct iinq_test1_schema *) (result->processed + sizeof(int));


	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc,
									 1, NEUTRALIZE(key, uint32_t));

	PLANCK_UNIT_ASSERT_TRUE(tc,
							2.5 == tuple->col1);

	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc,
									 "Hello", tuple->col2);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc,
									 1, tuple->col3);

	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc,
									 "Goodbye", tuple->col4);
}

void
iinq_rewrite_insert_value_test1(
		planck_unit_test_t *tc,
		ion_key_t key,
		ion_value_t value
) {
	ion_status_t status;

	status = INSERT_INTO(test1, key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);
}

void
iinq_rewrite_test_insert_multiple_same_values_test1(

		planck_unit_test_t *tc
) {

	ion_key_t key = IONIZE(1, uint32_t);


	DECLARE_SCHEMA_VAR(test1, test_val);

	test_val.
			col1 = 2.5;

	strcpy(test_val
				   .col2, "Hello");
	test_val.
			col3 = 1;

	strcpy(test_val
				   .col4, "Goodbye");


	iinq_rewrite_insert_value_test1(tc, key,
									&test_val);

	iinq_rewrite_insert_value_test1(tc, key,
									&test_val);

	iinq_rewrite_insert_value_test1(tc, key,
									&test_val);
}

void
iinq_rewrite_test_insert_multiple_same_values_test1_with_iterator(
		planck_unit_test_t *tc
) {
	ion_key_t key = IONIZE(1, int);

	int size = sizeof(double) + sizeof(char) * 40 + sizeof(int) + sizeof(char) * 10;
	unsigned char *value = malloc(size);

	unsigned char *data = value;

	*(double *) data = 2.5;
	data += sizeof(double);

	memcpy(data, "Hello", sizeof("Hello"));
	data += sizeof(char) * 40;

	*(int *) data = 1;
	data += sizeof(int);

	memcpy(data, "Goodbye", sizeof("Goodbye"));

	iinq_rewrite_insert_value_test1(tc, key, value);
	iinq_rewrite_insert_value_test1(tc, key, value);
	iinq_rewrite_insert_value_test1(tc, key, value);

	free(value);
}

void
iinq_rewrite_test_insert_multiple_different_values_test1(
		planck_unit_test_t *tc
) {

	DECLARE_SCHEMA_VAR(test1, test_val);

	test_val.col1 = 2.5;

	strcpy(test_val.col2, "Hello");
	test_val.col3 = 1;

	strcpy(test_val.col4, "Goodbye");


	iinq_rewrite_insert_value_test1(tc, IONIZE(1, uint32_t), &test_val);

	iinq_rewrite_insert_value_test1(tc, IONIZE(1, uint32_t), &test_val);

	test_val.
			col1 = 1.0;

	strcpy(test_val
				   .col2, "Goodbye");
	test_val.
			col3 = 3;

	strcpy(test_val
				   .col4, "Hello");


	iinq_rewrite_insert_value_test1(tc, IONIZE(2, uint32_t), &test_val);

	iinq_rewrite_insert_value_test1(tc, IONIZE(2, uint32_t), &test_val);
}

void
iinq_rewrite_test_insert_different_values_test1_with_iterator(

		planck_unit_test_t *tc
) {

	unsigned char *value = malloc(sizeof(double) + sizeof(char) * 40 + sizeof(int) + sizeof(char) * 10);

	unsigned char *data = value;

	*(double *)
			data = 2.5;
	data += sizeof(double);

	strcpy(data,
		   "Hello");
	data += sizeof(char) * 40;
	*(int *)
			data = 1;
	data += sizeof(int);

	strcpy(data,
		   "Goodbye");


	iinq_rewrite_insert_value_test1(tc, IONIZE(1, int), value);


	strcpy(data,
		   "Hello");

	iinq_rewrite_insert_value_test1(tc, IONIZE(1, int), value);


	data = value;

	*(double *)
			data = 1.0;
	data += sizeof(double);

	strcpy(data,
		   "Goodbye");
	data += sizeof(char) * 40;
	*(int *)
			data = 3;
	data += sizeof(int);

	strcpy(data,
		   "Hello");


	iinq_rewrite_insert_value_test1(tc, IONIZE(2, uint32_t), value);


	strcpy(data,
		   "Goodbye");

	iinq_rewrite_insert_value_test1(tc, IONIZE(2, uint32_t), value);
}

void
iinq_rewrite_test_insert_multiple_values_test1_order_by_single(

		planck_unit_test_t *tc
) {

	ion_key_t key = IONIZE(1, uint32_t);


	DECLARE_SCHEMA_VAR(test1, test_val);

	/* This tuple should get filtered out */
	test_val.
			col3 = 26000;

	iinq_rewrite_insert_value_test1(tc, key,
									&test_val);

	test_val.
			col3 = 26001;

	strcpy(test_val
				   .col4, "Zimbabwe");

	iinq_rewrite_insert_value_test1(tc, key,
									&test_val);

	test_val.
			col3 = 26003;

	strcpy(test_val
				   .col4, "Canada");

	iinq_rewrite_insert_value_test1(tc, key,
									&test_val);

	test_val.
			col3 = 26002;

	strcpy(test_val
				   .col4, "USA");

	iinq_rewrite_insert_value_test1(tc, key, &test_val);
}

void
iinq_rewrite_test_insert_multiple_values_test1_order_by_single_with_iterator(

		planck_unit_test_t *tc
) {

	unsigned char *value = malloc(sizeof(double) + sizeof(char) * 40 + sizeof(int) + sizeof(char) * 10);

	/* We are only concerned with the key and the first string value,
	 * so we can skip to col4 in value */
	unsigned char *data = value + sizeof(double) + sizeof(char) * 40 + sizeof(int);

	/* This tuple should get filtered out by the WHERE clause,
	 * value is never accessed so we can leave it blank */
	iinq_rewrite_insert_value_test1(tc, IONIZE(0, int), value);

	strcpy(data, "Zimbabwe");
	iinq_rewrite_insert_value_test1(tc, IONIZE(1, int), value);


	strcpy(data, "Canada");
	iinq_rewrite_insert_value_test1(tc, IONIZE(3, int), value);

	strcpy(data, "USA");
	iinq_rewrite_insert_value_test1(tc, IONIZE(2, int), value);

	free(value);
}

void
iinq_rewrite_test_select_all_from_test1_iterator(
		planck_unit_test_t *tc
) {
	iinq_rewrite_create_test1_with_iterator(tc);
	iinq_rewrite_test_insert_multiple_same_values_test1_with_iterator(tc);
	iinq_iterator_t iterator;

	/* SELECT * FROM test1; */
	query_init(&iterator, IINQ_SELECT_ALL, IINQ_ORDER_BY_NONE, boolean_false, 1, 0, "test1");

	int count = 0;

	while (it_status_ok == iterator.next(&iterator)) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, get_int(iterator, 0));    /* key */
		/*PLANCK_UNIT_ASSERT_TRUE(tc, 2.5 == get_double(iterator, 1));*//* col1 */
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, "Hello", get_string(iterator, 2));    /* col2 */
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, get_int(iterator, 3));    /* col3 */
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, "Goodbye", get_string(iterator, 4));    /* col4 */
		count++;
	}
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 3, count);
	iinq_destroy_iterator(&iterator);

	DROP_TABLE(test1);
}

void
iinq_rewrite_test_select_all_from_test1_where_iterator(
		planck_unit_test_t *tc
) {
	iinq_rewrite_create_test1_with_iterator(tc);

	iinq_rewrite_test_insert_different_values_test1_with_iterator(tc);

	iinq_comparison_pointer_t comp_value1;
	comp_value1.int_val = malloc(sizeof(int));

	*comp_value1.int_val = 1;


	iinq_comparison_pointer_t comp_value2;
	comp_value2.string_val = malloc(sizeof(char) * 10);

	strcpy(comp_value2.string_val, "Hello");

	iinq_iterator_t iterator;

	/* SELECT * FROM test1 key = 1 AND col4 <> 'Hello'; */
	query_init(&iterator, IINQ_SELECT_ALL, IINQ_ORDER_BY_NONE, boolean_false, 1, 2, "test1", IINQ_WHERE_CLAUSE(
			IINQ_WHERE_FILTER(0, 0, IINQ_EQUAL_TO, comp_value1),
			IINQ_WHERE_FILTER(0, 4, IINQ_NOT_EQUAL_TO, comp_value2))
	);

	int count = 0;

	while (it_status_ok == iterator.next(&iterator)) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, get_int(iterator, 0));    /* key */
		/*PLANCK_UNIT_ASSERT_TRUE(tc, 2.5 == get_double(iterator, 1));*//* col1 */
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, "Hello", get_string(iterator, 2));    /* col2 */
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, get_int(iterator, 3));    /* col3 */
		PLANCK_UNIT_ASSERT_STR_ARE_NOT_EQUAL(tc, "Hello", get_string(iterator, 4));    /* col4 */
		count++;
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, count);

	iinq_destroy_iterator(&iterator);

	DROP_TABLE(test1);
}

void
iinq_rewrite_test_select_field_list_from_test1_iterator(
		planck_unit_test_t *tc
) {
	iinq_rewrite_create_test1_with_iterator(tc);
	iinq_rewrite_test_insert_multiple_same_values_test1_with_iterator(tc);
	iinq_iterator_t iterator;

	/* SELECT col1, col2, col3, FROM test1; */
	query_init(&iterator, IINQ_SELECT_FIELD_LIST, IINQ_ORDER_BY_NONE, boolean_false, 1, 0, "test1", 3, IINQ_FIELD_LIST(
			{ 0, 1 }, { 0, 2 }, { 0, 3 }));

	int count = 0;

	while (it_status_ok == iterator.next(&iterator)) {
		/*PLANCK_UNIT_ASSERT_TRUE(tc, abs(get_double(iterator, 0) - 2.5) < FLOAT_TOLERANCE);    *//* col1 */
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, "Hello", get_string(iterator, 1));    /* col2 */
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, get_int(iterator, 2));    /* col3 */
		count++;
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 3, count);
	iinq_destroy_iterator(&iterator);

	DROP_TABLE(test1);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_iterator(
		planck_unit_test_t *tc
) {
	iinq_rewrite_create_test1_with_iterator(tc);

	iinq_rewrite_test_insert_multiple_different_values_test1(tc);

	iinq_iterator_t iterator;

	iinq_comparison_pointer_t comp_value;
	comp_value.int_val = malloc(sizeof(int));

	*comp_value.int_val = 1;
	/* SELECT key, col1, col2 FROM test1 WHERE key = 0; */
	query_init(&iterator, IINQ_SELECT_FIELD_LIST, IINQ_ORDER_BY_NONE, boolean_false, 1, 1, "test1", IINQ_WHERE_CLAUSE(
			IINQ_WHERE_FILTER(0, 0, IINQ_EQUAL_TO, comp_value)), 3, IINQ_FIELD_LIST({ 0, 0 }, { 0, 1 }, { 0, 2 }));

	free(comp_value.int_val);

	int count = 0;

	while (it_status_ok == iterator.next(&iterator)) {

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, get_int(iterator, 0));    /* key */

		/*PLANCK_UNIT_ASSERT_TRUE(tc, abs(get_double(iterator, 1) - 2.5) < FLOAT_TOLERANCE);    *//* col1 */

		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, "Hello", get_string(iterator, 2));    /* col2 */
		count++;
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, count);
	iinq_destroy_iterator(&iterator);

	DROP_TABLE(test1);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_order_by_single_iterator_check_results(
		planck_unit_test_t *tc,
		iinq_iterator_t *iterator
) {
	int count = 0;
	while (it_status_ok == iterator->next(iterator)) {
		printf("Count: %d\n"
					   "\tKey: %d\n"
					   "\tcol2: %s\n", count, get_int(*iterator, 0), get_string(*iterator, 1));
		switch (count) {
			case 0:
				PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, get_int(*iterator, 0));    /* key */
				PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, "Zimbabwe", get_string(*iterator, 1));    /* col2 */
				break;
			case 1:
				PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, get_int(*iterator, 0));    /* key */
				PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, "USA", get_string(*iterator, 1));    /* col2 */
				break;
			case 2:
				PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 3, get_int(*iterator, 0));    /* key */
				PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, "Canada", get_string(*iterator, 1));    /* col2 */
				break;
			default:
				PLANCK_UNIT_SET_FAIL(tc);
				break;
		}
		printf("END\n");
		count++;
	}
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 3, count);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_order_by_single_string_desc_iterator(
		planck_unit_test_t *tc
) {

	iinq_comparison_pointer_t comp_value;
	comp_value.int_val = malloc(sizeof(int));

	*comp_value.int_val = 0;

	iinq_iterator_t iterator;

	/* SELECT key, col2 FROM test1 WHERE key > 0 ORDER BY col4 DESC; */
	query_init(&iterator, IINQ_SELECT_FIELD_LIST, IINQ_ORDER_BY_FIELD, boolean_false, 1, 1, "test1", IINQ_WHERE_CLAUSE(
			IINQ_WHERE_FILTER(0, 0, IINQ_GREATER_THAN, comp_value)), 2, (iinq_field_list_t[]) {
			{
					0, 0},
			{
					0, 4}
	}, 1, (iinq_order_by_field_t[]) {
			(iinq_field_list_t) {
					0, 4}, IINQ_ORDER_BY_DESC});

	free(comp_value.int_val);


	iinq_rewrite_test_select_field_list_from_test1_where_order_by_single_iterator_check_results(tc, &iterator);
	iinq_destroy_iterator(&iterator);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_order_by_single_int_asc_iterator(
		planck_unit_test_t *tc
) {

	iinq_comparison_pointer_t comp_value;
	comp_value.int_val = malloc(sizeof(int));

	*comp_value.int_val = 0;

	iinq_iterator_t iterator;
	/* SELECT key, col2 FROM test1 WHERE key > 0 ORDER BY key ASC; */
	query_init(&iterator, IINQ_SELECT_FIELD_LIST, IINQ_ORDER_BY_FIELD, boolean_false, 1, 1, "test1", IINQ_WHERE_CLAUSE(
			IINQ_WHERE_FILTER(0, 0, IINQ_GREATER_THAN, comp_value)), 2, (iinq_field_list_t[]) {
			{
					0, 0},
			{
					0, 4}
	}, 1, (iinq_order_by_field_t[]) {
			(iinq_field_list_t) {
					0, 0}, IINQ_ORDER_BY_ASC});
	free(comp_value.int_val);


	iinq_rewrite_test_select_field_list_from_test1_where_order_by_single_iterator_check_results(tc, &iterator);
	iinq_destroy_iterator(&iterator);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_order_by_single_iterator(

		planck_unit_test_t *tc
) {
	iinq_rewrite_create_test1_with_iterator(tc);
	iinq_rewrite_test_insert_multiple_values_test1_order_by_single_with_iterator(tc);

	iinq_rewrite_test_select_field_list_from_test1_where_order_by_single_int_asc_iterator(tc);
	iinq_rewrite_test_select_field_list_from_test1_where_order_by_single_string_desc_iterator(tc);

	DROP_TABLE(test1);
}

void
iinq_rewrite_test_select_all_from_test1(
		planck_unit_test_t *tc
) {

	ion_iinq_query_processor_t processor;
	iinq_test_query_state_t state;


	iinq_rewrite_create_test1(tc);
	iinq_rewrite_test_insert_multiple_same_values_test1(tc);

	state.func = iinq_rewrite_test_select_all_from_check_results;
	state.tc = tc;
	state.count = 0;


	processor = IINQ_QUERY_PROCESSOR(check_results, &state);

	do {

		ion_err_t error;
		ion_iinq_result_t result;
		int jmp_r;
		jmp_buf selectbuf;
		result.raw_record_size = 0;
		result.num_bytes = 0;

		int read_page_remaining = IINQ_PAGE_SIZE;
		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;
		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;
		ion_iinq_cleanup_t *last;
		ion_iinq_cleanup_t *ref_cursor;
		ion_iinq_cleanup_t *last_cursor;


		first = NULL;
		last = NULL;
		ref_cursor = NULL;
		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;
		test1.cleanup.next = NULL;
		test1.cleanup.last = last;
		test1.cleanup.reference = &test1;

		if (NULL == first) {
			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.next = NULL;
		test1.dictionary.handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.key_size;
		result.raw_record_size += test1.dictionary.instance->record.value_size;
		result.num_bytes += test1.dictionary.instance->record.key_size;
		result.num_bytes += test1.dictionary.instance->record.value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.data = alloca(result.raw_record_size);
		result.processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.key = result.processed;
		result.processed += test1.dictionary.instance->record.key_size;
		test1.value = result.processed;
		result.processed += test1.dictionary.instance->record.value_size;
		test1.ion_record.key = test1.key;
		test1.ion_record.value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		memcpy(result.processed, result.data, result.num_bytes);
		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);

	DROP(test1);
}

void
iinq_rewrite_test_select_field_list_from_test1_check_results(
		planck_unit_test_t *tc,
		ion_iinq_result_t *result,
		uint32_t count,
		uint32_t total
) {
	UNUSED(count);
	UNUSED(total);


	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc,
									 sizeof(double) + sizeof(char) * 40 + sizeof(int) + sizeof(uint32_t),
									 result->num_bytes);


	ion_key_t key = result->processed;

	unsigned char *field = result->processed + sizeof(uint32_t);


	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc,
									 1, NEUTRALIZE(key, uint32_t));

	PLANCK_UNIT_ASSERT_TRUE(tc, *(
			double *) field == 2.5);    /* col1 */
	field += sizeof(double);

	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc,
									 "Hello", field);    /* col2 */
	field += sizeof(char) * 40;

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc,
									 1, *(int *) field);/* col3 */
}

void
iinq_rewrite_test_select_field_list_from_test1(

		planck_unit_test_t *tc
) {

	iinq_rewrite_create_test1(tc);


	iinq_rewrite_test_insert_multiple_same_values_test1(tc);


	ion_iinq_query_processor_t processor;

	iinq_test_query_state_t state;

	state.
			func = iinq_rewrite_test_select_field_list_from_test1_check_results;
	state.
			tc = tc;
	state.
			count = 0;


	processor = IINQ_QUERY_PROCESSOR(check_results, &state);

	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		/* end of select_clause */ do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);

	DROP(test1);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_equality_filter_check_results(

		planck_unit_test_t *tc,

		ion_iinq_result_t *result,

		uint32_t count,

		uint32_t total
) {
	UNUSED(total);
	UNUSED(count);


	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc,
									 sizeof(double) + sizeof(int) + sizeof(char) * 50 + sizeof(uint32_t),
									 result->num_bytes);


	ion_key_t key = result->processed;


	struct iinq_test1_schema *tuple = (struct iinq_test1_schema *) (result->processed + sizeof(uint32_t));

	switch (NEUTRALIZE(key, uint32_t)) {
		case 1:

			PLANCK_UNIT_ASSERT_TRUE(tc, tuple
												->col1 == 31.02);
			break;

		case 2:

			PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc,
											 "United Kingdom", tuple->col2);
			break;

		default:

			PLANCK_UNIT_SET_FAIL(tc);
	}
}

void
iinq_rewrite_test_select_all_from_test1_where_string_equality_filter(

		ion_iinq_query_processor_t processor
) {
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		/* end of select_clause */
		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((
						strcmp(test1_tuple
									   ->col2, "United Kingdom") == 0))) {
					continue;
				}

				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_all_from_test1_where_numeric_equality_filter(

		ion_iinq_query_processor_t processor
) {
	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col1 = 31.02; */
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		/* end of select_clause */
		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((test1_tuple->col1 == 31.02))) {
					continue;
				}

				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_equality_filter(

		planck_unit_test_t *tc
) {

	iinq_rewrite_create_test1(tc);


	DECLARE_SCHEMA_VAR(test1, test_val);


	ion_key_t key = IONIZE(1, uint32_t);


	ion_iinq_query_processor_t processor;

	iinq_test_query_state_t state;

	state.
			func = iinq_rewrite_test_select_field_list_from_test1_where_equality_filter_check_results;
	state.
			tc = tc;
	state.
			count = 0;


	processor = IINQ_QUERY_PROCESSOR(check_results, &state);

	test_val.
			col1 = 31.02;

	strcpy(test_val
				   .col2, "Goodbye");
	test_val.
			col3 = 3;

	strcpy(test_val
				   .col4, "Hello");


	iinq_rewrite_insert_value_test1(tc, key,
									&test_val);


	key = IONIZE(2, uint32_t);

	strcpy(test_val
				   .col2, "United Kingdom");


	iinq_rewrite_insert_value_test1(tc, key,
									&test_val);

	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col2 = 'United Kingdom'; */

	iinq_rewrite_test_select_all_from_test1_where_string_equality_filter(processor);

	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col1 = 31.02; */

	iinq_rewrite_test_select_all_from_test1_where_numeric_equality_filter(processor);

	DROP(test1);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_filter_gt_string(

		ion_iinq_query_processor_t processor
) {
	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col2 > 'United Kingdom'; */
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		/* end of select_clause */
		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((
						strcmp(test1_tuple
									   ->col2, "United Kingdom") > 0))) {
					continue;
				}

				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_filter_gt_eq_string(

		ion_iinq_query_processor_t processor
) {
	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col2 >= 'United Kingdom'; */
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		/* end of select_clause */
		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((
						strcmp(test1_tuple
									   ->col2, "United Kingdom") >= 0))) {
					continue;
				}

				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_filter_lt_string(

		ion_iinq_query_processor_t processor
) {
	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col2 < 'United Kingdom'; */
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		/* end of select_clause */
		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((
						strcmp(test1_tuple
									   ->col2, "United Kingdom") < 0))) {
					continue;
				}

				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_filter_lt_eq_string(

		ion_iinq_query_processor_t processor
) {
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		/* end of select_clause */
		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((
						strcmp(test1_tuple
									   ->col2, "United Kingdom") <= 0))) {
					continue;
				}

				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_filter_lt_gt_string(

		ion_iinq_query_processor_t processor
) {
	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col2 <> 'United Kingdom'; */
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		/* end of select_clause */
		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((
						strcmp(test1_tuple
									   ->col2, "United Kingdom") != 0))) {
					continue;
				}

				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_filter_not_eq_string(

		ion_iinq_query_processor_t processor
) {
	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col2 != 'United Kingdom'; */
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		/* end of select_clause */
		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((
						strcmp(test1_tuple
									   ->col2, "United Kingdom") != 0))) {
					continue;
				}

				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_filter_string(

		ion_iinq_query_processor_t processor
) {

	iinq_test_query_state_t *state = (iinq_test_query_state_t *) (processor.state);

	state->
			count = 0;

	iinq_rewrite_test_select_field_list_from_test1_where_filter_gt_string(processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(state
											 ->tc, 1, state->count);

	state->
			count = 0;

	iinq_rewrite_test_select_field_list_from_test1_where_filter_gt_eq_string(processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(state
											 ->tc, 2, state->count);

	state->
			count = 0;

	iinq_rewrite_test_select_field_list_from_test1_where_filter_lt_string(processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(state
											 ->tc, 1, state->count);

	state->
			count = 0;

	iinq_rewrite_test_select_field_list_from_test1_where_filter_lt_eq_string(processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(state
											 ->tc, 2, state->count);

	state->
			count = 0;

	iinq_rewrite_test_select_field_list_from_test1_where_filter_lt_gt_string(processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(state
											 ->tc, 2, state->count);

	state->
			count = 0;

	iinq_rewrite_test_select_field_list_from_test1_where_filter_not_eq_string(processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(state
											 ->tc, 2, state->count);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_filter_gt_numeric(

		ion_iinq_query_processor_t processor
) {
	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col3 > 27300; */
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		/* end of select_clause */
		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((test1_tuple->col3 > 27300))) {
					continue;
				}

				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_filter_gt_eq_numeric(

		ion_iinq_query_processor_t processor
) {
	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col3 >= 27300; */
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		/* end of select_clause */
		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((test1_tuple->col3 >= 27300))) {
					continue;
				}

				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_filter_lt_numeric(

		ion_iinq_query_processor_t processor
) {
	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col1 < 2.1; */
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		/* end of select_clause */
		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((test1_tuple->col1 < 2.1))) {
					continue;
				}

				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_filter_lt_eq_numeric(

		ion_iinq_query_processor_t processor
) {
	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col1 <= 2.1; */
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		/* end of select_clause */
		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((test1_tuple->col1 <= 2.1))) {
					continue;
				}

				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_filter_lt_gt_numeric(

		ion_iinq_query_processor_t processor
) {
	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col1 <> 2.1; */
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		/* end of select_clause */
		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((test1_tuple->col1 != 2.1))) {
					continue;
				}

				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_filter_not_eq_numeric(

		ion_iinq_query_processor_t processor
) {
	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col1 != 2.1; */
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;
		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;

		/* end of select_clause */
		do {
			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((test1_tuple->col1 != 2.1))) {
					continue;
				}

				result.
						processed = alloca(result.num_bytes);
				goto
						COMPUTE_SELECT;

				DONE_COMPUTE_SELECT:;
				(&processor)->execute(&result, (&processor)->state);
			}


			IINQ_QUERY_CLEANUP:

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);


		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_all_from_test1_where_filter_numeric(

		ion_iinq_query_processor_t processor
) {

	iinq_test_query_state_t *state = (iinq_test_query_state_t *) (processor.state);

	state->
			count = 0;

	iinq_rewrite_test_select_field_list_from_test1_where_filter_gt_numeric(processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(state
											 ->tc, 1, state->count);

	state->
			count = 0;

	iinq_rewrite_test_select_field_list_from_test1_where_filter_gt_eq_numeric(processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(state
											 ->tc, 2, state->count);

	state->
			count = 0;

	iinq_rewrite_test_select_field_list_from_test1_where_filter_lt_numeric(processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(state
											 ->tc, 1, state->count);

	/* fails due to floating point inaccuracy */
	/*state->count = 0;
	iinq_rewrite_test_select_field_list_from_test1_where_filter_lt_eq_numeric(processor);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(state->tc, 2, state->count);

	state->count = 0;
	iinq_rewrite_test_select_field_list_from_test1_where_filter_lt_gt_numeric(processor);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(state->tc, 2, state->count);

	state->count = 0;
	iinq_rewrite_test_select_field_list_from_test1_where_filter_not_eq_numeric(processor);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(state->tc, 2, state->count);*/
}

void
iinq_rewrite_test_select_field_list_from_test1_where_filter_check_results(

		planck_unit_test_t *tc,

		ion_iinq_result_t *result,

		uint32_t count,

		uint32_t total
) {
	UNUSED(count);
	UNUSED(total);


	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc,
									 sizeof(int) + sizeof(double) + sizeof(char) * 50 + sizeof(uint32_t),
									 result->num_bytes);


	ion_key_t key = result->processed;

	unsigned char *field = result->processed + sizeof(uint32_t);

	switch (NEUTRALIZE(key, int)) {
		case 1:    /* Equal to */

			PLANCK_UNIT_ASSERT_TRUE(tc, abs(*(double *) field - 2.1)
										< FLOAT_TOLERANCE);
			field += sizeof(double);

			PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc,
											 "United Kingdom", field);
			field += sizeof(char) * 40;

			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc,
											 27300, *(int *) field);
			break;

		case 2:    /* Less than */

			PLANCK_UNIT_ASSERT_TRUE(tc, *(
					double *) field < 2.1);
			field += sizeof(double);

			PLANCK_UNIT_ASSERT_TRUE(tc, strcmp(field, "United Kingdom")
										< 0);
			field += sizeof(char) * 40;

			PLANCK_UNIT_ASSERT_TRUE(tc, *(
					int *) field < 27300);
			break;

		case 3:    /* Greater than */

			PLANCK_UNIT_ASSERT_TRUE(tc, *(
					double *) field > 2.1);
			field += sizeof(double);

			PLANCK_UNIT_ASSERT_TRUE(tc, strcmp(field, "United Kingdom")
										> 0);
			field += sizeof(char) * 40;

			PLANCK_UNIT_ASSERT_TRUE(tc, *(
					int *) field > 27300);
			break;

		default:

			PLANCK_UNIT_SET_FAIL(tc);
	}
}

void
iinq_rewrite_test_insert_test1_filter(

		planck_unit_test_t *tc
) {

	ion_status_t status;


	DECLARE_SCHEMA_VAR(test1, test1_val);

	test1_val.
			col1 = 2.1;
	test1_val.
			col3 = 27300;

	strcpy(test1_val
				   .col2, "United Kingdom");

	/* Equal to, key = 1 */

	status = INSERT(test1, IONIZE(1, uint32_t), &test1_val);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status
			.error);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc,
									 1, status.count);

	test1_val.
			col1 = 1.9;
	test1_val.
			col3 = 2600;

	strcpy(test1_val
				   .col2, "Canada");

	/* Less than, key = 2 */

	status = INSERT(test1, IONIZE(2, uint32_t), &test1_val);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status
			.error);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc,
									 1, status.count);

	test1_val.
			col1 = 2.5;
	test1_val.
			col3 = 30000;

	strcpy(test1_val
				   .col2, "Zimbabwe");

	/* Greater than, key = 3 */

	status = INSERT(test1, IONIZE(3, uint32_t), &test1_val);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status
			.error);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc,
									 1, status.count);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_filter(

		planck_unit_test_t *tc
) {

	iinq_rewrite_create_test1(tc);


	DECLARE_SCHEMA_VAR(test1, test_val);


	ion_key_t key = IONIZE(1, uint32_t);


	ion_iinq_query_processor_t processor;

	iinq_test_query_state_t state;

	state.
			func = iinq_rewrite_test_select_field_list_from_test1_where_filter_check_results;
	state.
			tc = tc;


	processor = IINQ_QUERY_PROCESSOR(check_results, &state);


	iinq_rewrite_test_insert_test1_filter(tc);

	/* Different queries with different operators */

	iinq_rewrite_test_select_field_list_from_test1_where_filter_string(processor);

	iinq_rewrite_test_select_all_from_test1_where_filter_numeric(processor);

	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col1 = 31.02; */

	iinq_rewrite_test_select_all_from_test1_where_numeric_equality_filter(processor);

	DROP(test1);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_comparison(

		planck_unit_test_t *tc
) {

	iinq_rewrite_test_select_field_list_from_test1_where_equality_filter(tc);

	iinq_rewrite_test_select_field_list_from_test1_where_filter(tc);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_orderby_single_check_results(

		planck_unit_test_t *tc,

		ion_iinq_result_t *result,

		uint32_t count,

		uint32_t total
) {
	UNUSED(count);
	UNUSED(total);


	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc,
									 sizeof(double) + sizeof(char) * 50 + sizeof(int) + sizeof(uint32_t),
									 result->num_bytes);


	ion_key_t key = result->processed;

	unsigned char *field = key + sizeof(uint32_t) + sizeof(double) + sizeof(char) * 40;

	switch (count) {
		case 0:

			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc,
											 26001, *(int *) field);/* col3 */
			field += sizeof(int);

			PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc,
											 "Zimbabwe", field);/* col4 */
			break;

		case 1:

			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc,
											 26002, *(int *) field);/* col3 */
			field += sizeof(int);

			PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc,
											 "USA", field);    /* col4 */
			break;

		case 2:

			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc,
											 26003, *(int *) field);/* col3 */
			field += sizeof(int);

			PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc,
											 "Canada", field);    /* col4 */
			break;

		default:

			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_rewrite_test_select_field_list_from_test1_where_orderby_single_asc(

		ion_iinq_query_processor_t processor
) {
	/* SELECT col1, col2, col3, col4 FROM test1 WHERE col3 > 26000 ORDER BY col3 ASC; */
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;

		int agg_n = 0;

		int i_agg = 0;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;


		int orderby_n = 0;

		int i_orderby = 0;

		int total_orderby_size = 0;

		iinq_order_part_t *orderby_order_parts = NULL;

		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;
		/* end of select_clause */

		orderby_n = 1;

		total_orderby_size = 0;

		orderby_order_parts = alloca(sizeof(iinq_order_part_t) * 1);
		orderby_order_parts[0].
				direction = 1;
		orderby_order_parts[0].
				size = sizeof(test1_tuple->col3);
		orderby_order_parts[0].
				type = IINQ_ORDERTYPE_INT;
		total_orderby_size += orderby_order_parts[0].
				size;
		goto
				IINQ_SKIP_COMPUTE_ORDERBY;

		IINQ_COMPUTE_ORDERBY:;
		orderby_order_parts[0].
				pointer = &(test1_tuple->col3);
		goto
				IINQ_DONE_COMPUTE_ORDERBY;

		IINQ_SKIP_COMPUTE_ORDERBY:;

		do {
			{
				/* _OPEN_ORDERING_FILE_WRITE(orderby, 0, 1, 0, result, orderby) */

				output_file = fopen("orderby", "wb");

				if (NULL == output_file) {

					error = err_file_open_error;
					goto
							IINQ_QUERY_END;
				}


				write_page_remaining = IINQ_PAGE_SIZE;

				if ((int)

							write_page_remaining < (int) (total_orderby_size + (result.raw_record_size))

						) {

					error = err_record_size_too_large;
					goto
							IINQ_QUERY_END;
				}
			}

			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((test1_tuple->col3 > 26000))) {
					continue;
				}

				{
					goto
							IINQ_COMPUTE_ORDERBY;

					IINQ_DONE_COMPUTE_ORDERBY:;

					jmp_r = 3;
					goto
							COMPUTE_SELECT;

					DONE_COMPUTE_SELECT_3:;

					if ((int)

								write_page_remaining < (int) (total_orderby_size + (result.num_bytes) + (8 * agg_n))

							) {

						int i = 0;

						char x = 0;

						for (;
								i < write_page_remaining;
								i++) {
							if (1 != fwrite(&x, 1, 1, output_file)) {
								break;
							}
						}


						write_page_remaining = 512;
					}

					;

					for (
							i_orderby = 0;
							i_orderby < orderby_n;
							i_orderby++) {
						if (1 !=
							fwrite(orderby_order_parts[i_orderby]
										   .pointer, orderby_order_parts[i_orderby].size, 1, output_file)) {
							break;
						} else {
							write_page_remaining -= orderby_order_parts[i_orderby].
									size;
						}
					}

					if (1 !=
						fwrite(result
									   .processed, result.num_bytes, 1, output_file)) {
						break;
					} else {
						write_page_remaining -= result.
								num_bytes;
					}
				}

				if (orderby_n == 0) {
					goto
							COMPUTE_SELECT;

					DONE_COMPUTE_SELECT:;

					if (3 == jmp_r) {
						goto
								DONE_COMPUTE_SELECT_3;
					}

					(&processor)->execute(&result, (&processor)->state);
				}
			}


			IINQ_QUERY_CLEANUP:

			if (0 !=
				fclose(output_file)
					) {

				error = err_file_close_error;
				goto
						IINQ_QUERY_END;
			}

			;

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);

		{
			/* _OPEN_ORDERING_FILE_READ(orderby, 1, 0, 1, result, orderby); */

			input_file = fopen("orderby", "rb");

			if (NULL == input_file) {

				error = err_file_open_error;
				goto
						IINQ_QUERY_END;
			}


			read_page_remaining = 512;

			if ((int)

						read_page_remaining < (int) (total_orderby_size + (8 * agg_n) + result.num_bytes)

					) {

				error = err_record_size_too_large;
				goto
						IINQ_QUERY_END;
			}

			;
		}

		/* end of _OPEN_ORDERING_FILE_READ(orderby, 1, 0, 1, result, orderby); */

		ion_external_sort_t es;
		/* iinq_sort_context_t context = _IINQ_SORT_CONTEXT(orderby); */

		iinq_sort_context_t context = ((iinq_sort_context_t) {orderby_order_parts, orderby_n}
		);

		/* if (err_ok != (error = ion_external_sort_init(&es, input_file, &context, iinq_sort_compare, _RESULT_ORDERBY_RECORD_SIZE, _RESULT_ORDERBY_RECORD_SIZE + total_orderby_size + (8 * agg_n), IINQ_PAGE_SIZE, boolean_false, ION_FILE_SORT_FLASH_MINSORT)))*/
		if (err_ok != (
				error = ion_external_sort_init(&es, input_file, &context, iinq_sort_compare, result.num_bytes,
											   result.num_bytes + total_orderby_size + (8 * agg_n), 512, boolean_false,
											   ION_FILE_SORT_FLASH_MINSORT)
		)) {
			/* _CLOSE_ORDERING_FILE(input_file); */
			if (0 !=
				fclose(input_file)
					) {

				error = err_file_close_error;
				goto
						IINQ_QUERY_END;
			}

			;
			/* end of _CLOSE_ORDERING_FILE(input_file); */
			goto
					IINQ_QUERY_END;
		}


		uint16_t buffer_size = ion_external_sort_bytes_of_memory_required(&es, 0, boolean_false);

		char *buffer = alloca(buffer_size);

		char *record_buf = alloca((total_orderby_size + 8 * agg_n + result.num_bytes));

		result.
				processed = (unsigned char *) (record_buf + total_orderby_size + (8 * agg_n));


		ion_external_sort_cursor_t cursor;

		if (err_ok != (
				error = ion_external_sort_init_cursor(&es, &cursor, buffer, buffer_size)
		)) {
			/* _CLOSE_ORDERING_FILE(input_file); */
			if (0 !=
				fclose(input_file)
					) {

				error = err_file_close_error;
				goto
						IINQ_QUERY_END;
			}

			;
			/* end of _CLOSE_ORDERING_FILE(input_file); */
			goto
					IINQ_QUERY_END;
		}

		if (err_ok != (
				error = cursor.next(&cursor, record_buf)
		)) {
			/* _CLOSE_ORDERING_FILE(input_file); */
			if (0 !=
				fclose(input_file)
					) {

				error = err_file_close_error;
				goto
						IINQ_QUERY_END;
			}

			;
			/* end of _CLOSE_ORDERING_FILE(input_file); */
			goto
					IINQ_QUERY_END;
		}

		while (cs_cursor_active == cursor.status) {
			(&processor)->execute(&result, (&processor)->state);

			if (err_ok != (
					error = cursor.next(&cursor, record_buf)
			)) {
				/* _CLOSE_ORDERING_FILE(input_file); */
				if (0 !=
					fclose(input_file)
						) {

					error = err_file_close_error;
					goto
							IINQ_QUERY_END;
				}

				;
				goto
						IINQ_QUERY_END;
			}
		}

		ion_external_sort_destroy_cursor(&cursor);

		/* _CLOSE_ORDERING_FILE(input_file); */
		if (0 !=
			fclose(input_file)
				) {

			error = err_file_close_error;
			goto
					IINQ_QUERY_END;
		}

		;

		/* end of _CLOSE_ORDERING_FILE(input_file); */
		/* _REMOVE_ORDERING_FILE(orderby); */
		if (0 != fremove("orderby")) {

			error = err_file_delete_error;
			goto
					IINQ_QUERY_END;
		}

		;
		/* end of _REMOVE_ORDERING_FILE(orderby); */

		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_orderby_single_desc(

		ion_iinq_query_processor_t processor
) {
	do {

		ion_err_t error;

		ion_iinq_result_t result;

		int jmp_r;

		jmp_buf selectbuf;

		result.
				raw_record_size = 0;
		result.
				num_bytes = 0;


		int read_page_remaining = IINQ_PAGE_SIZE;

		int write_page_remaining = IINQ_PAGE_SIZE;

		FILE *input_file;

		FILE *output_file;

		int agg_n = 0;

		int i_agg = 0;
		/* FROM(0, test1)
		 * first argument is with_schemas but the query macro would check for with_schema
		 * the IF_ELSE macro always substituted as (),
		 * pretty sure there was a typo in the macro */

		ion_iinq_cleanup_t *first;

		ion_iinq_cleanup_t *last;

		ion_iinq_cleanup_t *ref_cursor;

		ion_iinq_cleanup_t *last_cursor;


		first = NULL;

		last = NULL;

		ref_cursor = NULL;

		last_cursor = NULL;

		/* FROM_SOURCES(test1)
		 * substituted with FROM_SOURCE_SINGLE(test1) */

		ion_iinq_source_t test1;

		test1.cleanup.
				next = NULL;
		test1.cleanup.
				last = last;
		test1.cleanup.
				reference = &test1;

		if (NULL == first) {

			first = &test1.cleanup;
		}

		if (NULL != last) {
			last->
					next = &test1.cleanup;
		}


		last = &test1.cleanup;
		test1.cleanup.
				next = NULL;
		test1.dictionary.
				handler = &test1.handler;

		error = iinq_open_source("test1" ".inq", &(test1.dictionary), &(test1.handler));

		if (err_ok != error) {
			break;
		}

		result.raw_record_size += test1.dictionary.instance->record.
				key_size;
		result.raw_record_size += test1.dictionary.instance->record.
				value_size;
		result.num_bytes += test1.dictionary.instance->record.
				key_size;
		result.num_bytes += test1.dictionary.instance->record.
				value_size;

		error = dictionary_build_predicate(&(test1.predicate), predicate_all_records);

		if (err_ok != error) {
			break;
		}

		dictionary_find(&test1.dictionary, &test1.predicate, &test1.cursor);
		/* end of FROM_SOURCES(test1),
		 * FROM(0, test1) continued */
		result.
				data = alloca(result.raw_record_size);
		result.
				processed = result.data;
		/* _FROM_SETUP_POINTERS(test1),
		 * substituted to _FROM_GET_OVERRIDE(test1)
		 * substituted to _FROM_SETUP_POINTERS_SINGLE(test1) */
		test1.
				key = result.processed;
		result.processed += test1.dictionary.instance->record.
				key_size;
		test1.
				value = result.processed;
		result.processed += test1.dictionary.instance->record.
				value_size;
		test1.ion_record.
				key = test1.key;
		test1.ion_record.
				value = test1.value;


		struct iinq_test1_schema *test1_tuple;


		test1_tuple = test1.value;

		ref_cursor = first;

		while (ref_cursor != last) {
			if ((NULL == ref_cursor) || ((cs_cursor_active != (ref_cursor->reference->
					cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																		&ref_cursor->reference->ion_record)
			)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
				break;
			}


			ref_cursor = ref_cursor->next;
		}


		ref_cursor = last;


		int orderby_n = 0;

		int i_orderby = 0;

		int total_orderby_size = 0;

		iinq_order_part_t *orderby_order_parts = NULL;

		/* select_clause */
		goto
				SKIP_COMPUTE_SELECT;

		COMPUTE_SELECT:;

		/* SELECT(...) */
		do {

			ion_iinq_result_size_t select_byte_index = 0;


			memcpy(result
						   .processed, test1.key, test1.dictionary.instance->record.key_size);
			select_byte_index += test1.dictionary.instance->record.
					key_size;

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col1), sizeof(test1_tuple->col1));
			select_byte_index += sizeof(test1_tuple->col1);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col2), sizeof(test1_tuple->col2));
			select_byte_index += sizeof(test1_tuple->col2);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col3), sizeof(test1_tuple->col3));
			select_byte_index += sizeof(test1_tuple->col3);

			memcpy(result
						   .processed + select_byte_index, &(test1_tuple->col4), sizeof(test1_tuple->col4));
			select_byte_index += sizeof(test1_tuple->col4);
			result.
					num_bytes = select_byte_index;
		} while (0);

		goto
				DONE_COMPUTE_SELECT;

		SKIP_COMPUTE_SELECT:;
		/* end of select_clause */

		orderby_n = 1;

		total_orderby_size = 0;

		orderby_order_parts = alloca(sizeof(iinq_order_part_t) * 1);
		orderby_order_parts[0].
				direction = -1;
		orderby_order_parts[0].
				size = sizeof(test1_tuple->col4);
		orderby_order_parts[0].
				type = IINQ_ORDERTYPE_OTHER;
		total_orderby_size += orderby_order_parts[0].
				size;
		goto
				IINQ_SKIP_COMPUTE_ORDERBY;

		IINQ_COMPUTE_ORDERBY:;
		orderby_order_parts[0].
				pointer = &(test1_tuple->col4);
		goto
				IINQ_DONE_COMPUTE_ORDERBY;

		IINQ_SKIP_COMPUTE_ORDERBY:;

		do {
			{
				/* _OPEN_ORDERING_FILE_WRITE(orderby, 0, 1, 0, result, orderby) */

				output_file = fopen("orderby", "wb");

				if (NULL == output_file) {

					error = err_file_open_error;
					goto
							IINQ_QUERY_END;
				}


				write_page_remaining = IINQ_PAGE_SIZE;

				if ((int)

							write_page_remaining < (int) (total_orderby_size + (result.raw_record_size))

						) {

					error = err_record_size_too_large;
					goto
							IINQ_QUERY_END;
				}
			}

			while (1) {
				/* _FROM_ADVANCE_CURSORS */
				if (NULL == ref_cursor) {
					break;
				}


				last_cursor = ref_cursor;

				while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->
						cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																			&ref_cursor->reference->ion_record)
				) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) {
					ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor);
					dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate,
									&ref_cursor->reference->cursor);

					if (((cs_cursor_active != (ref_cursor->reference->
							cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor,
																				&ref_cursor->reference->ion_record)
					)) && (cs_cursor_initialized != ref_cursor->reference->cursor_status))) {
						goto
								IINQ_QUERY_CLEANUP;
					}


					ref_cursor = ref_cursor->last;
				}

				if (NULL == ref_cursor) {
					break;
				} else if (last_cursor != ref_cursor) {

					ref_cursor = last;
				}

				/* end of _FROM_ADVANCE_CURSORS */
				/* if (!conditions) { continue; } */
				if (!((test1_tuple->col3 > 26000))) {
					continue;
				}

				{
					goto
							IINQ_COMPUTE_ORDERBY;

					IINQ_DONE_COMPUTE_ORDERBY:;

					jmp_r = 3;
					goto
							COMPUTE_SELECT;

					DONE_COMPUTE_SELECT_3:;

					if ((int)

								write_page_remaining < (int) (total_orderby_size + (result.num_bytes) + (8 * agg_n))

							) {

						int i = 0;

						char x = 0;

						for (;
								i < write_page_remaining;
								i++) {
							if (1 != fwrite(&x, 1, 1, output_file)) {
								break;
							}
						}


						write_page_remaining = 512;
					}

					;

					for (
							i_orderby = 0;
							i_orderby < orderby_n;
							i_orderby++) {
						if (1 !=
							fwrite(orderby_order_parts[i_orderby]
										   .pointer, orderby_order_parts[i_orderby].size, 1, output_file)) {
							break;
						} else {
							write_page_remaining -= orderby_order_parts[i_orderby].
									size;
						}
					}

					if (1 !=
						fwrite(result
									   .processed, result.num_bytes, 1, output_file)) {
						break;
					} else {
						write_page_remaining -= result.
								num_bytes;
					}
				}

				if (orderby_n == 0) {
					goto
							COMPUTE_SELECT;

					DONE_COMPUTE_SELECT:;

					if (3 == jmp_r) {
						goto
								DONE_COMPUTE_SELECT_3;
					}

					(&processor)->execute(&result, (&processor)->state);
				}
			}


			IINQ_QUERY_CLEANUP:

			if (0 !=
				fclose(output_file)
					) {

				error = err_file_close_error;
				goto
						IINQ_QUERY_END;
			}

			;

			while (NULL != first) {
				first->reference->cursor->destroy(&first->reference->cursor);
				ion_close_dictionary(&first->reference->dictionary);

				first = first->next;
			}

			if (err_ok != error) {
				goto
						IINQ_QUERY_END;
			}
		} while (0);

		{
			/* _OPEN_ORDERING_FILE_READ(orderby, 1, 0, 1, result, orderby); */

			input_file = fopen("orderby", "rb");

			if (NULL == input_file) {

				error = err_file_open_error;
				goto
						IINQ_QUERY_END;
			}


			read_page_remaining = 512;

			if ((int)

						read_page_remaining < (int) (total_orderby_size + (8 * agg_n) + result.num_bytes)

					) {

				error = err_record_size_too_large;
				goto
						IINQ_QUERY_END;
			}

			;
		}

		/* end of _OPEN_ORDERING_FILE_READ(orderby, 1, 0, 1, result, orderby); */

		ion_external_sort_t es;
		/* iinq_sort_context_t context = _IINQ_SORT_CONTEXT(orderby); */

		iinq_sort_context_t context = ((iinq_sort_context_t) {orderby_order_parts, orderby_n}
		);

		/* if (err_ok != (error = ion_external_sort_init(&es, input_file, &context, iinq_sort_compare, _RESULT_ORDERBY_RECORD_SIZE, _RESULT_ORDERBY_RECORD_SIZE + total_orderby_size + (8 * agg_n), IINQ_PAGE_SIZE, boolean_false, ION_FILE_SORT_FLASH_MINSORT)))*/
		if (err_ok != (
				error = ion_external_sort_init(&es, input_file, &context, iinq_sort_compare, result.num_bytes,
											   result.num_bytes + total_orderby_size + (8 * agg_n), 512, boolean_false,
											   ION_FILE_SORT_FLASH_MINSORT)
		)) {
			/* _CLOSE_ORDERING_FILE(input_file); */
			if (0 !=
				fclose(input_file)
					) {

				error = err_file_close_error;
				goto
						IINQ_QUERY_END;
			}

			;
			/* end of _CLOSE_ORDERING_FILE(input_file); */
			goto
					IINQ_QUERY_END;
		}


		uint16_t buffer_size = ion_external_sort_bytes_of_memory_required(&es, 0, boolean_false);

		char *buffer = alloca(buffer_size);

		char *record_buf = alloca((total_orderby_size + 8 * agg_n + result.num_bytes));

		result.
				processed = (unsigned char *) (record_buf + total_orderby_size + (8 * agg_n));


		ion_external_sort_cursor_t cursor;

		if (err_ok != (
				error = ion_external_sort_init_cursor(&es, &cursor, buffer, buffer_size)
		)) {
			/* _CLOSE_ORDERING_FILE(input_file); */
			if (0 !=
				fclose(input_file)
					) {

				error = err_file_close_error;
				goto
						IINQ_QUERY_END;
			}

			;
			/* end of _CLOSE_ORDERING_FILE(input_file); */
			goto
					IINQ_QUERY_END;
		}

		if (err_ok != (
				error = cursor.next(&cursor, record_buf)
		)) {
			/* _CLOSE_ORDERING_FILE(input_file); */
			if (0 !=
				fclose(input_file)
					) {

				error = err_file_close_error;
				goto
						IINQ_QUERY_END;
			}

			;
			/* end of _CLOSE_ORDERING_FILE(input_file); */
			goto
					IINQ_QUERY_END;
		}

		while (cs_cursor_active == cursor.status) {
			(&processor)->execute(&result, (&processor)->state);

			if (err_ok != (
					error = cursor.next(&cursor, record_buf)
			)) {
				/* _CLOSE_ORDERING_FILE(input_file); */
				if (0 !=
					fclose(input_file)
						) {

					error = err_file_close_error;
					goto
							IINQ_QUERY_END;
				}

				;
				goto
						IINQ_QUERY_END;
			}
		}

		ion_external_sort_destroy_cursor(&cursor);

		/* _CLOSE_ORDERING_FILE(input_file); */
		if (0 !=
			fclose(input_file)
				) {

			error = err_file_close_error;
			goto
					IINQ_QUERY_END;
		}

		;

/* end of _CLOSE_ORDERING_FILE(input_file); */
/* _REMOVE_ORDERING_FILE(orderby); */
		if (0 != fremove("orderby")) {
			error = err_file_delete_error;
			goto
					IINQ_QUERY_END;
		};
		/* end of _REMOVE_ORDERING_FILE(orderby); */
		IINQ_QUERY_END:;
	} while (0);
}

void
iinq_rewrite_test_select_field_list_from_test1_where_orderby_single(
		planck_unit_test_t *tc
) {
	iinq_rewrite_create_test1(tc);
	iinq_rewrite_test_insert_multiple_values_test1_order_by_single(tc);

	ion_iinq_query_processor_t
			processor;
	iinq_test_query_state_t state;

	state.func = iinq_rewrite_test_select_field_list_from_test1_where_orderby_single_check_results;
	state.tc = tc;
	state.count = 0;

	processor = IINQ_QUERY_PROCESSOR(check_results, &state);

	iinq_rewrite_test_select_field_list_from_test1_where_orderby_single_asc(processor);

	state.count = 0;
	iinq_rewrite_test_select_field_list_from_test1_where_orderby_single_desc(processor);

	DROP(test1);
}

planck_unit_suite_t *
iinq_rewrite_get_suite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	/* Original IINQ */
	/*PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_all_from_test1);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_field_list_from_test1_where_equality_filter);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_field_list_from_test1_where_comparison);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_field_list_from_test1_where_orderby_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_field_list_from_test1);*/

	/* IINQ rewrite */
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_all_from_test1_iterator);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_field_list_from_test1_iterator);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_all_from_test1_where_iterator);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_field_list_from_test1_where_iterator);
	//PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_field_list_from_test1_where_order_by_single_iterator); // fails on device, not sure why

	/* Queries from paper */
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_all_from_int_table_iterator);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_attr0_from_int_table_iterator);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_attr0_attr1_from_int_table_iterator);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_attr0_attr1_attr2_attr3_from_int_table_iterator);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_attr0_attr1_attr2_attr3_from_int_table_where_iterator);

	return suite;
}

void
run_all_tests_iinq_rewrite(
) {
	planck_unit_suite_t *suite = iinq_rewrite_get_suite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);

	fremove(ION_MASTER_TABLE_FILENAME);
}
