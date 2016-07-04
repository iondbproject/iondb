/**

@details		This file includes common components for oadictionary and oahash.

*/

#if !defined(LINEAR_HASH_DICTIONARY_H_)
#define LINEAR_HASH_DICTIONARY_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../dictionary_types.h"

#include "./../../file/SD_stdio_c_iface.h"

/**
@brief		The position in the hashmap.
 */
typedef char troolean_t;

/* todo update this so it works with lh */
typedef struct lhdict_cursor {
	dict_cursor_t	super;			/**< Cursor supertype this type inherits from */
	int				first_bucket;
	/**<First visited spot*/
	int				current_bucket;
	/**<current bucket being scanned */
	int				record_pntr;
	/**<next record to be emitted */
	char			status;			/*todo what is this for again as there are two status */

	troolean_t (*evaluate_predicate)(
		dict_cursor_t *,
		ion_key_t
	);

	ll_file_t *overflow;/**<Overflow file that is currently being used */
} lhdict_cursor_t;

#if defined(__cplusplus)
}
#endif

#endif /* LINEAR_HASH_DICTIONARY_H_ */
