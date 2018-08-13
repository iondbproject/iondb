/******************************************************************************/
/**
@file		test_iinq_device.c
@author		IonDB Project
@brief		Iinq tests to test scalability.
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

#include "test_iinq_device.h"

#define MIN_RECORDS							1
#define MAX_RECORDS							1000
#define RECORD_INCREMENT					1

#define OUTPUT_QUERY_RESULTS				0
#define OUTPUT_TIMES						1
#define OUTPUT_SQL_STATEMENTS				1
#define OUTPUT_INSERT_PROGRESS				0
#define OUTPUT_INSERT_PROGRESS_FREQUENCY	100

volatile unsigned long	insert_time		= 0;
unsigned int			total_inserts	= 0;

void
test_create_table1(
	planck_unit_test_t *tc
) {
#if OUTPUT_SQL_STATEMENTS
	printf("CREATE TABLE Table1 (ID INT, CharValue VARCHAR(30), IntValue INT, primary key(ID));\n");
#endif

	volatile unsigned long start_time, end_time;

	start_time = ion_time();

	ion_err_t error =
/*	  SQL_execute("CREATE TABLE Table1 (ID INT, CharValue VARCHAR(30), IntValue INT, primary key(ID));"); */
		create_table(0, key_type_numeric_signed, sizeof(int), IINQ_BITS_FOR_NULL(3) + (sizeof(int) * 2) + (sizeof(char) * 31));

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	end_time = ion_time();
#if OUTPUT_TIMES
	printf("Create Table time taken: %lu\n", end_time - start_time);
#endif

	iinq_open_source(0, &dictionary, &handler);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_type_numeric_signed, dictionary.instance->key_type);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(int), dictionary.instance->record.key_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, (sizeof(int) * 2) + (sizeof(char) * 31) + IINQ_BITS_FOR_NULL(3), dictionary.instance->record.value_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, iinq_is_key_field(0, 1));
	PLANCK_UNIT_ASSERT_FALSE(tc, iinq_is_key_field(0, 2));
	PLANCK_UNIT_ASSERT_FALSE(tc, iinq_is_key_field(0, 3));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, iinq_int, iinq_get_field_type(0, 1));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, iinq_null_terminated_string, iinq_get_field_type(0, 2));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, iinq_int, iinq_get_field_type(0, 3));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, IINQ_BITS_FOR_NULL(3), iinq_calculate_offset(0, 1));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, IINQ_BITS_FOR_NULL(3) + sizeof(int), iinq_calculate_offset(0, 2));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, IINQ_BITS_FOR_NULL(3) + sizeof(int) + sizeof(char) * 31, iinq_calculate_offset(0, 3));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, IINQ_BITS_FOR_NULL(3) + sizeof(int) * 2 + sizeof(char) * 31, iinq_calculate_offset(0, 4));
	ion_close_dictionary(&dictionary);
}

void
test_insert_prepared_record_table1(
	planck_unit_test_t	*tc,
	intmax_t			id,
	char				*char_value,
	int					int_value
) {
	volatile unsigned long start_time, end_time;

	start_time = ion_time();

/*	  iinq_prepared_sql *p = SQL_prepare("INSERT INTO Table1 VALUES (?, ?, ?);"); */
	iinq_prepared_sql *p = iinq_insert_0(NULL, NULL, NULL);

	iinq_set_param(p, 1, IONIZE(id, int));
	iinq_set_param(p, 2, char_value);
	iinq_set_param(p, 3, IONIZE(int_value, int));

	iinq_execute_prepared(p);

	end_time = ion_time();
	iinq_close_statement(p);
}

void
test_insert_record_table1(
	planck_unit_test_t	*tc,
	int					id,
	char				*char_value,
	int					int_value
) {
	iinq_execute_instantaneous(iinq_insert_0(IONIZE(id, int), char_value, IONIZE(int_value, int)));
}

