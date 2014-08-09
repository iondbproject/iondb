/******************************************************************************/
/**
 @file	 staticarray.c
 @author	 Raffi Kudlac
 @brief	 This is the test file responsible for testing the staticarray and the sahandler.
 */
/******************************************************************************/

#include "staticarrayclone.h"

#define MAX_HASH_TEST 100
#define STD_MAP_SIZE 10

/*
 @brief		calls all the tests that tests the datastructure

 @return 	returns the suite with all the test results
 */
CuSuite*
open_address_staticarray_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	create_tests(suite);
	insert_tests(suite);
	delete_tests(suite);
	get_tests(suite);
	update_tests(suite);
	destroy_tests(suite);
	//SUITE_ADD_TEST(suite,test_key_to_index);
	//SUITE_ADD_TEST(suite, show_for_raymon);

	return suite;
}

void
runalltests_open_address_staticarray()
{
	CuString *output = CuStringNew();
	CuSuite *suite = open_address_staticarray_getsuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);
}



/*
 @brief		calls all the create tests
 @params 	*suite
 CuSuite
 */
void
create_tests(
		CuSuite *suite
)
{
	SUITE_ADD_TEST(suite, test_create);
	SUITE_ADD_TEST(suite, test_create_invalid_key_size);
	SUITE_ADD_TEST(suite, test_create_invalid_value);
	SUITE_ADD_TEST(suite, test_create_max_size);
}

/*
 @brief		Tests the create method with valid parameters
 @params 	tc
 CuTest
 */
void
test_create(
		CuTest *tc
)
{
	ion_key_size_t key_size 		= 3;
	ion_value_size_t value_size 	= 100;
	sa_max_size_t array_size 		= 100;

	static_array_t st;

	printf("This is for test_create\n");
	printf("------------------------------------------------\n");

	sa_dictionary_create(&st, key_size, value_size, array_size,
			dictionary_compare_char_array);
	printf("finished create method\nNow Clearing heap memory\n\n");

	sa_max_size_t x;
	int length 	= sizeof(bucket_t);

	for (x = 0; x < array_size; x++)
	{
		CuAssertTrue(tc, ((bucket_t *)&(st.array[x*length]))->status == EMPTY);
	}

	free(st.array);
	st.array = NULL;
	printf("\n");
}

/*
 @brief		Tests the create method with an invalid key size
 @params 	tc
 CuTest
 */
void
test_create_invalid_key_size(
		CuTest *tc
)
{
	ion_key_size_t key_size 	= 7;
	ion_key_size_t key_size2 	= -5;
	ion_value_size_t value_size = 50;
	ion_key_size_t key_size3 	= 3;
	sa_max_size_t array_size 	= ipow(256, 3) + 13;

	static_array_t st;
	printf("This is for test_create_invalid_key_size\n");
	printf("------------------------------------------------\n");
	printf("A key size of 7 was entered when the max is 3. The error was "
			"caught\n\n");

	status_t result = sa_dictionary_create(&st, key_size, value_size,
			array_size, dictionary_compare_char_array);

	status_t result2 = sa_dictionary_create(&st, key_size2, value_size,
			array_size, dictionary_compare_char_array);

	status_t result3 = sa_dictionary_create(&st, key_size3, value_size,
				array_size, dictionary_compare_char_array);

	CuAssertTrue(tc, result 	== status_key_out_of_bounds);
	CuAssertTrue(tc, result2 	== status_key_out_of_bounds);
	CuAssertTrue(tc, result3 	== status_array_size_out_of_bounds);
}

/*
 @brief		Tests the create method with its max key size and a
 	 	 	 	hudge value size
 @params 	tc
 CuTest
 */
void
test_create_max_size(
		CuTest *tc
)
{
	ion_key_size_t key_size 	= 3;
	ion_value_size_t value_size = 10000;
	sa_max_size_t array_size 	= ipow(256, 3);

	static_array_t st;
	printf("This is for test_create_max_size\n");
	printf("------------------------------------------------\n");
	sa_dictionary_create(&st, key_size, value_size, array_size,
			dictionary_compare_char_array);
	printf("finished create method\nNow Clearing heap memory\n\n");
	sa_max_size_t x;
	int length 	= sizeof(bucket_t);

	for (x = 0; x < array_size; x++)
	{
		CuAssertTrue(tc, ((bucket_t *)&(st.array[x*length]))->status == EMPTY);
	}

	free(st.array);
	st.array = NULL;
	printf("\n");
}

/*
 @brief		Tests the create method with an invalid value
 @params 	tc
 CuTest
 */
void
test_create_invalid_value(
		CuTest *tc
)
{
	ion_key_size_t key_size 	= 1;
	ion_value_size_t value_size	= -10000;
	sa_max_size_t array_size 	= 256;
	static_array_t st;

	printf("This is for test_create_invalid_params_below\n");
	printf("------------------------------------------------\n\n");
	printf("A negative value size was entered. The error was caught\n\n");
	status_t result = sa_dictionary_create(&st, key_size, value_size,
			array_size, dictionary_compare_char_array);

	CuAssertTrue(tc, st.array_size 	== st.maxelements);
	CuAssertTrue(tc, st.key_size 	== 1);
	CuAssertTrue(tc, result 		== status_incorrect_value);
}



/*
 @brief		calls all the insert tests
 @params 	*suite
 CuSuite
 */
void
insert_tests(CuSuite *suite)
{
	SUITE_ADD_TEST(suite, test_insert_valid_params);
	SUITE_ADD_TEST(suite, test_insert_invalid_index);
	SUITE_ADD_TEST(suite, test_insert_collision);
}

/*
 @brief		Tests the insert method with valid parameters
 @params 	tc
 CuTest
 */
