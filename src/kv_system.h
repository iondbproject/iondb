/**
 @todo	Resolve issue between status_t and err_t.  Status_t is a struct that should have
 	 	 and err_t and number of item affected.
 */
#ifndef SYSTEM_H_
#define SYSTEM_H_

// #define ION_ARDUINO
#define USING_ECLIPSE 	0
// #define DEBUG 			1
#define IS_EQUAL 		0
#define ZERO			0

/* Only if we're on desktop do we want to flush. Otherwise we only do a printf. */
#ifndef ION_ARDUINO
#define DUMP(varname, format) printf("Variable %s = " format "\n", #varname, varname); fflush(stdout)
#else
#define DUMP(varname, format) printf("Variable %s = " format "\n", #varname, varname)
#endif /* Clause ION_ARDUINO */

#ifndef ION_ARDUINO
#define PANIC(stuff) printf("\t\t%s\n", stuff); fflush(stdout)
#else
#define PANIC(stuff) printf("\t\t%s\n", stuff)
#endif /* Clause ION_ARDUINO */

#define IONIZE(x) ({volatile typeof(x) _tmp = x; (ion_key_t) &_tmp; })
#define NEUTRALIZE(type, x) ( *((type *) x) )



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum status
{
	status_ok,            				/**< status_ok*/
	status_item_not_found,				/**< status_item_not_found */
	status_duplicate_key, 				/**< status_duplicate_key */
};

typedef char status_t;

#ifndef ION_ARDUINO
typedef unsigned char byte;
#endif

/**
This is the available key types for ION_DB.  All types will be based on system
defines.
@todo Discuss with Graeme
*/
typedef enum key_type
{
	/*key_type_char,						*< key is char
	key_type_uchar,						*< key is unsigned char
	key_type_short,						*< key is short
	key_type_ushort,					*< key is unsigned char
	key_type_int,						*< key is int
	key_type_uint,						*< key is unsigned int
	key_type_long,						*< key is long
	key_type_ulong,						*< key is unsigned long */
	key_type_numeric_signed,			/**< key is a signed numeric value */
	key_type_numeric_unsigned,			/**< key is a signed numeric value */
	key_type_char_array,				/**< key is char array (string) */
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
	err_dictionary_initialization_failed,
	err_could_not_delete_file,
	err_could_not_insert,
	err_unimplemented,
	err_file_incomplete_write,
	err_file_incomplete_read,
	err_file_bad_seek,
};

typedef char err_t;

/**
@brief		A dictionary key.
*/
typedef unsigned char				*ion_key_t;

/**
@brief		A dictionary value.
*/
typedef unsigned char						*ion_value_t;

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

typedef struct return_status{
	err_t	err;						/**< the error code */
	int		count;						/**< the number of items affected */
} return_status_t;

/**
@brief		Struct used to maintain information about size of key and value.
 */
typedef struct record_info
{
	int 			key_size;			/**< the size of the key in bytes */
	int 			value_size;			/**< the size of the value in bytes */
} record_info_t;

/**
@brief		Struct used to maintain key and value.
 */
typedef struct ion_record
{
	ion_key_t 			key;			/**< pointer to a key */
	ion_value_t			value;			/**< a pointer to value */
} ion_record_t;

#endif /* SYSTEM_H_ */