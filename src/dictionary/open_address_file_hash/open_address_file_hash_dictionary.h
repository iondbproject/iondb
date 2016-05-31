/**

@details 		This file includes common components for oadictionary and oahash.

*/

#if !defined(OPEN_ADDRESS_FILE_DICTIONARY_H_)
#define OPEN_ADDRESS_FILE_DICTIONARY_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../dictionary_types.h"

/**redefines file operations for arduino */
#include "./../../file/SD_stdio_c_iface.h"

/**
@brief		The position in the hashmap.
 */
typedef int 			hash_t;

typedef struct oafdict_cursor
{
	dict_cursor_t 		super;		/**< Cursor supertype this type inherits from */
	hash_t				first;		/**<First visited spot*/
	hash_t				current;	/**<Currently visited spot*/
	char				status;		/**@todo what is this for again as there are two status */
} oafdict_cursor_t;

#if defined(__cplusplus)
}
#endif

#endif /* OPEN_ADDRESS_FILE_DICTIONARY_H_ */
