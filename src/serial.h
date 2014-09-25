/*
 * serial.h
 *
 *  Created on: Sep 13, 2014
 *      Author: workstation
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define F_CPU 16000000UL
#define USART_BAUDRATE 19200
#define BAUD_PRESCALE ((( F_CPU / ( USART_BAUDRATE * 16UL ))) - 1)
#define BUFFER_SIZE 32

typedef struct uart_buffer
{
	unsigned char 	buffer[BUFFER_SIZE];

	unsigned char 	head;
	unsigned char 	tail;
	unsigned char 	count;

} uart_buffer_t;

int uart_putchar(
	char c,
	FILE *stream);

char uart_getchar();

void uart_init();

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_H_ */
