#include <Dictionary.h>

void setup()
{
    /* Begin sketch */
    Serial.begin(9600);

    Serial.println("Set-up complete.");

    /* Create Skip List dictionary instance: key type, key size, value size, dictionary size */
    Dictionary<string> *sl = new SkipList<string>(key_type_char_array, sizeof(int), 10, 20);

    /* Insert into skip list dictionary */
    Serial.println("Insert key_1 [1]");
    string key_1 = "key_1";
    int value = 1;
    sl->insert(key_1, value);

    /* Return the value inserted into the skip list dictionary */
    int returned_value = sl->get<int>(key_1);
    Serial.print("Retrieve key key_1 and got back: ");
    Serial.println(returned_value);

    Serial.println("Updating value of key key_1 to 2");

    /* Update value */
    int new_value = 2;
    sl->update(key_1, new_value);

    /* Verify update was successful */
    returned_value = sl->get<int>(key_1);
    Serial.print("Retrieve key key_1 after update and got back: ");
    Serial.println(returned_value);

    /* Delete key and value previously inserted */
    Serial.println("Delete key key_1");
    sl->deleteRecord(key_1);

    /* Verify delete was successful */
    Serial.println("Try and retrieve key key_1 after deletion");
    returned_value = sl->get<int>(key_1);
    Serial.print("Retrieve key key_1 and got back ");
    Serial.println(returned_value);
    /* Returned value was nonsense characters, therefore we know the deletion was successful */

    /* Insert 3 key-value pairs to later test cursor queries */
    Serial.println("Insert key_1 [100]");
    int value_1 = 100;
    sl->insert(key_1, value_1);

    Serial.println("Insert key_1 [200]");
    int value_2 = 200;
    sl->insert(key_1, value_2);

    Serial.println("Insert key_2 [300]");
    string key_2 = "key_2";
    int value_3 = 300;
    sl->insert(key_2, value_3);

    /* Perform equality query on the key of 3 */
    Serial.println("Testing equality query on key key_1: ");
    dict_cursor_t *cursor = sl->equality(key_1);
    ion_record_t record = sl->getRecord(cursor);
    string returned_key = *((string*)record.key);
    returned_value = *((int*)record.value);

    /* First record returned */
    Serial.print("Key: ");
    Serial.print(returned_key);
    Serial.print(" Value: ");
    Serial.println(returned_value);

    /* Second record returned */
    record = sl->getRecord(cursor);
    returned_key = *((string*)record.key);
    returned_value = *((int*)record.value);
    Serial.print("Key: ");
    Serial.print(returned_key);
    Serial.print(" Value: ");
    Serial.println(returned_value);

    /* Third record returned */
    record = sl->getRecord(cursor);
    returned_key = *((string*)record.key);
    returned_value = *((int*)record.value);
    Serial.print("Key: ");
    Serial.print(returned_key);
    Serial.print(" Value: ");
    Serial.println(returned_value);
    /* Value/key returned is nonsense, therefore we know there are no further records to search for */

    /* Perform an all records query */
    Serial.println("Testing all records query: ");
    cursor = sl->allRecords();
    record = sl->getRecord(cursor);
    returned_key = *((string*)record.key);
    returned_value = *((int*)record.value);

    /* First record returned */
    Serial.print("Key: ");
    Serial.print(returned_key);
    Serial.print(" Value: ");
    Serial.println(returned_value);

    /* Second record returned */
    record = sl->getRecord(cursor);
    returned_key = *((string*)record.key);
    returned_value = *((int*)record.value);
    Serial.print("Key: ");
    Serial.print(returned_key);
    Serial.print(" Value: ");
    Serial.println(returned_value);

    /* Third record returned */
    record = sl->getRecord(cursor);
    returned_key = *((string*)record.key);
    returned_value = *((int*)record.value);
    Serial.print("Key: ");
    Serial.print(returned_key);
    Serial.print(" Value: ");
    Serial.println(returned_value);

    /* Fourth record returned */
    record = sl->getRecord(cursor);
    returned_key = *((string*)record.key);
    returned_value = *((int*)record.value);
    Serial.print("Key: ");
    Serial.print(returned_key);
    Serial.print(" Value: ");
    Serial.println(returned_value);
    /* Value/key returned is nonsense, therefore we know there are no further records to search for */

    /* Perform range query on keys k for "key_0"<=k<="key_1" */
    Serial.println("Testing range query: \"key_0\"<=key<=\"key_1\" ");
    cursor = sl->range("key_0", "key_1");
    record = sl->getRecord(cursor);
    returned_key = *((string*)record.key);
    returned_value = *((int*)record.value);

    /* First record returned */
    Serial.print("Key: ");
    Serial.print(returned_key);
    Serial.print(" Value: ");
    Serial.println(returned_value);

    /* Second record returned */
    record = sl->getRecord(cursor);
    returned_key = *((string*)record.key);
    returned_value = *((int*)record.value);
    Serial.print("Key: ");
    Serial.print(returned_key);
    Serial.print(" Value: ");
    Serial.println(returned_value);

    /* Third record returned */
    record = sl->getRecord(cursor);
    returned_key = *((string*)record.key);
    returned_value = *((int*)record.value);
    Serial.print("Key: ");
    Serial.print(returned_key);
    Serial.print(" Value: ");
    Serial.println(returned_value);
    /* Value/key returned is nonsense, therefore we know there are no further records to search for */

    /* All methods have been tested */
    Serial.println("End SkipList (string key) test.");

    sl->destroy();
    delete sl;
}


void loop() {}