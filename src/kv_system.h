/**
 @todo	Resolve issue between status_t and err_t.  Status_t is a struct that should have
 	 	 and err_t and number of item affected.
 */
#if !defined(KV_SYSTEM_H_)
#define KV_SYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kv_io.h"

#define ION_USING_MASTER_TABLE 	1
#define USING_ECLIPSE 			0
#define DEBUG 					0
#define IS_EQUAL 				0
#define IS_GREATER				1
#define IS_LESS					-1
#define ZERO					0
#define BAUD_RATE				9600

/* Only if we're on desktop do we want to flush. Otherwise we only do a printf. */
#if !defined(ARDUINO)
#define DUMP(varname, format) printf("Variable %s = " format "\n", #varname, varname); fflush(stdout)
#else
#define DUMP(varname, format) printf("Variable %s = " format "\n", #varname, varname)
#endif /* Clause ARDUINO */

#if !defined(ARDUINO)
#define PANIC(stuff) printf("\t\t%s\n", stuff); fflush(stdout)
#else
#define PANIC(stuff) printf("\t\t%s\n", stuff)
#endif /* Clause ARDUINO */

#define IONIZE(something, type) (ion_key_t) &(type){(something)}
#define NEUTRALIZE(something, type) ( *((type *) (something)) )
#define IONIZE_VAL(varname, size) unsigned char varname[size]

typedef char status_t;

#if !defined(ARDUINO)
typedef unsigned char byte;
#endif

/**
This is the available key types for ION_DB.  All types will be based on system
defines.
@todo Discuss with Graeme
*/
typedef enum key_type
{
	key_type_numeric_signed,			/**< Key is a signed numeric value. */
	key_type_numeric_unsigned,			/**< Key is an unsigned numeric value.*/
	key_type_char_array,				/**< Key is char array. */
	key_type_null_terminated_string,	/**< Key is a null-terminated string.
										     Note that this needs padding out
										     to avoid reading memory one does
										     not own. */
} key_type_t;

enum error
{
	err_ok,
	err_item_not_found,
	err_duplicate_key,
	err_max_capacity,
	err_write_concern,
	err_colllection_destruction_error,
	err_invalid_predicate,
	err_out_of_memory,
	err_file_write_error,
	err_file_read_error,
	err_file_open_error,
	err_file_close_error,
	err_file_delete_error,
	err_dictionary_initialization_failed,
	err_unable_to_insert,
	err_file_incomplete_write,
	err_file_incomplete_read,
	err_file_bad_seek,
	err_not_in_primary_page,
	err_not_implemented,
	err_illegal_state,
	err_invalid_initial_size,
	err_uninitialized
};

typedef char err_t;

/**
@brief		A dictionary key.
*/
typedef unsigned char				*ion_key_t;

/**
@brief		A dictionary value.
*/
typedef unsigned char				*ion_value_t;

/**
@brief		The size(length) of a dictionary key in bytes.
 */
typedef int							ion_key_size_t;

/**
@brief		The size(length) of a dictionary value in bytes.
 */
typedef int 						ion_value_size_t;
typedef int							ion_dictionary_size_t;
// TODO
/**
@brief		A boolean.
@todo		Move to a common types file.
*/
typedef char						boolean_t;

typedef enum
{
	boolean_false = 0,
	boolean_true = 1,
} boolean_e;

typedef struct {
	err_t	err;						/**< the error code */
	int		count;						/**< the number of items affected */
} ion_status_t;

/**
@brief		Struct used to maintain information about size of key and value.
 */
typedef struct record_info
{
	ion_key_size_t 			key_size;			/**< the size of the key in bytes */
	ion_value_size_t 		value_size;			/**< the size of the value in bytes */
} record_info_t;

/**
@brief		Struct used to maintain key and value.
 */
typedef struct ion_record
{
	ion_key_t 			key;			/**< pointer to a key */
	ion_value_t			value;			/**< a pointer to value */
} ion_record_t;

#endif /* KV_SYSTEM_H_ */