void
test_insert_records_table1(
	planck_unit_test_t	*tc,
	int					num_records
) {
	intmax_t				i, n;
	volatile unsigned long	start_time, end_time;

	start_time = ion_time();

	for (i = 0; i < num_records; i++) {
		test_insert_record_table1(tc, i + 1, "regInsert", i + 5);
#if OUTPUT_INSERT_PROGRESS

		if ((i % OUTPUT_INSERT_PROGRESS_FREQUENCY == 0) || (i == num_records - 1))
#if defined(ARDUINO)
		{
			printf("Inserting regular (%d): %d%%\n", num_records, (i + 1) * 100 / num_records);
		}

#else
		{
			printf("Inserting regular (%d): %d%%\r", num_records, (i + 1) * 100 / num_records);
		}
		fflush(stdout);
#endif
#endif
	}

	printf("\n");

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("%d records inserted. Time taken: %lu\n", num_records, end_time - start_time);
#endif
}

void
test_insert_records_prep_table1(
	planck_unit_test_t	*tc,
	int					num_records
) {
	intmax_t				i, n;
	volatile unsigned long	start_time, end_time;

	start_time = ion_time();

	for (i = 0; i < num_records; i++) {
		test_insert_prepared_record_table1(tc, -i, "prepInsert", -i + 5);
#if OUTPUT_INSERT_PROGRESS

		if ((i % OUTPUT_INSERT_PROGRESS_FREQUENCY == 0) || (i == num_records - 1))
#if defined(ARDUINO)
		{
			printf("Inserting prepared (%d): %d%%\n", num_records, (i + 1) * 100 / num_records);
		}

#else
		{
			printf("Inserting prepared (%d): %d%%\r", num_records, (i + 1) * 100 / num_records);
		}
		fflush(stdout);
#endif
#endif
	}

	printf("\n");

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("%d prepared records inserted. Time taken: %lu\n", num_records, end_time - start_time);
#endif
}

void
test_select_all_records_table1(
	planck_unit_test_t *tc
) {
	volatile unsigned long start_time, end_time;

#if OUTPUT_SQL_STATEMENTS
	printf("SELECT * FROM Table1\n");
#endif

	start_time = ion_time();

/*	  iinq_result_set_t *rs1 = SQL_select("SELECT * FROM Table1;"); */
	iinq_result_set_t *rs1 = iinq_projection_init(iinq_dictionary_init(0, 3, predicate_all_records), 3, IINQ_PROJECTION_LIST(1, 2, 3));

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Init time taken: %lu\n", end_time - start_time);
#endif

	if ((NULL == rs1) || (err_ok != rs1->status.error)) {
		PLANCK_UNIT_SET_FAIL(tc);
	}

	start_time = ion_time();

	while (iinq_next(rs1)) {
#if OUTPUT_QUERY_RESULTS
		printf("ID: %i, ", *iinq_get_int(rs1, 1));
		printf("CharValue: %s, ", iinq_get_string(rs1, 2));
		printf("IntValue: %d\n", *iinq_get_int(rs1, 3));
#endif
	}

#if OUTPUT_QUERY_RESULTS
	printf("\n");
#endif

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Iteration time taken: %lu\n", end_time - start_time);
#endif

	start_time = ion_time();

	iinq_close_result_set(rs1);

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Closing time taken: %lu\n\n", end_time - start_time);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == rs1);
}

void
test_select_all_where_equal_table1(
	planck_unit_test_t *tc
) {
	volatile unsigned long start_time, end_time;

#if OUTPUT_SQL_STATEMENTS
	printf("SELECT * FROM Table1 WHERE IntValue = 50\n");
#endif

	start_time = ion_time();

/*	  iinq_result_set_t *rs1 = SQL_select("SELECT * FROM Table1 WHERE IntValue = 50;"); */
	iinq_result_set_t *rs1 = iinq_projection_init(iinq_selection_init(iinq_dictionary_init(0, 3, predicate_all_records), 1, IINQ_CONDITION_LIST(IINQ_CONDITION(3, iinq_equal, IONIZE(50, int)))), 3, IINQ_PROJECTION_LIST(1, 2, 3));

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Init time taken: %lu\n", end_time - start_time);
#endif

	if ((NULL == rs1) || (err_ok != rs1->status.error)) {
		PLANCK_UNIT_SET_FAIL(tc);
	}

	start_time = ion_time();

	while (iinq_next(rs1)) {
#if OUTPUT_QUERY_RESULTS
		printf("ID: %i, ", *iinq_get_int(rs1, 1));
		printf("CharValue: %s, ", iinq_get_string(rs1, 2));
		printf("IntValue: %d\n", *iinq_get_int(rs1, 3));
#endif
	}

#if OUTPUT_QUERY_RESULTS
	printf("\n");
#endif

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Iteration time taken: %lu\n", end_time - start_time);
#endif

	start_time = ion_time();

	iinq_close_result_set(rs1);

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Closing time taken: %lu\n\n", end_time - start_time);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == rs1);
}