void
test_insert_valid_params(
		CuTest *tc
)
{

	ion_key_size_t key_size 	= 1;
	ion_value_size_t value_size = 100;
	sa_max_size_t array_size 	= 256;

	static_array_t st;
	printf("This is for test_insert_valid_params\n");
	printf("------------------------------------------------\n");
	sa_dictionary_create(&st, key_size, value_size, array_size,
			dictionary_compare_char_array);
	printf("Array Created\n\n");

	unsigned char value[] 		= "This is a test insert";
	unsigned char key[] 		= "a";
	status_t result 			= sa_insert(&st, key, value);
	sa_max_size_t t 			= key_to_index(key, key_size);
	bucket_t *b 				= (bucket_t *) st.array;
	unsigned char* str 			= b[t].value;

	printf("The key is %llu\n", t);

	printf("status: %d\n", b[t].status);
	printf("%s\n\n", str);
	CuAssertTrue(tc, result 									== status_ok);
	CuAssertTrue(tc,strcmp((char*)b[t].value, (char*) value) 	== 0);
	CuAssertTrue(tc, b[t].status 								== OCCUPIED);

	free(st.array);
	st.array = NULL;
}

/*
 @brief		Tests the insert method with an invalid index
 @params 	tc
 CuTest
 */
void
test_insert_invalid_index(
		CuTest *tc
)
{
	ion_key_size_t key_size 	= 1;
	ion_value_size_t value_size = 100;
	sa_max_size_t array_size 	= 50;

	static_array_t st;
	printf("This is for test_insert_invalid_index\n");
	printf("------------------------------------------------\n");
	sa_dictionary_create(&st, key_size, value_size, array_size,
			dictionary_compare_char_array);
	printf("Array Created\n\n");

	unsigned char value[] 		= "This is a test insert";
	unsigned char key[] 		= "a";
	sa_max_size_t k 			= key_to_index(key, 1);
	status_t result 			= sa_insert(&st, key, value);

	printf("The key size is %d\nA key of '%s' was entered\n", key_size, key);
	printf("The key in integer form is %llu\n The array is %llu long\n", k,
			array_size);

	CuAssertTrue(tc, result 	== status_key_out_of_bounds);
	printf("The error was caught\n\n");

	free(st.array);
	st.array = NULL;
}

/*
 @brief		Tests the insert method with a collision
 @params 	tc
 CuTest
 */
void
test_insert_collision(
		CuTest *tc
)
{
	ion_key_size_t key_size 	= 1;
	ion_value_size_t value_size = 100;
	sa_max_size_t array_size 	= 256;

	static_array_t st;
	printf("This is for test_insert_collision\n");
	printf("------------------------------------------------\n");
	sa_dictionary_create(&st, key_size, value_size, array_size,
			dictionary_compare_char_array);
	printf("Array Created\n\n");

	unsigned char value[] 		= "This is a test insert";
	unsigned char key[] 		= "k";

	unsigned char value2[] 		= "Collision is comming";
	unsigned char key2[] 		= "k";

	status_t result 			= sa_insert(&st, key, value);
	printf("Information was inserted with the key of '%s'\n", key);
	CuAssertTrue(tc, result 	== status_ok);

	printf("More Information was inserted with the same key, '%s'\nThe error "
			"was caught and the insertion was not allowed\n\n", key);

	status_t result2 			= sa_insert(&st, key2, value2);
	CuAssertTrue(tc, result2 	== status_occupied);

	free(st.array);
	st.array 					= NULL;
}



/*
 @brief		calls all the delete tests
 @params 	*suite
 CuSuite
 */
void
delete_tests(
		CuSuite *suite
)
{
	SUITE_ADD_TEST(suite, test_delete_valid_case);
	SUITE_ADD_TEST(suite, test_delete_empty_slot);
	SUITE_ADD_TEST(suite, test_delete_invalid_keysize);
}

/*
 @brief		Tests the deleta method with valid case and params
 @params 	tc
 CuTest
 */
void
test_delete_valid_case(
		CuTest *tc
)
{
	ion_key_size_t key_size 	= 1;
	ion_value_size_t value_size	= 100;
	sa_max_size_t array_size 	= 256;

	static_array_t st;
	printf("This is for test_delete_valid_case\n");
	printf("------------------------------------------------\n");
	sa_dictionary_create(&st, key_size, value_size, array_size,
			dictionary_compare_char_array);
	printf("Array Created\n\n");

	unsigned char value[] 		= "This is a test insert";
	unsigned char key[] 		= "k";
	sa_max_size_t k 			= key_to_index(key, 1);

	status_t result 			= sa_insert(&st, key, value);
	printf("Information was inserted with the key of '%s'\n", key);
	CuAssertTrue(tc, result == status_ok);

	status_t d 					= sa_delete(&st, key);
	bucket_t *b 				= (bucket_t *) st.array;

	CuAssertTrue(tc, d 								== status_ok);
	CuAssertTrue(tc, b[k].status					== EMPTY);
	CuAssertTrue(tc, strcmp((char*)b[k].value, "")	== 0);
	printf("the value is %s \n", b[k].value);

	free(st.array);
	st.array 					= NULL;
}

/*
 @brief		Tests the delete method when deleting nothing
 @params 	tc
 CuTest
 */
void
test_delete_empty_slot(
		CuTest *tc
)
{

	ion_key_size_t key_size 	= 1;
	ion_value_size_t value_size = 100;
	sa_max_size_t array_size 	= 256;

	static_array_t st;
	printf("This is for test_delete_empty_slot\n");
	printf("------------------------------------------------\n");
	sa_dictionary_create(&st, key_size, value_size, array_size,
			dictionary_compare_char_array);
	printf("Array Created\n\n");
	unsigned char key[] 		= "k";

	status_t d = sa_delete(&st, key);
	CuAssertTrue(tc, d 			== status_empty_slot);

	free(st.array);
	st.array 					= NULL;
}

