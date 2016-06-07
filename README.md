![IonDB](http://stickerpants.com/uploads/IonDB_logo_final.svg)
=========
You might also be interested in our sister project, [LittleD - A relational database using 1kB of RAM or less](https://github.com/graemedouglas/LittleD).

> **Note:** IonDB is entering a period of rapid development and restructuring for the next few months. See the [Collaboration](#collaboration) section for more information.

#"What is this?"

Currently in the Arduino world, there doesn't exist an associative array or map implementation that is both easy to use *and* performance competitive. There also is little support for disk based storage options that don't involve writing it yourself. IonDB is fast, functional, and offers disk based storage out of the box.

In general, IonDB supports:

* Storing arbitrary values associated to a key
* Duplicate key support
* Range and Equality queries
* Disk based persistent data storage

IonDB has a paper that was published at IEEE (CCECE) 2015, which can be found [here.](http://ieeexplore.ieee.org/xpl/articleDetails.jsp?reload=true&tp=&arnumber=7129178)

##License

IonDB is licensed under the Apache License. For more information, please refer to [the license file](LICENSE.md).

##Collaboration

Pull requests are currently **not** recommended due to significant changes arriving in the near future. 

***

##Repository Setup for Contributors

Before editing any code, make sure you have `uncrustify` installed, version
0.63 or newer, and run

```bash
make setup
make hooks
```

in the repository root to setup necessary submodules and pre-commit hooks for formatting.

If you move the directory of your repository to a different path, make sure you re-run `make hooks`.

#Usage Guide

These inclusions are necessary for any IonDB usage:

```c
#include <SD.h> //If using file based implementations
#include "dictionary.h"
```

Then include some (or all) necessary implementation handlers:

```c
#include "slhandler.h"
#include "oadictionaryhandler.h"
#include "oafdictionaryhandler.h"
#include "ffdictionaryhandler.h"
#include "bpptreehandler.h"
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
|B+ Tree                | bpptree_init  |

####Dictionary size meanings:

|Implementation         |Dictionary Size            |
|--------------         |--------------             |
|Skiplist               | Skiplist maximum height   |
|Open Address Hash      | Number of records         |
|Open Address File Hash | Number of records         |
|Flat File              | Not used                  |
|B+ Tree                | Not used                  |

###Keys and values

Keys and values are **specific type agnostic**, there are only three categories of keys. Two macros are provided to bridge the gap between IonDB keys and concrete keys.

Note that keys and values are of a fixed size, which is set on a per-dictionary basis. Think carefully about the domain of the data being stored and pick appropriate sizes.

```c
/* Key creation */

//Suppose the key is an int
int my_key = 64;
ion_key_t key = IONIZE(my_key, int); //A prepared key

//Inline is fine too:
ion_key_t key = IONIZE(64, int);

//Any type is supported
unsigned long long my_key = 2147483648ull;
ion_key_t key = IONIZE(my_key, unsigned long long);

/* Key retrieval */

//Retrieve an int from a key
int my_key = NEUTRALIZE(key, int);

//Retrieve unsigned long long from a key
unsigned long long my_key = NEUTRALIZE(key, unsigned long long);
```

####Ionization functions
| Function | Type |
|----------|------|
| `IONIZE(key, atype)` | `IONIZE :: atype -> ion_key_t` |
| `NEUTRALIZE(atype, key)` | `NEUTRALIZE :: ion_key_t -> atype` |

####Key categories:

* key_type_numeric_signed
* key_type_numeric_unsigned
* key_type_char_array (String)

##File based implementations

An SD shield, and a FAT formatted SD card is required to work with IonDB. The Arduino ethernet shield is recommended. The following initialization is required when working with file bsaed implementations:

```c
//Use pin 10 if using an Uno, pin 53 if Mega
pinMode(10, OUTPUT);
//Change depending on what SD shield is used
SD.begin(4);
```

##Usage

###Insert

```c
ion_key_t key = IONIZE(some_key, int);
ion_value_t value = some_value;
dictionary_insert(&dictionary, key, value);
```

###Delete

```c
ion_key_t key = IONIZE(some_key, int);
dictionary_delete(&dictionary, key);
```

###Query

```c
ion_key_t key = IONIZE(some_key, int);
ion_value_t my_value = malloc(value_size); // Create buffer to hold returned value
dictionary_get(&dictionary, key, my_value);
// Process data
//...
free(my_value);
```

###Cursors  
A functional implementation exists for equality cursors (Multiple value query on same key) and range cursors (Query key-value pairs across a bound of keys), however a dictionary level interface for cursor access has yet to be finalized. Implementation level access is demonstrated in the Benchmark source.

##Full Example  
Written in Arduino compliant wiring.

```c
#include <SD.h>
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
    
    ion_key_t   key = IONIZE(42, int);
    ion_value_t value = (ion_value_t) "Hello IonDB";
    
    dictionary_insert(&dictionary, key, value);
    
    ion_value_t returned_value = (ion_value_t) malloc(60); //from value_size
    dictionary_get(&dictionary, key, returned_value);
    printf("Returned %s\n", returned_value);
    free(returned_value);
}

void loop() {}
```
