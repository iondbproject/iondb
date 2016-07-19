#if !defined(IINQ_H_)
#define IINQ_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <alloca.h>
#include "../dictionary/dictionary_types.h"
#include "../dictionary/ion_master_table.h"

typedef unsigned int iinq_result_size_t;

typedef struct {
	iinq_result_size_t	num_bytes;
	unsigned char		*data;
} iinq_result_t;

/**
@brief		Function pointer type for processing not data modifying IINQ
			queries.
*/
typedef	void	(*iinq_query_processor_func_t)(iinq_result_t*, void*);

#define IINQ_NEW_PROCESSOR_FUNC(name) \
void name(iinq_result_t *result, void* state)

typedef struct {
	iinq_query_processor_func_t	execute;
	void						*state;
} iinq_query_processor_t;

#define IINQ_QUERY_PROCESSOR(execute, state)	((iinq_query_processor_t){ execute, state })

typedef struct iinq_source iinq_source_t;

typedef struct iinq_cleanup {
	iinq_source_t		*reference;
	struct iinq_cleanup *next;

} iinq_cleanup_t;

struct iinq_source {
	dictionary_handler_t	handler;
	dictionary_t			dictionary;
	predicate_t				predicate;
	dict_cursor_t			*cursor;
	cursor_status_t			cursor_status;
	ion_key_t				key;
	ion_value_t				value;
	ion_record_t			ion_record;
	iinq_cleanup_t			cleanup;
};

err_t
iinq_create_source(
	char					*schema_file_name,
	key_type_t				key_type,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size
);

err_t
iinq_open_source(
	char					*schema_file_name,
	dictionary_t			*dictionary,
	dictionary_handler_t	*handler
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

err_t
iinq_drop(
	char *schema_file_name
);

#define CREATE_DICTIONARY(schema_name, key_type, key_size, value_size) \
iinq_create_source(#schema_name ".inq", key_type, key_size, value_size)

#define INSERT(schema_name, key, value) \
iinq_insert(#schema_name ".inq", key, value)

#define UPDATE(schema_name, key, value) \
iinq_insert(#schema_name ".inq", key, value)

#define DELETE(schema_name, key) \
iinq_delete(#schema_name ".inq", key)

#define DROP(schema_name)\
iinq_drop(#schema_name ".inq")

//iinq_insert_into(#schema_name ".inq", key, value)

#define SELECT_ALL \
iinq_result_size_t result_loc	= 0; \
iinq_cleanup_t *copyer			= first; \
while (NULL != copyer) { \
	memcpy(result.data+(result_loc), copyer->reference->key, copyer->reference->dictionary.instance->record.key_size); \
	result_loc += copyer->reference->dictionary.instance->record.key_size; \
	memcpy(result.data+(result_loc), copyer->reference->value, copyer->reference->dictionary.instance->record.value_size); \
	result_loc += copyer->reference->dictionary.instance->record.value_size; \
	copyer						= copyer->next; \
}
#if 0
iinq_result_size_t result_loc	= result.num_bytes; \
iinq_cleanup_t *copyer			= last; \
while (NULL != copyer) { \
	memcpy(result.data+(result_loc - copyer->reference->dictionary.instance->record.value_size), copyer->reference->value, copyer->reference->dictionary.instance->record.value_size); \
	result_loc -= copyer->reference->dictionary.instance->record.value_size; \
	memcpy(result.data+(result_loc - copyer->reference->dictionary.instance->record.key_size), copyer->reference->key, copyer->reference->dictionary.instance->record.key_size); \
	result_loc -= copyer->reference->dictionary.instance->record.key_size; \
	copyer						= copyer->last; \
}
#endif

#define FROM_SINGLE(source) \
	iinq_source_t source; \
	source.cleanup.next			= NULL; \
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
	iinq_open_source(#source ".inq", &(source.dictionary), &(source.handler)); \
	source.key					= alloca(source.dictionary.instance->record.key_size); \
	source.value				= alloca(source.dictionary.instance->record.value_size); \
	source.ion_record.key		= source.key; \
	source.ion_record.value		= source.value; \
	result.num_bytes			+= source.dictionary.instance->record.key_size; \
	result.num_bytes			+= source.dictionary.instance->record.value_size; \
	dictionary_build_predicate(&(source.predicate), predicate_all_records); \
	dictionary_find(&source.dictionary, &source.predicate, &source.cursor);

#define FROM_CHECK_CURSOR_SINGLE(source) \
	(cs_cursor_active == (source.cursor_status = source.cursor->next(source.cursor, &source.ion_record)) || cs_cursor_initialized == source.cursor_status)

#define FROM_CHECK_CURSOR(sources) \
	FROM_CHECK_CURSOR_SINGLE(sources)

#define FROM(sources) \
	iinq_cleanup_t	*first; \
	iinq_cleanup_t	*last; \
	first	= NULL; \
	last	= NULL; \
	FROM_SINGLE(sources) \
	result.data	= alloca(result.num_bytes); \
	while (1) { \
		if (!FROM_CHECK_CURSOR(sources)) { \
			break; \
		}

#define WHERE(condition) (condition)

#define QUERY(select, from, where, groupby, having, orderby, limit, when, p) \
do { \
	ion_status_t	status; \
	err_t			error; \
	iinq_result_t	result; \
	result.num_bytes= 0; \
	from/* This includes a loop declaration with some other stuff. */ \
		else if (!where) { \
			continue; \
		} \
		select \
		(p)->execute(&result, (p)->state); \
	} \
	while (NULL != first) { \
		ion_close_dictionary(&last->reference->dictionary); \
		first		= first->next; \
	}\
} while (0);

#if defined(__cplusplus)
}
#endif

#endif
