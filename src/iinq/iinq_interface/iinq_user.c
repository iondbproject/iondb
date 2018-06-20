/******************************************************************************/
/**
		@file		iinq_user.c
		@author		Dana Klamut, Kai Neubauer

@brief		This code contains definitions for iinq user functions
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

#include "iinq_user.h"

void
cleanup(
) {
	fdeleteall();
	fremove("1.ffs");
	fremove("2.ffs");
	fremove("3.ffs");
	fremove("4.ffs");
	fremove("5.ffs");
	fremove("6.ffs");
	fremove("0.inq");
	fremove("1.inq");
	fremove("2.inq");
	fremove("255.inq");
	fremove("ion_mt.tbl");
}

/* TODO: test OR and XOR once implemented */
int
main(
	void
) {
	/* Clean-up */
	cleanup();

	/* CREATE TABLE statement (table_id = 0)*/
/*  SQL_execute("CREATE TABLE Table1 (id INT, value INT, primary key(id));"); */
	create_table(0, key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2));

	/* Insert rows */
	for (int i = 0; i < 100; i++) {
/*		 iinq_prepared_sql p1 = SQL_prepare("INSERT INTO Table1 VALUES (?, ?);"); */
		iinq_prepared_sql p1 = insert_0(NULL, NULL);

		setParam(p1, 1, IONIZE(i, int));
		setParam(p1, 2, IONIZE(100 - i, int));
		execute(p1);
	}

	/* Test query */
	printf("SELECT * FROM Table1\n");

/*  iinq_result_set *rs1 = SQL_select("SELECT * FROM Table1;\n"); */
	iinq_result_set *rs1 = iinq_select(0, sizeof(int) + sizeof(int), 0, 2, IINQ_SELECT_LIST(1, 2));

	while (next(rs1)) {
		printf("ID: %d,", getInt(rs1, 1));
		printf(" VALUE: %d\n", getInt(rs1, 2));
	}

	printf("SELECT * FROM Table1 WHERE id > 50 AND id < 70;\n");
/*	 rs1 = SQL_select("SELECT * FROM Table1 WHERE id > 50 AND id < 70;\n"); */
	rs1 = iinq_select(0, sizeof(int) + sizeof(int), 2, 2, IINQ_CONDITION_LIST(IINQ_CONDITION(1, iinq_less_than, 70), IINQ_CONDITION(1, iinq_greater_than, 50)), IINQ_SELECT_LIST(1, 2));

	while (next(rs1)) {
		printf("ID: %d,", getInt(rs1, 1));
		printf(" VALUE: %d\n", getInt(rs1, 2));
	}

	printf("SELECT value, id FROM Table1 WHERE id > 50 AND id < 70;\n");
/*	 rs1 = SQL_select("SELECT value, id FROM Table1 WHERE id > 50 AND id < 70;\n"); */
	rs1 = iinq_select(0, sizeof(int) + sizeof(int), 2, 2, IINQ_CONDITION_LIST(IINQ_CONDITION(1, iinq_less_than, 70), IINQ_CONDITION(1, iinq_greater_than, 50)), IINQ_SELECT_LIST(2, 1));

	while (next(rs1)) {
		printf("VALUE: %d, ", getInt(rs1, 1));
		printf("ID: %d\n", getInt(rs1, 2));
	}

	printf("SELECT value FROM Table1 WHERE id > 50 AND id < 70;\n");
/*	 rs1 = SQL_select("SELECT value FROM Table1 WHERE id > 50 AND id < 70;\n"); */
	rs1 = iinq_select(0, sizeof(int), 2, 1, IINQ_CONDITION_LIST(IINQ_CONDITION(1, iinq_less_than, 70), IINQ_CONDITION(1, iinq_greater_than, 50)), IINQ_SELECT_LIST(2));

	while (next(rs1)) {
		printf("VALUE: %d\n", getInt(rs1, 1));
	}

/*  SQL_execute("DROP TABLE Table1;"); */
	drop_table(0);

	/* Clean-up */
	cleanup();

	return 0;
}
