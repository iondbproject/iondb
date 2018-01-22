#include <IonDB.h>

/* This example outlines how to use IonDB with user-defined (or non-C++ compliant)
   key or value types. For this example, the value type is a user-defined struct.
   For more examples of using non-C++ compliant key or value types, please see
   the example StringUsage. */

void
setup(
) {
  Serial.begin(9600);

  /* Create sample object */
  struct obj {
    String message;
    int num;
  };

  /* Create an instance of the object */
  obj ex = {"hi", 1};

  /* Copy the object to a char array for insertion */
  char bytes[sizeof(ex)];
  memcpy(bytes, &ex, sizeof(ex));

  /* Instantiate your dictionary of key type int and value type ion_value_t */
  Dictionary < int, ion_value_t > *dict = new SkipList < int, ion_value_t > (-1, key_type_numeric_signed, sizeof(int), sizeof(bytes), 7);

  /* Insert a record with key 3 and value ("hi", 1) */
  dict->insert(1, bytes);

  /* Retrieve a value of type ion_value_t by providing a key */
  ion_value_t  my_value = dict->get(1);

  /* Copy returned value back into struct object */
  obj ret_struct;
  memcpy(&ret_struct, my_value, sizeof(ret_struct));

  /* You should check the status on every operation to ensure good data integrity */
  if (err_ok != dict->last_status.error) {
    printf("Oh no! Something went wrong with my get operation\n");
  }

  /* Print returned struct values */
  printf("1 -> \nMessage: ");
  Serial.println(String(ret_struct.message));
  printf("Num: %i\n", (int) ret_struct.num);

  /* Update the value stored at a key */
  obj new_ex = {"bye", 2};

  /* Copy the object to a char array for insertion */
  char new_bytes[sizeof(new_ex)];
  memcpy(new_bytes, &new_ex, sizeof(new_ex));

  dict->update(1, new_bytes);

  /* Return new updated value */
  ion_value_t new_value = dict->get(1);

  /* Copy returned value back into struct object */
  obj new_struct;
  memcpy(&new_struct, new_value, sizeof(new_struct));

  /* Print new returned struct values */
  printf("1 -> \nMessage: ");
  Serial.println(String(new_struct.message));
  printf("Num: %i\n", (int) new_struct.num);

  /* Always clean up your dictionaries when you're done with them! */
  delete dict;
}

void
loop(
) {}