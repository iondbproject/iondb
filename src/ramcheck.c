#include <stdio.h>
#include <avr/io.h>
#include "milli/millisec.h"
#include "dictionary/dictionary.h"
#include "dictionary/skiplist/slhandler.h"
#include "serial.h"
#include "ram/ramutil.h"

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,_FDEV_SETUP_WRITE);

int main(void)
{
	uart_init();
	stdout = &mystdout;
	sei();
	int initial_r = free_ram();
	printf("Ram bare bones: %db\n", initial_r);

	dictionary_t dict;
	dictionary_handler_t handler;
	sldict_init(&handler);
	dictionary_create(&handler, &dict, key_type_numeric_signed, 2, 8, 7);
	int create_r = free_ram();
	printf("K:2 V:8 H:7, Ram remaining: %db (Usage: %db)\n", free_ram(), initial_r - create_r);

	short key = 6;
	char  value[8] = "test";
	int insert_r = free_ram();
	dictionary_insert(&dict, (ion_key_t) &key, (ion_value_t) value);
	printf("Single insert, Ram remaining: %db (Usage: %db)\n", free_ram(), insert_r - free_ram());

	dictionary_delete_dictionary(&dict);
	printf("Deleted dict, Ram remaining: %db (Usage: %db)\n", free_ram(), free_ram() - create_r);

	return 0;
}

