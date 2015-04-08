IonDB
=========

##Preamble

This inclusion is necessary for any IonDB usage:

```c
#include "dictionary.h"
```

Then include some (or all) necessary implementation handlers:

```c
#include "slhandler.h"
#include "oadictionaryhandler.h"
#include "oafdictionaryhandler.h"
#include "ffdictionaryhandler.h"
```

In the setup() function, initialize your dictionary (Skiplist shown):

```c
void setup() {
    //Declare the dictionary and handler structs
    dictionary_handler_t    handler;
    dictionary_t            dictionary;
    
    //Initialize handler
    sldict_init(&handler);
    
    //Create dictionary: Given handler, dictionary, key type, key size, value size, dict size
    dictionary_create(&handler, &dictionary, key_type_numeric_signed, 2, 8, 10);
}
```

####Implementation handler methods:

|Implementation         |Handler Method |
|--------------         |-------------- |
|Skiplist               | sldict_init  |
|Open Address Hash      | oadict_init   |
|Open Address File Hash | oafdict_init  |
|Flat File              | ffdict_init   |

####Valid key types:

* key_type_numeric_signed
* key_type_numeric_unsigned
* key_type_char_array (String)

####Dictionary size meanings:

|Implementation         |Dictionary Size|
|--------------         |-------------- |
|Skiplist               | Skiplist maximum height   |
|Open Address Hash      | Number of buckets ?   |
|Open Address File Hash | Number of buckets ?   |
|Flat File              | ???   |


##Usage

###Insert

```c
dictionary_insert(&dictionary, key, value);
```

###Delete

```c
dictionary_delete(&dictionary, key);
```

###Query

```c
ion_value_t my_value = malloc(value_size); // Create buffer to hold returned value
dictionary_get(&dictionary, key, my_value);
```

###Cursor queries

```c
TBA
```

##Full Example

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
    dictionary_create(&handler, &dictionary, key_type_numeric_signed, 2, 8, 10);
    
    ion_key_t   key; //Key creation needs to be discussed
    ion_value_t value;
    
    dictionary_insert(&dictionary, key, value);
    
    ion_value_t returned_value = malloc(8); //from value_size
    dictionary_get(&dictionary, key, returned_value);
}
```