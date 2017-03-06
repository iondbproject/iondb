/*INDENT-OFF*/
#if !defined(IINQ_H_)
#define IINQ_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../dictionary/dictionary_types.h"
#include "../dictionary/ion_master_table.h"

typedef unsigned int ion_iinq_result_size_t;

typedef struct {
	ion_iinq_result_size_t	num_bytes;
	unsigned char		*data;
} ion_iinq_result_t;

/**
@brief		Function pointer type for processing not data modifying IINQ
			queries.
*/
typedef	void	(*ion_iinq_query_processor_func_t)(ion_iinq_result_t*, void*);

#define IINQ_NEW_PROCESSOR_FUNC(name) \
void name(ion_iinq_result_t *result, void* state)

typedef struct {
	ion_iinq_query_processor_func_t	execute;
	void						*state;
} ion_iinq_query_processor_t;

#define IINQ_QUERY_PROCESSOR(execute, state)	((ion_iinq_query_processor_t){ execute, state })

typedef struct iinq_source ion_iinq_source_t;

typedef struct iinq_cleanup {
	ion_iinq_source_t		*reference;
	struct iinq_cleanup *next;
	struct iinq_cleanup *last;

} ion_iinq_cleanup_t;

struct iinq_source {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dictionary;
	ion_predicate_t				predicate;
	ion_dict_cursor_t			*cursor;
	ion_cursor_status_t			cursor_status;
	ion_key_t				key;
	ion_value_t				value;
	ion_record_t			ion_record;
	ion_iinq_cleanup_t			cleanup;
};

ion_err_t
iinq_create_source(
		char					*schema_file_name,
		ion_key_type_t				key_type,
		ion_key_size_t			key_size,
		ion_value_size_t		value_size
);

ion_err_t
iinq_open_source(
		char					*schema_file_name,
		ion_dictionary_t			*dictionary,
		ion_dictionary_handler_t	*handler
);

ion_status_t
iinq_insert(
		char 		*schema_file_name,
		ion_key_t	key,
		ion_value_t value
);

ion_status_t
iinq_update(
		char 		*schema_file_name,
		ion_key_t	key,
		ion_value_t value
);

ion_status_t
iinq_delete(
		char 		*schema_file_name,
		ion_key_t	key
);

ion_err_t
iinq_drop(
		char *schema_file_name
);

