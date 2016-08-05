/* *INDENT-OFF* */
#if !defined(IINQ_H_)
#define IINQ_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <alloca.h>
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
	void							*state;
} ion_iinq_query_processor_t;

#define IINQ_QUERY_PROCESSOR(execute, state)	((ion_iinq_query_processor_t){ execute, state })

typedef struct iinq_source ion_iinq_source_t;

typedef struct iinq_cleanup {
	ion_iinq_source_t	*reference;
	struct iinq_cleanup *next;
	struct iinq_cleanup *last;

} ion_iinq_cleanup_t;

struct iinq_source {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dictionary;
	ion_predicate_t				predicate;
	ion_dict_cursor_t			*cursor;
	ion_cursor_status_t			cursor_status;
	ion_key_t					key;
	ion_value_t					value;
	ion_record_t				ion_record;
	ion_iinq_cleanup_t			cleanup;
};

typedef enum {
	IINQ_AGGREGATE_UNINITIALIZED = 0,
	IINQ_AGGREGATE_INITIALIZED = 1
} iinq_aggregate_status_e;

typedef enum {
	IINQ_AGGREGATE_TYPE_INT,
	IINQ_AGGREGATE_TYPE_UINT,
	IINQ_AGGREGATE_TYPE_DOUBLE
} iinq_aggregate_type_e;

typedef uint8_t iinq_aggregate_status_t;
typedef uint8_t iinq_aggregate_type_t;

typedef union {
	int64_t		i64;
	uint64_t	u64;
	double 		f64;
} iinq_aggregate_value_t;

typedef struct {
	iinq_aggregate_type_t	type;
	iinq_aggregate_status_t	status;
	iinq_aggregate_value_t	value;
} iinq_aggregate_t;

typedef size_t iinq_size_t;

typedef int8_t iinq_order_direction_t;

typedef struct {
	void					*pointer;
	iinq_size_t				size;
	iinq_order_direction_t	direction;
} iinq_order_part_t;

ion_err_t
iinq_create_source(
	char						*schema_file_name,
	ion_key_type_t				key_type,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size
);

ion_err_t
iinq_open_source(
	char						*schema_file_name,
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

/* Some helper macros. See http://jhnet.co.uk/articles/cpp_magic */
#define SECOND(a, b, ...) b

#define IS_PROBE(...) SECOND(__VA_ARGS__, 0, 0)
#define PROBE() ~, 1

#define CAT(a,b) a ## b

#define NOT(x) IS_PROBE(CAT(_NOT_, x))
#define _NOT_0 PROBE()

#define BOOL(x) NOT(NOT(x))

#define IF_ELSE(condition) _IF_ELSE(BOOL(condition))
#define _IF_ELSE(condition) CAT(_IF_, condition)

#define _IF_1(...) __VA_ARGS__ _IF_1_ELSE
#define _IF_0(...)             _IF_0_ELSE

#define _IF_1_ELSE(...)
#define _IF_0_ELSE(...) __VA_ARGS__

#define PP_NARG(...) \
    PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
    PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
     _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
    _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
    _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
    _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
    _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
    _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
    _61,_62,_63,  N, ...) N
#define PP_RSEQ_N() \
    63,62,61,60,                   \
    59,58,57,56,55,54,53,52,51,50, \
    49,48,47,46,45,44,43,42,41,40, \
    39,38,37,36,35,34,33,32,31,30, \
    29,28,27,26,25,24,23,22,21,20, \
    19,18,17,16,15,14,13,12,11,10, \
     9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define DEFINE_SCHEMA(source_name, struct_def) \
struct iinq_ ## source_name ## _schema struct_def

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

