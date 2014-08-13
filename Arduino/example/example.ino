#include <dictionary.h>
#include <slhandler.h>

dictionary_handler_t skip_handler;
dictionary_t test_dict;

void
check_skiplist(
	skiplist_t 	*skiplist
)
{
	sl_node_t 	*cursor = skiplist->head;

	while(NULL != cursor->next[0])
	{
		int 		key 		= *((int*)cursor->next[0]->key);
		char* 		value 		= (char*) cursor->next[0]->value;
		sl_level_t 	level 		= cursor->next[0]->height + 1;
                Serial.print("k: ");
                Serial.print(key);
                Serial.print(" v: ");
                Serial.print(value);
                Serial.print(" l: ");
                Serial.print(level);
                Serial.print(" -- ");
		cursor 					= cursor->next[0];
	}

	Serial.println("END\n\n");
}

void setup()
{
  sldict_init(&skip_handler);
  dictionary_create(&skip_handler, &test_dict, key_type_numeric_signed, 4, 10, 7);
  Serial.begin(9600);
}

void loop()
{
  int key = millis() / 100;
  char value[10] = "test";
  //Serial.print("Insert ");
  //Serial.print(key);
  //Serial.println(" | 'val'...");
  dictionary_insert(&test_dict, (ion_key_t) &key, (ion_value_t) value);
  skiplist_t *sl = (skiplist_t*) test_dict.instance;
  check_skiplist(sl);
  delay(1000);
}
