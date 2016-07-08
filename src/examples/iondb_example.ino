#include <IonDB.h>

void
setup(
) {
	Serial.begin(9600);

	/* Instantiate your dictionary */
	Dictionary < int, int > *dict = new
									SkipList(
		key_type_numeric_signed,
		sizeof(int),
		sizeof(int),
		7
									);

	/* Insert a record with key 3 and value 10 */
	dict->insert(3, 10);

	/* Retrieve a value by providing a key */
	int my_value = dict->get(3);

	/* You should check the status on every operation to ensure good data integrity */
	if (err_ok != dict->last_status.error) {
		Serial.println("Oh no! Something went wrong with my get operation");
	}

	/* Update the value stored at a key */
	dict->update(3, 99);

	/* Delete the record stored at a key */
	dict->deleteRecord(3);

	/* ==== Advanced Operations ==== */

	/* If you store records with the same key, you can retrieve them using an equality query: */
	Cursor < int, int > *my_cursor = dict->equality(3);

	while (my_cursor.next()) {
		int key		= my_cursor.getKey();
		int value	= my_cursor.getValue();
		/* Do something with the key and value here */
	}

	/* Remember to clean up the cursor after you're done */
	delete my_cursor;

	/* We can do a simple query for a range, too */
	my_cursor = dict->range(1, 100);

	while (my_cursor.next()) {
		int key		= my_cursor.getKey();
		int value	= my_cursor.getValue();
		/* Do something with the key and value here */
	}

	delete my_cursor;

	/* Or, we can ask for everything in the dictionary: */
	my_cursor = dict->allRecords();

	while (my_cursor.next()) {
		int key		= my_cursor.getKey();
		int value	= my_cursor.getValue();
		/* Do something with the key and value here */
	}

	delete my_cursor;

	/* Always clean up your dictionaries when you're done with them! */
	delete dict;
}

void
loop(
) {}