#define SELECT_ALL \
ion_iinq_result_size_t result_loc	= 0; \
ion_iinq_cleanup_t *copyer			= first; \
while (NULL != copyer) { \
	memcpy(result.data+(result_loc), copyer->reference->key, copyer->reference->dictionary.instance->record.key_size); \
	result_loc += copyer->reference->dictionary.instance->record.key_size; \
	memcpy(result.data+(result_loc), copyer->reference->value, copyer->reference->dictionary.instance->record.value_size); \
	result_loc += copyer->reference->dictionary.instance->record.value_size; \
	copyer							= copyer->next; \
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

#define _FROM_WITH_SCHEMA_SINGLE(source) \
	struct iinq_ ## source ## _schema *source ## _tuple; \
	source ## _tuple = source.value;

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

/*
 * The last parameter, the variable arguments, is a black whole to swallow unused macro names.
 */
#define _FROM_SOURCE_GET_OVERRIDE(_1, _2, _3, _4, _5, _6, _7, _8, MACRO, ...) MACRO
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
 * So this one is pretty ugly.
 *
 * We "slide" the correct macro based on the number of arguments. At the end, we add a comma so that we don't get a
 * compiler warning when only passing in only ONE argument into the variable argument list.
*/
#define _FROM_SOURCES(...) _FROM_SOURCE_GET_OVERRIDE(__VA_ARGS__, _FROM_SOURCE_8, _FROM_SOURCE_7, _FROM_SOURCE_6, _FROM_SOURCE_5, _FROM_SOURCE_4, _FROM_SOURCE_3, _FROM_SOURCE_2, _FROM_SOURCE_1, THEBLACKHOLE)(__VA_ARGS__)

/* Here we define a number of FROM macros to facilitate up to 8 sources, with schemas. */
#define _FROM_SOURCE_WITH_SCHEMA_1(_1) _FROM_SOURCE_SINGLE(_1) _FROM_WITH_SCHEMA_SINGLE(_1)
#define _FROM_SOURCE_WITH_SCHEMA_2(_1, _2) _FROM_SOURCE_WITH_SCHEMA_1(_1) _FROM_SOURCE_WITH_SCHEMA_1(_2)
#define _FROM_SOURCE_WITH_SCHEMA_3(_1, _2, _3) _FROM_SOURCE_WITH_SCHEMA_2(_1, _2) _FROM_SOURCE_WITH_SCHEMA_1(_3)
#define _FROM_SOURCE_WITH_SCHEMA_4(_1, _2, _3, _4) _FROM_SOURCE_WITH_SCHEMA_3(_1, _2, _3) _FROM_SOURCE_WITH_SCHEMA_1(_4)
#define _FROM_SOURCE_WITH_SCHEMA_5(_1, _2, _3, _4, _5) _FROM_SOURCE_WITH_SCHEMA_4(_1, _2, _3, _4) _FROM_SOURCE_WITH_SCHEMA_1(_5)
#define _FROM_SOURCE_WITH_SCHEMA_6(_1, _2, _3, _4, _5, _6) _FROM_SOURCE_WITH_SCHEMA_5(_1, _2, _3, _4, _5) _FROM_SOURCE_WITH_SCHEMA_1(_6)
#define _FROM_SOURCE_WITH_SCHEMA_7(_1, _2, _3, _4, _5, _6, _7) _FROM_SOURCE_WITH_SCHEMA_6(_1, _2, _3, _4, _5, _6) _FROM_SOURCE_WITH_SCHEMA_1(_7)
#define _FROM_SOURCE_WITH_SCHEMA_8(_1, _2, _3, _4, _5, _6, _7, _8) _FROM_SOURCE_WITH_SCHEMA_7(_1, _2, _3, _4, _5, _6, _7) _FROM_SOURCE_WITH_SCHEMA_1(_8)
/*
 * Like it's cousin above, this one is also ugly. We again leverage the sliding macro trick.
*/
#define _FROM_SOURCES_WITH_SCHEMA(...) _FROM_SOURCE_GET_OVERRIDE(__VA_ARGS__, _FROM_SOURCE_WITH_SCHEMA_8, _FROM_SOURCE_WITH_SCHEMA_7, _FROM_SOURCE_WITH_SCHEMA_6, _FROM_SOURCE_WITH_SCHEMA_5, _FROM_SOURCE_WITH_SCHEMA_4, _FROM_SOURCE_WITH_SCHEMA_3, _FROM_SOURCE_WITH_SCHEMA_2, _FROM_SOURCE_WITH_SCHEMA_1, THEBLACKHOLE)(__VA_ARGS__)

#define _FROM_CHECK_CURSOR(sources) \
	_FROM_CHECK_CURSOR_SINGLE(sources)

#define FROM(with_schemas, ...) \
	ion_iinq_cleanup_t	*first; \
	ion_iinq_cleanup_t	*last; \
	ion_iinq_cleanup_t	*ref_cursor; \
	ion_iinq_cleanup_t	*last_cursor; \
	first		= NULL; \
	last		= NULL; \
	ref_cursor	= NULL; \
	last_cursor	= NULL; \
	/*IF_ELSE((with_schema))(_FROM_SOURCES_WITH_SCHEMA(__VA_ARGS__))(_FROM_SOURCES(__VA_ARGS__));*/ \
	/*_FROM_SOURCES(__VA_ARGS__)*/; \
	IF_ELSE(with_schema)(_FROM_SOURCES_WITH_SCHEMA(__VA_ARGS__))(_FROM_SOURCES(__VA_ARGS__)); \
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
#define HAVING(condition) (condition)

#define QUERY_SFW(select, from, where, limit, when, p) \
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
	} \
} while (0);

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
	} \
} while (0);

