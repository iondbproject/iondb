/**
 @todo	Resolve issue between status_t and err_t.  Status_t is a struct that should have
 	 	 and err_t and number of item affected.
 */
#ifndef SYSTEM_H_
#define SYSTEM_H_

#define USING_ECLIPSE 1
#define DEBUG 1
#define IS_EQUAL 0

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
typedef char						*ion_key_t;

/**
@brief		A dictionary value.
*/
typedef char						*ion_value_t;

// TODO
/**
@brief		A boolean.
@todo		Move to a common types file.
*/
typedef char						boolean_t;


#endif /* SYSTEM_H_ */