#define CREATE_DICTIONARY(schema_name, key_type, key_size, value_size) \
iinq_create_source(#schema_name ".inq", key_type, key_size, value_size)

#define INSERT(schema_name, key, value) \
iinq_insert(#schema_name ".inq", key, value)

#define UPDATE(schema_name, key, value) \
iinq_insert(#schema_name ".inq", key, value)

#define DELETE_FROM(schema_name, key) \
iinq_delete(#schema_name ".inq", key)

#define DROP(schema_name)\
iinq_drop(#schema_name ".inq")

#define SELECT_ALL \
ion_iinq_result_size_t result_loc	= 0; \
ion_iinq_cleanup_t *copyer			= first; \
while (NULL != copyer) { \
	memcpy(result.data+(result_loc), copyer->reference->key, copyer->reference->dictionary.instance->record.key_size); \
	result_loc += copyer->reference->dictionary.instance->record.key_size; \
	memcpy(result.data+(result_loc), copyer->reference->value, copyer->reference->dictionary.instance->record.value_size); \
	result_loc += copyer->reference->dictionary.instance->record.value_size; \
	copyer						= copyer->next; \
}

#define _FROM_SOURCE_SINGLE(source) \
	ion_iinq_source_t source; \
	source.cleanup.next			= NULL; \
	source.cleanup.last			= last; \
	source.cleanup.reference	= &source; \
	if (NULL == first) { \
		first					= &source.cleanup; \
	} \
	if (NULL != last) { \
		last->next				= &source.cleanup; \
	} \
	last						= &source.cleanup; \
	source.cleanup.next			= NULL; \
	source.dictionary.handler	= &source.handler; \
	error						= iinq_open_source(#source ".inq", &(source.dictionary), &(source.handler)); \
	if (err_ok != error) { \
		break; \
	} \
	source.key					= alloca(source.dictionary.instance->record.key_size); \
	source.value				= alloca(source.dictionary.instance->record.value_size); \
	source.ion_record.key		= source.key; \
	source.ion_record.value		= source.value; \
	result.num_bytes			+= source.dictionary.instance->record.key_size; \
	result.num_bytes			+= source.dictionary.instance->record.value_size; \
	error						= dictionary_build_predicate(&(source.predicate), predicate_all_records); \
	if (err_ok != error) { \
		break; \
	} \
	dictionary_find(&source.dictionary, &source.predicate, &source.cursor);

#define _FROM_CHECK_CURSOR_SINGLE(source) \
	(cs_cursor_active == (source.cursor_status = source.cursor->next(source.cursor, &source.ion_record)) || cs_cursor_initialized == source.cursor_status)

#define _FROM_ADVANCE_CURSORS \
		if (NULL == ref_cursor) { \
			break; \
		} \
		last_cursor		= ref_cursor; \
		/* Keep going backwards through sources until we find one we can advance. If we re-initialize any cursors, reset ref_cursor to last. */ \
		while (NULL != ref_cursor && (cs_cursor_active != (ref_cursor->reference->cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor, &ref_cursor->reference->ion_record)) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) { \
			ref_cursor->reference->cursor->destroy(&ref_cursor->reference->cursor); \
			dictionary_find(&ref_cursor->reference->dictionary, &ref_cursor->reference->predicate, &ref_cursor->reference->cursor); \
			if ((cs_cursor_active != (ref_cursor->reference->cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor, &ref_cursor->reference->ion_record)) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) { \
				goto IINQ_QUERY_CLEANUP; \
			} \
			ref_cursor	= ref_cursor->last; \
		} \
		if (NULL == ref_cursor) { \
			break; \
		} \
		else if (last_cursor != ref_cursor) { \
			ref_cursor	= last; \
		}

/* Here we define a number of FROM macros to facilitate up to 8 sources. */
#define _FROM_SOURCE_1(_1) _FROM_SOURCE_SINGLE(_1)
#define _FROM_SOURCE_2(_1, _2) _FROM_SOURCE_1(_1) _FROM_SOURCE_1(_2)
#define _FROM_SOURCE_3(_1, _2, _3) _FROM_SOURCE_2(_1, _2) _FROM_SOURCE_1(_3)
#define _FROM_SOURCE_4(_1, _2, _3, _4) _FROM_SOURCE_3(_1, _2, _3) _FROM_SOURCE_1(_4)
#define _FROM_SOURCE_5(_1, _2, _3, _4, _5) _FROM_SOURCE_4(_1, _2, _3, _4) _FROM_SOURCE_1(_5)
#define _FROM_SOURCE_6(_1, _2, _3, _4, _5, _6) _FROM_SOURCE_5(_1, _2, _3, _4, _5) _FROM_SOURCE_1(_6)
#define _FROM_SOURCE_7(_1, _2, _3, _4, _5, _6, _7) _FROM_SOURCE_6(_1, _2, _3, _4, _5, _6) _FROM_SOURCE_1(_7)
#define _FROM_SOURCE_8(_1, _2, _3, _4, _5, _6, _7, _8) _FROM_SOURCE_7(_1, _2, _3, _4, _5, _6, _7) _FROM_SOURCE_1(_8)
/*
 * The last parameter, the variable arguments, is a black whole to swallow unused macro names.
 */
#define _FROM_SOURCE_GET_OVERRIDE(_1, _2, _3, _4, _5, _6, _7, _8, MACRO, ...) MACRO
/*
 * So this one is pretty ugly.
 *
 * We "slide" the correct macro based on the number of arguments. At the end, we add a comma so that we don't get a
 * compiler warning when only passing in only ONE argument into the variable argument list.
*/
#define _FROM_SOURCES(...) _FROM_SOURCE_GET_OVERRIDE(__VA_ARGS__, _FROM_SOURCE_8, _FROM_SOURCE_7, _FROM_SOURCE_6, _FROM_SOURCE_5, _FROM_SOURCE_4, _FROM_SOURCE_3, _FROM_SOURCE_2, _FROM_SOURCE_1, THEBLACKWHOLE)(__VA_ARGS__)

#define _FROM_CHECK_CURSOR(sources) \
	_FROM_CHECK_CURSOR_SINGLE(sources)

#define FROM(...) \
	ion_iinq_cleanup_t	*first; \
	ion_iinq_cleanup_t	*last; \
	ion_iinq_cleanup_t	*ref_cursor; \
	ion_iinq_cleanup_t	*last_cursor; \
	first		= NULL; \
	last		= NULL; \
	ref_cursor	= NULL; \
	last_cursor	= NULL; \
	_FROM_SOURCES(__VA_ARGS__) \
	result.data	= alloca(result.num_bytes); \
	ref_cursor	= first; \
	/* Initialize all cursors except the last one. */ \
	while (ref_cursor != last) { \
		if (NULL == ref_cursor || (cs_cursor_active != (ref_cursor->reference->cursor_status = ref_cursor->reference->cursor->next(ref_cursor->reference->cursor, &ref_cursor->reference->ion_record)) && cs_cursor_initialized != ref_cursor->reference->cursor_status)) { \
			break; \
		} \
		ref_cursor = ref_cursor->next; \
	} \
	ref_cursor	= last; \
	while (1) { \
		_FROM_ADVANCE_CURSORS
/*if (!_FROM_CHECK_CURSOR(__VA_ARGS__)) {*/ \
		/*	break; */ \
		/*}*/

#define WHERE(condition) (condition)

#define QUERY(select, from, where, groupby, having, orderby, limit, when, p) \
do { \
	ion_err_t			error; \
	ion_iinq_result_t	result; \
	result.num_bytes	= 0; \
	from/* This includes a loop declaration with some other stuff. */ \
		if (!where) { \
			continue; \
		} \
		select \
		(p)->execute(&result, (p)->state); \
	} \
	IINQ_QUERY_CLEANUP: \
	while (NULL != first) { \
		first->reference->cursor->destroy(&first->reference->cursor); \
		ion_close_dictionary(&first->reference->dictionary); \
		first			= first->next; \
	}\
} while (0);

#if defined(__cplusplus)
}
#endif

#endif
/*INDENT-ON*/