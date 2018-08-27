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

#if !defined(IINQ_FUNCTIONS_H_)
#define IINQ_FUNCTIONS_H_

#define IINQ_ALLOW_DUPLICATES 1

/**
 * @defgroup DUMMY_FUNCTIONS Function calls that will be commented out and replaced by the Iinq parser.
 * @{
 */
#define SQL_execute(SQL_string) NULL/**< Executes a SQL statement immediately. */
#define SQL_prepare(SQL_string) NULL/**< Creates and returns a prepared statement. */
#define SQL_select(SQL_string)	NULL											/**< Creates and returns a result set for a query. */
/**
 * @}
 */

/**
* @defgroup QUERY_ARGUMENTS Macros created to make query code easier to read.
* @{
*/
#define IINQ_PROJECTION_LIST(...)											(iinq_field_num_t[]) { __VA_ARGS__ }/**< A field list for a projection. */
#define IINQ_CONDITION(left, op, right)										(iinq_where_params_t) { (left), (op), (right) }	/**< A condition to be used for a selection. */
#define IINQ_CONDITION_LIST(...)											(iinq_where_params_t[]) { __VA_ARGS__ }	/**< A list of conditions for a selection. */
#define IINQ_UPDATE_LIST(...)												(iinq_update_params_t[]) { __VA_ARGS__ }/**< A list of updates to be made in an update statement. */
#define IINQ_UPDATE(update_field, implicit_field, operator, field_value)	(iinq_update_params_t) { (update_field), (implicit_field), (operator), (field_value) }	/**< An update to a field to be made in an update statement. */

#define IINQ_ORDER_BY_LIST(...)												(iinq_order_by_field_t[]) { __VA_ARGS__ }	/**< A field list for a query to be ordered by. */
#define IINQ_ORDER_BY(field_num, direction)									(iinq_order_by_field_t) { (field_num), (direction) }/**< A field for a query to be ordered by. Can be in either ascending or descending order. */
/**
 * @}
 */

/**
* @defgroup RESULT_SET_GET Macros for retrieving a value from a result set.
* @{
*/
#define iinq_get_int(result_set, field_num)		(int *) iinq_get_object((result_set), (field_num))								/**< Retrieves an integer value from a result set. */
#define iinq_get_string(result_set, field_num)	(char *) iinq_get_object((result_set), (field_num))								/**< Retrieves a string value from a result set. */
#define iinq_get_object(result_set, field_num)	(iinq_check_null_indicator((result_set)->head->instance->null_indicators, field_num) ? NULL : ((result_set)->head->instance->fields[(field_num) - 1]))								/**< Retrieves an object from a result set. */
/**
 * @}
 */

/**
 * @brief Frees the memory associated with a result set.
 */
#define iinq_close_result_set(result_set) \
	(result_set)->head->instance->destroy(&(result_set)->head); \
	free((result_set)); \
	(result_set) = NULL;

/**
 * @brief Macro to determine the number of bytes needed for a null indicator bit array.
 */
#define IINQ_BITS_FOR_NULL(num_fields) \
	((num_fields) / CHAR_BIT + 1)

/**
 * @brief Frees the memory associated with a prepared statement.
 */
#define iinq_close_statement(p) \
	if ((p) != NULL) { \
		if ((p)->value != NULL) { \
			free((p)->value); \
		} \
		if ((p)->key != NULL) { \
			free((p)->key); \
		} \
		ion_close_dictionary(&(p)->dictionary); \
		free((p)); \
	}

/**
 * @brief Creates, executes, and destroys a prepared statement immediately.
 */
#define iinq_execute_instantaneous(p) \
	{ \
		iinq_prepared_sql *__IINQ_RESERVED = p; \
		iinq_execute_prepared(__IINQ_RESERVED); \
		iinq_close_statement(__IINQ_RESERVED); \
		__IINQ_RESERVED = NULL; \
	}

/**
* @defgroup NULL_INDICATORS Macros created to manipulate a null indicator bit array. 1 is NULL and 0 is not NULL.
* @{
*/
#define iinq_check_null_indicator(indicator_array, field_num) \
	(((iinq_null_indicator_t *) (indicator_array))[(((iinq_field_num_t) field_num) - 1) / CHAR_BIT] & (((iinq_field_num_t) 0x1) << ((((iinq_field_num_t) field_num) - 1) % CHAR_BIT)))	/**< Returns the status of the null indicator for a given field number. */

#define iinq_set_null_indicator(indicator_array, field_num) \
	(((iinq_null_indicator_t *) (indicator_array))[(((iinq_field_num_t) field_num) - 1) / CHAR_BIT] |= (((iinq_field_num_t) 0x1) << ((((iinq_field_num_t) field_num) - 1) % CHAR_BIT)))	/**< Sets the status of the null indicator to 1 (true). */

#define iinq_clear_null_indicator(indicator_array, field_num) \
	(((iinq_null_indicator_t *) (indicator_array))[(((iinq_field_num_t) field_num) - 1) / CHAR_BIT] &= ~(((iinq_field_num_t) 0x1) << ((((iinq_field_num_t) field_num) - 1) % CHAR_BIT)))/**< Sets the status of the null indicator to 0 (false). */
/**
 * @}
 */

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Type used for null indicator bit arrays.
 */
typedef unsigned char iinq_null_indicator_t;

/**
 * @brief Field numbers within tables and queries.
 */
typedef unsigned char iinq_field_num_t;

/**
 * @brief Information about where a field came from to easil
 */
