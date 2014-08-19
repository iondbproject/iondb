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
			&(test->dictionary),
			test->key_type,
			test->key_size,
			test->value_size,
			test->dictionary_size
		);
	
	CuAssertTrue(tc, err_ok == error);
	CuAssertTrue(
		tc, ((dictionary_parent_t *)(
			test->dictionary.instance)
		    )->record.key_size
				==
			 test->key_size);
	CuAssertTrue(
		tc, ((dictionary_parent_t *)(
			test->dictionary.instance)
		    )->record.value_size
				==
			 test->value_size);
}

void
dictionary_test_insert_get(
	generic_test_t	*test,
	int		num_to_insert,
	CuTest		*tc
)
{
	srand(time(NULL));
	
	if (0 > num_to_insert ||
	    num_to_insert * test->key_size > INT_MAX - 1000 ||
	    num_to_insert * test->value_size > INT_MAX - 1000
	)
	{
		num_to_insert = 10;
	}
	
	byte		keys[num_to_insert * test->key_size];
	byte		vals[num_to_insert * test->value_size];
	ion_value_t	test_val;
	err_t		error;
	
	int		i;
	int		j;
	
	for (i = 0; i < num_to_insert; i++)
	{
		for (j = 0; j < test->key_size; j++)
		{
			keys[j]	= 0x0;
		}
		for (j = 0; j < test->value_size; j++)
		{
			vals[j]	= 0;
		}
		
		j		= test->key_size;
		if (j > sizeof(int))
			j	= sizeof(int);
		
		memcpy(
			&keys[(i*test->key_size)],
			&i,
			j
		);
		
		j		= sizeof(int);
		if (j > test->value_size)
			j 	= test->value_size;
		
		memcpy(
			&vals[(i*test->value_size)],
			&i,
			j
		);
		
		error	= dictionary_insert(
				&(test->dictionary),
				&keys[(i*test->key_size)],
				&vals[(i*test->value_size)]
			);
		
		CuAssertTrue(tc, err_ok == error);
	}
	
	for (i = 0; i < num_to_insert; i++)
	{
		error	= dictionary_get(
				&(test->dictionary),
				&keys[(i*test->key_size)],
				&test_val
			);
		
		CuAssertTrue(tc, err_ok == error);
		
		j	= memcmp(
				&(vals[i*test->value_size]),
				test_val,
				test->value_size
			);
		
		free(test_val);
		
		CuAssertTrue(tc, 0 == j);
	}
}

void
dictionary_test_delete(
	generic_test_t	*test,
	ion_key_t	key_to_delete,
	boolean_t	free_value,
	CuTest		*tc
)
{
	err_t		error;
	ion_value_t	test_val;
	
	error	= dictionary_delete(
			&(test->dictionary),
			key_to_delete
		);
	
	CuAssertTrue(tc, err_ok == error);
	
	error	= dictionary_get(
			&(test->dictionary),
			key_to_delete,
			&test_val
		);
	
	if (free_value)
		free(test_val);
	
	CuAssertTrue(tc, err_item_not_found == error);
}

void
dictionary_test_update(
	generic_test_t	*test,
	ion_key_t	key_to_update,
	ion_value_t	update_with,
	CuTest		*tc
)
{
	err_t		error;
	ion_value_t	test_val;
	
	error	= dictionary_update(
			&(test->dictionary),
			key_to_update,
			update_with
		);
	
	CuAssertTrue(tc, err_ok == error);
	
	error	= dictionary_get(
			&(test->dictionary),
			key_to_update,
			&test_val
		);
	
	CuAssertTrue(tc, err_ok == error);
	
	CuAssertTrue(tc, 0 == memcmp(update_with, test_val, test->value_size));
	
	free(test_val);
}
