/**
 * @file flat_file_dictionary.h
 */
#ifndef FLAT_FILE_DICTIONARY_H_
#define FLAT_FILE_DICTIONARY_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "../dictionary_types.h"

/**redefines file operations for arduino */
#include "./../../file/kv_stdio_intercept.h"

typedef struct ffdict_cursor
{
	dict_cursor_t 			super;		/**< Cursor supertype this type inherits from */
	ion_fpos_t				current;	/**<Currently visited spot*/
	char				status;		/**@todo what is this for again as there are two status */
} ffdict_cursor_t;

#ifdef __cplusplus
}
#endif

#endif /* FLAT_FILE_DICTIONARY_H_ */
