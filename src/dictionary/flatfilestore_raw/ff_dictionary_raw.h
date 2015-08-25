/**
 * @file ffdictionary.h
 */
#ifndef FFDICTIONARY_H_
#define FFDICTIONARY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dicttypes.h"

typedef struct ffdict_cursor
{
	dict_cursor_t 			super;		/**< Cursor supertype this type inherits from */
	uint32_t				block_address;	/**<Currently visited spot*/
	uint16_t				block_current_record;
	char					status;		/**@todo what is this for again as there are two status */
} ffdict_cursor_t;

#ifdef __cplusplus
}
#endif

#endif /* FFDICTIONARY_H_ */
