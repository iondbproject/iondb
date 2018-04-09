/******************************************************************************/
/**
@file		test_iinq.c
@author		IonDB Project
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

#include "test_iinq_device.h"

int num_records = 30;
int iteration	= 0;

void
create_table1(
) {
	create_table("Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 30));
	printf("Table created.\n");
}

void
insert_records(
	int		num,
	char	*name,
	int		age
) {
	iinq_prepared_sql p2 = insert_Cats(num, name, age);

	execute(p2);
}

void
insert_prepared(
	int		num,
	char	*name,
	int		age
) {
	iinq_prepared_sql p2 = insert_Cats(num, name, NULL);

	setParam(p2, 3, (int *) age);
	execute(p2);
}

void
insert_many(
) {
	int						i;
	volatile unsigned long	start_time, end_time;

	printf("Inserting records.\n");
	start_time = ion_time();

	for (i = 0; i < 5; i++) {
		insert_records(num_records + 1, "'Beau'", num_records + 5);
		num_records++;
	}

	end_time = ion_time();

	printf("5 records inserted. Records in table: %i. Time taken: %lu\n", num_records, end_time - start_time);
}

void
insert_some(
	int num
) {
	int i;

	for (i = 0; i < num; i++) {
		insert_records(i + 1, "'Beau'", i + 6);
	}
}

void
insert_many_prep(
) {
	int						i;
	volatile unsigned long	start_time, end_time;

	printf("Inserting prep records.\n");
	start_time = ion_time();

	for (i = 0; i < 5; i++) {
		insert_prepared(num_records, "'Beau'", num_records + 5);
		num_records++;
	}

	end_time = ion_time();

	printf("5 records inserted. Records in table: %i. Time taken: %lu\n", num_records, end_time - start_time);
}

void
update_records11(
	int id
) {
	volatile unsigned long start_time, end_time;

	start_time	= ion_time();
	update(0, "Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 30), 3, 4, 7, 1, iinq_less_than, id + 1, 3, 0, 0, 90);
	end_time	= ion_time();
	printf("Time taken: %lu\n", end_time - start_time);
}

void
update_records13(
	int id
) {
	volatile unsigned long start_time, end_time;

	printf("Performing update.\n");
	start_time	= ion_time();
	update(0, "Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 30), 9, 4, 13, 3, iinq_equal, 90, 1, iinq_less_than_equal_to, id + 10, 2, iinq_not_equal, "'Minnie'", 3, 0, 0, 80);
	end_time	= ion_time();
	printf("Done update\n");
	printf("Time taken: %lu\n", end_time - start_time);
}

void
update_records31(
	int id
) {
	volatile unsigned long start_time, end_time;

	printf("Performing update.\n");
	start_time	= ion_time();
	update(0, "Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 30), 3, 12, 15, 1, iinq_less_than, id + 1, 3, 0, 0, 70, 1, 1, iinq_add, 1, 2, 0, 0, "'Chichi'");
	end_time	= ion_time();
	printf("Done update\n");
	printf("Time taken: %lu\n", end_time - start_time);
}

void
update_records33(
	int id
) {
	volatile unsigned long start_time, end_time;

	printf("Performing update.\n");
	start_time	= ion_time();
	update(0, "Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 30), 9, 12, 21, 1, iinq_less_than, id + 2, 1, iinq_greater_than, id - 3, 2, iinq_not_equal, "'Minnie'", 3, 0, 0, 60, 1, 1, iinq_add, 1, 2, 0, 0, "'Buttons'");
	end_time	= ion_time();
	printf("Done update\n");
	printf("Time taken: %lu\n", end_time - start_time);
}

void
delete_records(
	int id
) {
	volatile unsigned long start_time, end_time;

	printf("Delete in progress\n");
	start_time	= ion_time();
	delete_record(0, "Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 30), 3, 1, iinq_greater_than, id);
	end_time	= ion_time();
	printf("Done delete\n");
	printf("Time taken: %lu\n", end_time - start_time);
}

void
delete_some(
) {
	delete_records(num_records - 5);
}

void
delete_records_where(
	int id
) {
	volatile unsigned long start_time, end_time;

	printf("Delete in progress\n");
	start_time	= ion_time();
	delete_record(0, "Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 30), 9, 1, iinq_greater_than, id, 1, iinq_less_than, id + 6, 2, iinq_not_equal, "'Minnie'");
	end_time	= ion_time();
	printf("Done delete\n");
	printf("Time taken: %lu\n", end_time - start_time);
}

void
drop_table1(
) {
	drop_table("Cats.inq");
}

void
delete_many(
	int size
) {
	fremove(ION_MASTER_TABLE_FILENAME);
	fremove("1.ffs");
	fremove("1.val");
	fremove("Cats.inq");
	fremove("2.ffs");
	fremove("3.ffs");
	fremove("6.ffs");
	fremove("SEL.inq");
	fremove("DEL.inq");
	fremove("UPD.inq");

	printf("iteration %i\n", size % 5);
	create_table1();
	insert_some(size);

	printf("insert complete, delete now\n");
	delete_records(5);
	drop_table1();
	printf("delete complete\n");

	fremove(ION_MASTER_TABLE_FILENAME);
	fremove("1.ffs");
	fremove("1.val");
	fremove("Cats.inq");
	fremove("2.ffs");
	fremove("3.ffs");
	fremove("6.ffs");
	fremove("SEL.inq");
	fremove("DEL.inq");
	fremove("UPD.inq");
}

void
delete_many_where(
	int size
) {
	fremove(ION_MASTER_TABLE_FILENAME);
	fremove("1.ffs");
	fremove("1.val");
	fremove("Cats.inq");
	fremove("2.ffs");
	fremove("3.ffs");
	fremove("6.ffs");
	fremove("SEL.inq");
	fremove("DEL.inq");
	fremove("UPD.inq");

	printf("iteration %i\n", (size / 5));
	create_table1();
	insert_some(size);

	printf("insert complete, delete now\n");
	delete_records_where(5);
	drop_table1();
	printf("delete complete\n");

	fremove(ION_MASTER_TABLE_FILENAME);
	fremove("1.ffs");
	fremove("1.val");
	fremove("Cats.inq");
	fremove("2.ffs");
	fremove("3.ffs");
	fremove("6.ffs");
	fremove("SEL.inq");
	fremove("DEL.inq");
	fremove("UPD.inq");
}

void
delete_many_loop(
) {
	create_table1();
	insert_some(30);

	int i;

	for (i = 1; i <= 150; i++) {
		delete_records(i * 5);
	}

	drop_table1();
}

void
delete_many_where_loop(
) {
	create_table1();
	insert_some(750);

	int i;

	for (i = 1; i <= 150; i++) {
		delete_records_where(i * 5);
	}

	drop_table1();
}

void
update_many_loop(
) {
	create_table1();
	insert_some(num_records);

	int i;

	for (i = 1; i <= 5; i++) {
		printf("iteration: %i\n", iteration);
		update_records11(iteration * 5);
		iteration++;
	}

/*		update_records13(i*5); */
/*		update_records31(i*5); */
/*		update_records33(i*5); */
/*	} */

	drop_table1();
}

