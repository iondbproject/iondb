#include <stdio.h>
#include <avr/io.h>
#include "milli/millisec.h"
#include "dictionary/dictionary.h"
#include "dictionary/skiplist/slhandler.h"
#include "serial.h"

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,_FDEV_SETUP_WRITE);

int main(void)
{
	uart_init();
	stdout = &mystdout;
	sei();

	dictionary_t dict;
	dictionary_handler_t handler;
	sldict_init(&handler);
	dictionary_create(&handler, &dict, key_type_numeric_signed, 2, 8, 7);

	err_t status = err_ok;
	int count = 0;
	while(status != err_out_of_memory)
	{
		int key = count;
		char  value[8] = "test";
		status = dictionary_insert(&dict, (ion_key_t) &key, (ion_value_t) value);
		count++;
	}
	printf("Inserts done. Got: %d inserts.\n", count);

	return 0;
}

