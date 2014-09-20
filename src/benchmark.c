#include <stdio.h>
#include <avr/io.h>
#include "milli/millisec.h"
#include "dictionary/dictionary.h"
#include "dictionary/skiplist/slhandler.h"
#include "serial.h"

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,_FDEV_SETUP_WRITE);

void populate_withduplicate(dictionary_t*, int);
void selection(dictionary_t*, int);
void deletion(dictionary_t*, int);

/******************** STRUCTURE PARAMETERS **********************************/
/* Handler initialization function pointer: Change this for diff. structure */
err_t            (*handler_fptr)(dictionary_handler_t*) = sldict_init;
/* Type of key used */
key_type_t       key_type                               = key_type_numeric_signed;
/* Key size: Standard is sizeof(short) */
ion_key_size_t   key_size                               = sizeof(short);
/* Value size: Standard is 8 (8 char string) */
ion_value_size_t value_size                             = 8;
/* Size of structure. Meaning is dependent on what structure is being tested */
int              dict_size                              = 7;

/******************** TEST PARAMETERS **********************************/
#define 		  RAND_SEED                               1337
/* Upper bound of a key (if a key is an integer) */
#define           KEY_INT_MAX                             9999 /* Up for discussion */
#define           INSERT_COUNT                            55
#define           QUERY_COUNT                             25
#define           DELETE_COUNT                            25
void             (*population_fptr)(dictionary_t*, int) = populate_withduplicate;
void             (*select_fptr)(dictionary_t*, int)     = selection;
void             (*delete_fptr)(dictionary_t*, int)     = deletion;

int main(void)
{
	uart_init();
	ms_timer_init();
	stdout = &mystdout;
	sei();
	srand(RAND_SEED);

	int delta;
	dictionary_t           dict;
	dictionary_handler_t   handler;
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);

	ms_start_timer();
	population_fptr(&dict, INSERT_COUNT);
	delta = ms_stop_timer();
	printf("Insert time: %dms\n", delta);

	ms_start_timer();
	select_fptr(&dict, QUERY_COUNT);
	delta = ms_stop_timer();
	printf("Select time: %dms\n", delta);

	ms_start_timer();
	delete_fptr(&dict, DELETE_COUNT);
	delta = ms_stop_timer();
	printf("Delete time: %dms\n", delta);

	return 0;
}

void
populate_withduplicate(
    dictionary_t    *dict,
    int             count
)
{
  int i;
  for(i = 0; i < count; i++) /* Important: i is only an iteration and has no effect on the key */
  {
    int    key           = random() % KEY_INT_MAX;
    char*  value         = "IonDB Test String!!"; /* Also up for debate */
    dictionary_insert(dict, (ion_key_t) &key, (ion_value_t) value);
  }
}

void
selection(
  dictionary_t     *dict,
    int             count
)
{
  int i;
  for(i = 0; i < count; i++)
  {
    int           key       = random() % KEY_INT_MAX;
    char          value[value_size];
    dictionary_get(dict, (ion_key_t) &key, (ion_value_t) value);
    free(value); /* The get* mallocs memory for us. We must let it go */
  }
}

void
deletion(
   dictionary_t     *dict,
    int             count
)
{
  int i;
  for(i = 0; i < count; i++)
  {
    int           key       = random() % KEY_INT_MAX;
    dictionary_delete(dict, (ion_key_t) &key);
  }
}