void
test_select_all_where_equal_key_table1(
	planck_unit_test_t *tc
) {
	volatile unsigned long start_time, end_time;

#if OUTPUT_SQL_STATEMENTS
	printf("SELECT * FROM Table1 WHERE ID = 50\n");
#endif

	start_time = ion_time();

/*	  iinq_result_set_t *rs1 = SQL_select("SELECT * FROM Table1 WHERE ID = 50;"); */
	iinq_result_set_t *rs1 = iinq_projection_init(iinq_dictionary_init(0, 3, predicate_equality, IONIZE(50, int)), 3, IINQ_PROJECTION_LIST(1, 2, 3));

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Init time taken: %lu\n", end_time - start_time);
#endif

	if ((NULL == rs1) || (err_ok != rs1->status.error)) {
		PLANCK_UNIT_SET_FAIL(tc);
	}

	start_time = ion_time();

	while (iinq_next(rs1)) {
#if OUTPUT_QUERY_RESULTS
		printf("ID: %i, ", *iinq_get_int(rs1, 1));
		printf("CharValue: %s, ", iinq_get_string(rs1, 2));
		printf("IntValue: %d\n", *iinq_get_int(rs1, 3));
#endif
	}

#if OUTPUT_QUERY_RESULTS
	printf("\n");
#endif

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Iteration time taken: %lu\n", end_time - start_time);
#endif

	start_time = ion_time();

	iinq_close_result_set(rs1);

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Closing time taken: %lu\n\n", end_time - start_time);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == rs1);
}

void
test_select_all_where_range_table1(
	planck_unit_test_t *tc
) {
	volatile unsigned long start_time, end_time;

#if OUTPUT_SQL_STATEMENTS
	printf("SELECT * FROM Table1 WHERE IntValue >= 25 AND IntValue <= 75\n");
#endif

	start_time = ion_time();

/*	  iinq_result_set_t *rs1 = SQL_select("SELECT * FROM Table1 WHERE IntValue >= 25 AND IntValue <= 75;"); */
	iinq_result_set_t *rs1 = iinq_projection_init(iinq_selection_init(iinq_dictionary_init(0, 3, predicate_all_records), 2, IINQ_CONDITION_LIST(IINQ_CONDITION(3, iinq_less_than_equal_to, IONIZE(75, int)), IINQ_CONDITION(3, iinq_greater_than_equal_to, IONIZE(25, int)))), 3, IINQ_PROJECTION_LIST(1, 2, 3));

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Init time taken: %lu\n", end_time - start_time);
#endif

	if ((NULL == rs1) || (err_ok != rs1->status.error)) {
		PLANCK_UNIT_SET_FAIL(tc);
	}

	start_time = ion_time();

	while (iinq_next(rs1)) {
#if OUTPUT_QUERY_RESULTS
		printf("ID: %i, ", *iinq_get_int(rs1, 1));
		printf("CharValue: %s, ", iinq_get_string(rs1, 2));
		printf("IntValue: %d\n", *iinq_get_int(rs1, 3));
#endif
	}

#if OUTPUT_QUERY_RESULTS
	printf("\n");
#endif

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Iteration time taken: %lu\n", end_time - start_time);
#endif

	start_time = ion_time();

	iinq_close_result_set(rs1);

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Closing time taken: %lu\n\n", end_time - start_time);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == rs1);
}

