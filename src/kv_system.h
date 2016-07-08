/******************************************************************************/
/**
@file
@author		Scott Fazackerley, Eric Huang, Graeme Douglas
@brief		This file contains commonly used types and some helpful macros.
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see AUTHORS.md)
@par
			Licensed under the Apache License, Version 2.0 (the "License");
			you may not use this file except in compliance with the License.
			You may obtain a copy of the License at
					http://www.apache.org/licenses/LICENSE-2.0
@par
			Unless required by applicable law or agreed to in writing,
			software distributed under the License is distributed on an
			"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
			either express or implied. See the License for the specific
			language governing permissions and limitations under the
			License.
@todo		Resolve issue between status_t and err_t. Status_t is a struct that should have and err_t and number of item affected.
*/
/******************************************************************************/
#if !defined(KV_SYSTEM_H_)
#define KV_SYSTEM_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kv_io.h"

#define ION_USING_MASTER_TABLE	1
#define USING_ECLIPSE			0
#define DEBUG					0
#define IS_EQUAL				0
#define IS_GREATER				1
#define IS_LESS					-1
#define ZERO					0

/* ==================== ARDUINO CONDITIONAL COMPILATION ================================ */
#if !defined(ARDUINO)
/* Only if we're on desktop do we want to flush. Otherwise we only do a printf. */
#define DUMP(varname, format)	printf("Variable %s = " format "\n", # varname, varname);fflush(stdout)
#define PANIC(stuff)			printf("\t\t%s\n", stuff);fflush(stdout)
#else
#define DUMP(varname, format)	printf("Variable %s = " format "\n", # varname, varname)
#define PANIC(stuff)			printf("\t\t%s\n", stuff)
#endif /* Clause ARDUINO */

/* ==================== PC CONDITIONAL COMPILATION ===================================== */
#if !defined(ARDUINO)

/**
@brief		A byte type. Do not use this type directly.
@details	This type is only provided for compatibility with the Arduino. If you want a
			byte type, use @ref ion_byte_t instead.
*/
typedef unsigned char byte;

#endif
/* ===================================================================================== */

#define IONIZE(something, type)		(ion_key_t) &(type) { (something) }
#define NEUTRALIZE(something, type) (*((type *) (something)))
#define IONIZE_VAL(varname, size)	ion_byte_t varname[size]

#define UNUSED(x)					(void) ((x))

#define ION_STATUS_CREATE(error, count) \
	((ion_status_t) { (error), (count) } \
	)
#define ION_STATUS_INITIALIZE \
	((ion_status_t) { err_status_uninitialized, 0 } \
	)
#define ION_STATUS_ERROR(error) \
	((ion_status_t) { (error), 0 } \
	)
#define ION_STATUS_OK(count) \
	((ion_status_t) { err_ok, (count) } \
	)

/**
@brief		This is the available key types for ION_DB.  All types will be
			based on system defines.
*/
typedef enum key_type {
	/**> Key is a signed numeric value. */
	key_type_numeric_signed,
	/**> Key is an unsigned numeric value.*/
	key_type_numeric_unsigned,
	/**> Key is char array. */
	key_type_char_array,
	/**> Key is a null-terminated string.
		 Note that this needs padding out to avoid reading memory one does not own. */
	key_type_null_terminated_string,
} key_type_t;

