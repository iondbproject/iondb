/******************************************************************************/
/**
@file		iinq_functions.h
@author		Dana Klamut, Kai Neubauer
@brief		This code contains definitions for iinq pre-defined functions
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

#include "../../key_value/kv_system.h"
#include "../../util/sort/sort.h"
#include "../iinq.h"
#include "iinq_user_functions.h"

#if !defined(IINQ_FUNCTIONS_H_)
#define IINQ_FUNCTIONS_H_

#define IINQ_CONDITION(left, op, right)										(iinq_where_params_t) { (left), (op), (right) }
#define IINQ_CONDITION_LIST(...)											__VA_ARGS__
#define IINQ_UPDATE_LIST(...)												__VA_ARGS__
#define IINQ_UPDATE(update_field, implicit_field, operator, field_value)	(iinq_update_params_t) { (update_field), (implicit_field), (operator), (field_value) }

#define NULL_FIELD		NULL
#define PREPARED_FIELD	NULL

#if defined(__cplusplus)
extern "C" {
#endif

/**
@brief		This is the available operation types for IINQ.
*/
typedef enum IINQ_OPERATION_TYPE {
	/**> Operation to be performed is an INSERT. */
	iinq_insert_t,
	/**> Operation to be performed is a DELETE. */
	iinq_delete_t,
	/**> Operation to be performed is an UPDATE. */
	iinq_update_t
} iinq_operation_type_t;

/**git
@brief		Struct defining IINQ INSERT components.
@see		prepared_iinq
*/
typedef struct prepared_iinq iinq_prepared_sql;

struct prepared_iinq {
	unsigned char	*value;	/* Value parsed from the prepared statement */
	unsigned char	*key;	/* Key to be inserted */
	unsigned char	*table;	/* The table name, stored as a unique identifier */
};

/**
@brief		Struct defining IINQ SELECT iterator.
@see		select_iinq
*/
typedef struct select_iinq iinq_result_set;

struct select_iinq {
	ion_record_t		record;
	ion_dict_cursor_t	*cursor;
	int					*count;
	int					*num_recs;
	unsigned char		*value;
	unsigned char		*table_id;
	unsigned char		*fields;
	unsigned char		*num_fields;
	ion_status_t		status;
};

void
iinq_execute(
	iinq_table_id			table_id,
	void					*key,
	unsigned char			*value,
	iinq_operation_type_t	type
);

/**
@brief		This is the available boolean operator types for IINQ.
*/
typedef enum IINQ_BOOL_OPERATOR_TYPE {
	/**> Operator corresponding to "=". */
	iinq_equal,
	/**> Operator corresponding to "!=". */
	iinq_not_equal,
	/**> Operator corresponding to "<". */
	iinq_less_than,
	/**> Operator corresponding to "<=". */
	iinq_less_than_equal_to,
	/**> Operator corresponding to ">". */
	iinq_greater_than,
	/**> Operator corresponding to ">=". */
	iinq_greater_than_equal_to
} iinq_bool_operator_t;

/**
@brief		This is the available math operator types for IINQ.
*/
typedef enum IINQ_MATH_OPERATOR_TYPE {
	/**> Operator corresponding to "-". */
	iinq_subtract,
	/**> Operator corresponding to "+". */
	iinq_add,
	/**> Operator corresponding to "*". */
	iinq_multiply,
	/**> Operator corresponding to "/". */
	iinq_divide
} iinq_math_operator_t;

/**
@brief		This is the available data types for IINQ.
*/
typedef enum IINQ_FIELD_TYPE {
	/**> Field is a signed integer type. */
	iinq_int,
	/**> Field is a null-terminated string type. */
	iinq_null_terminated_string,
	/**> Field is a char array type. This requires padding to prevent reading memory that is not owned by the value. */
	iinq_char_array
} iinq_field_t;

void
SQL_execute(
	char *sql
);

iinq_prepared_sql
SQL_prepare(
	char *sql
);

iinq_result_set
SQL_select(
	char *sql
);

ion_cursor_status_t
iinq_next_record(
	ion_dict_cursor_t	*cursor,
	ion_record_t		*record
);

ion_boolean_t
where(
	iinq_table_id	id,
	ion_record_t	*record,
	int				num_fields,
	va_list			*where
);

typedef void (*iinq_print_table_t)(
	ion_dictionary_t *
);

typedef void *iinq_field_value_t;

struct IINQ_WHERE_PARAMS {
	int where_field;

	iinq_bool_operator_t operator; iinq_field_value_t field_value;
};

typedef struct IINQ_WHERE_PARAMS iinq_where_params_t;

struct IINQ_UPDATE_PARAMS {
	int update_field;
	int implicit_field;

	iinq_math_operator_t operator; iinq_field_value_t field_value;
};

typedef struct IINQ_UPDATE_PARAMS iinq_update_params_t;

#if defined(__cplusplus)
}
#endif

#endif /* IINQ_FUNCTIONS_H */
