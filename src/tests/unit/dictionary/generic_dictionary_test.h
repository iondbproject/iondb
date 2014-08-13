
#ifndef GENERIC_DICTIONARY_TEST_H
#define GENERIC_DICTIONARY_TEST_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "./../../CuTest.h"
#include "./../../../kv_system.h"
#include "./../../../dictionary/dicttypes.h"
#include "./../../../dictionary/dictionary.h"

typedef struct generic_test
{
	dictionary_t		*dictionary;
	void			(*init_dict_handler)(dictionary_handler_t *);
	key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;
	ion_dictionary_size_t	dictionary_size;
	dictionary_handler_t	handler;
} generic_test_t;

void
init_generic_dictionary_test(
	generic_test_t		*test,
	void			(*init_dict_handler)(dictionary_handler_t *),
	key_type_t		key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size,
	ion_dictionary_size_t	dictionary_size
);

void
dictionary_test_init(
	generic_test_t		*test,
	CuTest			*tc
);

#ifdef  __cplusplus
}
#endif

#endif
