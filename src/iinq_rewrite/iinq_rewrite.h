#ifndef PLANCK_UNIT_IINQ_REWRITE_H
#define PLANCK_UNIT_IINQ_REWRITE_H

#include "../iinq/iinq.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef void **ion_iinq_tuple_data_t;
typedef int	ion_iinq_tuple_size_t;

enum IINQ_FIELD_TYPE {
	IINQ_INT,
	IINQ_DOUBLE,
	IINQ_STRING
};

typedef uint8_t ion_iinq_field_type_t;

typedef struct {
	ion_iinq_tuple_data_t		fields;
	int							num_fields;
	ion_iinq_field_type_t		*field_type;
	ion_iinq_tuple_size_t 		size;
} ion_iinq_tuple_t;

typedef struct iinq_rewrite_source {
	ion_dictionary_t 	dictionary;
	ion_dict_cursor_t 	*cursor;
	ion_record_t 		record;
	char 				name[10];
} ion_iinq_rewrite_source_t;

ion_err_t
iinq_rewrite_open_source(
		char 						*name,
		ion_iinq_rewrite_source_t 	*source
);

enum IINQ_ITERATOR_STATUS {
	it_status_ok,
	it_status_invalid
};

typedef int ion_iinq_iterator_status_t;

typedef struct iterator *ion_iinq_iterator_t;

typedef ion_iinq_tuple_t*(*ion_iinq_iterator_next_t)(ion_iinq_iterator_t);

struct iterator {
	ion_iinq_rewrite_source_t 	*sources;
	ion_external_sort_cursor_t 	*es;
	ion_iinq_iterator_status_t 	iterator_status;
	ion_iinq_iterator_next_t	next;
	ion_iinq_tuple_t			tuple;
};

ion_iinq_iterator_t
init(ion_boolean_t field_list);

#if defined(__cplusplus)
}
#endif

#endif //PLANCK_UNIT_IINQ_REWRITE_H
