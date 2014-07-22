/**
 @todo	Resolve issue between status_t and err_t.  Status_t is a struct that should have
 	 	 and err_t and number of item affected.
 */
#ifndef SYSTEM_H_
#define SYSTEM_H_

#define USING_ECLIPSE 	1
#define DEBUG 			0
#define IS_EQUAL 		0
#define ZERO			0

#define DUMP(varname, format) printf("Variable %s = " format "\n", #varname, varname)

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

// TODO
/**
@brief		A boolean.
@todo		Move to a common types file.
*/
typedef char						boolean_t;

typedef enum
{
	true,
	false,
} boolean_e;

#endif /* SYSTEM_H_ */
