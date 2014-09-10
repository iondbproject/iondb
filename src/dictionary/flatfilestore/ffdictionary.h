/**
 * @file ffdictionary.h
 */
#ifndef FFDICTIONARY_H_
#define FFDICTIONARY_H_

typedef struct ffdict_cursor
{
	dict_cursor_t 		super;		/**< Cursor supertype this type inherits from */
	fpos_t				current;	/**<Currently visited spot*/
	char				status;		/**@todo what is this for again as there are two status */
} ffdict_cursor_t;

#endif /* FFDICTIONARY_H_ */
