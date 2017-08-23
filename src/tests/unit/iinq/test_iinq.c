/******************************************************************************/
/**
@file		test_iinq.c
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

#include "test_iinq.h"
#include "../../../util/sort/external_sort/external_sort.h"
#include "../../../util/sort/sort.h"
#include "../../../util/sort/external_sort/external_sort_types.h"

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

	status = INSERT_INTO(table, key, value);

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
iinq_rewrite_test_insert_multiple_different_values_test1_with_iterator(
		planck_unit_test_t *tc
) {
	int size = sizeof(double) + sizeof(char) * 40 + sizeof(int) + sizeof(char) * 10;
	unsigned char *value = malloc(size);

	double *col1 = value;
	char *col2 = value + sizeof(double);
	int *col3 = value + sizeof(double) + sizeof(char)*40;
	char *col4 = value + sizeof(double) + sizeof(char)*40 + sizeof(int);

	*col1 = 2.5;

	strcpy(col2, "Hello");
	*col3 = 1;

	strcpy(col4, "Goodbye");

	iinq_rewrite_insert_value_test1(tc, IONIZE(1, uint32_t), value);
	iinq_rewrite_insert_value_test1(tc, IONIZE(1, uint32_t), value);

	*col1 = 1.0;

	strcpy(col2, "Goodbye");
	*col3 = 3;

	strcpy(col4, "Hello");

	iinq_rewrite_insert_value_test1(tc, IONIZE(2, uint32_t), value);
	iinq_rewrite_insert_value_test1(tc, IONIZE(2, uint32_t), value);
}

void
iinq_rewrite_test_insert_different_values_test1_with_iterator(
		planck_unit_test_t *tc
) {

	unsigned char *value = malloc(sizeof(double) + sizeof(char) * 40 + sizeof(int) + sizeof(char) * 10);

	unsigned char *data = value;

	*(double *) data = 2.5;
	data += sizeof(double);

	strcpy(data, "Hello");
	data += sizeof(char) * 40;
	*(int *) data = 1;
	data += sizeof(int);

	strcpy(data, "Goodbye");

	iinq_rewrite_insert_value_test1(tc, IONIZE(1, int), value);

	strcpy(data, "Hello");

	iinq_rewrite_insert_value_test1(tc, IONIZE(1, int), value);

	data = value;

	*(double *) data = 1.0;
	data += sizeof(double);

	strcpy(data, "Goodbye");
	data += sizeof(char) * 40;
	*(int *) data = 3;
	data += sizeof(int);

	strcpy(data, "Hello");

	iinq_rewrite_insert_value_test1(tc, IONIZE(2, uint32_t), value);

	strcpy(data, "Goodbye");

	iinq_rewrite_insert_value_test1(tc, IONIZE(2, uint32_t), value);
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

	iinq_rewrite_test_insert_multiple_different_values_test1_with_iterator(tc);

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

planck_unit_suite_t *
iinq_rewrite_get_suite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	/* IINQ rewrite, uses long init function, hard to maintain */
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_all_from_test1_iterator);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_field_list_from_test1_iterator);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_all_from_test1_where_iterator);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_field_list_from_test1_where_iterator);
	//PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_field_list_from_test1_where_order_by_single_iterator); // fails on device, not sure why

	/* Simple queries, uses smaller init functions with Java generated functions */
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_all_from_int_table_iterator);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_attr0_from_int_table_iterator);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_attr0_attr1_from_int_table_iterator);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_attr0_attr1_attr2_attr3_from_int_table_iterator);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_rewrite_test_select_attr0_attr1_attr2_attr3_from_int_table_where_iterator);

	return suite;
}

void
run_all_tests_iinq(
) {
	planck_unit_suite_t *suite = iinq_rewrite_get_suite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);

	fremove(ION_MASTER_TABLE_FILENAME);
}
