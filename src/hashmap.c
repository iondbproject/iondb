/*
 ============================================================================
 Name        : KV.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include "kv_system.h"
//#include "oahash.h"
#include "./dictionary/dictionary.h"
#include "./dictionary/openaddresshash/oadictionaryhandler.h"

#include "io.h"

void test_query(dictionary_t* test_dictionary, int key, const record_info_t* record) {

	io_printf("Query\n");
	char* data;
	err_t error = dictionary_get(&*test_dictionary, (ion_key_t) &key,
			(ion_value_t *) &data);
	if (error == err_ok) {
		int idx;
		int key = *((int*) data);
		char* value;
		value = (char*)(data + record->key_size);
		io_printf("Key: %i\n", key);
		for (idx = 0; idx < record->value_size; idx++)
			io_printf("%c", value[idx]);
		io_printf("\n");
		free(data);
	} else {
		io_printf("error %i\n", error);
	}
}

int main(void) {
	int size;
	char ch;
	record_info_t record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size = 4;
	record.value_size = 10;
	size = 10;

	dictionary_handler_t map_handler;			//create handler for hashmap

	//TODO streamline process for
	oadict_init(&map_handler);					//register handler for hashmap

	//collection handler for test collection
	dictionary_t test_dictionary;

	//register the appropriate handler for a given collection
	dictionary_create(&map_handler, &test_dictionary,key_type_numeric_signed, record.key_size,record.value_size,size);

	io_printf("Insert\n");

	char value[record.value_size];
	int key = 6;		//key
	memcpy(value,"this is a ",record.value_size);

	dictionary_insert(&test_dictionary, (ion_key_t )&key, (ion_value_t )value);

	test_query(&test_dictionary, key, &record);

	memcpy(value,"this is b ",record.value_size);
	dictionary_update(&test_dictionary, (ion_key_t)&key, (ion_value_t)value);

	test_query(&test_dictionary, key, &record);
	/*//pointer to hashmap
	//hashmap_t hash_map;

	//oah_initialize(&hash_map, oah_compute_simple_hash, record.key_size, record.value_size, size);    			// just pick an arbitary size for testing atm

	 Read and process commands
	io_printf(">");
	while ( (ch = getchar()) != 'X')
	{
		int num;

		if (ch == 10)
		{
			// just burn the cr
		}
		else if (ch == 'P')
			oah_print(&hash_map, size, &record);
		else
		{

			scanf("%d", &num);

			if (ch == 'I')
			{
				io_printf("Insert\n");

				char value[10];
				int key = num;		//key
				memcpy(value,"this is a ",10);

				if (oah_insert(&hash_map,(char *)(&key), value) == err_duplicate_key)
				{
					io_printf("Duplicate Key\n");
				}

			}else if (ch == 'U')
			{
				io_printf("Update\n");

				char value[10];
				int key = num;		//key
				memcpy(value,"this updat",10);

				io_printf("Update\n");
				oah_update(&hash_map,(char *)(&key), value);
			}
			else if (ch == 'D')
			{
				io_printf("Delete\n");
				oah_delete(&hash_map, (char *)(&num));
			}
			else if (ch == 'Q')
			{
				io_printf("Query\n");
				char * data;
				err_t error = oah_query(&hash_map, (char *)(&num), &data);
				if (error == err_ok)
				{
					int idx;
					int key = *((int *)data);
					char * value;
					value = (char *)(data + record.key_size);
					io_printf("Key: %i\n",key);
					for (idx = 0; idx < record.value_size;idx++)
						io_printf("%c",value[idx]);
					io_printf("\n");
					free(data);
				}
				else
				{
					io_printf("error %i\n",error);
				}
			}
			else
				io_printf("Invalid operation: %c.\n", ch);

			getchar();
		}
		io_printf(">");
		fflush(stdin);
	}*/
	return EXIT_SUCCESS;
}
