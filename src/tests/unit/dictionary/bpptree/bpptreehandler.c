
#include "./../generic_dictionary_test.h"
#include "./../../../../dictionary/bpptree/bpptreehandler.h"

void
run_bpptreehandler_generic_test_set_1(
	CuTest		*tc
)
{
	generic_test_t	test;
	
	init_generic_dictionary_test(
		&test,
		bpptree_init,
		key_type_numeric_signed,
		sizeof(int),
		sizeof(int),
		-1		/* Dictionary size, for now is unbounded. */
	);
	
	dictionary_test_init(&test, tc);
	
	dictionary_test_insert_get(
		&test,
		10000,
		tc
	);

#if 0 
	/* Temporary test code that should be removed once formal unit tests have been written. */
	dictionary_t dict = *((dictionary_t*) &test);

	dictionary_insert(&dict, IONIZE(5),  IONIZE(3));
	dictionary_insert(&dict, IONIZE(5),  IONIZE(4));
	dictionary_insert(&dict, IONIZE(6),  IONIZE(5));
	dictionary_insert(&dict, IONIZE(12), IONIZE(6));
	dictionary_insert(&dict, IONIZE(13), IONIZE(7));
	dictionary_insert(&dict, IONIZE(14), IONIZE(8));
	dictionary_insert(&dict, IONIZE(15), IONIZE(9));

    ion_key_t   leq     = IONIZE(5);
    ion_key_t	geq 	= IONIZE(9);
    dict_cursor_t *cursor = NULL;
    predicate_t predicate;
    predicate.type = predicate_range;
    predicate.statement.range.leq_value = leq;
    predicate.statement.range.geq_value = geq;
    dict.handler->find(&dict, &predicate, &cursor);

    ion_record_t record;
    record.key      = (ion_key_t) malloc(dict.instance->record.key_size);
    record.value    = (ion_value_t) malloc(dict.instance->record.value_size);
    while(cursor->next(cursor, &record) != cs_end_of_results)
    {
        printf("key: %d value: %d\n", NEUTRALIZE(int, record.key), NEUTRALIZE(int, record.value));
    }
#endif

	int to_delete[] = {7, -9, 32, 1000001};
	int i;
	
	
	for (i = 0; i < sizeof(to_delete)/sizeof(int); i++)
	{
		dictionary_test_delete(
			&test,
			(ion_key_t)(&(to_delete[i])),
			tc
		);
	}
	
	int update_key;
	int update_value;
	
	update_key	= 1;
	update_value	= -12;
	
	dictionary_test_update(
		&test,
		(ion_key_t)(&update_key),
		(ion_value_t)(&update_value),
		tc
	);
	
	update_key	= 1;
	update_value	= 12;
	
	dictionary_test_update(
		&test,
		(ion_key_t)(&update_key),
		(ion_value_t)(&update_value),
		tc
	);
	
	update_key	= 12;
	update_value	= 1;
	
	dictionary_test_update(
		&test,
		(ion_key_t)(&update_key),
		(ion_value_t)(&update_value),
		tc
	);

	
	dictionary_delete_dictionary(&(test.dictionary));
}

CuSuite*
bpptreehandler_get_suite(
)
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, run_bpptreehandler_generic_test_set_1);

	return suite;
}

void
run_all_tests_bpptreehandler(
)
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= bpptreehandler_get_suite();
	
	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
	
	CuSuiteDelete(suite);
	CuStringDelete(output);
}
