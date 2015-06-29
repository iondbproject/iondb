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
#include "flash.h"



/**bind uart_putchar to file  */
static FILE uart_io = FDEV_SETUP_STREAM(uart_putchar, uart_getchar,_FDEV_SETUP_RW);

enum {
 BLINK_DELAY_MS = 500,
};

int main ( void )
{

	/**Initialise the uart and setup ISR's*/
	uart_init();

	/** Initialise timer */
	ms_timer_init();

	/**Bind to stdout for printf*/
	stdout = &uart_io;
	stdin  = &uart_io;

	/**Enable the Global Interrupt Enable flag so that interrupts can be processed */
	sei ();

	unsigned long cur_time;

	printf("enter the current epoch time: ");
	scanf("%lu", &cur_time);
	printf("\nThe time is: %lu \n", cur_time);
	ms_set_time(cur_time);
	printf("The current epoch time is: %lu\n", ms_get_time(NULL));
	/*
	while(1)
	{
		prinltf("Hi\n");
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

	char c;
	ion_value_t * v;
	v = (ion_value_t)malloc(dict.instance->record.value_size);

	for (;;) // Loop forever
	{
		ms_start_timer();
		c = uart_getchar();
		printf("%lu milliseconds have passed since the last character was pressed\n",ms_stop_timer());
		printf("Processing character : ");
		printf("%c\n",c);
		dictionary_insert(&dict, (ion_key_t) &c, (ion_value_t) (char*) {"toast"});
		dictionary_get(&dict, (ion_key_t) &c, (ion_value_t*) &v);
		printf("At %d we got %s.\n", c, v);
		printf("The current epoch time is: %lu\n", 	ms_get_time(NULL));
	}
	free(v);
	return 0;
}

