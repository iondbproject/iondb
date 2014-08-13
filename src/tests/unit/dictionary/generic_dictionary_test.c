#include "generic_dictionary_test.h"

void
init_generic_dictionary_test(
	generic_test_t		*test,
	void			(*init_dict_handler)(dictionary_handler_t *),
	key_type_t		key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size,
	ion_dictionary_size_t	dictionary_size
)
{
	test->init_dict_handler	= init_dict_handler;
	test->key_type		= key_type;
	test->key_size		= key_size;
	test->value_size	= value_size;
	test->dictionary_size	= dictionary_size;
}

void
dictionary_test_init(
	generic_test_t		*test,
	CuTest			*tc
)
{
	err_t			error;
	
	test->init_dict_handler(&(test->handler));
	
	error	= dictionary_create(
			&(test->handler),
			test->dictionary,
			test->key_type,
			test->key_size,
			test->value_size,
			test->dictionary_size
		);
	
	CuAssertTrue(tc, err_ok == error);
	CuAssertTrue(
		tc, ((dictionary_parent_t *)(
			test->dictionary->instance)
		    )->record.key_size
				==
			 test->key_size);
	CuAssertTrue(
		tc, ((dictionary_parent_t *)(
			test->dictionary->instance)
		    )->record.value_size
				==
			 test->value_size);
}
