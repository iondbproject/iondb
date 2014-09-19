/*
 * millisec.c
 *
 *  Created on: Sep 18, 2014
 *      Author: workstation
 */

#include "millisec.h"

ISR (
		TIMER1_COMPA_vect
)
{
    timer1_milliseconds++;
}

void
ms_timer_init
()
{
	  // CTC mode, Clock/8
	TCCR1B |= (1 << WGM12) | (1 << CS11);

	// Load the high byte, then the low byte
	// into the output compare
	OCR1AH = (CTC_MATCH_OVERFLOW >> 8);
	OCR1AL = CTC_MATCH_OVERFLOW;

	// Enable the compare match interrupt
	TIMSK1 |= (1 << OCIE1A);
}

unsigned long
ms_milliseconds
()
{
    unsigned long millis_return;

    // Ensure this cannot be disrupted
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        millis_return = timer1_milliseconds;
    }

    return millis_return;
}

void
ms_start_timer
()
{
	start_time = ms_milliseconds();
}

unsigned long
ms_stop_timer
()
{
	return (ms_milliseconds() - start_time);
}
