#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "./../../../CuTest.h"
//#include "./../../../../dictionary/flatfilestore/flatfile.h"
#include "./../../../../dictionary/dicttypes.h"
#include "./../../../../dictionary/dictionary.h"
#include "./../../../../dictionary/filehandler.h"
#include "./../../../../kv_system.h"
//#include "./../../../../dictionary/flatfilestore/ffdictionaryhandler.h"





/**
@brief		Validates the correct registration of function pointers
			for open address dictionary structure.

@param 		tc
				CuTest
 */
/*void
test_binary_file_handler_function_registration(
	CuTest		*tc
)
{

	FILE file_ptr = fopen("test.bin","w+");

	char * record;

	int key_size = 4;
	int value_size = 10;

	record = (char*)malloc(key_size+value_size);
}*/




binary_tree_file_handler_getsuite()
{
	CuSuite *suite = CuSuiteNew();

//	SUITE_ADD_TEST(suite, test_flat_file_handler_function_registration);
//	SUITE_ADD_TEST(suite, test_flat_file_handler_create_destroy);
//	SUITE_ADD_TEST(suite, test_flat_file_handler_simple_insert);
	//need simple query
/*	SUITE_ADD_TEST(suite, test_flat_file_handler_simple_delete);
	SUITE_ADD_TEST(suite, test_flat_file_handler_duplicate_insert_1);
	SUITE_ADD_TEST(suite, test_flat_file_handler_duplicate_insert_2);
	SUITE_ADD_TEST(suite, test_flat_file_handler_update_1);
	SUITE_ADD_TEST(suite, test_flat_file_handler_update_2);
	SUITE_ADD_TEST(suite, test_flat_file_handler_delete_1);
	SUITE_ADD_TEST(suite, test_flat_file_handler_delete_2);
	SUITE_ADD_TEST(suite, test_flat_file_handler_capacity);
	SUITE_ADD_TEST(suite, test_open_address_dictionary_predicate_equality);
	SUITE_ADD_TEST(suite, test_open_address_dictionary_predicate_range_signed);
	SUITE_ADD_TEST(suite, test_open_address_dictionary_predicate_range_unsigned);
	SUITE_ADD_TEST(suite, test_open_address_dictionary_cursor_equality);
	SUITE_ADD_TEST(suite, test_open_address_dictionary_handler_query_with_results);
	SUITE_ADD_TEST(suite, test_open_address_dictionary_handler_query_no_results);
	SUITE_ADD_TEST(suite, test_open_address_dictionary_cursor_range);*/

	return suite;
}

void
runalltests_binary_tree_handler()
{
/*	CuString	*output	= CuStringNew();
	CuSuite		*suite	= flat_file_handler_getsuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);*/

	FILE *key_file_ptr;
	FILE *value_file_ptr;

	key_file_ptr = fopen("btree_key.bin","w+");

	value_file_ptr = fopen("btree_value.bin","w+");

	char * record;

	int key_size = 4;
	int value_size = 10;

	record = (char*)malloc(key_size+value_size);

	ion_value_t str;
	str = (ion_value_t)malloc(10);

	int i;
	for (i = 3; i< 6; i++)
	{
		sprintf((char*)str,"thisis %02i",i);
		printf("%s \n", str);
		bt_insert(key_file_ptr, value_file_ptr, i, str);
	}
	sprintf((char*)str	,"this is %02i",1);
	bt_insert(key_file_ptr, value_file_ptr, 1, str);
	sprintf((char*)str,"this is %02i",2);
	bt_insert(key_file_ptr, value_file_ptr, 2, str);

	if (err_ok == bt_find(key_file_ptr, value_file_ptr, 1, str))
	{
		str[9] = '\0';
		printf("value %s\n",str);
	}
	else
	{
		printf("not found\n");
	}

	if (0 == bt_find(key_file_ptr, value_file_ptr, 10, str))
	{
			str[9] = '\0';
			printf("value %s\n",str);
		}
		else
		{
			printf("not found\n");
		}
	if (0 == bt_find(key_file_ptr, value_file_ptr, 3, str))
			{
					str[9] = '\0';
					printf("value %s\n",str);
				}
				else
				{
					printf("not found\n");
				}
	if (0 == bt_find(key_file_ptr, value_file_ptr, 4, str))
	{
			str[9] = '\0';
			printf("value %s\n",str);
		}
		else
		{
			printf("not found\n");
		}
	if ( 0 == bt_find(key_file_ptr, value_file_ptr, 5, str))
	{
			str[9] = '\0';
			printf("value %s\n",str);
		}
		else
		{
			printf("not found\n");
		}
	bt_delete(key_file_ptr,2);
	if (0 == bt_find(key_file_ptr, value_file_ptr, 2, str))
	{
			str[9] = '\0';
			printf("value %s\n",str);
		}
		else
		{
			printf("not found\n");
		}
	fclose(key_file_ptr);
	fclose(value_file_ptr);
}
