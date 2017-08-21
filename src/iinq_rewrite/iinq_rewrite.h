/******************************************************************************/
/**
@file		iinq_rewrite.h
@author		Kai Neubauer
@brief		Function declarations and type definitions for the new version of iinq.
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

#ifndef PLANCK_UNIT_IINQ_REWRITE_H
#define PLANCK_UNIT_IINQ_REWRITE_H

#include "../iinq/iinq.h"
#include "stdarg.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Macro to improve readability of query in init function. Not currently implemented */
#define IINQ_QUERY(select_clause, from_clause, where_clause, having_clause, order_by_clause, group_by_clause) select_clause, from_clause, where_clause, having_clause, order_by_clause, group_by_clause
/* Empty clause to be used in the case of missing clauses (e.g. ORDER BY was not given). */
#define IINQ_EMPTY_CLAUSE NULL

/* Makes the select part of the clause easier to read */
#define IINQ_SELECT(type, ...)            IF_ELSE(type)(IINQ_FIELD_LIST(__VA_ARGS__))(NULL)
#define IINQ_SELECT_ALL 0
#define IINQ_SELECT_FIELD_LIST 1

/* Makes the where clause easier to read */
#define IINQ_WHERE_CLAUSE(...) (iinq_where_filter_t[]) {__VA_ARGS__}
#define IINQ_WHERE_FILTER(source_num, field_num, operator, comp_value)    { source_num, field_num, operator, comp_value }

#define FLOAT_TOLERANCE                0.00001

/* Methods to get column values from a tuple */
#define get_int(iterator, field_num)    (NEUTRALIZE((iterator).query->tuple.fields[(field_num)], int))
#define get_double(iterator, field_num)    (NEUTRALIZE((iterator).query->tuple.fields[(field_num)], double))
#define get_string(iterator, field_num)    ((char*)((iterator).query->tuple.fields[(field_num)]))

#define IINQ_ORDER_BY_ASC        1
#define IINQ_ORDER_BY_DESC        -1

#define IINQ_FIELD_LIST(...)        (iinq_field_list_t[]) {__VA_ARGS__}

typedef enum IINQ_RETRIEVAL_LOCATION {
	IINQ_FROM_TABLE,
	IINQ_FROM_GROUP_BY,
	IINQ_FROM_ORDER_BY
} iinq_retrieval_location_t;

/**
@brief		Comparison operators available for a filter in the WHERE clause.
*/
enum IINQ_COMPARISON_OPERATOR {
	IINQ_EQUAL_TO,                // ==
	IINQ_NOT_EQUAL_TO,            // !=
	IINQ_GREATER_THAN,            // >
	IINQ_GREATER_EQUAL_TO,        // >=
	IINQ_LESS_THAN,               // <
	IINQ_LESS_THAN_EQUAL_TO,      // <=
};

/**
@brief		A type for the comparison operator used in a filter.
@see		enum IINQ_COMPARISON_OPERATOR
*/
typedef uint8_t iinq_comparison_operator_t;

typedef union {
	int *int_val;
	unsigned int *uint_val;
	double *double_val;
	char *string_val;
} iinq_comparison_pointer_t;

typedef struct {
	char *cur_key; /**< Memory allocated for the group by fields. */
	char *old_key; /**< Memory allocated for the group by fields. */
	char *record_buf; /**< Memory allocated for the record. */
	FILE *input_file; /**< Pointer to the file containing the grouped records */
	iinq_size_t group_by_size; /**< Size of the group by fields. */
	iinq_size_t record_size; /**< Size of the record stored in the group by file */
} iinq_group_by_t;

/**
@brief		A type containing information about a filter in the WHERE clause.
*/
typedef struct {
	int source_num; /**< The index of the source used in the comparion. */
	int field_num; /**< The field number within the source that will be compared. */
	iinq_comparison_operator_t comparison_operator; /**< The operator that will be used for the comparison. */
	iinq_comparison_pointer_t comp_value; /**< Generic pointer to the value that will be compared. */
} iinq_where_filter_t;

/**
@brief		A type for a field from a table.
@details	Points to the address of the data for a field.
*/
typedef void *iinq_field_t;

/**
@brief		A type for an array of fields that make up a tuple.
*/
typedef iinq_field_t *iinq_tuple_data_t;

/**
@brief		A type for the total size of the tuple data.
*/
typedef unsigned int iinq_tuple_size_t;