/*
 @brief		Tests the delete method with an invalid key size
 @params 	tc
 CuTest
 */
void
test_delete_invalid_keysize(
		CuTest *tc
)
{
	ion_key_size_t key_size 	= 1;
	ion_value_size_t value_size = 100;
	sa_max_size_t array_size 	= 100;

	static_array_t st;
	printf("This is for test_delete_invalid_keysize\n");
	printf("------------------------------------------------\n");
	sa_dictionary_create(&st, key_size, value_size, array_size,
			dictionary_compare_char_array);
	printf("Array Created\n\n");

	unsigned char value[] 		= "This is a test insert";
	unsigned char key[] 		= "a";
	unsigned char invalid_key[] = "k";
	sa_max_size_t k 			= key_to_index(invalid_key, 1);
	status_t result 			= sa_insert(&st, key, value);

	printf("Information was inserted with the key of '%s'\n", key);
	CuAssertTrue(tc, result 	== status_ok);

	printf("Tried to delete at index %llu when array only goes up to %llu\n\n",
			k, array_size);
	status_t d 					= sa_delete(&st, invalid_key);
	CuAssertTrue(tc, d 			== status_key_out_of_bounds);

	free(st.array);
	st.array 					= NULL;
}



/*
 @brief		calls all the get tests
 @params 	*suite
 CuSuite
 */
void
get_tests(
		CuSuite *suite
)
{
	SUITE_ADD_TEST(suite, test_get_valid_case);
	SUITE_ADD_TEST(suite, test_get_invalid_key);
	SUITE_ADD_TEST(suite, test_get_from_empty_location);
}

/*
 @brief		Tests the get method with valid parameters
 @params 	tc
 CuTest
 */
void
test_get_valid_case(
		CuTest *tc
)
{
	ion_key_size_t key_size 	= 1;
	ion_value_size_t value_size = 100;
	sa_max_size_t array_size 	= 256;

	static_array_t st;
	printf("This is for test_get_valid_case\n");
	printf("------------------------------------------------\n");
	sa_dictionary_create(&st, key_size, value_size, array_size,
			dictionary_compare_char_array);
	printf("Array Created\n\n");

	unsigned char value[] 		= "This is a test insert";
	unsigned char key[] 		= "a";
	unsigned char *stored_data;
	status_t result 			= sa_insert(&st, key, value);

	printf("Information was inserted with the key of '%s' and a value of "
			"\n '%s'\n", key, value);

	CuAssertTrue(tc, result 	== status_ok);

	status_t get_result 		= sa_get(&st, key, &stored_data);

	CuAssertTrue(tc, get_result == status_ok);
	printf("The value retrieved was: %s\n", stored_data);

	CuAssertTrue(tc, strcmp((char*) stored_data, (char*) value)
								== 0);

	printf("The memory address of the Data Structure is not returned\n\n");
	CuAssertTrue(tc, &stored_data
								!= &((bucket_t *) st.array)->value);

	free(st.array);
	st.array 					= NULL;
}

/*
 @brief		Tests the get method with invalid key size
 @params 	tc
 CuTest
 */
void
test_get_invalid_key(
		CuTest *tc
)
{
	ion_key_size_t key_size 	= 1;
	ion_value_size_t value_size = 100;
	sa_max_size_t array_size 	= 50;

	static_array_t st;
	printf("This is for test_get_invalid_key\n");
	printf("------------------------------------------------\n");
	sa_dictionary_create(&st, key_size, value_size, array_size,
			dictionary_compare_char_array);
	printf("Array Created\n\n");

	//unsigned char value[] = "This is a test insert";
	unsigned char key[] 		= "aa";
	unsigned char *stored_data;
	sa_max_size_t k 			= key_to_index(key, 1);

	status_t get_result 		= sa_get(&st, key, &stored_data);
	CuAssertTrue(tc, get_result == status_key_out_of_bounds);

	printf("The index location of %llu tried to be accessed but the max index"
			"is %llu\n\n", k, array_size);

	free(st.array);
	st.array 					= NULL;
}

/*
 @brief		Tests the get method when targeting a empty slot
 @params 	tc
 CuTest
 */
void
test_get_from_empty_location(
		CuTest *tc
)
{

	ion_key_size_t key_size 	= 1;
	ion_value_size_t value_size = 100;
	sa_max_size_t array_size 	= 256;

	static_array_t st;
	printf("This is for test_get_from_empty_location\n");
	printf("------------------------------------------------\n");
	sa_dictionary_create(&st, key_size, value_size, array_size,
			dictionary_compare_char_array);
	printf("Array Created\n\n");

	//unsigned char value[] = "This is a test insert";
	unsigned char key[] 		= "a";
	unsigned char *stored_data;
	sa_max_size_t k 			= key_to_index(key, 1);
	status_t get_result 		= sa_get(&st, key, &stored_data);

	CuAssertTrue(tc, get_result == status_empty_slot);
	printf("A value was looked for at index %llu but none was found and the "
			"error was caught\n\n", k);

	free(st.array);
	st.array 					= NULL;
}



/*
 @brief		calls all the update tests
 @params 	*suite
 CuSuite
 */
void
update_tests(
		CuSuite *suite
)
{
	SUITE_ADD_TEST(suite, test_update_valid_case);
	SUITE_ADD_TEST(suite, test_update_invalid_key);
	SUITE_ADD_TEST(suite, test_update_empty_location);
}

/*
 @brief		Tests the update method with valid parameters
 @params 	tc
 CuTest
 */
