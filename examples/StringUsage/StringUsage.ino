#include <IonDB.h>

/* This example outlines how to use Arduino String objects as keys or values with IonDB.
   This sketch exemplifies the use of Arduino Strings as values. */

void
setup(
) {
	Serial.begin(9600);

	/* Instantiate your dictionary of key type int and value type ion_value_t */
	Dictionary < int, ion_value_t > *dict = new SkipList < int, ion_value_t > (-1, key_type_numeric_signed, sizeof(int), sizeof("one"), 7);

	/* Insert a record with key 3 and value "one" */
	dict->insert(3, "one");

	dict->insert(4, "two");
    dict->insert(9, "six");
    dict->insert(42, "ten");

	/* Retrieve a value of type ion_value_t by providing a key */
	ion_value_t  my_value = dict->get(3);

	/* Cast the returned value to type String using Arduino String constructor */
    String string_value = String((char *) my_value);

	/* You should check the status on every operation to ensure good data integrity */
	if (err_ok != dict->last_status.error) {
		printf("Oh no! Something went wrong with my get operation\n");
	}

	printf("3 -> ");
    Serial.println(string_value);

    /* Update the value stored at a key */
    dict->update(3, "new");
    ion_value_t new_value = dict->get(3);

    /* Cast the returned value to type String using Arduino String constructor */
    String new_string = String((char *) new_value);

    printf("3 -> ");
    Serial.println(new_string);

	/* Always clean up your dictionaries when you're done with them! */
	delete dict;
}

void
loop(
) {}