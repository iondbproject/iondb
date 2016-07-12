#include <IonDB.h>

void
setup(
) {
	Serial.begin(9600);

	/* Instantiate your dictionary */
	Dictionary < int, int > *dict = new SkipList < int, int > (key_type_numeric_signed, sizeof(int), sizeof(int), 7);

	/* Insert a record with key 3 and value 10 */
	dict->insert(3, 10);

	dict->insert(4, 16);
	dict->insert(4, 17);
	dict->insert(4, 18);
	dict->insert(9, 44);
	dict->insert(42, 91);

	/* Retrieve a value by providing a key */
	int my_value = dict->get(3);

	/* You should check the status on every operation to ensure good data integrity */
	if (err_ok != dict->last_status.error) {
		printf("Oh no! Something went wrong with my get operation\n");
	}

	printf("3 -> %d\n", my_value);

	/* Update the value stored at a key */
	dict->update(3, 99);
	int new_value = dict->get(3);

	printf("3 -> %d\n", new_value);

	/* Delete the record stored at a key */
	dict->deleteRecord(3);

	/* ==== Advanced Operations ==== */

	/* If you store records with the same key, you can retrieve them using an equality query: */
	Cursor < int, int > *my_cursor = dict->equality(4);

	printf("EQUALITY QUERY\n");
	while (my_cursor->next()) {
		int key   = my_cursor->getKey();
		int value = my_cursor->getValue();
		/* Do something with the key and value here */
		printf("%d -> %d\n", key, value);
	}

	/* Remember to clean up the cursor after you're done */
	delete my_cursor;

	/* We can do a simple query for a range, too */
	my_cursor = dict->range(1, 100);

	printf("RANGE QUERY\n");
	while (my_cursor->next()) {
		int key   = my_cursor->getKey();
		int value = my_cursor->getValue();
		/* Do something with the key and value here */
		printf("%d -> %d\n", key, value);
	}

	delete my_cursor;
	
	/* Always clean up your dictionaries when you're done with them! */
	delete dict;
}

void
loop(
) {}
