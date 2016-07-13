#if !defined(IINQ_H_)
#define IINQ_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../dictionary/dictionary_types.h"
#include "../dictionary/ion_master_table.h"

#define SELECT_ALL ;

#define FROM_SINGLE(source) \
	struct { \
		dictionary_handler_t	handler; \
		dictionary_t			dictionary; \
		predicate_t				predicate; \
		dict_cursor_t			*cursor; \
		cursor_status_t			cursor_status; \
		ion_key_t				key; \
		ion_value_t				value; \
	} source; \
	source.dictinary.handler	= &source.handler; \
	iinq_init_source(STRINGIFY(source) ".inq", &(source.handler), &(source.dictionary)); \
	source.key					= alloca(source.dictionary.instace->record.key_size); \
	source.value				= alloca(source.dictionary.instace->record.key_size); \
	dictionary_build_predicate(&(source.predicate), predicate_all_records); \
	dictionary_find(&source.dictionary, &source.predicate, &source.cursor);

#define FROM_CHECK_CURSOR_SINGLE(source) \
	(cs_cursor_active == (source.cursor_status = source.cursor->next(cursor, &ion_record)) && cs_cursor_initialized == cursor_status)

#define FROM_CHECK_CURSOR(sources) \
	FROM_CHECK_CURSOR_SINGLE(sources)

#define FROM(sources) \
	FROM_SINGLE(sources) \
	while (1) { \
		if (!FROM_CURSOR_CHECK(sources)) { \
			break; \
		}

#define WHERE(condition) (condition)

#define QUERY(select, from, where, groupby, having, orderby, limit, when, p) \
	do { \
		ion_status_t	status; \
		err_t			error; \
		from/* This includes a loop declaration with some other stuff. */ \
		else if (where) { \
			continue; \
		} \
	} \
	select \
	} \
	while (0) { \
		; \
	}

#if defined(__cplusplus)
}
#endif

#endif
