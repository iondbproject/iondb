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
	int value_size = 10;
	long long array_size = 100;

	static_array_t st;
printf("This is for test_create\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("finished create method\nNow Clearing heap memory\n\n");
	long long x;

	for(x=0;x<array_size;x++){
		CuAssertTrue(tc,((bucket_t *)&(st.array[x]))->status == EMPTY);
	}

	free(st.array);
	st.array = NULL;
	printf("\n");

}

void
test_create_invalid_params_above(CuTest *tc
)
{
	int key_size = 7;
	int value_size = 10000;
	long long array_size = ipow(256,4) +13;

	static_array_t st;
printf("This is for test_create_invalid_params_above\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("finished create method\nNow Clearing heap memory\n\n");
	long long x;

	for(x=0;x<array_size;x++){
		CuAssertTrue(tc,((bucket_t *)&(st.array[x]))->status == EMPTY);
	}

	CuAssertTrue(tc,st.array_size==st.maxelements);
	CuAssertTrue(tc,st.key_size==4);
	CuAssertTrue(tc,st.value_size==1000);

	free(st.array);
	st.array = NULL;
}

void
test_create_invalid_params_below(CuTest *tc
)
{
	int key_size = -1;
	int value_size = -10000;
	long long array_size = 0;

	static_array_t st;
printf("This is for test_create_invalid_params_below\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("finished create method\nNow Clearing heap memory\n\n");
	long long x;

	for(x=0;x<array_size;x++){
		CuAssertTrue(tc,((bucket_t *)&(st.array[x]))->status == EMPTY);
	}

	CuAssertTrue(tc,st.array_size==st.maxelements);
	CuAssertTrue(tc,st.key_size==4);
	CuAssertTrue(tc,st.value_size==1000);

	free(st.array);
	st.array = NULL;
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

	long long t = key_to_index(key,key_size);// * sizeof(bucket_t);
	printf("The key is %llu\n", t);

	//special key to find the value location
	//long long value_key = t*st.value_size;

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
test_insert_invalid_small_value(
	CuTest *tc
)
{

	int key_size = 1;
	int value_size = 100;
	long long array_size = 0;

	static_array_t st;
printf("This is for test_insert_invalid_small_value\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	char value[] = "";
	char key[] = "a";

	printf("The value of '' was attempted to be inserted\n"
			"With a value size of %d\n",value_size);

	CuAssertTrue(tc,sa_insert(&st,key,value) ==status_incorrect_value);
	printf("The program responded with a incorrect value error\n\n");

	free(st.array);
	st.array = NULL;
}

void
test_insert_invalid_large_value(
	CuTest *tc
)
{

	int key_size = 1;
	int value_size = 5;
	long long array_size = 0;

	static_array_t st;
printf("This is for test_insert_invalid_large_value\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	char value[] = "This is too long";
	char key[] = "a";

	printf("The value of 'This is too long' was attempted to be inserted\n"
			"With a value size of %d\n",value_size);
	CuAssertTrue(tc,sa_insert(&st,key,value) ==status_incorrect_value);
	printf("The program responded with a incorrect value error\n\n");

	free(st.array);
	st.array = NULL;
}


void
test_insert_invalid_keysize(
		CuTest *tc
)
{

	int key_size = 1;
	int value_size = 100;
	long long array_size = 0;

	static_array_t st;
printf("This is for test_insert_invalid_keysize\n");
printf("------------------------------------------------\n");
	sa_dictionary_create(&st,key_size,value_size,array_size);
printf("Array Created\n\n");

	char value[] = "This is a test insert";
	char key[] = "aa";

	status_t result = sa_insert(&st,key,value);
	printf("The key size is %d\nA key of '%s' was entered\nThe error was caught\n\n",key_size,key);
	CuAssertTrue(tc,result ==status_incorrect_keysize);


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
	CuAssertTrue(tc,result ==status_incorrect_key);
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
	CuAssertTrue(tc,result2 == status_incorrect_key);

	free(st.array);
	st.array = NULL;

}

void
insert_tests(
	CuSuite *suite
)
{
	SUITE_ADD_TEST(suite, test_insert_valid_params);
	SUITE_ADD_TEST(suite,test_insert_invalid_small_value);
	SUITE_ADD_TEST(suite, test_insert_invalid_large_value);
	SUITE_ADD_TEST(suite, test_insert_invalid_keysize);
	SUITE_ADD_TEST(suite, test_insert_invalid_index);
	SUITE_ADD_TEST(suite, test_insert_collision);
}

void
create_tests(CuSuite *suite)
{
	SUITE_ADD_TEST(suite, test_create);
	SUITE_ADD_TEST(suite, test_create_invalid_params_above);
	SUITE_ADD_TEST(suite, test_create_invalid_params_below);

}

CuSuite*
open_address_staticarray_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	//create_tests(suite);
	insert_tests(suite);

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