/**
@brief		A type for referencing a field from a given table.
*/
typedef struct {
	int table_num; /**< The table number containing the field. Table index start at 0. */
	int field_num; /**< The field number within the table. Field index start at 0. */
} iinq_field_list_t;

/**
@brief		Data types for fields available in iinq.
*/
typedef enum IINQ_FIELD_TYPE {
	IINQ_INT, /**< Integer data type. */
	IINQ_UINT, /**< Unsigned integer data type. */
	IINQ_DOUBLE, /**< Double data type. */
	IINQ_STRING /**< String data type. */
} iinq_field_type_t;

/**
@brief		Type for a SELECT clause in iinq.
*/
typedef uint8_t iinq_select_type_t;

/**
@brief		Type for the size of a field in iinq.
*/
typedef unsigned int iinq_field_size_t;

/**
@brief		Type for the number of a fields in a query.
*/
typedef unsigned int iinq_num_fields_t;

/**
@brief		Type for schema information of a table used in iinq.
*/
typedef struct {
	iinq_num_fields_t num_fields; /**< The number of fields in the schema */
	iinq_field_type_t *field_type; /**< An array of the data types in the schema.*/
	iinq_field_size_t *field_size; /**< An array of the field sizes in the schema.*/
} iinq_schema_t;

/**
@brief		Function to create a table for an iinq query.
@details	Creates a dictionary file that is tracked by an iinq file. The dictionary is created using the master table.
@param		schema_file_name
 The name of the schema.
@param		key_type
 The key type used for IonDB functionality.
@param		key_size
 The key size used for IonDB functionality.
@param		value_size
 The key size used for IonDB functionality. The value will be a byte array of all tuple fields except for the key.
@param		schema
 The schema that the table follows. The key is always considered to be the first column.
@return
 The result of creating the table.
*/
ion_err_t
iinq_create_table(
		char *schema_file_name,
		ion_key_type_t key_type,
		ion_key_size_t key_size,
		ion_value_size_t value_size,
		iinq_schema_t *schema
);

/**
@brief		Type for a tuple in iinq.
*/
typedef struct {
	iinq_tuple_data_t fields; /**< Field data that is contained in the tuple. */
	iinq_schema_t *schema; /**< The schema that the tuple follows. */
	iinq_tuple_size_t size; /**< The total size of the fields in the tuple. */
	ion_boolean_t is_select_all; /**< Incicates whether the tuple was the result of a SELECT * query */
} iinq_tuple_t;

/**
@brief		Type for a table in iinq.
@details	This type represents what a table would look like in a database.
*/
typedef struct {
	ion_dictionary_t *dictionary; /**< Contains the data for the table. */
	ion_dict_cursor_t *cursor; /**< Cursor to iterate through the dictionary for a query. */
	ion_record_t record; /**< Record for the cursor's next function. */
	iinq_schema_t *schema; /**< Schema that the table follows. */
} iinq_table_t;

/**
@brief		Function to open an existing table for a query in iinq.
@param[in]	name
 Name of the table to open.
@param[out] table
 Pointer to the opened table.
@return
 The result of opening the table.
*/
ion_err_t
iinq_open_table(
		char *name,
		iinq_table_t *table
);

/**
@brief		Status types for an iterator in iinq.
*/
enum IINQ_ITERATOR_STATUS {
	it_status_ok,  /**< Iterator is valid. */
	it_status_sort_error, /**< Iterator encountered a sorting error. */
	it_status_end_of_results, /**< Iterator reached the end of the query. */
	it_status_memory_error, /**< Iterator failed to allocate enough memory to continue. */
	it_status_invalid /**< General case when iterator is invalid. */
};

/**
@brief		Type for the status of an iterator in iinq.
@see		enum IINQ_ITERATOR_STATUS
*/
typedef uint8_t iinq_iterator_status_t;

/**
@brief		Type for the sort portion of a query in iinq.
*/
typedef struct {
	ion_external_sort_cursor_t *cursor;  /**< Cursor to iterate through sorted records. */
	char *record_buf; /**< Memory allocated for the sorted record. */
	iinq_size_t size; /**< Size of the sort fields. */
} iinq_sort_t;

typedef ion_boolean_t (*iinq_predicate_t)(struct iterator *);