void
test_select_all_where_range_key_table1(
	planck_unit_test_t *tc
) {
	volatile unsigned long start_time, end_time;

#if OUTPUT_SQL_STATEMENTS
	printf("SELECT * FROM Table1 WHERE ID >= 25 AND ID <= 75\n");
#endif

	start_time = ion_time();

/*	  iinq_result_set_t *rs1 = SQL_select("SELECT * FROM Table1 WHERE ID >= 25 AND ID <= 75;"); */
	iinq_result_set_t *rs1 = iinq_projection_init(iinq_dictionary_init(0, 3, predicate_range, IONIZE(75, int), IONIZE(25, int)), 3, IINQ_PROJECTION_LIST(1, 2, 3));

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Init time taken: %lu\n", end_time - start_time);
#endif

	if ((NULL == rs1) || (err_ok != rs1->status.error)) {
		PLANCK_UNIT_SET_FAIL(tc);
	}

	start_time = ion_time();

	while (iinq_next(rs1)) {
#if OUTPUT_QUERY_RESULTS
		printf("ID: %i, ", *iinq_get_int(rs1, 1));
		printf("CharValue: %s, ", iinq_get_string(rs1, 2));
		printf("IntValue: %d\n", *iinq_get_int(rs1, 3));
#endif
	}

#if OUTPUT_QUERY_RESULTS
	printf("\n");
#endif

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Iteration time taken: %lu\n", end_time - start_time);
#endif

	start_time = ion_time();

	iinq_close_result_set(rs1);

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Closing time taken: %lu\n\n", end_time - start_time);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == rs1);
}

void
test_drop_table1(
	planck_unit_test_t *tc
) {
	volatile unsigned long start_time, end_time;

#if OUTPUT_SQL_STATEMENTS
	printf("DROP TABLE Table1;\n");
#endif

	start_time = ion_time();

	ion_err_t error =
/*	  SQL_execute("DROP TABLE Table1;"); */
		drop_table(0);

	end_time = ion_time();

#if OUTPUT_TIMES
	printf("Drop Table time taken: %lu\n\n", end_time - start_time);
#endif

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	error = iinq_open_source(0, &dictionary, &handler);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_file_open_error, error);
}

void
test_performance_regular_records(
	planck_unit_test_t *tc
) {
	int num_records;

	for (num_records = MIN_RECORDS; num_records <= MAX_RECORDS; num_records += RECORD_INCREMENT) {
		test_create_table1(tc);
		test_insert_records_table1(tc, num_records);
		test_select_all_records_table1(tc);
		test_select_all_where_equal_table1(tc);
		test_select_all_where_equal_key_table1(tc);
		test_select_all_where_range_table1(tc);
		test_select_all_where_range_key_table1(tc);
		test_drop_table1(tc);
		fremove(ION_MASTER_TABLE_FILENAME);
	}
}

void
test_performance_prepared_records(
	planck_unit_test_t *tc
) {
	int num_records;

	for (num_records = MIN_RECORDS; num_records <= MAX_RECORDS; num_records += RECORD_INCREMENT) {
		test_create_table1(tc);
		test_insert_records_prep_table1(tc, num_records);
		test_select_all_records_table1(tc);
		test_select_all_where_equal_table1(tc);
		test_select_all_where_equal_key_table1(tc);
		test_select_all_where_range_table1(tc);
		test_select_all_where_range_key_table1(tc);
		test_drop_table1(tc);
		fremove(ION_MASTER_TABLE_FILENAME);
	}
}

planck_unit_suite_t *
iinq_get_suite1(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_performance_regular_records);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_performance_prepared_records);

	return suite;
}

void
run_all_tests_iinq_device(
) {
	planck_unit_suite_t *suite1 = iinq_get_suite1();

	planck_unit_run_suite(suite1);
	planck_unit_destroy_suite(suite1);

	fdeleteall();
	fremove(ION_MASTER_TABLE_FILENAME);
}
