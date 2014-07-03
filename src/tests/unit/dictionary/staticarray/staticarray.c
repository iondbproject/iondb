/******************************************************************************/
/**
@file	 staticarray.c
@author	 Raffi Kudlac
@brief	 ###BRIEF HERE###
*/
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "./../../../CuTest.h"
#include "./../../../../dictionary/staticarray/staticarray.h"
#include "./../../../../dictionary/staticarray/sahandler.h"
#include "./../../../../dictionary/dicttypes.h"
#include "./../../../../dictionary/dictionary.h"

#define MAX_HASH_TEST 100
#define STD_MAP_SIZE 10


void
test_create(
	CuTest *tc
)
{

	int key_size = 3;
	int value_size = 100;
	long long array_size = 100;

	static_array_t st;
printf("This is for test_create\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("finished create method\nNow Clearing heap memory\n\n");
	long long x;
	int length = sizeof(bucket_t);

	for(x=0;x<array_size;x++)
	{
		CuAssertTrue(tc,((bucket_t *)&(st.array[x*length]))->status == EMPTY);
	}

	free(st.array);
	st.array = NULL;
	printf("\n");

}

void
test_create_invalid_key_size(
		CuTest *tc
)
{
	int key_size = 7;
	int key_size2 = -5;
	int value_size = 50;
	long long array_size = ipow(256,4) +13;

	static_array_t st;
printf("This is for test_create_invalid_key_size\n");
printf("------------------------------------------------\n");
printf("A key size of 7 was entered when the max is 3. The error was caught\n\n");
	status_t result = sa_dictionary_create(&st,key_size,value_size,array_size);
	CuAssertTrue(tc,result ==status_incorrect_keysize);

	status_t result2 = sa_dictionary_create(&st,key_size2,value_size,array_size);
	CuAssertTrue(tc,result2 ==status_incorrect_keysize);

}

void
test_create_max_size(
		CuTest *tc
)
{
	int key_size = 3;
	int value_size = 10000;
	long long array_size = 0;

	static_array_t st;
printf("This is for test_create_max_size\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("finished create method\nNow Clearing heap memory\n\n");
	long long x;
	int length = sizeof(bucket_t);

	for(x=0;x<array_size;x++)
	{
		CuAssertTrue(tc,((bucket_t *)&(st.array[x*length]))->status == EMPTY);
	}

	free(st.array);
	st.array = NULL;
	printf("\n");

}

void
test_create_invalid_value(
		CuTest *tc
)
{
	int key_size = 1;
	int value_size = -10000;
	long long array_size = 0;

	static_array_t st;
printf("This is for test_create_invalid_params_below\n");
printf("------------------------------------------------\n\n");
printf("A negative value size was entered. The error was caught\n\n");
	status_t result = sa_dictionary_create(&st,key_size,value_size,array_size);

	CuAssertTrue(tc,st.array_size==st.maxelements);
	CuAssertTrue(tc,st.key_size==1);
	CuAssertTrue(tc,result == status_incorrect_value);
}


void
test_insert_valid_params(
		CuTest *tc
)
{

	int key_size = 1;
	int value_size = 100;
	long long array_size = 256;

	static_array_t st;
printf("This is for test_insert_valid_params\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	char value[] = "This is a test insert";
	char key[] = "a";

	status_t result = sa_insert(&st,key,value);

	long long t = key_to_index(key,key_size);
	printf("The key is %llu\n", t);

	bucket_t *b = (bucket_t *) st.array;

	char* str = b[t].value;

	printf("status: %d\n",b[t].status);
	printf("%s\n\n",str);
	CuAssertTrue(tc,result==status_ok);
	CuAssertTrue(tc,strcmp(b[t].value,value)==0);
	CuAssertTrue(tc,b[t].status==OCCUPIED);

	free(st.array);
	st.array = NULL;

}

void
test_insert_invalid_index(
		CuTest *tc
)
{
	int key_size = 1;
	int value_size = 100;
	long long array_size = 50;

	static_array_t st;
printf("This is for test_insert_invalid_index\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	char value[] = "This is a test insert";
	char key[] = "a";

	long long k = key_to_index(key,1);

	status_t result = sa_insert(&st,key,value);
	printf("The key size is %d\nA key of '%s' was entered\n",key_size,key);
	printf("The key in integer form is %llu\n The array is %llu long\n",k,array_size);
	CuAssertTrue(tc,result ==status_incorrect_keysize);
	printf("The error was caught\n\n");


	free(st.array);
	st.array = NULL;

}

void
test_insert_collision(
		CuTest *tc
)
{
	int key_size = 1;
	int value_size = 100;
	long long array_size = 0;

	static_array_t st;
printf("This is for test_insert_collision\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	char value[] = "This is a test insert";
	char key[] = "k";

	char value2[]="Collision is comming";
	char key2[] = "k";

	status_t result = sa_insert(&st,key,value);
	printf("Information was inserted with the key of '%s'\n", key);
	CuAssertTrue(tc,result ==status_ok);

	printf("More Information was inserted with the same key, '%s'\nThe error was caught and the "
			"insertion was not allowed\n\n", key);

	status_t result2 = sa_insert(&st,key2,value2);
	CuAssertTrue(tc,result2 == status_occupied);

	free(st.array);
	st.array = NULL;

}

void
test_delete_valid_case(
		CuTest *tc
)
{
	int key_size = 1;
	int value_size = 100;
	long long array_size = 0;

	static_array_t st;
printf("This is for test_delete_valid_case\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	char value[] = "This is a test insert";
	char key[] = "k";
	long long k = key_to_index(key,1);


	status_t result = sa_insert(&st,key,value);
	printf("Information was inserted with the key of '%s'\n", key);
	CuAssertTrue(tc,result ==status_ok);

	status_t d = sa_delete(&st,key);
	CuAssertTrue(tc,d == status_ok);

	bucket_t *b = (bucket_t *) st.array;

	CuAssertTrue(tc, b[k].status == EMPTY);
	CuAssertTrue(tc, strcmp(b[k].value,"") );

	free(st.array);
	st.array = NULL;
}

void
test_delete_empty_slot(
		CuTest *tc
)
{

	int key_size = 1;
	int value_size = 100;
	long long array_size = 0;

	static_array_t st;
printf("This is for test_delete_empty_slot\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	//char value[] = "This is a test insert";
	char key[] = "k";
	//long long k = key_to_index(key,1);

	status_t d = sa_delete(&st,key);
	CuAssertTrue(tc,d == status_empty_slot);

	free(st.array);
	st.array = NULL;
}

void
test_delete_invalid_keysize(
		CuTest *tc
)
{
	int key_size = 1;
	int value_size = 100;
	long long array_size = 100;

	static_array_t st;
printf("This is for test_delete_invalid_keysize\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	char value[] = "This is a test insert";
	char key[] = "a";
	char invalid_key[] = "k";
	long long k = key_to_index(invalid_key,1);


	status_t result = sa_insert(&st,key,value);
printf("Information was inserted with the key of '%s'\n", key);
	CuAssertTrue(tc,result ==status_ok);

printf("Tried to delete at index %llu when array only goes up to %llu\n\n",k,array_size);
	status_t d = sa_delete(&st,invalid_key);
	CuAssertTrue(tc,d == status_incorrect_keysize);


	free(st.array);
	st.array = NULL;
}

void
test_get_valid_case(
		CuTest *tc
)
{
	int key_size = 1;
	int value_size = 100;
	long long array_size = 0;

	static_array_t st;
printf("This is for test_get_valid_case\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	char value[] = "This is a test insert";
	char key[] = "a";
	char *stored_data;
	//long long k = key_to_index(key,1);


	status_t result = sa_insert(&st,key,value);
printf("Information was inserted with the key of '%s' and a value of \n '%s'\n", key,value);
	CuAssertTrue(tc,result ==status_ok);

	status_t get_result = sa_get(&st,key,&stored_data);

	CuAssertTrue(tc,get_result ==status_ok);
printf("The value retrieved was: %s\n",stored_data);
	CuAssertTrue(tc,strcmp(stored_data,value)==0);

printf("The memory address of the Data Structure is not return\n\n");
	CuAssertTrue(tc,&stored_data != &((bucket_t *)st.array)->value);


}

void
test_get_invalid_key(
		CuTest *tc
)
{
	int key_size = 1;
	int value_size = 100;
	long long array_size = 50;

	static_array_t st;
printf("This is for test_get_invalid_key\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	//char value[] = "This is a test insert";
	char key[] = "a";
	char *stored_data;
	long long k = key_to_index(key,1);

	status_t get_result = sa_get(&st,key,&stored_data);
	CuAssertTrue(tc,get_result ==status_incorrect_keysize);
printf("The index location of %llu tried to be accessed but the max index is %llu\n\n",k,array_size);

}

void
test_get_from_empty_location(
		CuTest *tc
)
{

int key_size = 1;
	int value_size = 100;
	long long array_size = 0;

	static_array_t st;
printf("This is for test_get_from_empty_location\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	//char value[] = "This is a test insert";
	char key[] = "a";
	char *stored_data;
	long long k = key_to_index(key,1);

	status_t get_result = sa_get(&st,key,&stored_data);
	CuAssertTrue(tc,get_result ==status_empty_slot);
printf("A value was looked for at index %llu but none was found and the error was caught\n\n", k);

}

void
test_update_valid_case(
		CuTest *tc
)
{
	int key_size = 1;
	int value_size = 100;
	long long array_size = 0;

	static_array_t st;
printf("This is for test_update_valid_case\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	char value[] = "This is a test insert";
	char key[] = "a";
	char new_value[] = "The value was changed";
	long long k = key_to_index(key,1);


	status_t result = sa_insert(&st,key,value);
printf("Information was inserted with the key of '%s' and a value of \n '%s'\n", key,value);
	CuAssertTrue(tc,result ==status_ok);

printf("The old information was overwritten with '%s'\n",new_value);

	status_t update_result = sa_update(&st,key,new_value);
	CuAssertTrue(tc,update_result==status_ok);

	bucket_t *pointer = (bucket_t *)st.array;
	CuAssertTrue(tc,pointer[k].status == OCCUPIED);
	CuAssertTrue(tc,memcmp(pointer[k].value, new_value,value_size)==0);
printf("The information in the data structure is '%s'\n\n", pointer[k].value);
}

void
test_update_invalid_key(
		CuTest *tc
)
{

	int key_size = 1;
	int value_size = 100;
	long long array_size = 70;

	static_array_t st;
printf("This is for test_update_invalid_key\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	//char value[] = "This is a test insert";
	char key[] = "a";
	char new_value[] = "The value was changed";
	long long k = key_to_index(key,1);

printf("index %llu was attempted to be accessed with a max array size of %llu \n",k,array_size);
printf("The error was caught\n\n");
	status_t update_result = sa_update(&st,key,new_value);
	CuAssertTrue(tc,update_result==status_incorrect_keysize);

}

void
test_update_empty_location(
		CuTest *tc
)
{

	int key_size = 1;
	int value_size = 100;
	long long array_size = 0;

	static_array_t st;
printf("This is for test_update_empty_location\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	//char value[] = "This is a test insert";
	bucket_t *pointer = (bucket_t *)st.array;
	char key[] = "a";
	char new_value[] = "The value was changed from nothing";
	long long k = key_to_index(key,1);

	CuAssertTrue(tc,pointer[k].status==EMPTY);

printf("the value '%s' was inserted into a blank location using the update method\n",new_value);
	status_t update_result = sa_update(&st,key,new_value);


	CuAssertTrue(tc,update_result==status_ok);
	CuAssertTrue(tc,pointer[k].status==OCCUPIED);
	CuAssertTrue(tc,memcmp(pointer[k].value, new_value,value_size)==0);
}

void
test_destroy(
		CuTest *tc
)
{
	int key_size = 1;
	int value_size = 100;
	long long array_size = 0;

	static_array_t st;
printf("This is for test_update_empty_location\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	status_t destroy_result = sa_destroy(&st);

	CuAssertTrue(tc,destroy_result==status_ok);
	CuAssertTrue(tc,st.array==NULL);
}


void
insert_tests(
	CuSuite *suite
)
{
	SUITE_ADD_TEST(suite, test_insert_valid_params);
	SUITE_ADD_TEST(suite, test_insert_invalid_index);
	SUITE_ADD_TEST(suite, test_insert_collision);
}

void
create_tests(CuSuite *suite)
{
	SUITE_ADD_TEST(suite, test_create);
	SUITE_ADD_TEST(suite, test_create_invalid_key_size);
	SUITE_ADD_TEST(suite, test_create_invalid_value);
	SUITE_ADD_TEST(suite, test_create_max_size);

}

void
delete_tests(
		CuSuite *suite
)
{
	SUITE_ADD_TEST(suite,test_delete_valid_case);
	SUITE_ADD_TEST(suite,test_delete_empty_slot);
	SUITE_ADD_TEST(suite,test_delete_invalid_keysize);
}

void
get_tests(
		CuSuite *suite
)
{
	SUITE_ADD_TEST(suite,test_get_valid_case);
	SUITE_ADD_TEST(suite,test_get_invalid_key);
	SUITE_ADD_TEST(suite,test_get_from_empty_location);
}

void
update_tests(
		CuSuite *suite
)
{
	SUITE_ADD_TEST(suite,test_update_valid_case);
	SUITE_ADD_TEST(suite,test_update_invalid_key);
	SUITE_ADD_TEST(suite,test_update_empty_location);

}

void
destroy_tests(
		CuSuite *suite
)
{
	SUITE_ADD_TEST(suite,test_destroy);

}

void
test_key_to_index(
		CuTest *tc
)
{

printf("This is for test_key_to_index\n");
printf("------------------------------------------------\n");
	long long x;
	long long t = ipow(256,3);
	long long result;
	int size;
	double l = log10(256);

	printf("%llu\n",t);

	for(x=0;x<t;x++)
	{
		printf("The value of x is: %llu ",x);
		size = (int) (log10(x)/log10(256)) + 1;
		result = key_to_index((char *) &x,size);
		printf("The value returned is %llu The value of size is %d\n", result, size);
		CuAssertTrue(tc,result == x);
	}
}

CuSuite*
open_address_staticarray_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	//create_tests(suite);
	//insert_tests(suite);
	//delete_tests(suite);
	//get_tests(suite);
	//update_tests(suite);
	//destroy_tests(suite);
	SUITE_ADD_TEST(suite,test_key_to_index);


	return suite;
}

void
runalltests_open_address_staticarray()
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= open_address_staticarray_getsuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);
}

//----------------------------------------------------------------------------------
//Handler Tests Below


void
initalize_hash_handler_tests(
		CuTest *tc
)
{

printf("This is for initalize_hash_handler_tests\n");
printf("------------------------------------------------\n");

	dictionary_handler_t handler;

	status_t result =sadict_init(&handler);

	CuAssertTrue(tc,result==status_ok);
	CuAssertTrue(tc,handler.insert == &sadict_insert);
	CuAssertTrue(tc,handler.delete == &sadict_delete);
	CuAssertTrue(tc,handler.create_dictionary == &sadict_create);
	CuAssertTrue(tc,handler.get == &sadict_query);
	CuAssertTrue(tc,handler.update == &sadict_update);
	CuAssertTrue(tc,handler.delete_dictionary == &sadict_destroy);
}

void
create_hash_handler_tests(
		CuTest *tc
)
{
printf("This is for create_hash_handler_tests\n");
printf("------------------------------------------------\n");
	dictionary_handler_t 	handler;
	dictionary_t 			dictionary;

	status_t result = sadict_create(2,20,0,&handler,&dictionary);
	CuAssertTrue(tc,result==status_ok);
	CuAssertTrue(tc,dictionary.instance!=NULL);
	CuAssertTrue(tc,dictionary.handler != NULL);

}

void
destroy_hash_handler_tests(
		CuTest *tc
)
{
printf("This is for destroy_hash_handler_tests\n");
printf("------------------------------------------------\n");
	dictionary_handler_t 	handler;
	dictionary_t 			dictionary;

	status_t result = sadict_create(2,20,0,&handler,&dictionary);
	CuAssertTrue(tc,result==status_ok);
	CuAssertTrue(tc,dictionary.instance!=NULL);
	CuAssertTrue(tc,dictionary.handler != NULL);

	status_t r = sadict_destroy(&dictionary);

	CuAssertTrue(tc,r==status_ok);
	CuAssertTrue(tc,dictionary.instance==NULL);
	CuAssertTrue(tc,dictionary.handler==NULL);


}

CuSuite*
open_address_hash_handler_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite,initalize_hash_handler_tests);
	SUITE_ADD_TEST(suite,create_hash_handler_tests);
	SUITE_ADD_TEST(suite,destroy_hash_handler_tests);
	return suite;
}

void
runalltests_open_address_hash_handler()
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= open_address_hash_handler_getsuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);
}
