/******************************************************************************/
/**
@file	 staticarray.c
@author	 Raffi Kudlac
@brief	 This is the test file responsible for testing the staticarray and the sahandler.
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


/*
 @brief		Tests the create method with its max key size and a hudge value size
 @params 	tc
 	 	 	 	 	 CuTest
 */
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

	status_t result = sa_insert(&st,key,value);
printf("Information was inserted with the key of '%s' and a value of \n '%s'\n", key,value);
	CuAssertTrue(tc,result ==status_ok);

	status_t get_result = sa_get(&st,key,&stored_data);

	CuAssertTrue(tc,get_result ==status_ok);
printf("The value retrieved was: %s\n",stored_data);
	CuAssertTrue(tc,strcmp(stored_data,value)==0);

printf("The memory address of the Data Structure is not returned\n\n");
	CuAssertTrue(tc,&stored_data != &((bucket_t *)st.array)->value);

	free(st.array);
	st.array = NULL;
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

	free(st.array);
	st.array = NULL;
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

	free(st.array);
	st.array = NULL;
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

	free(st.array);
	st.array = NULL;
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

	free(st.array);
	st.array = NULL;
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

	free(st.array);
	st.array = NULL;
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


/*
 @brief		calls all the insert tests
 @params 	*suite
 	 	 	 	 	 CuSuite
 */
void
insert_tests(
	CuSuite *suite
)
{
	SUITE_ADD_TEST(suite, test_insert_valid_params);
	SUITE_ADD_TEST(suite, test_insert_invalid_index);
	SUITE_ADD_TEST(suite, test_insert_collision);
}


/*
 @brief		calls all the create tests
 @params 	*suite
 	 	 	 	 	 CuSuite
 */
void
create_tests(CuSuite *suite)
{
	SUITE_ADD_TEST(suite, test_create);
	SUITE_ADD_TEST(suite, test_create_invalid_key_size);
	SUITE_ADD_TEST(suite, test_create_invalid_value);
	SUITE_ADD_TEST(suite, test_create_max_size);

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
	SUITE_ADD_TEST(suite,test_delete_valid_case);
	SUITE_ADD_TEST(suite,test_delete_empty_slot);
	SUITE_ADD_TEST(suite,test_delete_invalid_keysize);
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
	SUITE_ADD_TEST(suite,test_get_valid_case);
	SUITE_ADD_TEST(suite,test_get_invalid_key);
	SUITE_ADD_TEST(suite,test_get_from_empty_location);
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
	SUITE_ADD_TEST(suite,test_update_valid_case);
	SUITE_ADD_TEST(suite,test_update_invalid_key);
	SUITE_ADD_TEST(suite,test_update_empty_location);

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
	SUITE_ADD_TEST(suite,test_destroy);

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
	long long x;
	long long t = ipow(256,1);
	long long tt = ipow(256,2);
	long long ttt = ipow(256,3);
	long long result;
	int size;

	printf("%llu\n",t);

	for(x=0;x<t;x++)
	{
		printf("The value of x is: %llu ",x);
		//size = sizeof(long long);
		size = 1;
		result = key_to_index((char *) &x,size);
		printf("The value returned is %llu The value of size is %d\n", result, size);
		CuAssertTrue(tc,result == x);
	}

	for(x=t;x<tt;x++)
	{
		printf("The value of x is: %llu ",x);
		//size = sizeof(long long);
		size = 2;
		result = key_to_index((char *) &x,size);
		printf("The value returned is %llu The value of size is %d\n", result, size);
		CuAssertTrue(tc,result == x);
	}

	for(x=tt;x<ttt;x++)
	{
		printf("The value of x is: %llu ",x);
		//size = sizeof(long long);
		size = 3;
		result = key_to_index((char *) &x,size);
		printf("The value returned is %llu The value of size is %d\n", result, size);
		CuAssertTrue(tc,result == x);
	}
}


void
show_for_raymon(
		CuTest *tc
)
{
	int key_size;
	int array_size;
	int value_size;



	printf("Please enter a key size up to three to work with\n");
	scanf("%d",&key_size);

	printf("Please enter a array size to work with. (A array size of 0 will make a max length array\n");
	scanf("%d",&array_size);

	printf("Please enter a value size to work with\n");
	scanf("%d", &value_size);

	char user[value_size];
	char key[value_size];
	char value[value_size];

	char *v;
	static_array_t st;

	sa_dictionary_create(&st,key_size,value_size,array_size);

	printf("The Static Array was created with the parameters given above\n");
	printf("Enter\nquit - to stop\ni - to insert\nd - to delete\nu - to update\ng - to get\n"
			"p - to print the whole array\n");


	scanf("%s", user);
	while(strcmp(user,"quit") != 0)
	{
		if(strcmp(user,"i")==0)
		{
			printf("Enter a value to insert\n");
			scanf("%s",value);
			printf("Enter a key for the value\n");
			scanf("%s",key);

			sa_insert(&st,key,value);
			printf("The information was inserted\n");

		}
		else if (strcmp(user,"d")==0)
		{
			printf("Enter a key that holds the information you want to delete\n");
			scanf("%s",key);
			sa_delete(&st,key);
			printf("The information was deleted\n");
		}
		else if(strcmp(user,"u")==0)
		{
			printf("Enter a value to replace an existing one\n");
			scanf("%s",value);
			printf("Enter a key for the value\n");
			scanf("%s",key);

			sa_update(&st,key,value);
			printf("The information was updated\n");

		}
		else if(strcmp(user,"g")==0)
		{
			printf("Enter the key that you want to extract the value for\n");
			scanf("%s",key);

			sa_get(&st,key,&v);
			printf("The value returned was: %s\n",v);

		}
		else if (strcmp(user,"p")==0)
		{
			int x;

			for(x =0;x<st.array_size;x++)
			{
				sa_get(&st,(char *) &x,&v);
				printf(" The value at index %d is '%s' \n",x,v);
			}

			printf("\n");
		}
		else
		{
			printf("Wrong input. Try again\n");
		}

		printf("Please enter another command:");
		scanf("%s", user);
	}

	printf("Done");

	free(st.array);
	st.array = NULL;

}

/*
 @brief		calls all the tests that tests the datastructure

 @return 	returns the suite with all the test results
 */
CuSuite*
open_address_staticarray_getsuite()
{
	CuSuite *suite = CuSuiteNew();

//	create_tests(suite);
//	insert_tests(suite);
//	delete_tests(suite);
//	get_tests(suite);
//	update_tests(suite);
//	destroy_tests(suite);
//  SUITE_ADD_TEST(suite,test_key_to_index);
	SUITE_ADD_TEST(suite,show_for_raymon);

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

	status_t result =sadict_init(&handler);

	CuAssertTrue(tc,result==status_ok);
	CuAssertTrue(tc,handler.insert == &sadict_insert);
	CuAssertTrue(tc,handler.delete == &sadict_delete);
	CuAssertTrue(tc,handler.create_dictionary == &sadict_create);
	CuAssertTrue(tc,handler.get == &sadict_query);
	CuAssertTrue(tc,handler.update == &sadict_update);
	CuAssertTrue(tc,handler.delete_dictionary == &sadict_destroy);
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
	dictionary_handler_t 	handler;
	dictionary_t 			dictionary;

	status_t result = sadict_create(2,20,0,&handler,&dictionary);
	CuAssertTrue(tc,result==status_ok);
	CuAssertTrue(tc,dictionary.instance!=NULL);
	CuAssertTrue(tc,dictionary.handler != NULL);

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


/*
 @brief		Calls all tests relivent to the handler

 @return	returns the suite with all the test results
 */
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