#define _AGGREGATES_SETUP(n) \
	int num_agg	= (n);	\
	int i_agg	= 0;	\
	iinq_aggregates_t aggregates[num_agg]; \
	for (i_agg = 0; i_agg < num_agg; i_agg++) { \
		aggregates[i_agg].initalized	= 0; \
    }

/**
@brief		This is a helper macro for users. It gives the correct value
			of the i-th aggregate, based on the aggregates value type.
*/
#define AGGREGATE(i) (IINQ_AGGREGATE_TYPE_INT == aggregates[(i)].type ? aggregates[(i)].value.i64 : (IINQ_AGGREGATE_TYPE_UINT == aggregates[(i)].type ? aggregates[(i)].value.u64 : aggregates[(i)].value.f64))

/**
@brief		This is a helper macro, where we pass in which aggregate we are
			presently working with.
*/
#define _AGGREGATE_PRE_COMPUTE(n) \
	i_agg = n

#define MAX(expr) \
	if (!(aggregates[i_agg].initialized) || ((double)(expr)) > aggregates[i_agg].value.f64) { \
		aggregates[i_agg].f64 = ((double)(expr)); \
	}

#define MIN(expr) \
	if (!(aggregates[i_agg].initialized) || ((double)(expr)) < aggregates[i_agg].value.f64) { \
		aggregates[i_agg].f64 = ((double)(expr)); \
	}

/*
 * The last parameter, the variable arguments, is a black whole to swallow unused macro names.
 */
#define _AGGREGATES_GET_OVERRIDE(_1, _2, _3, _4, _5, _6, _7, _8, MACRO, ...) MACRO
#define _AGGREGATES_SINGLE(compute, n) _AGGREGATE_PRE_COMPUTE(n); compute
/* Here we define a number of macros to facilitate up to 8 total aggregate expressions. */
#define _AGGREGATES_1(_1) _AGGREGATES_SINGLE(_1, 0)
#define _AGGREGATES_2(_1, _2) _AGGREGATES_1(_1) _AGGREGATES_SINGLE(_2, 1)
#define _AGGREGATES_3(_1, _2, _3) _AGGREGATES_2(_1, _2) _AGGREGATES_SINGLE(_3, 2)
#define _AGGREGATES_4(_1, _2, _3, _4) _AGGREGATES_3(_1, _2, _3) _AGGREGATES_SINGLE(_4, 3)
#define _AGGREGATES_5(_1, _2, _3, _4, _5) _AGGREGATES_4(_1, _2, _3, _4) _AGGREGATES_SINGLE(_5, 4)
#define _AGGREGATES_6(_1, _2, _3, _4, _5, _6) _AGGREGATES_5(_1, _2, _3, _4, _5) _AGGREGATES_SINGLE(_6, 5)
#define _AGGREGATES_7(_1, _2, _3, _4, _5, _6, _7) _AGGREGATES_6(_1, _2, _3, _4, _5, _6) _AGGREGATES_SINGLE(_7, 6)
#define _AGGREGATES_8(_1, _2, _3, _4, _5, _6, _7, _8) _AGGREGATES_7(_1, _2, _3, _4, _5, _6, _7) _AGGREGATES_SINGLE(_8, 7)
/*
 * Like it's cousin above, this one is also ugly. We again leverage the sliding macro trick.
*/
#define _AGGREGATES(...) _AGGREGATES_GET_OVERRIDE(__VA_ARGS__, _AGGREGATES_8, _AGGREGATES_7, _AGGREGATES_6, _AGGREGATES_5, _AGGREGATES_4, _AGGREGATES_3, _AGGREGATES_2, _AGGREGATES_1, THEBLACKHOLE)(__VA_ARGS__)
#define AGGREGATES(...) \
	_AGGREGATES_SETUP(PP_NARG(__VA_ARGS__)); \
	goto IINQ_SKIP_COMPUTE_AGGREGATES; \
	IINQ_COMPUTE_AGGREGATES: ; \
	if (/* current key is same as last key. */) { \
        _AGGREGATES(__VA_ARGS__); \
    } \
	goto IINQ_DONE_COMPUTE_AGGREGATES; \
	IINQ_SKIP_COMPUTER_AGGREGATES: ;

