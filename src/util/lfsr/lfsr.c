/******************************************************************************/
/**
@file		lfsr.c
@author		Scott Fazackerley
@brief		A linear-feedback shift register pseudo-random number generator.
@details	This code implements a simple random number generator that
			generates a subset of possible integral values (16 bit unsigned)
			such that there are no collisions within the cycle generated.
@copyright	Copyright 2017
			The University of British Columbia,
			IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without 
	modification, are permitted provided that the following conditions are met:
	
@par 1.Redistributions of source code must retain the above copyright notice, 
	this list of conditions and the following disclaimer.
	
@par 2.Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation 
	and/or other materials provided with the distribution.
	
@par 3.Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software without
	specific prior written permission. 
	
@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
	POSSIBILITY OF SUCH DAMAGE.
*/
/******************************************************************************/

#include "lfsr.h"

void
lfsr_init_start_state(
	uint16_t	seed,
	lfsr_t		*instance
) {
	instance->start_state	= seed;
	instance->lfsr_value	= seed;
}

/**
@brief
*/
uint16_t
lfsr_get_next(
	lfsr_t *instance
) {
	unsigned lsb = instance->lfsr_value & 1;/* Get LSB (i.e., the output bit). */

	instance->lfsr_value >>= 1;	/* Shift register */

	if (lsb == 1) {
		/* Only apply toggle mask if output bit is 1 */
		instance->lfsr_value ^= 0xB400u;/* Apply toggle mask, value has 1 at bits corresponding
													* to taps, 0 elsewhere. */
	}

	return instance->lfsr_value;
}

/**
@brief
*/
void
lfsr_reset(
	lfsr_t *instance
) {
	instance->lfsr_value = instance->start_state;
}
