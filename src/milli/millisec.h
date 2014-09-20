/*
 * millisec.h
 *
 *  Created on: Sep 18, 2014
 *      Author: workstation
 */

#ifndef MILLISEC_H_
#define MILLISEC_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#define F_CPU 16000000UL

#define CTC_MATCH_OVERFLOW ((F_CPU / 1000) / 8)

static volatile unsigned long long 		timer1_milliseconds;
static volatile unsigned long long		start_time;

unsigned long
ms_milliseconds();

void
ms_timer_init();

void
ms_start_timer
();

unsigned long
ms_stop_timer
();

void
ms_set_time(unsigned long current_epoch_time);

unsigned long
ms_get_time(void * something);


#endif /* MILLISEC_H_ */
