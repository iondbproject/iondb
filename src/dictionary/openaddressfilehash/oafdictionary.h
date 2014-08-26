/**

@details 		This file includes common components for oadictionary and oahash.

*/

#ifndef OAFDICTIONARY_H_
#define OAFDICTIONARY_H_

#include "./../dicttypes.h"

/**
@brief		The position in the hashmap.
 */
typedef int 			hash_t;

typedef struct oadict_cursor
{
	dict_cursor_t 		super;		/**< Cursor supertype this type inherits from */
	hash_t				first;		/**<First visited spot*/
	hash_t				current;	/**<Currently visited spot*/
	char				status;		/**@todo what is this for again as there are two status */
} oafdict_cursor_t;

#endif /* OADICTIONARY_H_ */
