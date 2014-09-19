#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "milli/millisec.h"
#include "dictionary/dictionary.h"
#include "dictionary/skiplist/slhandler.h"
#include "serial.h"

/**bind uart_putchar to file  */
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,_FDEV_SETUP_WRITE);

int main(void)
{
	uart_init();
	ms_timer_init();
	stdout = &mystdout;
	sei();

	return 0;
}
