/******************************************************************************/
/**
@file		kv_system.h
@author		Scott Fazackerley, Eric Huang, Graeme Douglas
@brief		This file contains commonly used types and some helpful macros.
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

#if !defined(KV_SYSTEM_H_)
#define KV_SYSTEM_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#include <malloc.h>
#else /* Not windows, then use the proper header */
#include <alloca.h>
#endif

/* Only on PC */
#if !defined(ARDUINO)
#define fremove(x)	remove(x)
#define frewind(x)	rewind(x)
#define fdeleteall()
#endif

#define ION_USING_MASTER_TABLE	1
#define ION_USING_ECLIPSE		0
#define ION_DEBUG				0
#define ION_IS_EQUAL			0
#define ION_IS_GREATER			1
#define ION_IS_LESS				-1
#define ION_ZERO				0
#define ION_RETURN_VALUE		0x73/* Magic default return value to be easy to spot */

/**
@brief		Since the arduino conforms to 8.3 syntax, that's 8 + 3 = 11 + 1 (null terminator) characters.
*/
#define ION_MAX_FILENAME_LENGTH 12

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

#define IONIZE(something, type)		& (type) { (something) }
#define NEUTRALIZE(something, type) (*((type *) (something)))
#define IONIZE_VAL(varname, size)	ion_byte_t varname[size]

#define UNUSED(x)					(void) ((x))

#define ION_STATUS_CREATE(error, count) \
	((ion_status_t) { (error), (count) } \
	)
#define ION_STATUS_INITIALIZE \
	((ion_status_t) { err_uninitialized, 0 } \
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
typedef enum ION_KEY_TYPE {
	/**> Key is a signed numeric value. */
	key_type_numeric_signed,
	/**> Key is an unsigned numeric value.*/
	key_type_numeric_unsigned,
	/**> Key is char array. */
	key_type_char_array,
	/**> Key is a null-terminated string.
		 Note that this needs padding out to avoid reading memory one does not own. */
	key_type_null_terminated_string,
} ion_key_type_t;

/**
@brief		This is the available dictionary types for ION_DB.  All types will be
			based on system defines.
*/
typedef enum ION_DICTIONARY_TYPE {
	/**> Dictionary type is a B+tree implementation. */
	dictionary_type_bpp_tree_t,
	/**> Dictionary type is a Flat File implementation.*/
	dictionary_type_flat_file_t,
	/**> Dictionary type is an Open Address File Hash implementation. */
	dictionary_type_open_address_file_hash_t,
	/**> Dictionary type is an Open Address Hash implementation. */
	dictionary_type_open_address_hash_t,
	/**> Dictionary type is a Skip List implementation. */
	dictionary_type_skip_list_t,
	/**> Dictionary type is a Linear Hash implementation. */
	dictionary_type_linear_hash_t,
	/**> Dictionary type is not initialized. */
	dictionary_type_error_t
} ion_dictionary_type_t;

/**
@brief		The list of all possible errors generated by any IonDB
			operation.
*/
enum ION_ERROR {
	/**> An error code describing the situation where everything is OK. */
	err_ok,
	/**> An error code describing the situation where an item is not found. */
	err_item_not_found,
	/**> An error code describing the situation where duplicate key is used
		 inappropriately. */
	err_duplicate_key,
	/**> An error code describing the situation where a structure is asked
		 to grow beyond it's capacity. */
	err_max_capacity,
	/**> An error code describing the situation where an error occurred in
		 destroying a dictionary. */
	err_dictionary_destruction_error,
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
	/**> An error code describing the situation where an insert operation could
		 not be completed. */
	err_unable_to_insert,
	/**> An error code describing the situation where a seek operation could
		 not be made to completion. */
	err_file_bad_seek,
	/**> An error code describing the situation where a file operation hit
		 the EOF. */
	err_file_hit_eof,
	/**> An error code describing the situation where a requested operation
		 is not implemented. */
	err_not_implemented,
	/**> An error code describing the situation where specified size is
		 illegal, invalid, or otherwise unreasonable. */
	err_invalid_initial_size,
	/**> An error code returned when a dictionary of the same name as
		 an existing dictionary is attempted to be created. */
	err_duplicate_dictionary_error,
	/**> An error code describing the situation a system object or dictionary
		 was not properly initialized. */
	err_uninitialized,
	/**> An error code describing the situation where something is out of
		 valid bounds. */
	err_out_of_bounds,
	/**> An error code describing the situation where an operation would
		 violate the sorted precondition. */
	err_sorted_order_violation
};

/**
@brief		The error type used to store error codes.
@details	This allows us to have finite control over the size of error codes
			returned.
*/
typedef char ion_err_t;

/**
@brief		A byte type.
@details	This is an unsigned, integral value capable of holding the range of a byte.
*/
typedef unsigned char ion_byte_t;

/**
@brief		A file position type.
*/
typedef long ion_fpos_t;
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
typedef unsigned int ion_dictionary_size_t;

/**
@brief		A boolean type.
*/
typedef char ion_boolean_t;

/**
@brief		Boolean values.
*/
typedef enum ION_BOOLEAN {
	/**> Truthy boolean value. */
	boolean_false	= 0,
	/**> Falsy boolean value. */
	boolean_true	= 1,
} ion_boolean_e;

/**
@brief		A type for the number of results changed during an operation.
*/
typedef int ion_result_count_t;

/**
@brief		A status object that describes the result of a dictionary
			operation.
*/
typedef struct {
	ion_err_t			error;
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
} ion_record_info_t;

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
