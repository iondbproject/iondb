/*
 * sample.c
 *
 * Sample code that shows the functionality of printf of printf and uart_getchar();
 *
 */


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





