/**

@details		This file includes common components for oadictionary and oahash.
*/

#if !defined(OPEN_ADDRESS_FILE_DICTIONARY_H_)
#define OPEN_ADDRESS_FILE_DICTIONARY_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../dictionary_types.h"

/*edefines file operations for arduino */
#include "../../file/sd_stdio_c_iface.h"

typedef struct oafdict_cursor {
	ion_dict_cursor_t	super;			/**< Cursor supertype this type inherits from */
	ion_hash_t			first;			/**<First visited spot*/
	ion_hash_t			current;		/**<Currently visited spot*/
	char				status;		/*todo what is this for again as there are two status */
} ion_oafdict_cursor_t;

#if defined(__cplusplus)
}
#endif

#endif /* OPEN_ADDRESS_FILE_DICTIONARY_H_ */