void
test_update_valid_case(
		CuTest *tc
)
{
	ion_key_size_t key_size 	= 1;
	ion_value_size_t value_size = 100;
	sa_max_size_t array_size 	= 256;

	static_array_t st;
	printf("This is for test_update_valid_case\n");
	printf("------------------------------------------------\n");
	sa_dictionary_create(&st, key_size, value_size, array_size,
			dictionary_compare_char_array);
	printf("Array Created\n\n");

	unsigned char value[] 		= "This is a test insert";
	unsigned char key[] 		= "a";
	unsigned char new_value[] 	= "The value was changed";
	sa_max_size_t k 			= key_to_index(key, 1);
	status_t result 			= sa_insert(&st, key, value);

	printf("Information was inserted with the key of '%s' and a value of \n "
			"'%s'\n", key, value);

	CuAssertTrue(tc, result 	== status_ok);

	printf("The old information was overwritten with '%s'\n", new_value);

	status_t update_result 		= sa_update(&st, key, new_value);

	CuAssertTrue(tc,
				update_result 	== status_ok);

	bucket_t *pointer 			= (bucket_t *) st.array;

	CuAssertTrue(tc,
			pointer[k].status 	== OCCUPIED);

	CuAssertTrue(tc, memcmp(pointer[k].value, new_value, value_size)
								== 0);
	printf("The information in the data structure is '%s'\n\n",
			pointer[k].value);

	free(st.array);
	st.array 					= NULL;
}

/*
 @brief		Tests the get method with an invalid key size
 @params 	tc
 CuTest
 */
void
test_update_invalid_key(
		CuTest *tc
)
{

	ion_key_size_t key_size 	= 1;
	ion_value_size_t value_size = 100;
	sa_max_size_t array_size 	= 70;

	static_array_t st;
	printf("This is for test_update_invalid_key\n");
	printf("------------------------------------------------\n");
	sa_dictionary_create(&st, key_size, value_size, array_size,
			dictionary_compare_char_array);
	printf("Array Created\n\n");

	unsigned char key[] 		= "a";
	unsigned char new_value[] 	= "The value was changed";
	sa_max_size_t k 			= key_to_index(key, 1);

	printf("index %llu was attempted to be accessed with a max array size "
			"of %llu \n", k, array_size);
	printf("The error was caught\n\n");

	status_t update_result 		= sa_update(&st, key, new_value);
	CuAssertTrue(tc, update_result
								== status_key_out_of_bounds);
	free(st.array);
	st.array 					= NULL;
}

/*
 @brief		Tests the update method when updating a empty slot
 @params 	tc
 CuTest
 */
void
test_update_empty_location(
		CuTest *tc
)
{
	ion_key_size_t key_size 	= 1;
	ion_value_size_t value_size = 100;
	sa_max_size_t array_size 	= 256;

	static_array_t st;
	printf("This is for test_update_empty_location\n");
	printf("------------------------------------------------\n");
	sa_dictionary_create(&st, key_size, value_size, array_size,
			dictionary_compare_char_array);
	printf("Array Created\n\n");

	bucket_t *pointer 			= (bucket_t *) st.array;
	unsigned char key[] 		= "a";
	unsigned char new_value[] 	= "The value was changed from nothing";
	sa_max_size_t k 			= key_to_index(key, 1);

	CuAssertTrue(tc, pointer[k].status
								== EMPTY);

	printf("the value '%s' was inserted into a blank location using the update"
			" method\n", new_value);

	status_t update_result 		= sa_update(&st, key, new_value);

	CuAssertTrue(tc, update_result
								== status_ok);

	CuAssertTrue(tc, pointer[k].status
								== OCCUPIED);

	CuAssertTrue(tc, memcmp(pointer[k].value, new_value, value_size)
								== 0);
	free(st.array);
	st.array 					= NULL;
}



/*
 @brief		calls the destroy test
 @params 	*suite
 CuSuite
 */
void
destroy_tests(
		CuSuite *suite
)
{
	SUITE_ADD_TEST(suite, test_destroy);
}

/*
 @brief		Tests the destroy method with valid parameters
 @params 	tc
 CuTest
 */
void
test_destroy(
		CuTest *tc
)
{
	ion_key_size_t key_size 	= 1;
	ion_value_size_t value_size = 100;
	sa_max_size_t array_size 	= 256;

	static_array_t st;
	printf("This is for test_update_empty_location\n");
	printf("------------------------------------------------\n");
	sa_dictionary_create(&st, key_size, value_size, array_size,
			dictionary_compare_char_array);
	printf("Array Created\n\n");

	status_t destroy_result 	= sa_destroy(&st);

	CuAssertTrue(tc, destroy_result
								== status_ok);
	CuAssertTrue(tc, st.array 	== NULL);
}



/*
@brief		Tests the key to index method
 @params 	*tc
 CuTest
 */
void
test_key_to_index(
		CuTest *tc
)
{

	printf("This is for test_key_to_index\n");
	printf("------------------------------------------------\n");
	sa_max_size_t x;
	sa_max_size_t t 	= ipow(256, 1);
	sa_max_size_t tt 	= ipow(256, 2);
	sa_max_size_t ttt 	= ipow(256, 3);
	sa_max_size_t result;
	int size;

	printf("%llu\n", t);

	for (x = 0; x < t; x++)
	{
		printf("The value of x is: %llu ", x);
		//size = sizeof(sa_max_size_t);
		size 	= 1;
		result 	= key_to_index((unsigned char *) &x, size);
		printf("The value returned is %llu The value of size is %d\n", result,
				size);
		CuAssertTrue(tc, result == x);
	}

	for (x = t; x < tt; x++)
	{
		printf("The value of x is: %llu ", x);
		//size = sizeof(sa_max_size_t);
		size 	= 2;
		result 	= key_to_index((unsigned char *) &x, size);
		printf("The value returned is %llu The value of size is %d\n", result,
				size);
		CuAssertTrue(tc, result == x);
	}

	for (x = tt; x < ttt; x++)
	{
		printf("The value of x is: %llu ", x);
		//size = sizeof(sa_max_size_t);
		size 	= 3;
		result 	= key_to_index((unsigned char *) &x, size);
		printf("The value returned is %llu The value of size is %d\n", result,
				size);
		CuAssertTrue(tc, result == x);
	}
}

