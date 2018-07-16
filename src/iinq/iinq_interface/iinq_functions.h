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

#define _STRINGIZE(x)														# x
#define STRINGIZE(x)														_STRINGIZE(x)

/* Dummy functions for code that will be parsed */
#define SQL_execute(SQL_string)												NULL
#define SQL_prepare(SQL_string)												NULL
#define SQL_select(SQL_string)												NULL

#define IINQ_SELECT_LIST(...)												(iinq_field_num_t[]) { __VA_ARGS__ }
#define IINQ_CONDITION(left, op, right)										(iinq_where_params_t) { (left), (op), (right) }
#define IINQ_CONDITION_LIST(...)											(iinq_where_params_t[]) { __VA_ARGS__ }
#define IINQ_UPDATE_LIST(...)												(iinq_update_params_t[]) { __VA_ARGS__ }
#define IINQ_UPDATE(update_field, implicit_field, operator, field_value)	(iinq_update_params_t) { (update_field), (implicit_field), (operator), (field_value) }

#define IINQ_ORDER_BY_LIST(...)												(iinq_order_by_field_t[]) { __VA_ARGS__ }
#define IINQ_ORDER_BY(field_num, direction)									(iinq_order_by_field_t) { (field_num), (direction) }

#define iinq_get_int(result_set, field_num)									(int *) iinq_get_object((result_set), (field_num))
#define iinq_get_string(result_set, field_num)								(char *) iinq_get_object((result_set), (field_num))
#define iinq_get_object(result_set, field_num)								(int *) (iinq_check_null_indicator((result_set)->record.value, field_num) ? NULL : (((unsigned char *) (result_set)->record.value) + (result_set)->offset[(field_num) - 1]))

#define iinq_close_result_set(result_set)									(result_set)->destroy(&(result_set))
#define iinq_next(result_set)												(result_set)->next(result_set)

#define IINQ_BITS_FOR_NULL(num_fields) \
	((num_fields) / CHAR_BIT + 1)

#define iinq_close_statement(p) \
	if ((p) != NULL) { \
		if ((p)->value != NULL) { \
			free((p)->value); \
		} \
		if ((p)->key != NULL) { \
			free((p)->key); \
		} \
		free((p)); \
	}

void *__IINQ_RESERVED;

#define iinq_execute_instantaneous(p) \
	__IINQ_RESERVED = p; \
	execute(__IINQ_RESERVED); \
	iinq_close_statement((iinq_prepared_sql *) __IINQ_RESERVED); \
	__IINQ_RESERVED = NULL;

#define iinq_check_null_indicator(indicator_array, field_num) \
	(((iinq_null_indicator_t *) (indicator_array))[(((iinq_field_num_t) field_num) - 1) / CHAR_BIT] & (((iinq_field_num_t) 0x1) << ((((iinq_field_num_t) field_num) - 1) % CHAR_BIT)))

#define iinq_set_null_indicator(indicator_array, field_num) \
	(((iinq_null_indicator_t *) (indicator_array))[(((iinq_field_num_t) field_num) - 1) / CHAR_BIT] |= (((iinq_field_num_t) 0x1) << ((((iinq_field_num_t) field_num) - 1) % CHAR_BIT)))

#define iinq_clear_null_indicator(indicator_array, field_num) \
	(((iinq_null_indicator_t *) (indicator_array))[(((iinq_field_num_t) field_num) - 1) / CHAR_BIT] &= ~(((iinq_field_num_t) 0x1) << ((((iinq_field_num_t) field_num) - 1) % CHAR_BIT)))

#if defined(__cplusplus)
extern "C" {
#endif

typedef unsigned char iinq_null_indicator_t;

typedef unsigned char iinq_field_num_t;

/**
@brief		Type for detailing an ORDER BY for a field.
*/
typedef struct {
	iinq_field_num_t		field_num;	/**< The field number of the field to sort by. */
	iinq_order_direction_t	direction;	/**< The direction of the sort. ASC is 1, DESC is -1. */
} iinq_order_by_field_t;

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
	iinq_null_indicator_t	*null_indicators;
	ion_value_t				value;	/* Value parsed from the prepared statement */
	ion_key_t				key;/* Key to be inserted */
	iinq_table_id			table;	/* The table name, stored as a unique identifier */
};

/**
@brief		Struct defining IINQ SELECT iterator.
@see		select_iinq
*/
typedef struct select_iinq iinq_result_set;

typedef ion_boolean_t (*iinq_next_t)(
	iinq_result_set *
);

typedef void (*iinq_destroy_result_set_t)(
	iinq_result_set *
);

typedef struct IINQ_WHERE_PARAMS iinq_where_params_t;

typedef struct IINQ_DICTIONARY iinq_dictionary_ref_t;

struct IINQ_DICTIONARY {
	ion_boolean_t				temp_dictionary;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;
	ion_dict_cursor_t			*cursor;
	ion_predicate_t				predicate;
};

typedef struct {
	ion_external_sort_cursor_t	*cursor;/**< Cursor to iterate through sorted records. */
	char						*record_buf;/**< Memory allocated for the sorted record. */
	iinq_size_t					size;	/**< Size of the sort fields. */
} iinq_sort_t;

struct select_iinq {
	ion_record_t				record;
	iinq_dictionary_ref_t		dictionary_ref;
	iinq_table_id				table_id;
	iinq_field_num_t			*fields;
	iinq_field_num_t			num_fields;
	iinq_next_t					next;
	unsigned int				num_wheres;
	iinq_where_params_t			*wheres;
	ion_status_t				status;
	unsigned int				*offset;
	iinq_sort_t					iinq_sort;
	iinq_destroy_result_set_t	destroy;
};

ion_err_t
iinq_execute(
	iinq_table_id			table_id,
	ion_key_t				key,
	ion_value_t				value,
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
	/**> Field is an unsigned integer type. */
	iinq_unsigned_int,
	/**> Field is a floating type */
	iinq_float,
	/**> Field is a null-terminated string type. */
	iinq_null_terminated_string,
	/**> Field is a char array type. This requires padding to prevent reading memory that is not owned by the value. */
	iinq_char_array
} iinq_field_t;

ion_cursor_status_t
iinq_next_record(
	ion_dict_cursor_t	*cursor,
	ion_record_t		*record
);

typedef void *iinq_field_value_t;

struct IINQ_WHERE_PARAMS {
	int						where_field;
	iinq_bool_operator_t	bool_operator;
	iinq_field_value_t		field_value;
};

ion_boolean_t
where(
	iinq_table_id		id,
	ion_record_t		*record,
	int					num_wheres,
	iinq_where_params_t *where
);

typedef void (*iinq_print_table_t)(
	ion_dictionary_t *
);

struct IINQ_UPDATE_PARAMS {
	int						update_field;
	int						implicit_field;
	iinq_math_operator_t	math_operator;
	iinq_field_value_t		field_value;
};

typedef struct IINQ_UPDATE_PARAMS iinq_update_params_t;

#if defined(__cplusplus)
}
#endif

#endif /* IINQ_FUNCTIONS_H */
