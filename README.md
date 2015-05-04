IonDB
=========

##Preamble

These inclusions are necessary for any IonDB usage:

```c
#include <SD.h>
#include "dictionary.h"
```

Then include some (or all) necessary implementation handlers:

```c
#include "slhandler.h"
#include "oadictionaryhandler.h"
#include "oafdictionaryhandler.h"
#include "ffdictionaryhandler.h"
```

In the setup() function, initialize a dictionary (Shown here, a skiplist):

```c
void setup() {
    //Declare the dictionary and handler structs
    dictionary_handler_t    handler;
    dictionary_t            dictionary;
    
    //Initialize handler
    sldict_init(&handler);
    
    //Create dictionary: Given handler, dictionary, key type, key size, value size, dict size
    dictionary_create(&handler, &dictionary, key_type_numeric_signed, sizeof(int), 60, 10);
}
```

####Implementation handler methods:

|Implementation         |Handler Method |
|--------------         |-------------- |
|Skiplist               | sldict_init   |
|Open Address Hash      | oadict_init   |
|Open Address File Hash | oafdict_init  |
|Flat File              | ffdict_init   |

####Dictionary size meanings:

|Implementation         |Dictionary Size|
|--------------         |-------------- |
|Skiplist               | Skiplist maximum height   |
|Open Address Hash      | Number of records    |
|Open Address File Hash | Number of records    |
|Flat File              | Not used   |

###Keys and values

Keys and values are **specific type agnostic**, there are only three categories of keys. Two macros are provided to bridge the gap between IonDB keys and concrete keys.

```c
/* Key creation */

//Suppose the key is an int
int my_key = 64;
ion_key_t key = IONIZE(my_key); //A prepared key

//Inline is fine too:
ion_key_t key = IONIZE(64);

//Any type is supported
unsigned long long my_key = 2147483648ull;
ion_key_t key = IONIZE(my_key);

/* Key retrieval */

//Retrieve an int from a key
int my_key = NEUTRALIZE(int, key);

//Retrieve unsigned long long from a key
unsigned long long my_key = NEUTRALIZE(unsigned long long, key);
```

####Ionization functions
| Function | Type |
|----------|------|
| IONIZE(any) | IONIZE :: any -> ion_key_t |
| NEUTRALIZE(atype, key) | NEUTRALIZE :: ion_key_t -> atype |

####Key categories:

* key_type_numeric_signed
* key_type_numeric_unsigned
* key_type_char_array (String)

##Usage

###Insert

```c
ion_key_t key = IONIZE(some_key);
ion_value_t value = some_value;
dictionary_insert(&dictionary, key, value);
```

###Delete

```c
ion_key_t key = IONIZE(some_key);
dictionary_delete(&dictionary, key);
```

###Query

```c
ion_key_t key = IONIZE(some_key);
ion_value_t my_value = malloc(value_size); // Create buffer to hold returned value
dictionary_get(&dictionary, key, my_value);
// Process data
//...
free(my_value);
```

##Full Example  
Written in Arduino compliant wiring.

```c
#include "dictionary.h"
#include "slhandler.h"

void setup() {
    //Declare the dictionary and handler structs
    dictionary_handler_t    handler;
    dictionary_t            dictionary;
    
    //Initialize handler
    sldict_init(&handler);
    
    //Create dictionary: Given handler, dictionary, key type, key size, value size, dict size
    dictionary_create(&handler, &dictionary, key_type_numeric_signed, sizeof(int), 60, 10);
    
    ion_key_t   key = IONIZE(42);
    ion_value_t value = (ion_value_t) "Hello IonDB";
    
    dictionary_insert(&dictionary, key, value);
    
    ion_value_t returned_value = malloc(60); //from value_size
    dictionary_get(&dictionary, key, returned_value);
    printf("Returned %s\n", returned_value);
    free(returned value);
}

void loop() {}
```