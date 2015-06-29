/**

@details 		This file includes common components for oadictionary and oahash.

*/

#ifndef LHDICTIONARY_H_
#define LHFDICTIONARY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "./../dicttypes.h"

/**
@brief		The position in the hashmap.
 */
typedef int 			hash_t;

/**@ todo update this so it works with lh */
typedef struct lhdict_cursor
{
	dict_cursor_t 		super;		/**< Cursor supertype this type inherits from */
	hash_t				first;		/**<First visited spot*/
	hash_t				current;	/**<Currently visited spot*/
	char				status;		/**@todo what is this for again as there are two status */
} lhdict_cursor_t;

#ifdef __cplusplus
}
#endif

#endif /* LHDICTIONARY_H_ */
