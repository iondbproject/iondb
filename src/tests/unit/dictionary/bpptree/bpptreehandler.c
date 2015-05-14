
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

	dictionary_t dict = *((dictionary_t*) &test);

	dictionary_insert(&dict, IONIZE(5), IONIZE(3));
	dictionary_insert(&dict, IONIZE(5), IONIZE(4));

    ion_key_t   key     = IONIZE(5);
    dict_cursor_t *cursor = NULL;
    predicate_t predicate;
    predicate.type = predicate_equality;
    predicate.statement.equality.equality_value = key;
    dict.handler->find(&dict, &predicate, &cursor);

    ion_record_t record;
    record.key      = (ion_key_t) malloc(dict.instance->record.key_size);
    record.value    = (ion_value_t) malloc(dict.instance->record.value_size);

    while(cursor->next(cursor, &record) != cs_end_of_results)
    {
        printf("key: %d value: %d\n", NEUTRALIZE(int, record.key), NEUTRALIZE(int, record.value));
    }

	// hey kill me once you are sure that method works!!!
#if 0
	bpptree_t *bpptree = (bpptree_t *) ((dictionary_t *) (&test))->instance;
	file_offset_t offset = -1;
	file_offset_t offset2 = -1;
	int key1 = 0;
	int key2 = 0;
	bErrType err1 = bFindFirstGreaterOrEqual(bpptree->tree, &key1, &offset);
	bErrType err2 = bFindKey(bpptree->tree, &key2, &offset2);

	printf("%ld == %ld\n", offset, offset2);
	printf("%d == %d\n", err1, err2);fflush(stdout);
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