/**
@brief		The list of all possible errors generated by any IonDB
			operation.
@todo		Change colllection -> dictionary.
*/
enum error {
	/**> An error code describing the situation where everything is OK. */
	err_ok,
	/**> An error code describing the situation where the status has not
		 been initialized yet. */
	err_status_uninitialized,
	/**> An error code describing the situation where an item is not found. */
	err_item_not_found,
	/**> An error code describing the situation where duplicate key is used
		 inappropariately. */
	err_duplicate_key,
	/**> An error code describing the situation where a structure is asked
		 to grow beyond it's capacity. */
	err_max_capacity,
	/**> An error code describing the situation where a configuration setting
		 for a write concern is problematic. */
	err_write_concern,
	/**> An error code describing the situation where an error occured in
		 destroying a dictionary. */
	err_colllection_destruction_error,
	/**> An error code describing the situation where a predicate is invalid. */
	err_invalid_predicate,
	/**> An error code describing the situation where a memory allocation
		 could not occur due to the system being out of allocatable memory. */
	err_out_of_memory,
	/**> An error code describing the situation where a write operation
		 has failed. */
	err_file_write_error,
	/**> An error code describing the situation where a read operation
		 has failed. */
	err_file_read_error,
	/**> An error code describing the situation where an open operation
		 has failed. */
	err_file_open_error,
	/**> An error code describing the situation where an close operation
		 has failed. */
	err_file_close_error,
	/**> An error code describing the situation where an delete operation
		 has failed. */
	err_file_delete_error,
	/**> An error code describing the situation where a dictionary has failed
		 has failed to initialize. */
	err_dictionary_initialization_failed,
	/**> An error code describing the situation where an insert operation could
		 not be completed. */
	err_unable_to_insert,
	/**> An error code describing the situation where a write operation could
		 not be made to completion. */
	err_file_incomplete_write,
	/**> An error code describing the situation where a read operation could
		 not be made to completion. */
	err_file_incomplete_read,
	/**> An error code describing the situation where a seek operation could
		 not be made to completion. */
	err_file_bad_seek,
	/**> An error code describing the situation where a requested item could
		 not be found in the primary page. */
	err_not_in_primary_page,
	/**> An error code describing the situation where a requested operation
		 is not implemented. */
	err_not_implemented,
	/**> An error code describing the situation where a system object has
		 been encountered in an illegal state. */
	err_illegal_state,
	/**> An error code describing the situation where specified size is
		 illegal, invalid, or otherwise unreasonable. */
	err_invalid_initial_size,
	/**> An error code describing the situation a system object was not
		 properly initialized. */
	err_uninitialized
};

/**
@brief		The error type used to store error codes.
@details	This allows us to have finite control over the size of error codes
			returned.
*/
typedef char err_t;

/**
@brief		A byte type.
@details	This is an unsigned, integral value capable of holding the range of a byte.
*/
typedef unsigned char ion_byte_t;

/**
@brief		A dictionary key.
*/
typedef void *ion_key_t;

/**
@brief		A dictionary value.
*/
typedef void *ion_value_t;

/**
@brief		The size (length) of a dictionary key in bytes.
*/
typedef int ion_key_size_t;

/**
@brief		The size (length) of a dictionary value in bytes.
*/
typedef int ion_value_size_t;

/**
@brief		The implementation specific size of the dictionary.
@details	Each implementation uses the dictionary size variable
			differently. It may be important to preserve the ability
			for variables of this type to become negative.
*/
typedef int ion_dictionary_size_t;

/**
@brief		A boolean type.
*/
typedef char boolean_t;

/**
@brief		Boolean values.
*/
typedef enum {
	/**> Truthy boolean value. */
	boolean_false	= 0,
	/**> Falsy boolean value. */
	boolean_true	= 1,
} boolean_e;

/**
@brief		A type for the number of results changed during an operation.
*/
typedef int ion_result_count_t;

/**
@brief		A status object that describes the result of a dictionary
			operation.
*/
typedef struct {
	err_t				error;
	/**< The error code. */
	ion_result_count_t	count;	/**< The number of items affected. */
} ion_status_t;

/**
@brief		A type for record statuses used in some structures.
@details	This is used in at least the open address hash tables and
			the flat file dictionaries.
*/
typedef char ion_record_status_t;

/**
@brief		Struct used to maintain information about size of key and value.
*/
typedef struct record_info {
	ion_key_size_t		key_size;
	/**< the size of the key in bytes */
	ion_value_size_t	value_size;		/**< the size of the value in bytes */
} record_info_t;

/**
@brief		Struct used to maintain key and value.
*/
typedef struct ion_record {
	ion_key_t	key;
	/**< pointer to a key */
	ion_value_t value;	/**< a pointer to value */
} ion_record_t;

#if defined(__cplusplus)
}
#endif

#endif /* KV_SYSTEM_H_ */
