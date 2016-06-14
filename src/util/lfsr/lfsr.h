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
#if !defined(_LFSR_H_)
#define _LFSR_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

/**
@brief		An instance type for the random number generator.
*/
typedef struct lfsr {
	uint16_t	start_state;/**< Any nonzero start state for the generator. */
	uint16_t	lfsr_value;	/**< The current state of the generator. */
} lfsr_t;

/**
@brief		Initialize the seed for the random number generator.
@param		seed
				The initial start state represented as an integer
				for the random number generator.
@param		instance
				A pointer to the random number generator object to be
				initialized. This is caller allocated.
*/
void
lfsr_init_start_state(
	uint16_t	seed,
	lfsr_t		*instance
);

/**
@brief		Generate the next random number.
@param		instance
				A pointer to the random number generator object that we
				wish to use to generate the next random item.
@returns	The next random integer from the random number generator.
*/
uint16_t
lfsr_get_next(
	lfsr_t *instance
);

/**
@brief		Reset the random number generator to it's initial state.
@param		instance
				A pointer to the random number generator object
				to reset.
*/
void
lfsr_reset(
	lfsr_t *instance
);

#if defined(__cplusplus)
}
#endif

#endif
