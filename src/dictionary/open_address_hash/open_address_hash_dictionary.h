/**

@details		This file includes common components for oadictionary and oahash.
*/

#if !defined(ION_OPEN_ADDRESS_DICTIONARY_H_)
#define ION_OPEN_ADDRESS_DICTIONARY_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../dictionary_types.h"

/**
@brief		The position in the hashmap.
*/
typedef int ion_hash_t;

typedef struct oadict_cursor {
	ion_dict_cursor_t	super;			/**< Cursor supertype this type inherits from */
	ion_hash_t			first;			/**<First visited spot*/
	ion_hash_t			current;		/**<Currently visited spot*/
	char				status;		/*todo what is this for again as there are two status */
} ion_oadict_cursor_t;

#if defined(__cplusplus)
}
#endif

#endif /* OPEN_ADDRESS_DICTIONARY_H_ */
