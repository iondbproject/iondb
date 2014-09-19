/*
 * sample.c
 *
 * Sample code that shows the functionality of printf of printf and uart_getchar();
 *
 */

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "milli/millisec.h"
#include "dictionary/dictionary.h"
#include "dictionary/skiplist/slhandler.h"
#include "serial.h"

/**bind uart_putchar to file  */
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,_FDEV_SETUP_WRITE);

enum {
 BLINK_DELAY_MS = 500,
};

int main ( void )
{

	/**Initialise the uart and setup ISR's*/
	uart_init();

	ms_timer_init();

	/**Bind to stdout for printf*/
	stdout = &mystdout;

	/**Enable the Global Interrupt Enable flag so that interrupts can be processed */
	sei ();

	/*
	while(1)
	{
		printf("Hi\n");
		//set pin 5 high to turn led on
		PORTB |= _BV(PORTB5);
		_delay_ms(BLINK_DELAY_MS);

		//set pin 5 low to turn led off
		PORTB &= ~_BV(PORTB5);
		_delay_ms(BLINK_DELAY_MS);
	 }*/

	dictionary_t dict;
	dictionary_handler_t handler;

	printf("Initialising handler\n");
	sldict_init(&handler);
	dictionary_create(&handler, &dict, key_type_numeric_signed, 2, 8, 7);

	int count = 0;
	char c;
	ion_value_t v;
	v = malloc(dict.instance->record.value_size);

	for (;;) // Loop forever
	{
		ms_start_timer();
		c = uart_getchar();
		printf("Processing character (%lums): ", ms_stop_timer());
		printf("%c\n",c);
		err_t status = dictionary_insert(&dict, (ion_key_t) &c, (ion_value_t) (char*) {"toast"});
		if(status == err_out_of_memory) {
			printf("End of memory, got %d inserts.\n", count);
		}
		else { count++; }
		dictionary_get(&dict, (ion_key_t) &c, v);
		printf("At %d we got %s.\n", c, (char*) v);
	}
	free(v);
	return 0;
}