#define _ORDERING_SETUP(name, n) \
	int name ## _n		= (n);	\
	int i_ ## name		= 0;	\
	int total_ ## name ## _size \
						= 0; \
	iinq_ordering_part_t name ## _order_parts[(name ## _n)]; \

#define _ASCENDING_INDICATOR	 1
#define _DESCENDING_INDICATOR	-1;

/*
 * This is a macro intended to be used with numerical and boolean expressions.
 */
#define _CREATE_MEMCPY_STACK_ADDRESS_FOR_NUMERICAL_EXPRESSION(expr) ( \
    8 == sizeof(expr) ? (void *)(&(uint64_t){(expr)}) : \
    ( \
        4 == sizeof(expr) ? (void *)(&(uint32_t){(expr)}) : \
        ( \
            2 == sizeof(expr) ? (void *)(&(uint16_t){(expr)}) : (void *)(&(uint8_t){(expr)}) \
        ) \
    ) \
)

/*
 * We need the ability to treat expressions resulting in
 */
#define ASCENDING(expr)		(expr, _CREATE_MEMCPY_STACK_ADDRESS_FOR_NUMERICAL_EXPRESSION(expr), sizeof((expr)), _ASCENDING_INDICATING)
#define ASC(expr)			ASCENDING(expr)
#define DESCENDING(expr)	(expr, _CREATE_MEMCPY_STACK_ADDRESS_FOR_NUMERICAL_EXPRESSION(expr), sizeof((expr)), _DESCENDING_INDICATING)
#define DESC(expr)			DESCENDING(expr)

#define _FIRST_MACRO_TUPLE4(_1, _2, _3, _4)		_1
#define _SECOND_MACRO_TUPLE4(_1, _2, _3, _4)	_2
#define _THIRD_MACRO_TUPLE4(_1, _2, _3, _4)		_3
#define _FOURTH_MACRO_TUPLE4(_1, _2, _3, _4)	_4

#define _SETUP_ORDERBY_SINGLE(t, n) \
	orderby_order_parts[(n)].direction	= _FOURTH_MACRO_TUPLE4 t; \
	orderby_order_parts[(n)].size		= _THIRD_FIRST_MACRO_TUPLE4 t; \
	total_orderby_size					+= orderby_order_parts[(n)].size;

#define _PREPARE_ORDERING_KEY_ORDERBY_SINGLE(t, n) \
	memcpy(orderby_order_parts[(n)]

#define ORDERBY(...) \
	_ORDERING_SETUP(orderby, PP_NARG(__VAR_ARGS__)) \
	/* Setup for each ordering part. */ \
	char orderby_data[total_orderby_size]; \
	memset(orderby_data, 0, total_orderby_size); \
	IINQ_ORDERBY_PREPARE: \
	do { \
		/* Setup code. */ \
		_PREPARE_ORDERING_KEYS(__VAR_ARGS__); \
	} while(0);

#define _GROUPBY_SINGLE(_1) apple
#define _GROUPBY_1(_1) _GROUPBY_SINGLE(_1)
#define GROUPBY(...)

#define MATERIALIZED_QUERY(select, aggregate_exprs, from, where, groupby, having, orderby, limit, when, p) \
do { \
	ion_err_t			error; \
	FILE				*input_file; \
	FILE				*output_file; \
	ion_iinq_result_t	result; \
	result.num_bytes	= 0; \
	aggregates_exprs \
	from/* This includes a loop declaration with some other stuff. */ \
		if (!where) { \
			continue; \
		} \
		goto IINQ_COMPUTE_AGGREGATES; \
		IINQ_DONE_COMPUTE_AGGREGATES: ; \
	} \
	/* Group by. */ \
	/* Aggregate function loop. */ \
	while (/* Has more records. */) { \
    } \
	/* Order by. */ \
	/* Projection. */ \
	while (/* Result has more records. */) { \
		select \
		(p)->execute(&result, (p)->state); \
	} \
	IINQ_QUERY_CLEANUP: \
	while (NULL != first) { \
		first->reference->cursor->destroy(&first->reference->cursor); \
		ion_close_dictionary(&first->reference->dictionary); \
		first			= first->next; \
	} \
} while (0);

#if defined(__cplusplus)
}
#endif

#endif
/* *INDENT-ON* */