//-----------------------------------------------------------------------------

//Handler Tests Below


/*
 @brief		Calls all tests relivent to the handler

 @return	returns the suite with all the test results
 */
CuSuite*
open_address_hash_handler_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, initalize_hash_handler_tests);
	SUITE_ADD_TEST(suite, create_hash_handler_tests);
	SUITE_ADD_TEST(suite, destroy_hash_handler_tests);

	SUITE_ADD_TEST(suite, test_valid_find);
	SUITE_ADD_TEST(suite, test_blank_find);
	SUITE_ADD_TEST(suite, test_invalid_key_equal_find);

	SUITE_ADD_TEST(suite, test_valid_range_find);
	SUITE_ADD_TEST(suite, test_one_range_find);
	SUITE_ADD_TEST(suite, test_full_valid_range_find);
	SUITE_ADD_TEST(suite, test_blank_valid_range_find);

	SUITE_ADD_TEST(suite, test_invalid_lower_bound_find);
	SUITE_ADD_TEST(suite, test_invalid_upper_bound_find);
	SUITE_ADD_TEST(suite, test_invalid_bounds_range_find);

	return suite;
}

void
runalltests_open_address_hash_handler()
{
	CuString *output = CuStringNew();
	CuSuite *suite = open_address_hash_handler_getsuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);
}



/*
 @brief		Tests the initalization of the handler
 @params 	*tc
 CuTest
 */
void
initalize_hash_handler_tests(
		CuTest *tc
)
{

	printf("This is for initalize_hash_handler_tests\n");
	printf("------------------------------------------------\n");

	dictionary_handler_t handler;

	status_t result = sadict_init(&handler);

	CuAssertTrue(tc, result 		== status_ok);
	CuAssertTrue(tc, handler.insert == &sadict_insert);
	CuAssertTrue(tc, handler.delete == &sadict_delete);
	CuAssertTrue(tc, handler.create_dictionary
									== &sadict_create);
	CuAssertTrue(tc, handler.get 	== &sadict_query);
	CuAssertTrue(tc, handler.update == &sadict_update);
	CuAssertTrue(tc, handler.delete_dictionary
									== &sadict_destroy);
}

/*
 @brief		Tests the create method of the handler
 @params 	*tc
 CuTest
 */
void
create_hash_handler_tests(
		CuTest *tc
)
{
	printf("This is for create_hash_handler_tests\n");
	printf("------------------------------------------------\n");
	dictionary_handler_t handler;
	dictionary_t dictionary;

	status_t result = sadict_create(key_type_char_array,1, 20, 256,
			dictionary_compare_char_array, &handler, &dictionary);
	CuAssertTrue(tc, result 			== status_ok);
	CuAssertTrue(tc, dictionary.instance!= NULL);
	CuAssertTrue(tc, dictionary.handler != NULL);
}

/*
 @brief		Tests the destroy method of the handler
 @params 	*tc
 CuTest
 */
void
destroy_hash_handler_tests(
		CuTest *tc
)
{
	printf("This is for destroy_hash_handler_tests\n");
	printf("------------------------------------------------\n");
	dictionary_handler_t handler;
	dictionary_t dictionary;

	status_t result = sadict_create(key_type_char_array,1, 20, 256,
			dictionary_compare_char_array, &handler, &dictionary);
	CuAssertTrue(tc, result 			 	== status_ok);
	CuAssertTrue(tc, dictionary.instance	!= NULL);
	CuAssertTrue(tc, dictionary.handler 	!= NULL);

	status_t r 							 	= sadict_destroy(&dictionary);

	CuAssertTrue(tc, r						== status_ok);
	CuAssertTrue(tc, dictionary.instance 	== NULL);
	CuAssertTrue(tc, dictionary.handler	 	== NULL);
}



/*
 @brief		Tests a valid case of the find method where the
 	 	 	 predicate is a equality
 @params 	*tc
 CuTest
 */
void
test_valid_find(
		CuTest *tc
)
{
	printf("This is for test_valid_find\n");
	printf("------------------------------------------------\n");
	//Sets up data until comment marker
	ion_key_size_t key_size 	= 1;
	ion_value_size_t value_size	= 20;
	sa_max_size_t array_size 	= 256;

	unsigned char value[]	 	= "This is a test";
	unsigned char key[] 	 	= "a";
	unsigned char v[20];

	dictionary_t dictionary;
	dict_cursor_t	*cursor;
	predicate_t	pred;
	dictionary_handler_t handler;

	pred.type 								= predicate_equality;
	pred.statement.equality.equality_value 	= key;

	sadict_init(&handler);
	dictionary_create(&handler, &dictionary, key_type_char_array,
			key_size,value_size,array_size);

	handler.insert(&dictionary, key, value);
	sa_max_size_t t = key_to_index(key, key_size);

	//done setting up data

	printf("Right before the find\n");

	status_t result 				= handler.find(&dictionary, &pred, &cursor);
	printf("After the find\n");

	sadict_cursor_t *sadict_cursor 	= (sadict_cursor_t *)(cursor);
	printf("The correct key has been found \n");

	//Tests that all the information is where it should be
	CuAssertTrue(tc, t 				== sadict_cursor->current);
	CuAssertTrue(tc, result 		== err_ok);
	CuAssertTrue(tc, t 				== sadict_cursor->first);
	CuAssertTrue(tc, cursor->status == cs_cursor_initialized);
	CuAssertTrue(tc, cursor->next   == &sadict_equality_next);

	cursor->next(cursor,v);
	printf("The value returned was %s \n\n",v);

	//makes sure that the right information is returned
	CuAssertTrue(tc, cursor->status == cs_end_of_results);
	CuAssertTrue(tc, strcmp((char *) v, (char *) value)
									== 0);

	//calls next again. There is no next
	cursor_status_t y 				= cursor->next(cursor,v);
	CuAssertTrue(tc, y 				== cs_end_of_results);
}
/*
 @brief		Tests a case of the find method where the
 	 	 	predicate is a equality and the location is empty.

 @params 	*tc
 CuTest
 */