typedef struct IINQ_FIELD_INFO {
	/**> The id for the table that the field originated from. */
	iinq_table_id_t		table_id;
	/**> The field number within the original table that the field originated from. */
	iinq_field_num_t	field_num;
} iinq_field_info_t;

/**
* @defgroup SORT_DIRECTIONS Directions used when sorting a query.
* @{
*/
#define IINQ_ASC	1	/**< Sort values in ascending order. */
#define IINQ_DESC	-1	/**< Sort values in descending order. */

/*
 * @}
 */

/**
@brief		Type for detailing an ORDER BY for a field.
*/
typedef struct {
	/**> The field number of the field to sort by. */
	iinq_field_num_t		field_num;
	/**> The direction of the sort. ASC is 1, DESC is -1. */
	iinq_order_direction_t	direction;
} iinq_order_by_field_t;

/**
@brief		These is the available operation types for IINQ.
*/
typedef enum IINQ_OPERATION_TYPE {
	/**> Operation to be performed is an INSERT. */
	iinq_insert_t,
	/**> Operation to be performed is a DELETE. */
	iinq_delete_t,
	/**> Operation to be performed is an UPDATE. */
	iinq_update_t
} iinq_operation_type_t;

/**
@brief		Struct defining IINQ INSERT components.
@see		prepared_iinq
*/
typedef struct prepared_iinq iinq_prepared_sql;

struct prepared_iinq {
	/**> Value parsed from the prepared statement */
	ion_value_t					value;
	/**> Key to be inserted */
	ion_key_t					key;
	/**> The table name, stored as a unique identifier */
	iinq_table_id_t				table;
	/**> Dictionary to perform the operation on */
	ion_dictionary_t			dictionary;
	/**> Handler for the dictionary */
	ion_dictionary_handler_t	handler;
	/**> Operation to be performed */
	iinq_operation_type_t		operation_type;
};

/**
 * @brief Type used for defining a where condition.
 * @see IINQ_WHERE_PARAMS
 */
typedef struct IINQ_WHERE_PARAMS iinq_where_params_t;

/**
 * @brief Type used for defining a sort operation
 */
typedef struct {
	/**> Cursor to iterate through sorted records. */
	ion_external_sort_cursor_t	*cursor;
	/**> Memory allocated for the sorted record. */
	char						*record_buf;
	/**> Size of the sort fields. */
	iinq_size_t					size;
} iinq_sort_t;

/**
 * @brief Available query operator types.
 */
typedef enum IINQ_QUERY_OPERATOR_TYPE {
	iinq_dictionary_operator_e, iinq_external_sort_e, iinq_projection_e, iinq_selection_e
} iinq_query_operator_type_t;

/**
 * @brief A query operator parent type.
 * @see IINQ_OPERATOR_PARENT
 */
typedef struct IINQ_OPERATOR_PARENT iinq_query_operator_parent_t;

/**
 * @brief A query operator instance type.
 * @see IINQ_OPERATOR
 */
typedef struct IINQ_OPERATOR iinq_query_operator_t;

typedef void (*iinq_destroy_operator_t)(
	iinq_query_operator_t **
);

struct IINQ_RESULT_SET {
	ion_status_t			status;
	iinq_query_operator_t	*head;
	iinq_query_operator_t	*tail;
};

/* typedef iinq_query_operator_t iinq_result_set_t; */
typedef struct IINQ_RESULT_SET iinq_result_set_t;

typedef ion_boolean_t (*iinq_operator_next_t)(
	iinq_query_operator_t *
);

typedef struct IINQ_OPERATOR_PARENT {
	iinq_field_num_t			num_fields;
	iinq_null_indicator_t		*null_indicators;
	iinq_field_info_t			*field_info;
	ion_value_t					*fields;
	iinq_query_operator_type_t	type;
	iinq_query_operator_t		*input_operator;
	iinq_query_operator_t		*parent_operator;
	iinq_destroy_operator_t		destroy;
};

struct IINQ_OPERATOR {
	ion_status_t					status;
	iinq_query_operator_parent_t	*instance;
};

ion_err_t
iinq_execute(
	ion_dictionary_t		*dictionary,
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

struct IINQ_WHERE_PARAMS {
	unsigned int			where_field;
	iinq_bool_operator_t	bool_operator;
	ion_value_t				field_value;
};

/**
 * @brief Function that performs a selection for updates and deletes.
 * @details Queries use a generated function instead of this one.
 *
 * @param id
 *	  Id for the table that we are performing the selection on.
 * @param record
 *	  Record that the selection will be performed on.
 * @param num_wheres
 *	  Number of conditions that the selection contains.
 * @param where
 *	  Array of conditions.
 * @return
 *	  Whether the selection was successful or not.
 */
ion_boolean_t
where(
	iinq_table_id_t		id,
	ion_record_t		*record,
	int					num_wheres,
	iinq_where_params_t *where
);

/**
 * @brief Structure for updating a field in an update statement.
 */
struct IINQ_UPDATE_PARAMS {
	/**> The field number to be updated. */
	iinq_field_num_t		update_field;
	/**> Field to be used in implicit update. 0 indicates that there is no implicit field. */
	iinq_field_num_t		implicit_field;
	/**> Operator to be used for the update. */
	iinq_math_operator_t	math_operator;
	/**> Value to use in the update. */
	ion_value_t				field_value;
};

/**
 * @brief Type used for updating a field in an update statement.
 * @see IINQ_UPDATE_PARAMS
 */
typedef struct IINQ_UPDATE_PARAMS iinq_update_params_t;

#if defined(__cplusplus)
}
#endif

#endif /* IINQ_FUNCTIONS_H */
