#include "dictionary/openaddresshash/oadictionary.h"
#include "dictionary/dicttypes.h"
#include "dictionary/dictionary.h"
#include "dictionary/openaddresshash/oadictionaryhandler.h"


int size;
record_info_t record_info;
dictionary_handler_t map_handler; 	//create handler for hashmap
dictionary_t test_dictionary;		//collection handler for test collection

void setup()
{

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size = 4;
	record_info.value_size = 10;
	size = 10;

	Serial.begin(9600);
	Serial.flush();
	while (!Serial) {}
	Serial.println("ready!");	
	
	Serial.println("initialization done.");
	Serial.flush();
	
	Serial.println("BEGIN\n");
	Serial.flush();
	

	oadict_init(&map_handler);		//register handler for hashmap

}

void loop()
{
	//register the appropriate handler for a given collection
	dictionary_create(&map_handler, &test_dictionary, key_type_numeric_signed, record_info.key_size, record_info.value_size,size);

	char * str;
	str = (char *)malloc(10);
	int32_t i;

	for (i = 0; i < size; i++)
	{
		sprintf((char*)str, "value : %i ", i);
		Serial.println("Inserting values");
		Serial.flush();
		test_dictionary.handler->insert(&test_dictionary, (ion_key_t)&i, (ion_value_t)str);
	}	
	
	for (i = 0; i < size; i++)
	{
		Serial.println("get values");
		test_dictionary.handler->get(&test_dictionary, (ion_key_t)&i, (ion_value_t)str);
		Serial.println(str);
		Serial.flush();
	}		
	sprintf((char*)str, "value : A ");
	i = 5;
	test_dictionary.handler->update(&test_dictionary, (ion_key_t)&i, (ion_value_t)str);
	
	for (i = 0; i < size; i++)
	{
		Serial.println("get values");
		test_dictionary.handler->get(&test_dictionary, (ion_key_t)&i, (ion_value_t)str);
		Serial.println(str);
		Serial.flush();
	}

	dict_cursor_t 			*cursor;			//create a new cursor pointer

	//create a new predicate statement
	predicate_t 			predicate;
	predicate.type 			= predicate_range;
	
	//need to prepare predicate correctly
	predicate.statement.range.geq_value = (ion_key_t)malloc(sizeof(int32_t));
	
	i = 1;
	memcpy(predicate.statement.range.geq_value,(ion_key_t)&i,sizeof(int32_t));

	predicate.statement.range.leq_value = (ion_key_t)malloc(sizeof(int32_t));
	
	i = 5;
	memcpy(predicate.statement.range.leq_value,(ion_key_t)&i,sizeof(int32_t));
	
	//user must allocate memory before calling next()
	ion_record_t 					record;
	record.key 					= (ion_key_t)malloc(record_info.key_size);
	record.value 					= (ion_value_t)malloc(record_info.value_size);
	
	Serial.println("running query");

	test_dictionary.handler->find(&test_dictionary, &predicate, &cursor);
	
	while( cs_cursor_active == cursor->next(cursor, &record))
	{
		Serial.print("key: ");
		int32_t key_val = *((int32_t*)record.key);
		Serial.print(key_val);
		Serial.print(" value: ");
		Serial.println((char *)record.value);	
	}	
	test_dictionary.handler->delete_dictionary(&test_dictionary);
	free(str);
	while(1);
}