void
test_blank_find(
		CuTest *tc
)
{

	printf("This is for test_blank_find\n");
	printf("------------------------------------------------\n");
	//Sets up data until comment marker
	ion_key_size_t key_size 			 	= 1;
	ion_value_size_t value_size 			 = 20;
	sa_max_size_t array_size				= 256;

	unsigned char value[]	 				= "This is a test";
	unsigned char key[] 	 				= "a";
	unsigned char invalid_key[] 			= "b";

	dictionary_t dictionary;
	dict_cursor_t	*cursor;
	predicate_t	pred;
	dictionary_handler_t handler;

	pred.type 								= predicate_equality;
	pred.statement.equality.equality_value 	= invalid_key;

	sadict_init(&handler);
	dictionary_create(&handler, &dictionary, key_type_char_array, key_size,
			value_size,array_size);

	handler.insert(&dictionary, key, value);

	//done setting up data

	printf("Right before the find\n");
	status_t result = handler.find(&dictionary, &pred, &cursor);
	printf("After the find\n");

	CuAssertTrue(tc, result 		== status_empty_slot);
	CuAssertTrue(tc, cursor->status	== cs_end_of_results);
}
/*
 @brief		Tests a case of the find method where the
 	 	 	 predicate is a equality and a invalid key is entered
 @params 	*tc
 CuTest
 */
void
test_invalid_key_equal_find(
		CuTest *tc
)
{
	printf("This is for test_valid_find\n");
	printf("------------------------------------------------\n");
	//Sets up data until comment marker
	ion_key_size_t key_size 			 	= 1;
	ion_value_size_t value_size 			= 20;
	sa_max_size_t array_size 				= 50;
	unsigned char key[] 	 				= "a";

	dictionary_t dictionary;
	dict_cursor_t	*cursor;
	predicate_t	pred;
	dictionary_handler_t handler;

	pred.type 								= predicate_equality;
	pred.statement.equality.equality_value 	= key;

	sadict_init(&handler);
	dictionary_create(&handler, &dictionary, key_type_char_array,
			key_size,value_size,array_size);

	//done setting up data

	printf("Right before the find\n");
	status_t result = handler.find(&dictionary, &pred, &cursor);
	printf("After the find\n");

	//Tests that all the information is where it should be
	CuAssertTrue(tc, result 		== status_key_out_of_bounds);
	CuAssertTrue(tc, cursor->status == cs_end_of_results);
}



/*
 @brief		Tests a valid case of the find method where the
 	 	 	 predicate is a range
 @params 	*tc
 CuTest
 */
void
test_valid_range_find(
		CuTest *tc
)
{
	printf("This is for test_valid_range_find\n");
	printf("------------------------------------------------\n");
	//Sets up data until comment marker
	ion_key_size_t key_size 			 	= 1;
	ion_value_size_t value_size				= 20;
	sa_max_size_t array_size				= 256;

	unsigned char upper[] 	 				= "g";
	unsigned char lower[] 	 				= "d";
	unsigned char key[20];
	unsigned char v[20];

	dictionary_t dictionary;
	dict_cursor_t	*cursor;
	predicate_t	pred;
	dictionary_handler_t handler;

	pred.type 								= predicate_range;
	pred.statement.range.geq_value 			= upper;
	pred.statement.range.leq_value 			= lower;

	sadict_init(&handler);
	dictionary_create(&handler, &dictionary, key_type_char_array,
	key_size,value_size,array_size);

	int x = 97;
	int t = 100;

	for(x = 97; x < 123; x++)
	{
		*key = x;
		handler.insert(&dictionary, (unsigned char *) &x, (unsigned char *) &x);
	}

	printf("Right before the find\n");
	status_t result = handler.find(&dictionary, &pred, &cursor);
	printf("After the find\n");

	sadict_cursor_t *sadict_cursor 	= (sadict_cursor_t *)(cursor);

	//Tests that all the information is where it should be
	CuAssertTrue(tc, t 				== sadict_cursor->current);
	CuAssertTrue(tc, result 		== err_ok);
	CuAssertTrue(tc, t 				== sadict_cursor->first);
	CuAssertTrue(tc, 103 			== sadict_cursor->last);
	CuAssertTrue(tc, cursor->status == cs_cursor_initialized);
	CuAssertTrue(tc, cursor->next   == &sadict_range_next);

	cursor->next(cursor,v);

	t = 101;

	printf("'next' was called and the value returned was %s \n",v);

	//Tests that all the information is where it should be
	CuAssertTrue(tc, t 							== sadict_cursor->current);
	CuAssertTrue(tc, result 					== err_ok);
	CuAssertTrue(tc, 100 						== sadict_cursor->first);
	CuAssertTrue(tc, cursor->status 			== cs_cursor_active);
	CuAssertTrue(tc, strcmp((char *) v,"d")		== 0);

	cursor->next(cursor,v);
	cursor->next(cursor,v);

	 t = 103;
	//Tests that all the information is where it should be
	CuAssertTrue(tc, t 							== sadict_cursor->current);
	CuAssertTrue(tc, t 							== sadict_cursor->last);
	CuAssertTrue(tc, result 					== err_ok);
	CuAssertTrue(tc, cursor->status 			== cs_cursor_active);
	CuAssertTrue(tc, strcmp((char *) v, "f")	== 0);

	cursor->next(cursor,v);

	CuAssertTrue(tc, cursor->status 			== cs_end_of_results);
}
/*
 @brief		Tests a valid case of the find method where the
 	 	 	 predicate is a range and the two bounds are the same
 @params 	*tc
 CuTest
 */
