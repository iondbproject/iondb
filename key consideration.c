// Problem: Keys and values are defined as ion_key_t and ion_value_t (aliased unsigned char*). 
//Conversion between standard data types and these types is cumbersome and difficult to deal with.

//Demonstration:

//raw types
int key = 5;
char* value = "Hello!";

//"ionized" types
ion_key_t   c_key   = (ion_key_t)   &key;
ion_value_t c_value = (ion_value_t) value;

//Insertion
dictionary_insert(&dict, c_key, c_value); // OK
// OR inline
dictionary_insert(&dict, (ion_key_t) &key, (ion_value_t) value);

//Passing in literal values is not possible:
dictionary_insert(&dict, (ion_key_t) 4, (ion_value_t) "Hello!"); //Error





//Converting key -> raw type

int x = 5;
ion_key_t key = (ion_key_t) &x;

int y = *((int*) key); //Cast to int pointer, dereference int pointer


//Proposals: Macro/function to do key/raw conversion? Perhaps just one to "ionize" literals?

#include <stdio.h>

typedef unsigned char* ion_key_t;

#define IONIZE(x) ({volatile typeof(x) _tmp = x; (ion_key_t) &_tmp; })
#define NEUTRALIZE(type, x) ( *((type *) x) )

int main(void) {
    // your code goes here
    int t = 600000;
    ion_key_t toast= ionize(100);
    printf("%p\n", toast);
    printf("%d\n", neutralize(int, toast) );
    return 0;
}
