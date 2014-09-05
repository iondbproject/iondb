/******************************************************************************/
/**
@file		slhandler.c
@author		Kris Wallperington
@brief		Unit tests for skiplist handler interface.
*/
/******************************************************************************/

#include "sltests.h"

/**
@brief 		Helper function that constructs a sample dictionary based on the
			given parameters.

@param 		dictionary
				Dictionary to initialize
@param 		handler
				Handler to bind
@param 		record
				Secondary information within dictionary
@param 		key_type
				Type of key used
@param 		size
				Size of dictionary
 */
void
create_test_collection(
	dictionary_t			*dictionary,
	dictionary_handler_t 	*handler,
	record_info_t 			*record,
	key_type_t 				key_type,
	int						size,
	int 					num_elements
)
{
	sldict_init(handler);

	dictionary_create(
			handler,
			dictionary,
			key_type,
			record->key_size,
			record->value_size,
			size
	);

	/* Populate dictionary */
	char* 	value = "DATA";
	int 	i;
	for(i = 0; i < num_elements; i++)
	{
		dictionary_insert(dictionary, (ion_key_t) &i, (ion_value_t) value);
	}
}

/**
@brief 		Helper function to create a collection using standard condition
			variables.

@param 		dictionary
				Dictionary to initialize
@param 		handler
				Handler to bind
 */
void
create_test_collection_std_conditions(
	dictionary_t 			*dictionary,
	dictionary_handler_t 	*handler
)
{
	/* This means keysize 4 and valuesize 10 */
	record_info_t 			record 			= {4, 10};
	key_type_t 				key_type 		= key_type_numeric_signed;
	int 					size 			= 7;
	int 					num_elements 	= 100;

	create_test_collection(
			dictionary,
			handler,
			&record,
			key_type,
			size,
			num_elements
	);
}

/**
@brief 		Tests the creation of a handler and verifies all function pointers
			have been correctly bound.

@param 		tc
				CuTest dependency
 */
void
test_collection_handler_binding(
	CuTest 		*tc
)
{
	PRINT_HEADER();
	dictionary_handler_t 	handler;
	sldict_init(&handler);

	CuAssertTrue(tc, handler.insert				== &sldict_insert);
	CuAssertTrue(tc, handler.create_dictionary	== &sldict_create_dictionary);
	CuAssertTrue(tc, handler.update				== &sldict_update);
	CuAssertTrue(tc, handler.delete				== &sldict_delete);
	CuAssertTrue(tc, handler.delete_dictionary	== &sldict_delete_dictionary);
}

/**
@brief 		Tests the creation of a collection and verifies all properties
			have been correctly initialized.

@param 		tc
				CuTest dependency
 */
void
test_collection_creation(
	CuTest 		*tc
)
{
	PRINT_HEADER();
	dictionary_t 			dict;
	dictionary_handler_t 	handler;
	record_info_t 			record 			= {4,10};
	key_type_t 				key_type 		= key_type_numeric_signed;
	int 					size 			= 50;
	int 					num_elements 	= 25;

	create_test_collection(
			&dict,
			&handler,
			&record,
			key_type,
			size,
			num_elements
	);
}

/**
@brief 		Creates the suite to test using CuTest.
@return 	Pointer to a CuTest suite.
 */
CuSuite*
skiplist_handler_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	/* Creation tests */
	SUITE_ADD_TEST(suite, test_collection_handler_binding);

	return suite;
}

/**
@brief 		Runs all skiplist related tests and outputs the result.
 */
void
runalltests_skiplist_handler()
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= skiplist_handler_getsuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("----\nSkiplist Handler Tests:\n%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);
}