void
test_one_range_find(
		CuTest *tc
)
{
	printf("This is for test_full_valid_range_find\n");
	printf("------------------------------------------------\n");
	//Sets up data until comment marker
	ion_key_size_t key_size 			 	= 1;
	ion_value_size_t value_size 			= 20;
	sa_max_size_t array_size 				= 256;

	unsigned char upper[] 	 				= "c";
	unsigned char lower[] 	 				= "c";
	unsigned char key[20];
	unsigned char v[20];

	dictionary_t dictionary;
	dict_cursor_t	*cursor;
	predicate_t	pred;
	dictionary_handler_t handler;

	pred.type 								= predicate_range;
	pred.statement.range.geq_value 			= upper;
	pred.statement.range.leq_value 			= lower;

	sadict_init(&handler);
	dictionary_create(&handler, &dictionary, key_type_char_array,
	key_size,value_size,array_size);

	int x = 97;
	int t = 97;

	for(x = 97; x < 123; x++)
	{
		*key = x;
		handler.insert(&dictionary, (unsigned char *) &x, (unsigned char *) &x);
	}

	printf("Right before the find\n");
	status_t result = handler.find(&dictionary, &pred, &cursor);
	printf("After the find\n");

	sadict_cursor_t *sadict_cursor 	= (sadict_cursor_t *)(cursor);

	//Tests that all the information is where it should be
	printf("The current status is %d \n", sadict_cursor->current );

	t = 99;
	CuAssertTrue(tc, t 				== sadict_cursor->current);
	CuAssertTrue(tc, result 		== err_ok);
	CuAssertTrue(tc, t 				== sadict_cursor->first);
	CuAssertTrue(tc, t 				== sadict_cursor->last);
	CuAssertTrue(tc, cursor->status == cs_cursor_initialized);
	CuAssertTrue(tc, cursor->next   == &sadict_range_next);

	cursor->next(cursor,v);
	CuAssertTrue(tc, strcmp((char *) v,(char *)&t)
									== 0);
	cursor->next(cursor,v);
	CuAssertTrue(tc, cursor->status == cs_end_of_results);
}
/*
 @brief		Tests a valid case of the find method where the
 	 	 	 predicate is a range and the two bounds cover the whole array
 @params 	*tc
 CuTest
 */
void
test_full_valid_range_find(
		CuTest *tc
)
{

	printf("This is for test_full_valid_range_find\n");
	printf("------------------------------------------------\n");
	//Sets up data until comment marker
	ion_key_size_t key_size 		= 1;
	ion_value_size_t value_size 	= 20;
	sa_max_size_t array_size 		= 256;

	unsigned char upper[] 	 		= "z";
	unsigned char lower[] 	 		= "a";
	unsigned char key[20];
	unsigned char v[20];

	dictionary_t dictionary;
	dict_cursor_t	*cursor;
	predicate_t	pred;
	dictionary_handler_t handler;

	pred.type 						= predicate_range;
	pred.statement.range.geq_value	= upper;
	pred.statement.range.leq_value 	= lower;

	sadict_init(&handler);
	dictionary_create(&handler, &dictionary, key_type_char_array,
	key_size,value_size,array_size);

	int x = 97;
	int t = 97;

	for(x = 97; x < 123; x++)
	{
		*key = x;
		handler.insert(&dictionary, (unsigned char *) &x, (unsigned char *) &x);
	}

	printf("Right before the find\n");
	status_t result = handler.find(&dictionary, &pred, &cursor);
	printf("After the find\n");

	sadict_cursor_t *sadict_cursor 	= (sadict_cursor_t *)(cursor);

	//Tests that all the information is where it should be
	printf("The current status is %d \n", sadict_cursor->current );


	CuAssertTrue(tc, t 				== sadict_cursor->current);
	CuAssertTrue(tc, result 		== err_ok);
	CuAssertTrue(tc, t 				== sadict_cursor->first);
	CuAssertTrue(tc, 122 			== sadict_cursor->last);
	CuAssertTrue(tc, cursor->status == cs_cursor_initialized);
	CuAssertTrue(tc, cursor->next   == &sadict_range_next);

	int j = 0;

	for(j = 97; j<122; j++)
	{
		cursor->next(cursor,v);
		CuAssertTrue(tc, strcmp((char *) v, (char *)&j)
									== 0);
	}

	t = 122;
	CuAssertTrue(tc, t 					== sadict_cursor->current);
	CuAssertTrue(tc, t 					== sadict_cursor->last);
	CuAssertTrue(tc, result 			== err_ok);
	CuAssertTrue(tc, cursor->status 	== cs_cursor_active);

	cursor->next(cursor,v);
	CuAssertTrue(tc, cursor->status 	== cs_end_of_results);
}



/*
 @brief		Tests a valid case of the find method where the
 	 	 	 predicate is a range and the entries are empty
 @params 	*tc
 CuTest
 */
