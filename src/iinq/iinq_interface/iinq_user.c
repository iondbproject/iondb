/******************************************************************************/
/**
@file		iinq_user.c
@author		Dana Klamut
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
#include "iinq_query_iterator.h"

void
uppercase(
	char	*string,
	char	uppercase[]
) {
	int i;
	int len = (int) strlen(string);

	for (i = 0; i < len; i++) {
		uppercase[i] = (char) toupper(string[i]);
	}

	uppercase[i] = '\0';
}

char *
next_keyword(
	char *keyword
) {
	if (0 == strncmp("SELECT", keyword, 6)) {
		return "FROM";
	}

	if (0 == strncmp("FROM", keyword, 4)) {
		return "WHERE";
	}

	if (0 == strncmp("WHERE", keyword, 5)) {
		return "ORDERBY";
	}

	if (0 == strncmp("ORDERBY", keyword, 7)) {
		return "GROUPBY";
	}

	if (0 == strncmp("GROUPBY", keyword, 7)) {
		return NULL;
	}
	else {
		return NULL;
	}
}

ion_err_t
get_clause(
	char	*keyword,
	char	sql[],
	char	clause[]
) {
	char	*end_pointer, *start_pointer, *curr_key;
	int		start_pos, end_pos;

	/* Get position of next keyword (if exists) */
	curr_key = next_keyword(keyword);

	while (1) {
		if (NULL != curr_key) {
			end_pointer = strstr(sql, curr_key);

			if (NULL != end_pointer) {
				break;
			}
		}
		else {
			end_pointer = NULL;
			break;
		}

		curr_key = next_keyword(curr_key);
	}

	start_pointer = strstr(sql, keyword);

	if (NULL == start_pointer) {
		return err_uninitialized;
	}

	if (NULL == end_pointer) {
		end_pos		= (int) (strlen(sql) + 1);
		start_pos	= (int) (start_pointer - sql);
	}
	else {
		end_pos		= (int) (end_pointer - sql);
		start_pos	= (int) (start_pointer - sql);
	}

	/* Get clause */
	memcpy(clause, &sql[start_pos], end_pos - 1);
	clause[(end_pos - start_pos) - 1] = '\0';
	printf("%s/done\n", clause);

	return err_ok;
}

ion_query_iterator_t
SQL_query(
	char *sql_string
) {
	char uppercase_sql[(int) strlen(sql_string)];

	uppercase(sql_string, uppercase_sql);
	printf("%s\n", uppercase_sql);

	char		select_clause[50];
	ion_err_t	err = get_clause("SELECT", uppercase_sql, select_clause);
	char		select_fields[40];

	if (err == err_ok) {
		memcpy(select_fields, &select_clause[7], (int) strlen(select_clause));
	}

	printf("%sdone\n", select_fields);

	char from_clause[50];

	get_clause("FROM", uppercase_sql, from_clause);
	printf("%sdone\n", from_clause);

	char where_clause[50];

	get_clause("WHERE", uppercase_sql, where_clause);
	printf("%sdone\n", where_clause);

	char orderby_clause[50];

	get_clause("ORDERBY", uppercase_sql, orderby_clause);
	printf("%sdone\n", orderby_clause);

	char groupby_clause[50];

	get_clause("GROUPBY", uppercase_sql, groupby_clause);
	printf("%sdone\n", groupby_clause);

	ion_predicate_t predicate;

	if (0 == strncmp("*", select_fields, 1)) {
		dictionary_build_predicate(&predicate, predicate_all_records);
	}

	ion_query_iterator_t t;

	t.init = query_init;

	ion_record_t ion_record;

	/* Primary key - tuple ID */
	ion_record.key		= malloc(sizeof(ion_dictionary_id_t));

	/* Corresponds to select field list types */
	ion_record.value	= malloc(sizeof(int));

	return t;
}

int
main(
	void
) {
	ion_query_iterator_t iterator = SQL_query("Select * FRoM R wHere m>0 orderby hatsize groupbY hair");

	return 0;
}
