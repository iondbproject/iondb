/**
 * @file ffdictionary.h
 */
#ifndef FFDICTIONARY_H_
#define FFDICTIONARY_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ffdict_cursor
{
	dict_cursor_t 		super;		/**< Cursor supertype this type inherits from */
	fpos_t				current;	/**<Currently visited spot*/
	char				status;		/**@todo what is this for again as there are two status */
} ffdict_cursor_t;

#ifdef __cplusplus
}
#endif

#endif /* FFDICTIONARY_H_ */