void
test_blank_valid_range_find(
		CuTest *tc
)
{
	printf("This is for test_blank_valid_range_find\n");
	printf("------------------------------------------------\n");
	//Sets up data until comment marker
	ion_key_size_t key_size 		= 1;
	ion_value_size_t value_size 	= 20;
	sa_max_size_t array_size 		= 256;

	unsigned char upper[] 	 		= "d";
	unsigned char lower[] 	 		= "a";
	unsigned char v[20];

	dictionary_t dictionary;
	dict_cursor_t	*cursor;
	predicate_t	pred;
	dictionary_handler_t handler;

	pred.type 						= predicate_range;
	pred.statement.range.geq_value 	= upper;
	pred.statement.range.leq_value 	= lower;

	sadict_init(&handler);
	dictionary_create(&handler, &dictionary, key_type_char_array,
	key_size,value_size,array_size);

	int t = 97;

	printf("Right before the find\n");
	status_t result = handler.find(&dictionary, &pred, &cursor);
	printf("After the find\n");

	sadict_cursor_t *sadict_cursor 	= (sadict_cursor_t *)(cursor);

	//Tests that all the information is where it should be
	printf("The current status is %d \n", sadict_cursor->current );


	CuAssertTrue(tc, t 				== sadict_cursor->current);
	CuAssertTrue(tc, result 		== err_ok);
	CuAssertTrue(tc, t 				== sadict_cursor->first);
	CuAssertTrue(tc, 100 			== sadict_cursor->last);
	CuAssertTrue(tc, cursor->status == cs_cursor_initialized);
	CuAssertTrue(tc, cursor->next   == &sadict_range_next);

	int j = 0;

	for(j = 97; j<101; j++)
	{
		cursor->next(cursor, v);
		printf("The value is '%s'\n", v);
		CuAssertTrue(tc, strcmp((char *) v, "")
									== 0);
	}

	cursor->next(cursor,v);
	CuAssertTrue(tc, cursor->status == cs_end_of_results);
}

/*
 @brief		Tests a invalid case of the find method where the
 	 	 	 predicate is a range and the lower bound (index) is invalid
 @params 	*tc
 CuTest
 */
void
test_invalid_lower_bound_find(
		CuTest *tc
)
{
	printf("This is for test_invalid_upper_bound_find\n");
	printf("------------------------------------------------\n");
	//Sets up data until comment marker
	ion_key_size_t key_size 		= 1;
	ion_value_size_t value_size 	= 20;
	sa_max_size_t array_size 		= 50;

	//upper bound is below the lower bound
	unsigned char to_high[] 	 	= ")";
	unsigned char to_low[] 	 		= "a";

	dictionary_t dictionary;
	dict_cursor_t	*cursor;
	predicate_t	pred;
	dictionary_handler_t handler;

	pred.type 						= predicate_range;
	pred.statement.range.geq_value 	= to_high;
	pred.statement.range.leq_value 	= to_low;

	sadict_init(&handler);
	dictionary_create(&handler, &dictionary, key_type_char_array,
	key_size,value_size,array_size);


	printf("Right before the find\n");
	status_t result = handler.find(&dictionary, &pred, &cursor);
	printf("After the find\n");

	CuAssertTrue(tc, result 		== status_key_out_of_bounds);
	CuAssertTrue(tc, cursor->status == cs_end_of_results);
}

/*
 @brief		Tests a invalid case of the find method where the
 	 	 	 predicate is a range and the upper bound (index) is invalid
 @params 	*tc
 CuTest
 */
void
test_invalid_upper_bound_find(
		CuTest *tc
)
{
	printf("This is for test_invalid_upper_bound_find\n");
	printf("------------------------------------------------\n");
	//Sets up data until comment marker
	ion_key_size_t key_size 		= 1;
	ion_value_size_t value_size 	= 20;
	sa_max_size_t array_size 		= 50;

	//upper bound is below the lower bound
	unsigned char to_high[] 	 	= "b";
	unsigned char to_low[] 	 		= ")";

	dictionary_t dictionary;
	dict_cursor_t	*cursor;
	predicate_t	pred;
	dictionary_handler_t handler;

	pred.type 						= predicate_range;
	pred.statement.range.geq_value 	= to_high;
	pred.statement.range.leq_value 	= to_low;

	sadict_init(&handler);
	dictionary_create(&handler, &dictionary, key_type_char_array,
	key_size,value_size,array_size);


	printf("Right before the find\n");
	status_t result = handler.find(&dictionary, &pred, &cursor);
	printf("After the find\n");

	CuAssertTrue(tc, result 		== status_key_out_of_bounds);
	CuAssertTrue(tc, cursor->status == cs_end_of_results);
}

/*
 @brief		Tests a invalid case of the find method where the
 	 	 	 predicate is a range and the lower bound (index) is heigher then
 	 	 	 the upper bound
 @params 	*tc
 CuTest
 */
void
test_invalid_bounds_range_find(
		CuTest *tc
)
{
	printf("This is for test_invalid_bounds_range_find\n");
	printf("------------------------------------------------\n");
	//Sets up data until comment marker
	ion_key_size_t key_size 		= 1;
	ion_value_size_t value_size 	= 20;
	sa_max_size_t array_size 		= 256;

	//upper bound is below the lower bound
	unsigned char upper[] 	 		= "b";
	unsigned char lower[] 	 		= "h";

	dictionary_t dictionary;
	dict_cursor_t	*cursor;
	predicate_t	pred;
	dictionary_handler_t handler;

	pred.type 						= predicate_range;
	pred.statement.range.geq_value 	= upper;
	pred.statement.range.leq_value 	= lower;

	sadict_init(&handler);
	dictionary_create(&handler, &dictionary, key_type_char_array,
	key_size,value_size,array_size);

	printf("Right before the find\n");
	//calls the find method
	status_t result = handler.find(&dictionary, &pred, &cursor);
	io_printf("After the find\n");

	//Tests that all the information is where it should be

	CuAssertTrue(tc, result 		== status_key_out_of_bounds);
	CuAssertTrue(tc, cursor->status == cs_end_of_results);
}
