#if !defined(GENERIC_DICTIONARY_TEST_H_)
#define GENERIC_DICTIONARY_TEST_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>
#include <limits.h>
#include "../../planckunit/src/planck_unit.h"
#include "../../../key_value/kv_system.h"
#include "../../../dictionary/dictionary_types.h"
#include "./../../../dictionary/dictionary.h"
#include "./../../../dictionary/ion_master_table.h"

#define GTEST_DATA "Test data, please ignore! 123 123 abc abc"

int
get_count_index_by_key(
	ion_key_t		needle,
	ion_key_t		*haystack,
	int				length,
	ion_dictionary_t	*dictionary
);

typedef struct generic_test {
	ion_dictionary_t dictionary;

	void (*init_dict_handler)(
		ion_dictionary_handler_t *
	);

	ion_key_type_t				key_type;
	ion_key_size_t			key_size;
	ion_value_size_t		value_size;
	ion_dictionary_size_t	dictionary_size;
	ion_dictionary_handler_t	handler;
} ion_generic_test_t;

void
init_generic_dictionary_test(
	ion_generic_test_t			*test,
	void (					*init_dict_handler)(ion_dictionary_handler_t *),
	ion_key_type_t				key_type,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size,
	ion_dictionary_size_t	dictionary_size
);

void
cleanup_generic_dictionary_test(
	ion_generic_test_t *test
);

void
dictionary_test_init(
	ion_generic_test_t		*test,
	planck_unit_test_t	*tc
);

void
dictionary_test_insert_get(
	ion_generic_test_t		*test,
	int					num_to_insert,
	ion_key_t			*keys,
	ion_result_count_t	*counts,
	int					length,
	planck_unit_test_t	*tc
);

void
dictionary_test_insert_get_edge_cases(
	ion_generic_test_t		*test,
	ion_key_t			*keys,
	ion_result_count_t	*counts,
	int					length,
	planck_unit_test_t	*tc
);

void
dictionary_test_delete(
	ion_generic_test_t		*test,
	ion_key_t			key_to_delete,
	ion_result_count_t	count,
	planck_unit_test_t	*tc
);

void
dictionary_test_update(
	ion_generic_test_t		*test,
	ion_key_t			key_to_update,
	ion_value_t			update_with,
	ion_result_count_t	count,
	planck_unit_test_t	*tc
);

void
dictionary_test_equality(
	ion_generic_test_t		*test,
	ion_key_t			eq_key,
	planck_unit_test_t	*tc
);

void
dictionary_test_range(
	ion_generic_test_t		*test,
	ion_key_t			lower_bound,
	ion_key_t			upper_bound,
	planck_unit_test_t	*tc
);

void
dictionary_test_all_records(
	ion_generic_test_t		*test,
	int					expected_count,
	planck_unit_test_t	*tc
);

void
dictionary_test_open_close(
	ion_generic_test_t		*test,
	planck_unit_test_t	*tc
);

#if defined(__cplusplus)
}
#endif

#endif
