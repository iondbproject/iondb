/*
 * sample.c
 *
 * Sample code that shows the functionality of printf of printf and uart_getchar();
 *
 */


/*
#include <stdio.h>

#include "serial.h"

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,_FDEV_SETUP_WRITE);

int main ( void )
{

	uart_init();

	stdout = &mystdout;

	sei (); // Enable the Global Interrupt Enable flag so that interrupts can be processed
	char c;

	for (;;) // Loop forever
	{
		c = uart_getchar();
		printf("Processing character : ");
		printf("%c\n",c);
	}
}



*/
#include <stdio.h>
#include "dictionary/dictionary.h"
#include "dictionary/skiplist/slhandler.h"

#include "serial.h"

	stdout = &mystdout;

 	sei (); // Enable the Global Interrupt Enable flag so that interrupts can be processed


	dictionary_t dict;
	dictionary_handler_t handler;

	sldict_init(&handler);
	dictionary_create(&handler, &dict, key_type_numeric_signed, 2, 8, 7);

 	char c;
	char* v;

	for (;;) // Loop forever
	{
		c = uart_getchar();
		printf("Processing character : ");
		printf("%c\n",c);
		dictionary_insert(&dict, (ion_key_t) &c, (ion_value_t) (char*) {"toast"});
		dictionary_get(&dict, (ion_key_t) &c, (ion_value_t*) &v);
		printf("At %d we got %s.\n", c, v);
		free(v);
	}
}