void
select_records(
	int age
) {
	volatile unsigned long start_time, end_time;

	printf("Select in progress\n");
	start_time = ion_time();

	iinq_result_set rs1 = iinq_select(0, "Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 30), 3, 2, 5, 1, iinq_greater_than, age, 1, 2);

	end_time = ion_time();
	printf("Done select\n");
	printf("Time taken: %lu\n\n", end_time - start_time);

	while (next(&rs1)) {
		printf("ID: %i,", getInt(&rs1, 1));
		printf(" name: %s\n", getString(&rs1, 1));
	}

	printf("\n");
}

void
select_many(
) {
	select_records(num_records - 5);
}

void
select_many_loop(
) {
	create_table1();

	insert_some(30);

	int i;

	for (i = 1; i <= 150; i++) {
		select_records(i * 5);
	}

	drop_table1();
}

planck_unit_suite_t *
iinq_get_suite1(
) {
	int i;

	planck_unit_suite_t *suite = planck_unit_new_suite();

/*	PLANCK_UNIT_ADD_TO_SUITE(suite, create_table1); */
/*  */
/*	for (i = 0; i < 5; i++) { */
/*		PLANCK_UNIT_ADD_TO_SUITE(suite, insert_many); */
/*	} */
/*  */
/*	for (i = 0; i < 5; i++) { */
/*		PLANCK_UNIT_ADD_TO_SUITE(suite, select_many); */
/*		PLANCK_UNIT_ADD_TO_SUITE(suite, delete_some); */
/*	} */

/*	PLANCK_UNIT_ADD_TO_SUITE(suite, delete_many_loop); */
/*	PLANCK_UNIT_ADD_TO_SUITE(suite, delete_many_where_loop); */
	PLANCK_UNIT_ADD_TO_SUITE(suite, update_many_loop);
/*	PLANCK_UNIT_ADD_TO_SUITE(suite, select_many_loop); */
/*	PLANCK_UNIT_ADD_TO_SUITE(suite, print_table_dict); */

/*	PLANCK_UNIT_ADD_TO_SUITE(suite, delete_many_where_loop); */
/*	PLANCK_UNIT_ADD_TO_SUITE(suite, update_many11_loop); */

/*	PLANCK_UNIT_ADD_TO_SUITE(suite, update_records); */
/*	PLANCK_UNIT_ADD_TO_SUITE(suite, delete_records); */
/*	PLANCK_UNIT_ADD_TO_SUITE(suite, select_records); */
/*	PLANCK_UNIT_ADD_TO_SUITE(suite, print_table_dict); */
/*	PLANCK_UNIT_ADD_TO_SUITE(suite, drop_table1); */

	return suite;
}

void
run_all_tests_iinq_device(
	int records
) {
/*	fdeleteall(); */
/*	fremove(ION_MASTER_TABLE_FILENAME); */
/*	fremove("1.ffs"); */
/*	fremove("1.val"); */
/*	fremove("Cats.inq"); */
/*	fremove("2.ffs"); */
/*	fremove("3.ffs"); */
/*	fremove("6.ffs"); */
/*	fremove("SEL.inq"); */
/*	fremove("DEL.inq"); */
/*	fremove("UPD.inq"); */

	printf("num records in table: %i\n", records);
	num_records = records;

	planck_unit_suite_t *suite1 = iinq_get_suite1();

	planck_unit_run_suite(suite1);
	planck_unit_destroy_suite(suite1);

	fdeleteall();
	fremove(ION_MASTER_TABLE_FILENAME);
	fremove("1.ffs");
	fremove("1.val");
	fremove("Cats.inq");
	fremove("2.ffs");
	fremove("3.ffs");
	fremove("6.ffs");
	fremove("SEL.inq");
	fremove("DEL.inq");
	fremove("UPD.inq");
}
