/******************************************************************************/
/**
@file
@author		Scott Fazackerley
@brief		A linear-feedback shift register pseudo-random number generator.
@details	This code implements a simple random number generator that
			generates a subset of possible integral values (16 bit unsigned)
			such that there are no collisions within the cycle generated.
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see @ref AUTHORS.md)
@par
			Licensed under the Apache License, Version 2.0 (the "License");
			you may not use this file except in compliance with the License.
			You may obtain a copy of the License at
					http://www.apache.org/licenses/LICENSE-2.0
@par
			Unless required by applicable law or agreed to in writing,
			software distributed under the License is distributed on an
			"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
			either express or implied. See the License for the specific
			language governing permissions and limitations under the
			License.
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