/**
@brief		Type for the sort portion of a query in iinq.
*/
typedef struct {
	iinq_table_t *tables; /**< Pointer to the table tables for the queries. */
	int num_tables; /**< Number of tables for the query. */
	iinq_group_by_t *group_by; /**< Pointer to members used in the GROUP BY clause */
	iinq_sort_t *sort; /**< Pointer to the sort used for the ORDER BY clause. */
	/* TODO: remove the filter (softcoded) member and use predicate (hardcoded) instead */
	iinq_where_filter_t *filter; /**< Pointer to the filters used for the WHERE clause. */
	int num_filters; /**< Number of filters used for the query. */
	iinq_select_type_t select_type; /**< Type of SELECT clause. */
	iinq_tuple_t tuple; /**< Tuple that contains a record from the query. */
	iinq_predicate_t predicate; /**< Predicate function for the query. */
} iinq_query_t;

/**
@brief		Function pointer for the iterator's next method.
*/
typedef iinq_iterator_status_t (*iinq_iterator_next_t)(struct iterator *);

/**
@brief		Iterator type used for executing queries.
*/
typedef struct iterator {
	iinq_query_t *query; /**< Contains the members necessary to execute the query. */
	iinq_iterator_status_t status;/**< State of the iterator. */
	iinq_iterator_next_t next;/**< Function to move the iterator forward by one row. */
} iinq_iterator_t;

/**
@brief		Types of ORDER BY clauses available in iinq.
*/
enum ORDER_BY_TYPE {
	IINQ_ORDER_BY_NONE,
	IINQ_ORDER_BY_FIELD,
	IINQ_ORDER_BY_EXPR
};

/**
@brief		Types for an ORDER BY clause in iinq.
*/
typedef uint8_t iinq_order_by_type_t;

/**
@brief		Type for detailing an ORDER BY for a field.
*/
typedef struct {
	iinq_field_list_t field_info; /**< The table and field number of the field to sort by. */
	iinq_order_direction_t direction; /**< The direction of the sort. ASC is 1, DESC is -1. */
} iinq_order_by_field_t;

/**
@brief		Initializer function for an iterator. Only used for testing, smaller init functions should be used for actual queries.
@param[out]	it
 Initialized iterator is assigned to this pointer.
@param[in] select_type
 The select type of the query.
@param[in] order_by_type
 The order by type of the query.
@param[in] num_tables
 The number of tables in the query.
@param[in] num_filters
 The number of filters in the query.
@param[in] ...
 Other parameters if required by the query. If the query does not require a parameter, an error may occur if it is left in.
 The order of the parameters is as follows:
 	char *table_name
 		Name of table to open for query (each table is a separate argument),
 	iinq_where_filter_t * filter
 		Where filter for the query (each filter is a separate argument),
 	int num_fields
 		Number of fields in the field list for the SELECT clause,
 	iinq_field_list_t *field_list
 		Array of field information for the SELECT clause,
 	iinq_order_by_field_t *order_by_field
 		Pointer to information about which field to order by,

@return
 The status of the created iterator.
*/
iinq_iterator_status_t
query_init(
		iinq_iterator_t *it,
		iinq_select_type_t select_type,
		iinq_order_by_type_t order_by_type,
		ion_boolean_t has_group_by,
		int num_tables,
		int num_filters,
		...
);

void
iinq_destroy_iterator(
		iinq_iterator_t *it
);

void
iinq_init_tuple_from_table_field_list(iinq_iterator_t *it, int num_fields, iinq_field_list_t *field_list);

iinq_iterator_status_t
iinq_query_init_select_all_from_table(iinq_iterator_t *it, char *table_name, iinq_iterator_next_t next,
									  iinq_predicate_t predicate);

iinq_iterator_status_t
iinq_query_init_select_field_list_from_table(iinq_iterator_t *it, char *table_name, iinq_iterator_next_t next,
											 iinq_predicate_t predicate, int num_fields,
											 iinq_field_list_t *field_list);

iinq_iterator_status_t
iinq_next_from_table_no_predicate(iinq_iterator_t *it);

iinq_iterator_status_t
iinq_next_from_table_with_predicate(iinq_iterator_t *it);

ion_err_t
drop_table(char *schema_file_name);

ion_status_t
iinq_insert_into(
		char *schema_file_name,
		ion_key_t key,
		ion_value_t value);

#define DROP_TABLE(table_name) drop_table(# table_name ".inq")
#define INSERT_INTO(schema_name, key, value) iinq_insert_into(# schema_name ".inq", key, value)

void
iinq_print_tuple(
		iinq_iterator_t *it
);

#if defined(__cplusplus)
}
#endif

#endif //PLANCK_UNIT_IINQ_REWRITE_H